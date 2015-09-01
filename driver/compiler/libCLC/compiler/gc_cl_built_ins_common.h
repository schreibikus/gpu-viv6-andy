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


#ifndef __gc_cl_built_ins_common_h_
#define __gc_cl_built_ins_common_h_

static clsBUILTIN_FUNCTION CommonBuiltinFunctions[] =
{
    /* Common Functions */

    {clvEXTENSION_NONE,     "clamp",  T_GENTYPE, 3, {T_GENTYPE, T_GENTYPE, T_GENTYPE}, {0}, {1, 1, 1}, 1},

    {clvEXTENSION_NONE,     "min",    T_GENTYPE,    2, {T_GENTYPE, T_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "min",    T_FLOAT2,     2, {T_FLOAT2, T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "min",    T_FLOAT3,     2, {T_FLOAT3, T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "min",    T_FLOAT4,     2, {T_FLOAT4, T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "min",    T_FLOAT8,     2, {T_FLOAT8, T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "min",    T_FLOAT16,    2, {T_FLOAT16, T_FLOAT}, {0}, {0}, 1},

    {clvEXTENSION_NONE,     "max",    T_GENTYPE,    2, {T_GENTYPE, T_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "max",    T_FLOAT2,     2, {T_FLOAT2, T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "max",    T_FLOAT3,     2, {T_FLOAT3, T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "max",    T_FLOAT4,     2, {T_FLOAT4, T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "max",    T_FLOAT8,     2, {T_FLOAT8, T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "max",    T_FLOAT16,    2, {T_FLOAT16, T_FLOAT}, {0}, {0}, 1},

    {clvEXTENSION_NONE,     "sign",   T_FLOAT,    1, {T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "sign",   T_FLOAT2,     1, {T_FLOAT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "sign",   T_FLOAT3,     1, {T_FLOAT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "sign",   T_FLOAT4,     1, {T_FLOAT4}, {0}, {0}, 1},

    {clvEXTENSION_NONE,     "fmix",   T_FLOAT, 3, {T_FLOAT, T_FLOAT, T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "fmix",   T_FLOAT2,  3, {T_FLOAT2,  T_FLOAT2,  T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "fmix",   T_FLOAT3,  3, {T_FLOAT3,  T_FLOAT3,  T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "fmix",   T_FLOAT4,  3, {T_FLOAT4,  T_FLOAT4,  T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "fmix",   T_FLOAT2,  3, {T_FLOAT2,  T_FLOAT2,  T_FLOAT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "fmix",   T_FLOAT3,  3, {T_FLOAT3,  T_FLOAT3,  T_FLOAT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "fmix",   T_FLOAT4,  3, {T_FLOAT4,  T_FLOAT4,  T_FLOAT4}, {0}, {0}, 1},

    {clvEXTENSION_NONE,     "mix",    T_FLOAT, 3, {T_FLOAT, T_FLOAT, T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "mix",    T_FLOAT2,  3, {T_FLOAT2,  T_FLOAT2,  T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "mix",    T_FLOAT3,  3, {T_FLOAT3,  T_FLOAT3,  T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "mix",    T_FLOAT4,  3, {T_FLOAT4,  T_FLOAT4,  T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "mix",    T_FLOAT2,  3, {T_FLOAT2,  T_FLOAT2,  T_FLOAT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "mix",    T_FLOAT3,  3, {T_FLOAT3,  T_FLOAT3,  T_FLOAT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "mix",    T_FLOAT4,  3, {T_FLOAT4,  T_FLOAT4,  T_FLOAT4}, {0}, {0}, 1},

    /* Angle and Trigonometry Functions */
    {clvEXTENSION_NONE,     "radians",              T_FLOAT,    1, {T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "radians",              T_FLOAT2,   1, {T_FLOAT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "radians",              T_FLOAT3,   1, {T_FLOAT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "radians",              T_FLOAT4,   1, {T_FLOAT4}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "radians",              T_FLOAT8,   1, {T_FLOAT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "radians",              T_FLOAT16,  1, {T_FLOAT16}, {0}, {0}, 1},

    {clvEXTENSION_NONE,     "degrees",              T_FLOAT,    1, {T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "degrees",              T_FLOAT2,   1, {T_FLOAT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "degrees",              T_FLOAT3,   1, {T_FLOAT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "degrees",              T_FLOAT4,   1, {T_FLOAT4}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "degrees",              T_FLOAT8,   1, {T_FLOAT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "degrees",              T_FLOAT16,  1, {T_FLOAT16}, {0}, {0}, 1},

    {clvEXTENSION_NONE,     "step",                 T_F_GENTYPE,  2, {T_F_GENTYPE, T_F_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "step",                 T_FLOAT2,     2, {T_FLOAT,     T_FLOAT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "step",                 T_FLOAT3,     2, {T_FLOAT,     T_FLOAT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "step",                 T_FLOAT4,     2, {T_FLOAT,     T_FLOAT4}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "step",                 T_FLOAT8,     2, {T_FLOAT,     T_FLOAT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "step",                 T_FLOAT16,    2, {T_FLOAT,     T_FLOAT16}, {0}, {0}, 1},

    {clvEXTENSION_NONE,     "smoothstep",           T_FLOAT,    3, {T_FLOAT,       T_FLOAT,    T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "smoothstep",           T_FLOAT2,   3, {T_FLOAT2,      T_FLOAT2,   T_FLOAT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "smoothstep",           T_FLOAT3,   3, {T_FLOAT3,      T_FLOAT3,   T_FLOAT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "smoothstep",           T_FLOAT4,   3, {T_FLOAT4,      T_FLOAT4,   T_FLOAT4}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "smoothstep",           T_FLOAT2,   3, {T_FLOAT,       T_FLOAT,    T_FLOAT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "smoothstep",           T_FLOAT3,   3, {T_FLOAT,       T_FLOAT,    T_FLOAT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "smoothstep",           T_FLOAT4,   3, {T_FLOAT,       T_FLOAT,    T_FLOAT4}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "smoothstep",           T_FLOAT8,   3, {T_FLOAT8,      T_FLOAT8,   T_FLOAT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "smoothstep",           T_FLOAT16,  3, {T_FLOAT16,     T_FLOAT16,  T_FLOAT16}, {0}, {0}, 1},

    /* Geometric Functions */
    {clvEXTENSION_NONE,        "cross",                  T_GENTYPE,  2, {T_GENTYPE, T_GENTYPE}, {0}, {1, 1}, 1},

    {clvEXTENSION_NONE,     "fast_length",          T_FLOAT,    1, {T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "fast_length",          T_FLOAT,    1, {T_FLOAT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "fast_length",          T_FLOAT,    1, {T_FLOAT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "fast_length",          T_FLOAT,    1, {T_FLOAT4}, {0}, {0}, 1},

    {clvEXTENSION_NONE,     "length",               T_FLOAT,    1, {T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "length",               T_FLOAT,    1, {T_FLOAT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "length",               T_FLOAT,    1, {T_FLOAT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "length",               T_FLOAT,    1, {T_FLOAT4}, {0}, {0}, 1},

    {clvEXTENSION_NONE,     "distance",             T_FLOAT,    2, {T_FLOAT,    T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "distance",             T_FLOAT,    2, {T_FLOAT2,   T_FLOAT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "distance",             T_FLOAT,    2, {T_FLOAT3,   T_FLOAT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "distance",             T_FLOAT,    2, {T_FLOAT4,   T_FLOAT4}, {0}, {0}, 1},

    {clvEXTENSION_NONE,     "fast_distance",        T_FLOAT,    2, {T_FLOAT,    T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "fast_distance",        T_FLOAT,    2, {T_FLOAT2,   T_FLOAT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "fast_distance",        T_FLOAT,    2, {T_FLOAT3,   T_FLOAT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "fast_distance",        T_FLOAT,    2, {T_FLOAT4,   T_FLOAT4}, {0}, {0}, 1},

    {clvEXTENSION_NONE,     "dot",                  T_FLOAT,    2, {T_FLOAT,    T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "dot",                  T_FLOAT,    2, {T_FLOAT2,   T_FLOAT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "dot",                  T_FLOAT,    2, {T_FLOAT3,   T_FLOAT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "dot",                  T_FLOAT,    2, {T_FLOAT4,   T_FLOAT4}, {0}, {0}, 1},

    {clvEXTENSION_NONE,     "cross",                T_FLOAT3,   2, {T_FLOAT3, T_FLOAT3}, {0}, {0}, 1},

    {clvEXTENSION_NONE,     "normalize",            T_FLOAT,    1, {T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "normalize",            T_FLOAT2,   1, {T_FLOAT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "normalize",            T_FLOAT3,   1, {T_FLOAT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "normalize",            T_FLOAT4,   1, {T_FLOAT4}, {0}, {0}, 1},

    {clvEXTENSION_NONE,     "fast_normalize",       T_FLOAT,    1, {T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "fast_normalize",       T_FLOAT2,   1, {T_FLOAT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "fast_normalize",       T_FLOAT3,   1, {T_FLOAT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "fast_normalize",       T_FLOAT4,   1, {T_FLOAT4}, {0}, {0}, 1},

    {clvEXTENSION_NONE,     "faceforward",          T_FLOAT,    3, {T_FLOAT,    T_FLOAT,    T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "faceforward",          T_FLOAT2,   3, {T_FLOAT2,   T_FLOAT2,   T_FLOAT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "faceforward",          T_FLOAT3,   3, {T_FLOAT3,   T_FLOAT3,   T_FLOAT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "faceforward",          T_FLOAT4,   3, {T_FLOAT4,   T_FLOAT4,   T_FLOAT4}, {0}, {0}, 1},

    {clvEXTENSION_NONE,     "reflect",              T_FLOAT,    2, {T_FLOAT,    T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "reflect",              T_FLOAT2,   2, {T_FLOAT2,   T_FLOAT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "reflect",              T_FLOAT3,   2, {T_FLOAT3,   T_FLOAT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "reflect",              T_FLOAT4,   2, {T_FLOAT4,   T_FLOAT4}, {0}, {0}, 1},

    {clvEXTENSION_NONE,     "refract",              T_FLOAT,    3, {T_FLOAT,    T_FLOAT,    T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "refract",              T_FLOAT2,   3, {T_FLOAT2,   T_FLOAT2,   T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "refract",              T_FLOAT3,   3, {T_FLOAT3,   T_FLOAT3,   T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "refract",              T_FLOAT4,   3, {T_FLOAT4,   T_FLOAT4,   T_FLOAT}, {0}, {0}, 1},

    /* Vector Relational Functions */
    {clvEXTENSION_NONE,     "isequal",              T_I_GENTYPE,    2, {T_F_GENTYPE, T_F_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "isnotequal",           T_I_GENTYPE,    2, {T_F_GENTYPE, T_F_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "isgreater",            T_I_GENTYPE,    2, {T_F_GENTYPE, T_F_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "isgreaterequal",       T_I_GENTYPE,    2, {T_F_GENTYPE, T_F_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "isless",               T_I_GENTYPE,    2, {T_F_GENTYPE, T_F_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "islessequal",          T_I_GENTYPE,    2, {T_F_GENTYPE, T_F_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "islessgreater",        T_I_GENTYPE,    2, {T_F_GENTYPE, T_F_GENTYPE}, {0}, {1, 1}, 1},

    {clvEXTENSION_NONE,     "isordered",            T_I_GENTYPE,    2, {T_F_GENTYPE, T_F_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "isunordered",          T_I_GENTYPE,    2, {T_F_GENTYPE, T_F_GENTYPE}, {0}, {1, 1}, 1},

    {clvEXTENSION_NONE,     "isfinite",             T_I_GENTYPE,    1, {T_F_GENTYPE}, {0}, {1}, 1},
    {clvEXTENSION_NONE,     "isnan",                T_I_GENTYPE,    1, {T_F_GENTYPE}, {0}, {1}, 1},
    {clvEXTENSION_NONE,     "isinf",                T_I_GENTYPE,    1, {T_F_GENTYPE}, {0}, {1}, 1},
    {clvEXTENSION_NONE,     "isnormal",             T_I_GENTYPE,    1, {T_F_GENTYPE}, {0}, {1}, 1},
    {clvEXTENSION_NONE,     "signbit",              T_I_GENTYPE,    1, {T_F_GENTYPE}, {0}, {1}, 1},
    {clvEXTENSION_NONE,     "lgamma",               T_F_GENTYPE,    1, {T_F_GENTYPE}, {0}, {1}, 1},
    {clvEXTENSION_NONE,     "lgamma_r",             T_F_GENTYPE,    2, {T_F_GENTYPE, T_I_GENTYPE}, {0, 1}, {1, 0}, 0},

    {clvEXTENSION_NONE,     "shuffle",              T_I_GENTYPE,    2, {T_CHAR2,     T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle",              T_I_GENTYPE,    2, {T_CHAR4,     T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle",              T_I_GENTYPE,    2, {T_CHAR8,     T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle",              T_I_GENTYPE,    2, {T_CHAR16,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle",              T_U_GENTYPE,    2, {T_UCHAR2,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle",              T_U_GENTYPE,    2, {T_UCHAR4,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle",              T_U_GENTYPE,    2, {T_UCHAR8,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle",              T_U_GENTYPE,    2, {T_UCHAR16,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle",              T_I_GENTYPE,    2, {T_SHORT2,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle",              T_I_GENTYPE,    2, {T_SHORT4,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle",              T_I_GENTYPE,    2, {T_SHORT8,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle",              T_I_GENTYPE,    2, {T_SHORT16,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle",              T_U_GENTYPE,    2, {T_USHORT2,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle",              T_U_GENTYPE,    2, {T_USHORT4,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle",              T_U_GENTYPE,    2, {T_USHORT8,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle",              T_U_GENTYPE,    2, {T_USHORT16,  T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle",              T_I_GENTYPE,    2, {T_INT2,      T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle",              T_I_GENTYPE,    2, {T_INT4,      T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle",              T_I_GENTYPE,    2, {T_INT8,      T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle",              T_I_GENTYPE,    2, {T_INT16,     T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle",              T_U_GENTYPE,    2, {T_UINT2,     T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle",              T_U_GENTYPE,    2, {T_UINT4,     T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle",              T_U_GENTYPE,    2, {T_UINT8,     T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle",              T_U_GENTYPE,    2, {T_UINT16,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle",              T_I_GENTYPE,    2, {T_LONG2,     T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle",              T_I_GENTYPE,    2, {T_LONG4,     T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle",              T_I_GENTYPE,    2, {T_LONG8,     T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle",              T_I_GENTYPE,    2, {T_LONG16,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle",              T_U_GENTYPE,    2, {T_ULONG2,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle",              T_U_GENTYPE,    2, {T_ULONG4,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle",              T_U_GENTYPE,    2, {T_ULONG8,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle",              T_U_GENTYPE,    2, {T_ULONG16,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle",              T_F_GENTYPE,    2, {T_FLOAT2,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle",              T_F_GENTYPE,    2, {T_FLOAT4,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle",              T_F_GENTYPE,    2, {T_FLOAT8,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle",              T_F_GENTYPE,    2, {T_FLOAT16,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},

    /* Vivante Primitive Common Functions */

    {clvEXTENSION_NONE,     "viv_clamp",  T_GENTYPE, 3, {T_GENTYPE, T_GENTYPE, T_GENTYPE}, {0}, {1, 1, 1}, 1},

    {clvEXTENSION_NONE,     "viv_min",    T_GENTYPE,    2, {T_GENTYPE, T_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "viv_min",    T_FLOAT2,     2, {T_FLOAT2, T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_min",    T_FLOAT3,     2, {T_FLOAT3, T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_min",    T_FLOAT4,     2, {T_FLOAT4, T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_min",    T_FLOAT8,     2, {T_FLOAT8, T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_min",    T_FLOAT16,    2, {T_FLOAT16, T_FLOAT}, {0}, {0}, 1},

    {clvEXTENSION_NONE,     "viv_max",    T_GENTYPE,    2, {T_GENTYPE, T_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "viv_max",    T_FLOAT2,     2, {T_FLOAT2, T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_max",    T_FLOAT3,     2, {T_FLOAT3, T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_max",    T_FLOAT4,     2, {T_FLOAT4, T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_max",    T_FLOAT8,     2, {T_FLOAT8, T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_max",    T_FLOAT16,    2, {T_FLOAT16, T_FLOAT}, {0}, {0}, 1},

    {clvEXTENSION_NONE,     "viv_sign",   T_FLOAT,    1, {T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_sign",   T_FLOAT2,     1, {T_FLOAT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_sign",   T_FLOAT3,     1, {T_FLOAT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_sign",   T_FLOAT4,     1, {T_FLOAT4}, {0}, {0}, 1},

    {clvEXTENSION_NONE,     "viv_fmix",   T_FLOAT, 3, {T_FLOAT, T_FLOAT, T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_fmix",   T_FLOAT2,  3, {T_FLOAT2,  T_FLOAT2,  T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_fmix",   T_FLOAT3,  3, {T_FLOAT3,  T_FLOAT3,  T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_fmix",   T_FLOAT4,  3, {T_FLOAT4,  T_FLOAT4,  T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_fmix",   T_FLOAT2,  3, {T_FLOAT2,  T_FLOAT2,  T_FLOAT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_fmix",   T_FLOAT3,  3, {T_FLOAT3,  T_FLOAT3,  T_FLOAT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_fmix",   T_FLOAT4,  3, {T_FLOAT4,  T_FLOAT4,  T_FLOAT4}, {0}, {0}, 1},

    {clvEXTENSION_NONE,     "viv_mix",    T_FLOAT, 3, {T_FLOAT, T_FLOAT, T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_mix",    T_FLOAT2,  3, {T_FLOAT2,  T_FLOAT2,  T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_mix",    T_FLOAT3,  3, {T_FLOAT3,  T_FLOAT3,  T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_mix",    T_FLOAT4,  3, {T_FLOAT4,  T_FLOAT4,  T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_mix",    T_FLOAT2,  3, {T_FLOAT2,  T_FLOAT2,  T_FLOAT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_mix",    T_FLOAT3,  3, {T_FLOAT3,  T_FLOAT3,  T_FLOAT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_mix",    T_FLOAT4,  3, {T_FLOAT4,  T_FLOAT4,  T_FLOAT4}, {0}, {0}, 1},

    /* Vivante Primitive Angle and Trigonometry Functions */
    {clvEXTENSION_NONE,     "viv_radians",              T_FLOAT,    1, {T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_radians",              T_FLOAT2,   1, {T_FLOAT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_radians",              T_FLOAT3,   1, {T_FLOAT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_radians",              T_FLOAT4,   1, {T_FLOAT4}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_radians",              T_FLOAT8,   1, {T_FLOAT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_radians",              T_FLOAT16,  1, {T_FLOAT16}, {0}, {0}, 1},

    {clvEXTENSION_NONE,     "viv_degrees",              T_FLOAT,    1, {T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_degrees",              T_FLOAT2,   1, {T_FLOAT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_degrees",              T_FLOAT3,   1, {T_FLOAT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_degrees",              T_FLOAT4,   1, {T_FLOAT4}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_degrees",              T_FLOAT8,   1, {T_FLOAT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_degrees",              T_FLOAT16,  1, {T_FLOAT16}, {0}, {0}, 1},

    {clvEXTENSION_NONE,     "viv_step",                 T_F_GENTYPE,  2, {T_F_GENTYPE, T_F_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "viv_step",                 T_FLOAT2,     2, {T_FLOAT,     T_FLOAT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_step",                 T_FLOAT3,     2, {T_FLOAT,     T_FLOAT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_step",                 T_FLOAT4,     2, {T_FLOAT,     T_FLOAT4}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_step",                 T_FLOAT8,     2, {T_FLOAT,     T_FLOAT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_step",                 T_FLOAT16,    2, {T_FLOAT,     T_FLOAT16}, {0}, {0}, 1},

    {clvEXTENSION_NONE,     "viv_smoothstep",           T_FLOAT,    3, {T_FLOAT,       T_FLOAT,    T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_smoothstep",           T_FLOAT2,   3, {T_FLOAT2,      T_FLOAT2,   T_FLOAT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_smoothstep",           T_FLOAT3,   3, {T_FLOAT3,      T_FLOAT3,   T_FLOAT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_smoothstep",           T_FLOAT4,   3, {T_FLOAT4,      T_FLOAT4,   T_FLOAT4}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_smoothstep",           T_FLOAT2,   3, {T_FLOAT,       T_FLOAT,    T_FLOAT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_smoothstep",           T_FLOAT3,   3, {T_FLOAT,       T_FLOAT,    T_FLOAT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_smoothstep",           T_FLOAT4,   3, {T_FLOAT,       T_FLOAT,    T_FLOAT4}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_smoothstep",           T_FLOAT8,   3, {T_FLOAT8,      T_FLOAT8,   T_FLOAT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_smoothstep",           T_FLOAT16,  3, {T_FLOAT16,     T_FLOAT16,  T_FLOAT16}, {0}, {0}, 1},

    /* Vivante Primitive Geometric Functions */
    {clvEXTENSION_NONE,        "viv_cross",          T_GENTYPE,    2, {T_GENTYPE, T_GENTYPE}, {0}, {1, 1}, 1},

    {clvEXTENSION_NONE,     "viv_fast_length",          T_FLOAT,    1, {T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_fast_length",          T_FLOAT,    1, {T_FLOAT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_fast_length",          T_FLOAT,    1, {T_FLOAT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_fast_length",          T_FLOAT,    1, {T_FLOAT4}, {0}, {0}, 1},

    {clvEXTENSION_NONE,     "viv_length",               T_FLOAT,    1, {T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_length",               T_FLOAT,    1, {T_FLOAT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_length",               T_FLOAT,    1, {T_FLOAT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_length",               T_FLOAT,    1, {T_FLOAT4}, {0}, {0}, 1},

    {clvEXTENSION_NONE,     "viv_distance",             T_FLOAT,    2, {T_FLOAT,    T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_distance",             T_FLOAT,    2, {T_FLOAT2,   T_FLOAT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_distance",             T_FLOAT,    2, {T_FLOAT3,   T_FLOAT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_distance",             T_FLOAT,    2, {T_FLOAT4,   T_FLOAT4}, {0}, {0}, 1},

    {clvEXTENSION_NONE,     "viv_fast_distance",        T_FLOAT,    2, {T_FLOAT,    T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_fast_distance",        T_FLOAT,    2, {T_FLOAT2,   T_FLOAT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_fast_distance",        T_FLOAT,    2, {T_FLOAT3,   T_FLOAT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_fast_distance",        T_FLOAT,    2, {T_FLOAT4,   T_FLOAT4}, {0}, {0}, 1},

    {clvEXTENSION_NONE,     "viv_dot",                  T_FLOAT,    2, {T_FLOAT,    T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_dot",                  T_FLOAT,    2, {T_FLOAT2,   T_FLOAT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_dot",                  T_FLOAT,    2, {T_FLOAT3,   T_FLOAT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_dot",                  T_FLOAT,    2, {T_FLOAT4,   T_FLOAT4}, {0}, {0}, 1},

    {clvEXTENSION_NONE,     "viv_cross",                T_FLOAT3,   2, {T_FLOAT3, T_FLOAT3}, {0}, {0}, 1},

    {clvEXTENSION_NONE,     "viv_normalize",            T_FLOAT,    1, {T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_normalize",            T_FLOAT2,   1, {T_FLOAT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_normalize",            T_FLOAT3,   1, {T_FLOAT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_normalize",            T_FLOAT4,   1, {T_FLOAT4}, {0}, {0}, 1},

    {clvEXTENSION_NONE,     "viv_fast_normalize",       T_FLOAT,    1, {T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_fast_normalize",       T_FLOAT2,   1, {T_FLOAT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_fast_normalize",       T_FLOAT3,   1, {T_FLOAT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_fast_normalize",       T_FLOAT4,   1, {T_FLOAT4}, {0}, {0}, 1},

    {clvEXTENSION_NONE,     "viv_faceforward",          T_FLOAT,    3, {T_FLOAT,    T_FLOAT,    T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_faceforward",          T_FLOAT2,   3, {T_FLOAT2,   T_FLOAT2,   T_FLOAT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_faceforward",          T_FLOAT3,   3, {T_FLOAT3,   T_FLOAT3,   T_FLOAT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_faceforward",          T_FLOAT4,   3, {T_FLOAT4,   T_FLOAT4,   T_FLOAT4}, {0}, {0}, 1},

    {clvEXTENSION_NONE,     "viv_reflect",              T_FLOAT,    2, {T_FLOAT,    T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_reflect",              T_FLOAT2,   2, {T_FLOAT2,   T_FLOAT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_reflect",              T_FLOAT3,   2, {T_FLOAT3,   T_FLOAT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_reflect",              T_FLOAT4,   2, {T_FLOAT4,   T_FLOAT4}, {0}, {0}, 1},

    {clvEXTENSION_NONE,     "viv_refract",              T_FLOAT,    3, {T_FLOAT,    T_FLOAT,    T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_refract",              T_FLOAT2,   3, {T_FLOAT2,   T_FLOAT2,   T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_refract",              T_FLOAT3,   3, {T_FLOAT3,   T_FLOAT3,   T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_refract",              T_FLOAT4,   3, {T_FLOAT4,   T_FLOAT4,   T_FLOAT}, {0}, {0}, 1},

    /* Vivante Primitive Vector Relational Functions */
    {clvEXTENSION_NONE,     "viv_isequal",              T_I_GENTYPE,    2, {T_F_GENTYPE, T_F_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "viv_isnotequal",           T_I_GENTYPE,    2, {T_F_GENTYPE, T_F_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "viv_isgreater",            T_I_GENTYPE,    2, {T_F_GENTYPE, T_F_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "viv_isgreaterequal",       T_I_GENTYPE,    2, {T_F_GENTYPE, T_F_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "viv_isless",               T_I_GENTYPE,    2, {T_F_GENTYPE, T_F_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "viv_islessequal",          T_I_GENTYPE,    2, {T_F_GENTYPE, T_F_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "viv_islessgreater",        T_I_GENTYPE,    2, {T_F_GENTYPE, T_F_GENTYPE}, {0}, {1, 1}, 1},

    {clvEXTENSION_NONE,     "viv_isordered",            T_I_GENTYPE,    2, {T_F_GENTYPE, T_F_GENTYPE}, {0}, {1, 1}, 1},
    {clvEXTENSION_NONE,     "viv_isunordered",          T_I_GENTYPE,    2, {T_F_GENTYPE, T_F_GENTYPE}, {0}, {1, 1}, 1},

    {clvEXTENSION_NONE,     "viv_isfinite",             T_I_GENTYPE,    1, {T_F_GENTYPE}, {0}, {1}, 1},
    {clvEXTENSION_NONE,     "viv_isnan",                T_I_GENTYPE,    1, {T_F_GENTYPE}, {0}, {1}, 1},
    {clvEXTENSION_NONE,     "viv_isinf",                T_I_GENTYPE,    1, {T_F_GENTYPE}, {0}, {1}, 1},
    {clvEXTENSION_NONE,     "viv_isnormal",             T_I_GENTYPE,    1, {T_F_GENTYPE}, {0}, {1}, 1},
    {clvEXTENSION_NONE,     "viv_signbit",              T_I_GENTYPE,    1, {T_F_GENTYPE}, {0}, {1}, 1},
    {clvEXTENSION_NONE,     "viv_lgamma",               T_F_GENTYPE,    1, {T_F_GENTYPE}, {0}, {1}, 1},
    {clvEXTENSION_NONE,     "viv_lgamma_r",             T_F_GENTYPE,    2, {T_F_GENTYPE, T_I_GENTYPE}, {0, 1}, {1, 0}, 0},

    {clvEXTENSION_NONE,     "viv_shuffle",              T_I_GENTYPE,    2, {T_CHAR2,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle",              T_I_GENTYPE,    2, {T_CHAR4,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle",              T_I_GENTYPE,    2, {T_CHAR8,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle",              T_I_GENTYPE,    2, {T_CHAR16,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle",              T_U_GENTYPE,    2, {T_UCHAR2,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle",              T_U_GENTYPE,    2, {T_UCHAR4,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle",              T_U_GENTYPE,    2, {T_UCHAR8,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle",              T_U_GENTYPE,    2, {T_UCHAR16,  T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle",              T_I_GENTYPE,    2, {T_SHORT2,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle",              T_I_GENTYPE,    2, {T_SHORT4,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle",              T_I_GENTYPE,    2, {T_SHORT8,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle",              T_I_GENTYPE,    2, {T_SHORT16,  T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle",              T_U_GENTYPE,    2, {T_USHORT2,  T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle",              T_U_GENTYPE,    2, {T_USHORT4,  T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle",              T_U_GENTYPE,    2, {T_USHORT8,  T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle",              T_U_GENTYPE,    2, {T_USHORT16, T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle",              T_I_GENTYPE,    2, {T_INT2,     T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle",              T_I_GENTYPE,    2, {T_INT4,     T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle",              T_I_GENTYPE,    2, {T_INT8,     T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle",              T_I_GENTYPE,    2, {T_INT16,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle",              T_U_GENTYPE,    2, {T_UINT2,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle",              T_U_GENTYPE,    2, {T_UINT4,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle",              T_U_GENTYPE,    2, {T_UINT8,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle",              T_U_GENTYPE,    2, {T_UINT16,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle",              T_I_GENTYPE,    2, {T_LONG2,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle",              T_I_GENTYPE,    2, {T_LONG4,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle",              T_I_GENTYPE,    2, {T_LONG8,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle",              T_I_GENTYPE,    2, {T_LONG16,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle",              T_U_GENTYPE,    2, {T_ULONG2,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle",              T_U_GENTYPE,    2, {T_ULONG4,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle",              T_U_GENTYPE,    2, {T_ULONG8,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle",              T_U_GENTYPE,    2, {T_ULONG16,  T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle",              T_F_GENTYPE,    2, {T_FLOAT2,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle",              T_F_GENTYPE,    2, {T_FLOAT4,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle",              T_F_GENTYPE,    2, {T_FLOAT8,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle",              T_F_GENTYPE,    2, {T_FLOAT16,  T_U_GENTYPE}, {0}, {0}, 0, 0, 1},

    {clvEXTENSION_NONE,     "shuffle#",               T_I_GENTYPE,       2, {T_CHAR2,   T_U_GENTYPE}, {1, 1}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#",               T_I_GENTYPE,       2, {T_CHAR4,   T_U_GENTYPE}, {1, 1}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#",               T_I_GENTYPE,       2, {T_CHAR8,   T_U_GENTYPE}, {1, 1}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#",               T_I_GENTYPE,       2, {T_CHAR16,   T_U_GENTYPE}, {1, 1}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#",               T_U_GENTYPE,       2, {T_UCHAR2,   T_U_GENTYPE}, {1, 1}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#",               T_U_GENTYPE,       2, {T_UCHAR4,   T_U_GENTYPE}, {1, 1}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#",               T_U_GENTYPE,       2, {T_UCHAR8,   T_U_GENTYPE}, {1, 1}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#",               T_U_GENTYPE,       2, {T_UCHAR16,   T_U_GENTYPE}, {1, 1}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#",               T_I_GENTYPE,       2, {T_SHORT2,   T_U_GENTYPE}, {1, 1}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#",               T_I_GENTYPE,       2, {T_SHORT4,   T_U_GENTYPE}, {1, 1}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#",               T_I_GENTYPE,       2, {T_SHORT8,   T_U_GENTYPE}, {1, 1}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#",               T_I_GENTYPE,       2, {T_SHORT16,   T_U_GENTYPE}, {1, 1}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#",               T_U_GENTYPE,       2, {T_USHORT2,   T_U_GENTYPE}, {1, 1}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#",               T_U_GENTYPE,       2, {T_USHORT4,   T_U_GENTYPE}, {1, 1}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#",               T_U_GENTYPE,       2, {T_USHORT8,   T_U_GENTYPE}, {1, 1}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#",               T_U_GENTYPE,       2, {T_USHORT16,   T_U_GENTYPE}, {1, 1}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#",               T_I_GENTYPE,       2, {T_INT2,   T_U_GENTYPE}, {1, 1}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#",               T_I_GENTYPE,       2, {T_INT4,   T_U_GENTYPE}, {1, 1}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#",               T_I_GENTYPE,       2, {T_INT8,   T_U_GENTYPE}, {1, 1}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#",               T_I_GENTYPE,       2, {T_INT16,   T_U_GENTYPE}, {1, 1}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#",               T_U_GENTYPE,       2, {T_UINT2,   T_U_GENTYPE}, {1, 1}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#",               T_U_GENTYPE,       2, {T_UINT4,   T_U_GENTYPE}, {1, 1}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#",               T_U_GENTYPE,       2, {T_UINT8,   T_U_GENTYPE}, {1, 1}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#",               T_U_GENTYPE,       2, {T_UINT16,   T_U_GENTYPE}, {1, 1}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#",               T_I_GENTYPE,       2, {T_LONG2,   T_U_GENTYPE}, {1, 1}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#",               T_I_GENTYPE,       2, {T_LONG4,   T_U_GENTYPE}, {1, 1}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#",               T_I_GENTYPE,       2, {T_LONG8,   T_U_GENTYPE}, {1, 1}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#",               T_I_GENTYPE,       2, {T_LONG16,   T_U_GENTYPE}, {1, 1}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#",               T_U_GENTYPE,       2, {T_ULONG2,   T_U_GENTYPE}, {1, 1}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#",               T_U_GENTYPE,       2, {T_ULONG4,   T_U_GENTYPE}, {1, 1}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#",               T_U_GENTYPE,       2, {T_ULONG8,   T_U_GENTYPE}, {1, 1}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#",               T_U_GENTYPE,       2, {T_ULONG16,   T_U_GENTYPE}, {1, 1}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#",               T_F_GENTYPE,       2, {T_FLOAT2,   T_U_GENTYPE}, {1, 1}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#",               T_F_GENTYPE,       2, {T_FLOAT4,   T_U_GENTYPE}, {1, 1}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#",               T_F_GENTYPE,       2, {T_FLOAT8,   T_U_GENTYPE}, {1, 1}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#",               T_F_GENTYPE,       2, {T_FLOAT16,   T_U_GENTYPE}, {1, 1}, {0}, 0},

    {clvEXTENSION_NONE,     "shuffle#1",               T_I_GENTYPE,       2, {T_CHAR2,   T_U_GENTYPE}, {1, 0}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#1",               T_I_GENTYPE,       2, {T_CHAR4,   T_U_GENTYPE}, {1, 0}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#1",               T_I_GENTYPE,       2, {T_CHAR8,   T_U_GENTYPE}, {1, 0}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#1",               T_I_GENTYPE,       2, {T_CHAR16,   T_U_GENTYPE}, {1, 0}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#1",               T_U_GENTYPE,       2, {T_UCHAR2,   T_U_GENTYPE}, {1, 0}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#1",               T_U_GENTYPE,       2, {T_UCHAR4,   T_U_GENTYPE}, {1, 0}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#1",               T_U_GENTYPE,       2, {T_UCHAR8,   T_U_GENTYPE}, {1, 0}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#1",               T_U_GENTYPE,       2, {T_UCHAR16,   T_U_GENTYPE}, {1, 0}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#1",               T_I_GENTYPE,       2, {T_SHORT2,   T_U_GENTYPE}, {1, 0}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#1",               T_I_GENTYPE,       2, {T_SHORT4,   T_U_GENTYPE}, {1, 0}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#1",               T_I_GENTYPE,       2, {T_SHORT8,   T_U_GENTYPE}, {1, 0}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#1",               T_I_GENTYPE,       2, {T_SHORT16,   T_U_GENTYPE}, {1, 0}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#1",               T_U_GENTYPE,       2, {T_USHORT2,   T_U_GENTYPE}, {1, 0}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#1",               T_U_GENTYPE,       2, {T_USHORT4,   T_U_GENTYPE}, {1, 0}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#1",               T_U_GENTYPE,       2, {T_USHORT8,   T_U_GENTYPE}, {1, 0}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#1",               T_U_GENTYPE,       2, {T_USHORT16,   T_U_GENTYPE}, {1, 0}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#1",               T_I_GENTYPE,       2, {T_INT2,   T_U_GENTYPE}, {1, 0}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#1",               T_I_GENTYPE,       2, {T_INT4,   T_U_GENTYPE}, {1, 0}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#1",               T_I_GENTYPE,       2, {T_INT8,   T_U_GENTYPE}, {1, 0}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#1",               T_I_GENTYPE,       2, {T_INT16,   T_U_GENTYPE}, {1, 0}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#1",               T_U_GENTYPE,       2, {T_UINT2,   T_U_GENTYPE}, {1, 0}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#1",               T_U_GENTYPE,       2, {T_UINT4,   T_U_GENTYPE}, {1, 0}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#1",               T_U_GENTYPE,       2, {T_UINT8,   T_U_GENTYPE}, {1, 0}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#1",               T_U_GENTYPE,       2, {T_UINT16,   T_U_GENTYPE}, {1, 0}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#1",               T_I_GENTYPE,       2, {T_LONG2,   T_U_GENTYPE}, {1, 0}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#1",               T_I_GENTYPE,       2, {T_LONG4,   T_U_GENTYPE}, {1, 0}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#1",               T_I_GENTYPE,       2, {T_LONG8,   T_U_GENTYPE}, {1, 0}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#1",               T_I_GENTYPE,       2, {T_LONG16,   T_U_GENTYPE}, {1, 0}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#1",               T_U_GENTYPE,       2, {T_ULONG2,   T_U_GENTYPE}, {1, 0}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#1",               T_U_GENTYPE,       2, {T_ULONG4,   T_U_GENTYPE}, {1, 0}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#1",               T_U_GENTYPE,       2, {T_ULONG8,   T_U_GENTYPE}, {1, 0}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#1",               T_U_GENTYPE,       2, {T_ULONG16,   T_U_GENTYPE}, {1, 0}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#1",               T_F_GENTYPE,       2, {T_FLOAT2,   T_U_GENTYPE}, {1, 0}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#1",               T_F_GENTYPE,       2, {T_FLOAT4,   T_U_GENTYPE}, {1, 0}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#1",               T_F_GENTYPE,       2, {T_FLOAT8,   T_U_GENTYPE}, {1, 0}, {0}, 0},
    {clvEXTENSION_NONE,     "shuffle#1",               T_F_GENTYPE,       2, {T_FLOAT16,   T_U_GENTYPE}, {1, 0}, {0}, 0},

/* This is a more precise definition of shuffle2 than the one in use */
    {clvEXTENSION_NONE,     "shuffle2",               T_I_GENTYPE,       3, {T_CHAR2,    T_CHAR2,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle2",               T_I_GENTYPE,       3, {T_CHAR4,    T_CHAR4,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle2",               T_I_GENTYPE,       3, {T_CHAR8,    T_CHAR8,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle2",               T_I_GENTYPE,       3, {T_CHAR16,   T_CHAR16,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle2",               T_U_GENTYPE,       3, {T_UCHAR2,   T_UCHAR2,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle2",               T_U_GENTYPE,       3, {T_UCHAR4,   T_UCHAR4,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle2",               T_U_GENTYPE,       3, {T_UCHAR8,   T_UCHAR8,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle2",               T_U_GENTYPE,       3, {T_UCHAR16,  T_UCHAR16,  T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle2",               T_I_GENTYPE,       3, {T_SHORT2,   T_SHORT2,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle2",               T_I_GENTYPE,       3, {T_SHORT4,   T_SHORT4,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle2",               T_I_GENTYPE,       3, {T_SHORT8,   T_SHORT8,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle2",               T_I_GENTYPE,       3, {T_SHORT16,  T_SHORT16,  T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle2",               T_U_GENTYPE,       3, {T_USHORT2,  T_USHORT2,  T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle2",               T_U_GENTYPE,       3, {T_USHORT4,  T_USHORT4,  T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle2",               T_U_GENTYPE,       3, {T_USHORT8,  T_USHORT8,  T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle2",               T_U_GENTYPE,       3, {T_USHORT16, T_USHORT16, T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle2",               T_I_GENTYPE,       3, {T_INT2,     T_INT2,     T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle2",               T_I_GENTYPE,       3, {T_INT4,     T_INT4,     T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle2",               T_I_GENTYPE,       3, {T_INT8,     T_INT8,     T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle2",               T_I_GENTYPE,       3, {T_INT16,    T_INT16,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle2",               T_U_GENTYPE,       3, {T_UINT2,    T_UINT2,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle2",               T_U_GENTYPE,       3, {T_UINT4,    T_UINT4,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle2",               T_U_GENTYPE,       3, {T_UINT8,    T_UINT8,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle2",               T_U_GENTYPE,       3, {T_UINT16,   T_UINT16,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle2",               T_I_GENTYPE,       3, {T_LONG2,    T_LONG2,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle2",               T_I_GENTYPE,       3, {T_LONG4,    T_LONG4,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle2",               T_I_GENTYPE,       3, {T_LONG8,    T_LONG8,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle2",               T_I_GENTYPE,       3, {T_LONG16,   T_LONG16,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle2",               T_U_GENTYPE,       3, {T_ULONG2,   T_ULONG2,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle2",               T_U_GENTYPE,       3, {T_ULONG4,   T_ULONG4,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle2",               T_U_GENTYPE,       3, {T_ULONG8,   T_ULONG8,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle2",               T_U_GENTYPE,       3, {T_ULONG16,  T_ULONG16,  T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle2",               T_F_GENTYPE,       3, {T_FLOAT2,   T_FLOAT2,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle2",               T_F_GENTYPE,       3, {T_FLOAT4,   T_FLOAT4,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle2",               T_F_GENTYPE,       3, {T_FLOAT8,   T_FLOAT8,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "shuffle2",               T_F_GENTYPE,       3, {T_FLOAT16,  T_FLOAT16,  T_U_GENTYPE}, {0}, {0}, 0, 0, 1},

    {clvEXTENSION_NONE,     "viv_shuffle2",               T_I_GENTYPE,       3, {T_CHAR2,    T_CHAR2,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle2",               T_I_GENTYPE,       3, {T_CHAR4,    T_CHAR4,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle2",               T_I_GENTYPE,       3, {T_CHAR8,    T_CHAR8,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle2",               T_I_GENTYPE,       3, {T_CHAR16,   T_CHAR16,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle2",               T_U_GENTYPE,       3, {T_UCHAR2,   T_UCHAR2,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle2",               T_U_GENTYPE,       3, {T_UCHAR4,   T_UCHAR4,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle2",               T_U_GENTYPE,       3, {T_UCHAR8,   T_UCHAR8,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle2",               T_U_GENTYPE,       3, {T_UCHAR16,  T_UCHAR16,  T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle2",               T_I_GENTYPE,       3, {T_SHORT2,   T_SHORT2,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle2",               T_I_GENTYPE,       3, {T_SHORT4,   T_SHORT4,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle2",               T_I_GENTYPE,       3, {T_SHORT8,   T_SHORT8,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle2",               T_I_GENTYPE,       3, {T_SHORT16,  T_SHORT16,  T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle2",               T_U_GENTYPE,       3, {T_USHORT2,  T_USHORT2,  T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle2",               T_U_GENTYPE,       3, {T_USHORT4,  T_USHORT4,  T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle2",               T_U_GENTYPE,       3, {T_USHORT8,  T_USHORT8,  T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle2",               T_U_GENTYPE,       3, {T_USHORT16, T_USHORT16, T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle2",               T_I_GENTYPE,       3, {T_INT2,     T_INT2,     T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle2",               T_I_GENTYPE,       3, {T_INT4,     T_INT4,     T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle2",               T_I_GENTYPE,       3, {T_INT8,     T_INT8,     T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle2",               T_I_GENTYPE,       3, {T_INT16,    T_INT16,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle2",               T_U_GENTYPE,       3, {T_UINT2,    T_UINT2,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle2",               T_U_GENTYPE,       3, {T_UINT4,    T_UINT4,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle2",               T_U_GENTYPE,       3, {T_UINT8,    T_UINT8,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle2",               T_U_GENTYPE,       3, {T_UINT16,   T_UINT16,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle2",               T_I_GENTYPE,       3, {T_LONG2,    T_LONG2,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle2",               T_I_GENTYPE,       3, {T_LONG4,    T_LONG4,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle2",               T_I_GENTYPE,       3, {T_LONG8,    T_LONG8,    T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle2",               T_I_GENTYPE,       3, {T_LONG16,   T_LONG16,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle2",               T_U_GENTYPE,       3, {T_ULONG2,   T_ULONG2,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle2",               T_U_GENTYPE,       3, {T_ULONG4,   T_ULONG4,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle2",               T_U_GENTYPE,       3, {T_ULONG8,   T_ULONG8,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle2",               T_U_GENTYPE,       3, {T_ULONG16,  T_ULONG16,  T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle2",               T_F_GENTYPE,       3, {T_FLOAT2,   T_FLOAT2,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle2",               T_F_GENTYPE,       3, {T_FLOAT4,   T_FLOAT4,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle2",               T_F_GENTYPE,       3, {T_FLOAT8,   T_FLOAT8,   T_U_GENTYPE}, {0}, {0}, 0, 0, 1},
    {clvEXTENSION_NONE,     "viv_shuffle2",               T_F_GENTYPE,       3, {T_FLOAT16,  T_FLOAT16,  T_U_GENTYPE}, {0}, {0}, 0, 0, 1},


    {clvEXTENSION_NONE,     "shuffle2#",               T_I_GENTYPE,       3, {T_CHAR2,   T_CHAR2,   T_U_GENTYPE}, {1,1,1}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#",               T_I_GENTYPE,       3, {T_CHAR4,   T_CHAR4,   T_U_GENTYPE}, {1,1,1}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#",               T_I_GENTYPE,       3, {T_CHAR8,   T_CHAR8,   T_U_GENTYPE}, {1,1,1}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#",               T_I_GENTYPE,       3, {T_CHAR16,   T_CHAR16,   T_U_GENTYPE}, {1,1,1}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#",               T_U_GENTYPE,       3, {T_UCHAR2,   T_UCHAR2,   T_U_GENTYPE}, {1,1,1}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#",               T_U_GENTYPE,       3, {T_UCHAR4,   T_UCHAR4,   T_U_GENTYPE}, {1,1,1}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#",               T_U_GENTYPE,       3, {T_UCHAR8,   T_UCHAR8,   T_U_GENTYPE}, {1,1,1}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#",               T_U_GENTYPE,       3, {T_UCHAR16,   T_UCHAR16,   T_U_GENTYPE}, {1,1,1}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#",               T_I_GENTYPE,       3, {T_SHORT2,   T_SHORT2,   T_U_GENTYPE}, {1,1,1}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#",               T_I_GENTYPE,       3, {T_SHORT4,   T_SHORT4,   T_U_GENTYPE}, {1,1,1}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#",               T_I_GENTYPE,       3, {T_SHORT8,   T_SHORT8,   T_U_GENTYPE}, {1,1,1}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#",               T_I_GENTYPE,       3, {T_SHORT16,   T_SHORT16,   T_U_GENTYPE}, {1,1,1}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#",               T_U_GENTYPE,       3, {T_USHORT2,   T_USHORT2,   T_U_GENTYPE}, {1,1,1}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#",               T_U_GENTYPE,       3, {T_USHORT4,   T_USHORT4,   T_U_GENTYPE}, {1,1,1}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#",               T_U_GENTYPE,       3, {T_USHORT8,   T_USHORT8,   T_U_GENTYPE}, {1,1,1}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#",               T_U_GENTYPE,       3, {T_USHORT16,   T_USHORT16,   T_U_GENTYPE}, {1,1,1}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#",               T_I_GENTYPE,       3, {T_INT2,   T_INT2,   T_U_GENTYPE}, {1,1,1}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#",               T_I_GENTYPE,       3, {T_INT4,   T_INT4,   T_U_GENTYPE}, {1,1,1}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#",               T_I_GENTYPE,       3, {T_INT8,   T_INT8,   T_U_GENTYPE}, {1,1,1}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#",               T_I_GENTYPE,       3, {T_INT16,   T_INT16,   T_U_GENTYPE}, {1,1,1}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#",               T_U_GENTYPE,       3, {T_UINT2,   T_UINT2,   T_U_GENTYPE}, {1,1,1}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#",               T_U_GENTYPE,       3, {T_UINT4,   T_UINT4,   T_U_GENTYPE}, {1,1,1}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#",               T_U_GENTYPE,       3, {T_UINT8,   T_UINT8,   T_U_GENTYPE}, {1,1,1}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#",               T_U_GENTYPE,       3, {T_UINT16,   T_UINT16,   T_U_GENTYPE}, {1,1,1}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#",               T_F_GENTYPE,       3, {T_FLOAT2,   T_FLOAT2,   T_U_GENTYPE}, {1,1,1}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#",               T_F_GENTYPE,       3, {T_FLOAT4,   T_FLOAT4,   T_U_GENTYPE}, {1,1,1}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#",               T_F_GENTYPE,       3, {T_FLOAT8,   T_FLOAT8,   T_U_GENTYPE}, {1,1,1}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#",               T_F_GENTYPE,       3, {T_FLOAT16,  T_FLOAT16,  T_U_GENTYPE}, {1,1,1}, {0}, 0, 0, 0},

    {clvEXTENSION_NONE,     "shuffle2#1",               T_I_GENTYPE,       3, {T_CHAR2,   T_CHAR2,   T_U_GENTYPE}, {1,1,0}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#1",               T_I_GENTYPE,       3, {T_CHAR4,   T_CHAR4,   T_U_GENTYPE}, {1,1,0}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#1",               T_I_GENTYPE,       3, {T_CHAR8,   T_CHAR8,   T_U_GENTYPE}, {1,1,0}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#1",               T_I_GENTYPE,       3, {T_CHAR16,   T_CHAR16,   T_U_GENTYPE}, {1,1,0}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#1",               T_U_GENTYPE,       3, {T_UCHAR2,   T_UCHAR2,   T_U_GENTYPE}, {1,1,0}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#1",               T_U_GENTYPE,       3, {T_UCHAR4,   T_UCHAR4,   T_U_GENTYPE}, {1,1,0}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#1",               T_U_GENTYPE,       3, {T_UCHAR8,   T_UCHAR8,   T_U_GENTYPE}, {1,1,0}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#1",               T_U_GENTYPE,       3, {T_UCHAR16,   T_UCHAR16,   T_U_GENTYPE}, {1,1,0}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#1",               T_I_GENTYPE,       3, {T_SHORT2,   T_SHORT2,   T_U_GENTYPE}, {1,1,0}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#1",               T_I_GENTYPE,       3, {T_SHORT4,   T_SHORT4,   T_U_GENTYPE}, {1,1,0}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#1",               T_I_GENTYPE,       3, {T_SHORT8,   T_SHORT8,   T_U_GENTYPE}, {1,1,0}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#1",               T_I_GENTYPE,       3, {T_SHORT16,   T_SHORT16,   T_U_GENTYPE}, {1,1,0}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#1",               T_U_GENTYPE,       3, {T_USHORT2,   T_USHORT2,   T_U_GENTYPE}, {1,1,0}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#1",               T_U_GENTYPE,       3, {T_USHORT4,   T_USHORT4,   T_U_GENTYPE}, {1,1,0}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#1",               T_U_GENTYPE,       3, {T_USHORT8,   T_USHORT8,   T_U_GENTYPE}, {1,1,0}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#1",               T_U_GENTYPE,       3, {T_USHORT16,   T_USHORT16,   T_U_GENTYPE}, {1,1,0}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#1",               T_I_GENTYPE,       3, {T_INT2,   T_INT2,   T_U_GENTYPE}, {1,1,0}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#1",               T_I_GENTYPE,       3, {T_INT4,   T_INT4,   T_U_GENTYPE}, {1,1,0}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#1",               T_I_GENTYPE,       3, {T_INT8,   T_INT8,   T_U_GENTYPE}, {1,1,0}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#1",               T_I_GENTYPE,       3, {T_INT16,   T_INT16,   T_U_GENTYPE}, {1,1,0}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#1",               T_U_GENTYPE,       3, {T_UINT2,   T_UINT2,   T_U_GENTYPE}, {1,1,0}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#1",               T_U_GENTYPE,       3, {T_UINT4,   T_UINT4,   T_U_GENTYPE}, {1,1,0}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#1",               T_U_GENTYPE,       3, {T_UINT8,   T_UINT8,   T_U_GENTYPE}, {1,1,0}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#1",               T_U_GENTYPE,       3, {T_UINT16,   T_UINT16,   T_U_GENTYPE}, {1,1,0}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#1",               T_F_GENTYPE,       3, {T_FLOAT2,   T_FLOAT2,   T_U_GENTYPE}, {1,1,0}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#1",               T_F_GENTYPE,       3, {T_FLOAT4,   T_FLOAT4,   T_U_GENTYPE}, {1,1,0}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#1",               T_F_GENTYPE,       3, {T_FLOAT8,   T_FLOAT8,   T_U_GENTYPE}, {1,1,0}, {0}, 0, 0, 0},
    {clvEXTENSION_NONE,     "shuffle2#1",               T_F_GENTYPE,       3, {T_FLOAT16,  T_FLOAT16,  T_U_GENTYPE}, {1,1,0}, {0}, 0, 0, 0},

    {clvEXTENSION_NONE,        "select",          T_GENTYPE,      3, {T_GENTYPE, T_GENTYPE, T_I_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,        "select",          T_GENTYPE,      3, {T_GENTYPE, T_GENTYPE, T_U_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE,        "bitselect",      T_GENTYPE,      3, {T_GENTYPE, T_GENTYPE, T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE,     "any",              T_INT,          1, {T_I_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "all",              T_INT,          1, {T_I_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE,        "viv_select",       T_GENTYPE,      3, {T_GENTYPE, T_GENTYPE, T_I_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,        "viv_select",       T_GENTYPE,      3, {T_GENTYPE, T_GENTYPE, T_U_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE,        "viv_bitselect",      T_GENTYPE,      3, {T_GENTYPE, T_GENTYPE, T_GENTYPE}, {0}, {0}, 1},

    {clvEXTENSION_NONE,     "viv_any",          T_INT,          1, {T_I_GENTYPE}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "viv_all",          T_INT,          1, {T_I_GENTYPE}, {0}, {0}, 1},

    /* Matrix Functions */
    {clvEXTENSION_NONE,     "matrixCompMult",       T_MAT2,     2, {T_MAT2,         T_MAT2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "matrixCompMult",       T_MAT3,     2, {T_MAT3,         T_MAT3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,     "matrixCompMult",       T_MAT4,     2, {T_MAT4,         T_MAT4}, {0}, {0}, 1},

    /* Async copy and prefetch */
    {clvEXTENSION_NONE,     "async_work_group_copy",        T_EVENT_T,    4, {T_GENTYPE, T_GENTYPE, T_SIZE_T, T_EVENT_T}, {1, 1, 0, 0}, {0, 0, 1, 1}, 1},
    {clvEXTENSION_NONE,     "async_work_group_strided_copy",    T_EVENT_T,    5, {T_GENTYPE, T_GENTYPE, T_SIZE_T, T_SIZE_T, T_EVENT_T}, {1, 1, 0, 0, 0}, {0, 0, 1, 1, 1}, 1},
    {clvEXTENSION_NONE,     "wait_group_events",    T_VOID,        2, {T_INT, T_EVENT_T}, {0, 1}, {1, 0}, 1},
    {clvEXTENSION_NONE,     "prefetch",            T_VOID,        2, {T_GENTYPE, T_SIZE_T}, {1, 0}, {0, 1}, 1},

    /* Atomic Functions */
    {clvEXTENSION_NONE,        "atomic_add",      T_INT,    2, {T_INT,  T_INT},  {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atomic_add",      T_UINT,   2, {T_UINT, T_UINT}, {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atomic_add",      T_INT,    2, {T_INT,  T_UINT}, {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atomic_add",      T_UINT,   2, {T_UINT, T_INT},  {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atomic_sub",      T_INT,    2, {T_INT,  T_INT},  {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atomic_sub",      T_UINT,   2, {T_UINT, T_UINT}, {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atomic_sub",      T_INT,    2, {T_INT,  T_UINT}, {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atomic_sub",      T_UINT,   2, {T_UINT, T_INT},  {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atomic_inc",      T_INT,    1, {T_INT},  {1}, {0}, 1},
    {clvEXTENSION_NONE,        "atomic_inc",      T_UINT,   1, {T_UINT}, {1}, {0}, 1},
    {clvEXTENSION_NONE,        "atomic_dec",      T_INT,    1, {T_INT},  {1}, {0}, 1},
    {clvEXTENSION_NONE,        "atomic_dec",      T_UINT,   1, {T_UINT}, {1}, {0}, 1},
    {clvEXTENSION_NONE,        "atomic_xchg",     T_INT,    2, {T_INT,  T_INT},  {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atomic_xchg",     T_UINT,   2, {T_UINT, T_UINT}, {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atomic_xchg",     T_INT,    2, {T_INT,  T_UINT}, {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atomic_xchg",     T_UINT,   2, {T_UINT, T_INT},  {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atomic_xchg",     T_FLOAT,  2, {T_FLOAT, T_FLOAT}, {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atomic_xchg",     T_FLOAT,  2, {T_FLOAT, T_UINT},  {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atomic_xchg",     T_FLOAT,  2, {T_FLOAT, T_INT},   {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atomic_cmpxchg",      T_INT,    3, {T_INT,  T_INT,  T_INT},  {1, 0, 0}, {0, 1, 1}, 1},
    {clvEXTENSION_NONE,        "atomic_cmpxchg",      T_UINT,   3, {T_UINT, T_UINT, T_UINT}, {1, 0, 0}, {0, 1, 1}, 1},
    {clvEXTENSION_NONE,        "atomic_cmpxchg",      T_INT,    3, {T_INT,  T_INT,  T_UINT}, {1, 0, 0}, {0, 1, 1}, 1},
    {clvEXTENSION_NONE,        "atomic_cmpxchg",      T_UINT,   3, {T_UINT, T_UINT, T_INT},  {1, 0, 0}, {0, 1, 1}, 1},
    {clvEXTENSION_NONE,        "atomic_cmpxchg",      T_INT,    3, {T_INT,  T_UINT, T_INT},  {1, 0, 0}, {0, 1, 1}, 1},
    {clvEXTENSION_NONE,        "atomic_cmpxchg",      T_UINT,   3, {T_UINT, T_INT,  T_UINT}, {1, 0, 0}, {0, 1, 1}, 1},
    {clvEXTENSION_NONE,        "atomic_cmpxchg",      T_INT,    3, {T_INT,  T_UINT, T_UINT}, {1, 0, 0}, {0, 1, 1}, 1},
    {clvEXTENSION_NONE,        "atomic_cmpxchg",      T_UINT,   3, {T_UINT, T_INT,  T_INT},  {1, 0, 0}, {0, 1, 1}, 1},
    {clvEXTENSION_NONE,        "atomic_min",      T_INT,    2, {T_INT,  T_INT},  {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atomic_min",      T_UINT,   2, {T_UINT, T_UINT}, {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atomic_max",      T_INT,    2, {T_INT,  T_INT},  {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atomic_max",      T_UINT,   2, {T_UINT, T_UINT}, {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atomic_or",      T_INT,    2, {T_INT,  T_INT},  {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atomic_or",      T_UINT,   2, {T_UINT, T_UINT}, {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atomic_or",      T_INT,    2, {T_INT,  T_UINT}, {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atomic_or",      T_UINT,   2, {T_UINT, T_INT},  {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atomic_and",      T_INT,    2, {T_INT,  T_INT},  {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atomic_and",      T_UINT,   2, {T_UINT, T_UINT}, {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atomic_and",      T_INT,    2, {T_INT,  T_UINT}, {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atomic_and",      T_UINT,   2, {T_UINT, T_INT},  {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atomic_xor",      T_INT,    2, {T_INT,  T_INT},  {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atomic_xor",      T_UINT,   2, {T_UINT, T_UINT}, {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atomic_xor",      T_INT,    2, {T_INT,  T_UINT}, {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atomic_xor",      T_UINT,   2, {T_UINT, T_INT},  {1, 0}, {0, 1}, 1},

    /* Alias of atomic functions used in conformance tests */
    {clvEXTENSION_NONE,        "atom_add",      T_INT,    2, {T_INT,  T_INT},  {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atom_add",      T_UINT,   2, {T_UINT, T_UINT}, {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atom_add",      T_INT,    2, {T_INT,  T_UINT}, {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atom_add",      T_UINT,   2, {T_UINT, T_INT},  {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atom_sub",      T_INT,    2, {T_INT,  T_INT},  {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atom_sub",      T_UINT,   2, {T_UINT, T_UINT}, {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atom_sub",      T_INT,    2, {T_INT,  T_UINT}, {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atom_sub",      T_UINT,   2, {T_UINT, T_INT},  {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atom_inc",      T_INT,    1, {T_INT},  {1}, {0}, 1},
    {clvEXTENSION_NONE,        "atom_inc",      T_UINT,   1, {T_UINT}, {1}, {0}, 1},
    {clvEXTENSION_NONE,        "atom_dec",      T_INT,    1, {T_INT},  {1}, {0}, 1},
    {clvEXTENSION_NONE,        "atom_dec",      T_UINT,   1, {T_UINT}, {1}, {0}, 1},
    {clvEXTENSION_NONE,        "atom_xchg",     T_INT,    2, {T_INT,  T_INT},  {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atom_xchg",     T_UINT,   2, {T_UINT, T_UINT}, {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atom_xchg",     T_INT,    2, {T_INT,  T_UINT}, {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atom_xchg",     T_UINT,   2, {T_UINT, T_INT},  {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atom_xchg",     T_FLOAT,  2, {T_FLOAT, T_FLOAT}, {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atom_xchg",     T_FLOAT,  2, {T_FLOAT, T_UINT},  {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atom_xchg",     T_FLOAT,  2, {T_FLOAT, T_INT},   {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atom_cmpxchg",  T_INT,    3, {T_INT,  T_INT,  T_INT},  {1, 0, 0}, {0, 1, 1}, 1},
    {clvEXTENSION_NONE,        "atom_cmpxchg",  T_UINT,   3, {T_UINT, T_UINT, T_UINT}, {1, 0, 0}, {0, 1, 1}, 1},
    {clvEXTENSION_NONE,        "atom_cmpxchg",  T_INT,    3, {T_INT,  T_INT,  T_UINT}, {1, 0, 0}, {0, 1, 1}, 1},
    {clvEXTENSION_NONE,        "atom_cmpxchg",  T_UINT,   3, {T_UINT, T_UINT, T_INT},  {1, 0, 0}, {0, 1, 1}, 1},
    {clvEXTENSION_NONE,        "atom_cmpxchg",  T_INT,    3, {T_INT,  T_UINT, T_INT},  {1, 0, 0}, {0, 1, 1}, 1},
    {clvEXTENSION_NONE,        "atom_cmpxchg",  T_UINT,   3, {T_UINT, T_INT,  T_UINT}, {1, 0, 0}, {0, 1, 1}, 1},
    {clvEXTENSION_NONE,        "atom_cmpxchg",  T_INT,    3, {T_INT,  T_UINT, T_UINT}, {1, 0, 0}, {0, 1, 1}, 1},
    {clvEXTENSION_NONE,        "atom_cmpxchg",  T_UINT,   3, {T_UINT, T_INT,  T_INT},  {1, 0, 0}, {0, 1, 1}, 1},
    {clvEXTENSION_NONE,        "atom_min",      T_INT,    2, {T_INT,  T_INT},  {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atom_min",      T_UINT,   2, {T_UINT, T_UINT}, {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atom_max",      T_INT,    2, {T_INT,  T_INT},  {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atom_max",      T_UINT,   2, {T_UINT, T_UINT}, {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atom_or",       T_INT,    2, {T_INT,  T_INT},  {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atom_or",       T_UINT,   2, {T_UINT, T_UINT}, {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atom_or",       T_INT,    2, {T_INT,  T_UINT}, {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atom_or",       T_UINT,   2, {T_UINT, T_INT},  {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atom_and",      T_INT,    2, {T_INT,  T_INT},  {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atom_and",      T_UINT,   2, {T_UINT, T_UINT}, {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atom_and",      T_INT,    2, {T_INT,  T_UINT}, {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atom_and",      T_UINT,   2, {T_UINT, T_INT},  {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atom_xor",      T_INT,    2, {T_INT,  T_INT},  {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atom_xor",      T_UINT,   2, {T_UINT, T_UINT}, {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atom_xor",      T_INT,    2, {T_INT,  T_UINT}, {1, 0}, {0, 1}, 1},
    {clvEXTENSION_NONE,        "atom_xor",      T_UINT,   2, {T_UINT, T_INT},  {1, 0}, {0, 1}, 1},

#if _SUPPORT_PRINTF_BUILTIN
    {clvEXTENSION_NONE,        "printf",        T_INT,   1, {T_CHAR},  {1}, {0}, 1, 0, 0, 1},
#endif

};

#define _cldCommonBuiltinFunctionCount (sizeof(CommonBuiltinFunctions) / sizeof(clsBUILTIN_FUNCTION))

static gceSTATUS
_GenRadiansCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;
    clsROPERAND    constantROperand;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);
    gcmASSERT(clmIsElementTypeFloating(clmGEN_CODE_elementType_GET(IOperand->dataType)));

    /* mul result, degrees, _PI / 180 */
    clsROPERAND_InitializeFloatOrVecOrMatConstant(&constantROperand,
                              clmGenCodeDataType(T_FLOAT),
                              _PI / (gctFLOAT)180.0);

    status = clGenArithmeticExprCode(Compiler,
                    PolynaryExpr->exprBase.base.lineNo,
                    PolynaryExpr->exprBase.base.stringNo,
                    clvOPCODE_MUL,
                    IOperand,
                    &OperandsParameters[0].rOperands[0],
                    &constantROperand);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenDegreesCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;
    clsROPERAND    constantROperand;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);
    gcmASSERT(clmIsElementTypeFloating(clmGEN_CODE_elementType_GET(IOperand->dataType)));

    /* mul result, radians, 180 / _PI */
    clsROPERAND_InitializeFloatOrVecOrMatConstant(&constantROperand,
                              clmGenCodeDataType(T_FLOAT),
                              (gctFLOAT)180.0 / _PI);

    status = clGenArithmeticExprCode(Compiler,
                    PolynaryExpr->exprBase.base.lineNo,
                    PolynaryExpr->exprBase.base.stringNo,
                    clvOPCODE_MUL,
                    IOperand,
                    &OperandsParameters[0].rOperands[0],
                    &constantROperand);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenScalarMinCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
   gceSTATUS    status;
   clsSELECTION_CONTEXT   selectionContextNanX, selectionContextNanY;
   clsIOPERAND intermIOperands[2];
   clsROPERAND intermROperands[2];
   clsROPERAND infROperand, unsignROperand;
   int i;
   /* Verify the arguments. */
   clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
   clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
   gcmASSERT(OperandCount == 2);
   gcmASSERT(OperandsParameters);
   gcmASSERT(IOperand);

   if(OperandsParameters[0].rOperands[0].dataType.elementType == clvTYPE_FLOAT) {
    clsROPERAND_InitializeIntOrIVecConstant(&infROperand,
                        clmGenCodeDataType(T_UINT),
                        0x7f800000);
    clsROPERAND_InitializeIntOrIVecConstant(&unsignROperand,
                        clmGenCodeDataType(T_UINT),
                        0x7fffffff);

    for(i = 0; i<2; i++){
        clsIOPERAND_New(Compiler, &intermIOperands[i], clmGenCodeDataType(T_UINT));
        clsROPERAND_InitializeUsingIOperand(&intermROperands[i], &intermIOperands[i]);
        status = clGenBitwiseExprCode(Compiler,
                PolynaryExpr->exprBase.base.lineNo,
                PolynaryExpr->exprBase.base.stringNo,
                clvOPCODE_BITWISE_AND,
                &intermIOperands[i],
                &unsignROperand,
                &OperandsParameters[i].rOperands[0]);
    }

    status = clDefineSelectionBegin(Compiler,
                                    CodeGenerator,
                                    gcvTRUE,
                                    &selectionContextNanX);
    if (gcmIS_ERROR(status)) return status;

    /*|x| > inf, x == nan */
    status = clGenSelectionCompareConditionCode(Compiler,
                                                CodeGenerator,
                                                &selectionContextNanX,
                                                PolynaryExpr->exprBase.base.lineNo,
                                                PolynaryExpr->exprBase.base.stringNo,
                                                clvCONDITION_GREATER_THAN,
                                                &intermROperands[0],
                                                &infROperand);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionTrueOperandBegin(Compiler,
                                            CodeGenerator,
                                            &selectionContextNanX);

    if (gcmIS_ERROR(status)) return status;

    /*x == Nan, return to Y */
    status = clGenGenericCode1(Compiler,
                PolynaryExpr->exprBase.base.lineNo,
                PolynaryExpr->exprBase.base.stringNo,
                clvOPCODE_ASSIGN,
                IOperand,
                &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) return status;



    status = clDefineSelectionTrueOperandEnd(Compiler,
                                    CodeGenerator,
                                    &selectionContextNanX,
                                    gcvFALSE);

    if (gcmIS_ERROR(status)) return status;

    /*x != Nan  */

    status = clDefineSelectionFalseOperandBegin(Compiler,
                                            CodeGenerator,
                                            &selectionContextNanX);
    if (gcmIS_ERROR(status)) return status;

    {
    status = clDefineSelectionBegin(Compiler,
                                    CodeGenerator,
                                    gcvTRUE,
                                    &selectionContextNanY);
    if (gcmIS_ERROR(status)) return status;

    /* Y == Nan */
    status = clGenSelectionCompareConditionCode(Compiler,
                                                CodeGenerator,
                                                &selectionContextNanY,
                                                PolynaryExpr->exprBase.base.lineNo,
                                                PolynaryExpr->exprBase.base.stringNo,
                                                clvCONDITION_GREATER_THAN,
                                                &intermROperands[1],
                                                &infROperand);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionTrueOperandBegin(Compiler,
                                            CodeGenerator,
                                            &selectionContextNanY);

    if (gcmIS_ERROR(status)) return status;

    /*Y == Nan, return to x */
    status = clGenGenericCode1(Compiler,
                PolynaryExpr->exprBase.base.lineNo,
                PolynaryExpr->exprBase.base.stringNo,
                clvOPCODE_ASSIGN,
                IOperand,
                &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionTrueOperandEnd(Compiler,
                                    CodeGenerator,
                                    &selectionContextNanY,
                                    gcvFALSE);

    if (gcmIS_ERROR(status)) return status;



    status = clDefineSelectionFalseOperandBegin(Compiler,
                                            CodeGenerator,
                                            &selectionContextNanY);
    if (gcmIS_ERROR(status)) return status;

    /*Normal case, x != Nan, y != Nan*/
    status = clGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_MIN,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) return status;


    status = clDefineSelectionFalseOperandEnd(Compiler,
                                    CodeGenerator,
                                    &selectionContextNanY);

    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionEnd(Compiler,
                                CodeGenerator,
                                &selectionContextNanY);
    if (gcmIS_ERROR(status)) return status;
    }
    status = clDefineSelectionFalseOperandEnd(Compiler,
                                    CodeGenerator,
                                    &selectionContextNanX);

    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionEnd(Compiler,
                                CodeGenerator,
                                &selectionContextNanX);
    return status;
   }
   else{ /*Integer cases */
      return clGenGenericCode2(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               clvOPCODE_MIN,
                               IOperand,
                               &OperandsParameters[0].rOperands[0],
                               &OperandsParameters[1].rOperands[0]);
   }
}

static gceSTATUS
_GenMinCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
   /* Verify the arguments. */
   clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
   clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
   gcmASSERT(OperandCount == 2);
   gcmASSERT(OperandsParameters);
   gcmASSERT(IOperand);

   if(OperandsParameters[0].rOperands[0].dataType.elementType == clvTYPE_FLOAT &&
      !(CodeGenerator->fpConfig & cldFpFINITE_MATH_ONLY)) {
      if(IOperand &&
         clmGEN_CODE_IsVectorDataType(OperandsParameters[0].dataTypes[0].def)) { /*Floating point vector case with no finite math */
         return _GenBuiltinVectorCode(Compiler,
                                      CodeGenerator,
                                      PolynaryExpr,
                                      OperandCount,
                                      OperandsParameters,
                                      IOperand,
                                      _GenScalarMinCode);
      }
      else {
         return _GenScalarMinCode(Compiler,
                                  CodeGenerator,
                                  PolynaryExpr,
                                  OperandCount,
                                  OperandsParameters,
                                  IOperand);
      }
   }
   else{ /* integer or floating case without INF or NAN consideration */
      return clGenGenericCode2(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               clvOPCODE_MIN,
                               IOperand,
                               &OperandsParameters[0].rOperands[0],
                               &OperandsParameters[1].rOperands[0]);
   }
}

static gceSTATUS
_GenScalarMaxCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
   gceSTATUS    status;
   clsSELECTION_CONTEXT   selectionContextNanX, selectionContextNanY;
   clsIOPERAND intermIOperands[2];
   clsROPERAND intermROperands[2];
   clsROPERAND infROperand, unsignROperand;
   int i;
   /* Verify the arguments. */
   clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
   clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
   gcmASSERT(OperandCount == 2);
   gcmASSERT(OperandsParameters);
   gcmASSERT(IOperand);

   if(OperandsParameters[0].rOperands[0].dataType.elementType == clvTYPE_FLOAT) {
    clsROPERAND_InitializeIntOrIVecConstant(&infROperand,
                        clmGenCodeDataType(T_UINT),
                        0x7f800000);
    clsROPERAND_InitializeIntOrIVecConstant(&unsignROperand,
                        clmGenCodeDataType(T_UINT),
                        0x7fffffff);

    for(i = 0; i<2; i++){
        clsIOPERAND_New(Compiler, &intermIOperands[i], clmGenCodeDataType(T_UINT));
        clsROPERAND_InitializeUsingIOperand(&intermROperands[i], &intermIOperands[i]);
        status = clGenBitwiseExprCode(Compiler,
                PolynaryExpr->exprBase.base.lineNo,
                PolynaryExpr->exprBase.base.stringNo,
                clvOPCODE_BITWISE_AND,
                &intermIOperands[i],
                &unsignROperand,
                &OperandsParameters[i].rOperands[0]);
    }

    status = clDefineSelectionBegin(Compiler,
                                    CodeGenerator,
                                    gcvTRUE,
                                    &selectionContextNanX);
    if (gcmIS_ERROR(status)) return status;

    /*|x| > inf, x == nan */
    status = clGenSelectionCompareConditionCode(Compiler,
                                                CodeGenerator,
                                                &selectionContextNanX,
                                                PolynaryExpr->exprBase.base.lineNo,
                                                PolynaryExpr->exprBase.base.stringNo,
                                                clvCONDITION_GREATER_THAN,
                                                &intermROperands[0],
                                                &infROperand);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionTrueOperandBegin(Compiler,
                                            CodeGenerator,
                                            &selectionContextNanX);

    if (gcmIS_ERROR(status)) return status;

    /*x == Nan, return to Y */
    status = clGenGenericCode1(Compiler,
                PolynaryExpr->exprBase.base.lineNo,
                PolynaryExpr->exprBase.base.stringNo,
                clvOPCODE_ASSIGN,
                IOperand,
                &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) return status;



    status = clDefineSelectionTrueOperandEnd(Compiler,
                                    CodeGenerator,
                                    &selectionContextNanX,
                                    gcvFALSE);

    if (gcmIS_ERROR(status)) return status;

    /*x != Nan  */

    status = clDefineSelectionFalseOperandBegin(Compiler,
                                            CodeGenerator,
                                            &selectionContextNanX);
    if (gcmIS_ERROR(status)) return status;

    {
    status = clDefineSelectionBegin(Compiler,
                                    CodeGenerator,
                                    gcvTRUE,
                                    &selectionContextNanY);
    if (gcmIS_ERROR(status)) return status;

    /* Y == Nan */
    status = clGenSelectionCompareConditionCode(Compiler,
                                                CodeGenerator,
                                                &selectionContextNanY,
                                                PolynaryExpr->exprBase.base.lineNo,
                                                PolynaryExpr->exprBase.base.stringNo,
                                                clvCONDITION_GREATER_THAN,
                                                &intermROperands[1],
                                                &infROperand);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionTrueOperandBegin(Compiler,
                                            CodeGenerator,
                                            &selectionContextNanY);

    if (gcmIS_ERROR(status)) return status;

    /*Y == Nan, return to x */
    status = clGenGenericCode1(Compiler,
                PolynaryExpr->exprBase.base.lineNo,
                PolynaryExpr->exprBase.base.stringNo,
                clvOPCODE_ASSIGN,
                IOperand,
                &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionTrueOperandEnd(Compiler,
                                    CodeGenerator,
                                    &selectionContextNanY,
                                    gcvFALSE);

    if (gcmIS_ERROR(status)) return status;



    status = clDefineSelectionFalseOperandBegin(Compiler,
                                            CodeGenerator,
                                            &selectionContextNanY);
    if (gcmIS_ERROR(status)) return status;

    /*Normal case, x != Nan, y != Nan*/
    status = clGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_MAX,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) return status;


    status = clDefineSelectionFalseOperandEnd(Compiler,
                                    CodeGenerator,
                                    &selectionContextNanY);

    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionEnd(Compiler,
                                CodeGenerator,
                                &selectionContextNanY);
    if (gcmIS_ERROR(status)) return status;
    }
    status = clDefineSelectionFalseOperandEnd(Compiler,
                                    CodeGenerator,
                                    &selectionContextNanX);

    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionEnd(Compiler,
                                      CodeGenerator,
                                      &selectionContextNanX);
    return status;
   }
   else{ /*Integer cases */
        return clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_MAX,
                                 IOperand,
                                 &OperandsParameters[0].rOperands[0],
                                 &OperandsParameters[1].rOperands[0]);
   }
}

static gceSTATUS
_GenMaxCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
   /* Verify the arguments. */
   clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
   clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
   gcmASSERT(OperandCount == 2);
   gcmASSERT(OperandsParameters);
   gcmASSERT(IOperand);

   if(OperandsParameters[0].rOperands[0].dataType.elementType == clvTYPE_FLOAT &&
      !(CodeGenerator->fpConfig & cldFpFINITE_MATH_ONLY)) {
      if(IOperand &&
         clmGEN_CODE_IsVectorDataType(OperandsParameters[0].dataTypes[0].def)) { /*Floating point vector case with no finite math */
         /* special handling for vector case */
         return _GenBuiltinVectorCode(Compiler,
                                      CodeGenerator,
                                      PolynaryExpr,
                                      OperandCount,
                                      OperandsParameters,
                                      IOperand,
                                      _GenScalarMaxCode);
      }
      else {
         return _GenScalarMaxCode(Compiler,
                                  CodeGenerator,
                                  PolynaryExpr,
                                  OperandCount,
                                  OperandsParameters,
                                  IOperand);
      }
   }
   else { /* integer or floating case without INF or NAN consideration */
      return clGenGenericCode2(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               clvOPCODE_MAX,
                               IOperand,
                               &OperandsParameters[0].rOperands[0],
                               &OperandsParameters[1].rOperands[0]);
   }
}

static gceSTATUS
_GenClampCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS        status;
    clsIOPERAND        intermIOperand;
    clsROPERAND        intermROperand;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    if (cloCOMPILER_OptimizationEnabled(Compiler, clvOPTIMIZATION_CALCULATION)
        && clsROPERAND_IsFloatOrVecConstant(&OperandsParameters[1].rOperands[0], 0.0)
        && clsROPERAND_IsFloatOrVecConstant(&OperandsParameters[2].rOperands[0], 1.0))
    {
        status = clGenGenericCode1(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_SATURATE,
                                IOperand,
                                &OperandsParameters[0].rOperands[0]);

        if (gcmIS_ERROR(status)) return status;
    }
    else
    {
        /* max t0, opd0, opd1 */
        clsIOPERAND_New(Compiler, &intermIOperand, IOperand->dataType);

        status = clGenGenericCode2(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_MAX,
                                &intermIOperand,
                                &OperandsParameters[0].rOperands[0],
                                &OperandsParameters[1].rOperands[0]);

        if (gcmIS_ERROR(status)) return status;

        /* min result, t0, opd2 */
        clsROPERAND_InitializeUsingIOperand(&intermROperand, &intermIOperand);

        status = clGenGenericCode2(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_MIN,
                                IOperand,
                                &intermROperand,
                                &OperandsParameters[2].rOperands[0]);

        if (gcmIS_ERROR(status)) return status;
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenFDimCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;
    clsSELECTION_CONTEXT   selectionContextNanXY, selectionContextXOverY;
    clsIOPERAND intermIOperands[2];
    clsROPERAND intermROperands[2];
    clsROPERAND infROperand, nanROperand, unsignROperand, zeroROperand, fMaxROperand;
    int i;
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);


    clsROPERAND_InitializeIntOrIVecConstant(&infROperand,
                                            clmGenCodeDataType(T_UINT),
                                            0x7f800000);
    clsROPERAND_InitializeIntOrIVecConstant(&fMaxROperand,
                                            clmGenCodeDataType(T_UINT),
                                            0x7f7fffff);
    clsROPERAND_InitializeIntOrIVecConstant(&nanROperand,
                                            clmGenCodeDataType(T_UINT),
                                            0x7fc00000);
    clsROPERAND_InitializeIntOrIVecConstant(&unsignROperand,
                                        clmGenCodeDataType(T_UINT),
                                        0x7fffffff);
    clsROPERAND_InitializeIntOrIVecConstant(&zeroROperand,
                                        clmGenCodeDataType(T_UINT),
                                        0x0);


    for(i = 0; i<2; i++){
        clsIOPERAND_New(Compiler, &intermIOperands[i], clmGenCodeDataType(T_UINT));
        clsROPERAND_InitializeUsingIOperand(&intermROperands[i], &intermIOperands[i]);
        status = clGenBitwiseExprCode(Compiler,
                PolynaryExpr->exprBase.base.lineNo,
                PolynaryExpr->exprBase.base.stringNo,
                clvOPCODE_BITWISE_AND,
                &intermIOperands[i],
                &unsignROperand,
                &OperandsParameters[i].rOperands[0]);
    }

    status = clGenGenericCode2(Compiler,
                    PolynaryExpr->exprBase.base.lineNo,
                    PolynaryExpr->exprBase.base.stringNo,
                    clvOPCODE_MAX,
                    &intermIOperands[0],
                    &intermROperands[0],
                    &intermROperands[1]);

    status = clDefineSelectionBegin(Compiler,
                                    CodeGenerator,
                                    gcvTRUE,
                                    &selectionContextNanXY);
    if (gcmIS_ERROR(status)) return status;

    intermROperands[0].dataType.elementType = clvTYPE_UINT;
    /*|x| + |Y| > inf, x == nan */
    status = clGenSelectionCompareConditionCode(Compiler,
                                                CodeGenerator,
                                                &selectionContextNanXY,
                                                PolynaryExpr->exprBase.base.lineNo,
                                                PolynaryExpr->exprBase.base.stringNo,
                                                clvCONDITION_GREATER_THAN,
                                                &intermROperands[0],
                                                &infROperand);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionTrueOperandBegin(Compiler,
                                            CodeGenerator,
                                            &selectionContextNanXY);

    if (gcmIS_ERROR(status)) return status;

    /*max(|x|, |y|)  == Nan, return to nan */
    status = clGenGenericCode1(Compiler,
                PolynaryExpr->exprBase.base.lineNo,
                PolynaryExpr->exprBase.base.stringNo,
                clvOPCODE_ASSIGN,
                IOperand,
                &nanROperand);

    if (gcmIS_ERROR(status)) return status;



    status = clDefineSelectionTrueOperandEnd(Compiler,
                                    CodeGenerator,
                                    &selectionContextNanXY,
                                    gcvFALSE);

    if (gcmIS_ERROR(status)) return status;

    /* |x| |y| != Nan  */

    status = clDefineSelectionFalseOperandBegin(Compiler,
                                            CodeGenerator,
                                            &selectionContextNanXY);
    if (gcmIS_ERROR(status)) return status;

    {
    status = clDefineSelectionBegin(Compiler,
                                    CodeGenerator,
                                    gcvTRUE,
                                    &selectionContextXOverY);
    if (gcmIS_ERROR(status)) return status;

    status = clGenSelectionCompareConditionCode(Compiler,
                                                CodeGenerator,
                                                &selectionContextXOverY,
                                                PolynaryExpr->exprBase.base.lineNo,
                                                PolynaryExpr->exprBase.base.stringNo,
                                                clvCONDITION_GREATER_THAN,
                                                 &OperandsParameters[0].rOperands[0],
                                                 &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionTrueOperandBegin(Compiler,
                                            CodeGenerator,
                                            &selectionContextXOverY);

    if (gcmIS_ERROR(status)) return status;

    /*x > y */

    status = clGenArithmeticExprCode(Compiler,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo,
                     clvOPCODE_SUB,
                     IOperand,
                     &OperandsParameters[0].rOperands[0],
                     &OperandsParameters[1].rOperands[0]);

    if(CodeGenerator->supportRTNE == 0){
        /*In rtz mode, finite+finite = finite, never goes to Inf */
        /*To avoid max(|x|,|y|) < inf but x-y == inf */
        clsROPERAND_InitializeUsingIOperand(&intermROperands[1], IOperand);
        intermIOperands[0].dataType.elementType = clvTYPE_FLOAT;

        /* r0 = (x-y) - max(|x|, |y|), if it is nan, we have inf-inf, do nothing */
        status = clGenArithmeticExprCode(Compiler,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo,
                         clvOPCODE_SUB,
                         &intermIOperands[0],
                         &intermROperands[1],
                         &intermROperands[0]);

        intermROperands[0].dataType.elementType = clvTYPE_UINT;

        clmGEN_CODE_IF(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo,
                         &intermROperands[0],
                         clvCONDITION_EQUAL,
                         &infROperand);
        /*(x-y) - max(|x|, |y|) == inf, this means (x-y) == inf but |x| and |y| are finite */
        /* return to the max_finite_float value*/
        status = clGenGenericCode1(Compiler,
                    PolynaryExpr->exprBase.base.lineNo,
                    PolynaryExpr->exprBase.base.stringNo,
                    clvOPCODE_ASSIGN,
                    IOperand,
                    &fMaxROperand);


        if (gcmIS_ERROR(status)) return status;

        clmGEN_CODE_ELSE(Compiler,
                               CodeGenerator,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo);

        /* do nothing, since we already have x - y */
        clmGEN_CODE_ENDIF(Compiler,
                             CodeGenerator,
                             PolynaryExpr->exprBase.base.lineNo,
                             PolynaryExpr->exprBase.base.stringNo);
    }

    status = clDefineSelectionTrueOperandEnd(Compiler,
                                    CodeGenerator,
                                    &selectionContextXOverY,
                                    gcvFALSE);

    if (gcmIS_ERROR(status)) return status;



    status = clDefineSelectionFalseOperandBegin(Compiler,
                                            CodeGenerator,
                                            &selectionContextXOverY);
    if (gcmIS_ERROR(status)) return status;

    /* x <= y, return zero*/
    status = clGenGenericCode1(Compiler,
                PolynaryExpr->exprBase.base.lineNo,
                PolynaryExpr->exprBase.base.stringNo,
                clvOPCODE_ASSIGN,
                IOperand,
                &zeroROperand);

    if (gcmIS_ERROR(status)) return status;


    status = clDefineSelectionFalseOperandEnd(Compiler,
                                    CodeGenerator,
                                    &selectionContextXOverY);

    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionEnd(Compiler,
                                CodeGenerator,
                                &selectionContextXOverY);
    if (gcmIS_ERROR(status)) return status;
    }
    status = clDefineSelectionFalseOperandEnd(Compiler,
                                    CodeGenerator,
                                    &selectionContextNanXY);

    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionEnd(Compiler,
                                CodeGenerator,
                                &selectionContextNanXY);
    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;

OnError:
   return status;

}

static gceSTATUS
_GenMixCode(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN cloIR_POLYNARY_EXPR PolynaryExpr,
IN gctUINT OperandCount,
IN clsGEN_CODE_PARAMETERS * OperandsParameters,
IN clsIOPERAND * IOperand
)
{
    gceSTATUS    status;
    clsIOPERAND    intermIOperands[2];
    clsROPERAND    intermROperands[2];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    /* sub t0, y, x */
    clsIOPERAND_New(Compiler, &intermIOperands[0], OperandsParameters[0].dataTypes[0].def);

    status = clGenArithmeticExprCode(Compiler,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo,
                     clvOPCODE_SUB,
                     &intermIOperands[0],
                     &OperandsParameters[1].rOperands[0],
                     &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) return status;

    /* mul t1, a, t0 */
    clsIOPERAND_New(Compiler, &intermIOperands[1], OperandsParameters[0].dataTypes[0].def);
    clsROPERAND_InitializeUsingIOperand(&intermROperands[0], &intermIOperands[0]);

    status = clGenArithmeticExprCode(Compiler,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo,
                     clvOPCODE_MUL,
                     &intermIOperands[1],
                     &OperandsParameters[2].rOperands[0],
                     &intermROperands[0]);

    if (gcmIS_ERROR(status)) return status;

    /* add result, t1, x */
    clsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);

    status = clGenArithmeticExprCode(Compiler,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo,
                     clvOPCODE_ADD,
                     IOperand,
                     &OperandsParameters[0].rOperands[0],
                     &intermROperands[1]);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenStepCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = clGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_STEP,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenSmoothStepCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS        status;
    clsIOPERAND        intermIOperands[7];
    clsROPERAND        intermROperands[7];
    clsROPERAND        constantROperand;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    /* sub t0, x (operand2), edge0 (operand0) */
    clsIOPERAND_New(Compiler, &intermIOperands[0], OperandsParameters[2].dataTypes[0].def);

    status = clGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_SUB,
                                    &intermIOperands[0],
                                    &OperandsParameters[2].rOperands[0],
                                    &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) return status;

    /* sub t1, edge1 (operand1), edge0 (operand0) */
    clsIOPERAND_New(Compiler, &intermIOperands[1], OperandsParameters[1].dataTypes[0].def);

    status = clGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_SUB,
                                    &intermIOperands[1],
                                    &OperandsParameters[1].rOperands[0],
                                    &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) return status;

    /* div t2, t0, t1 */
    clsIOPERAND_New(Compiler, &intermIOperands[2], intermIOperands[0].dataType);
    clsROPERAND_InitializeUsingIOperand(&intermROperands[0], &intermIOperands[0]);
    clsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);

    status = clGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_DIV,
                                    &intermIOperands[2],
                                    &intermROperands[0],
                                    &intermROperands[1]);

    if (gcmIS_ERROR(status)) return status;

    /* sat t3, t2 */
    clsIOPERAND_New(Compiler, &intermIOperands[3], intermIOperands[2].dataType);
    clsROPERAND_InitializeUsingIOperand(&intermROperands[2], &intermIOperands[2]);

    status = clGenGenericCode1(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_SATURATE,
                            &intermIOperands[3],
                            &intermROperands[2]);

    if (gcmIS_ERROR(status)) return status;

    /* mul t4, t3, t3 */
    clsIOPERAND_New(Compiler, &intermIOperands[4], intermIOperands[3].dataType);
    clsROPERAND_InitializeUsingIOperand(&intermROperands[3], &intermIOperands[3]);

    status = clGenArithmeticExprCode(Compiler,
                    PolynaryExpr->exprBase.base.lineNo,
                    PolynaryExpr->exprBase.base.stringNo,
                    clvOPCODE_MUL,
                    &intermIOperands[4],
                    &intermROperands[3],
                    &intermROperands[3]);

    if (gcmIS_ERROR(status)) return status;

    /* add t5, t3, t3    ==> mul t5, t3, 2 */
    clsIOPERAND_New(Compiler, &intermIOperands[5], intermIOperands[3].dataType);

    status = clGenArithmeticExprCode(Compiler,
                    PolynaryExpr->exprBase.base.lineNo,
                    PolynaryExpr->exprBase.base.stringNo,
                    clvOPCODE_ADD,
                    &intermIOperands[5],
                    &intermROperands[3],
                    &intermROperands[3]);
    if (gcmIS_ERROR(status)) return status;

    /* sub t6, 3.0, t5 */
    clsIOPERAND_New(Compiler, &intermIOperands[6], intermIOperands[5].dataType);
    clsROPERAND_InitializeFloatOrVecOrMatConstant(&constantROperand,
                              clmGenCodeDataType(T_FLOAT),
                              (gctFLOAT)3.0);

    clsROPERAND_InitializeUsingIOperand(&intermROperands[5], &intermIOperands[5]);

    status = clGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_SUB,
                                    &intermIOperands[6],
                                    &constantROperand,
                                    &intermROperands[5]);

    if (gcmIS_ERROR(status)) return status;

    /* mul result, t4, t6 */
    clsROPERAND_InitializeUsingIOperand(&intermROperands[4], &intermIOperands[4]);
    clsROPERAND_InitializeUsingIOperand(&intermROperands[6], &intermIOperands[6]);

    status = clGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_MUL,
                                    IOperand,
                                    &intermROperands[4],
                                    &intermROperands[6]);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenSignCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = clGenGenericCode1(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_SIGN,
                            IOperand,
                            &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenLengthCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS        status;
    clsIOPERAND        intermIOperands[2];
    clsROPERAND        intermROperands[2];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    clsIOPERAND_New(Compiler, &intermIOperands[0], OperandsParameters[0].dataTypes[0].def);
    clsROPERAND_InitializeUsingIOperand(&intermROperands[0], &intermIOperands[0]);


    if(clmGEN_CODE_IsScalarDataType(OperandsParameters[0].dataTypes[0].def)) /*Scalar case */
    {
        /* abs result, x */
        status = clGenGenericCode1(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_ABS,
                                IOperand,
                                &OperandsParameters[0].rOperands[0]);

        if (gcmIS_ERROR(status)) return status;
    }
    else
    {
        clsIOPERAND        expIOperands[2];
        clsROPERAND        expROperands[2], maskROperand, tempROperand, toRcpROperand, minROperand;
        gctUINT8 i, vSize;
        vSize =    clmGEN_CODE_vectorSize_GET(OperandsParameters[0].dataTypes[0].def);

        clsROPERAND_InitializeIntOrIVecConstant(&maskROperand,
                            clmGenCodeDataType(T_INT),
                            (gctINT) 0x7f800000);

        clsROPERAND_InitializeIntOrIVecConstant(&toRcpROperand,
                            clmGenCodeDataType(T_INT),
                            (gctINT) 0x7f000000);

        clsROPERAND_InitializeIntOrIVecConstant(&minROperand,
                            clmGenCodeDataType(T_INT),
                            (gctINT) 0x00800000);
        for(i = 0; i<2; i++){
            clsIOPERAND_New(Compiler, &expIOperands[i], clmGenCodeDataType(T_INT));
            clsROPERAND_InitializeUsingIOperand(&expROperands[i], &expIOperands[i]);
        }
        for(i = 0; i<vSize; i++){
            clmROPERAND_vectorComponent_GET(&tempROperand, &OperandsParameters[0].rOperands[0], i);

            if(i == 0){
                status = clGenArithmeticExprCode(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_BITWISE_AND,
                                &expIOperands[0],
                                &maskROperand,
                                &tempROperand);
            }
            else{
                status = clGenArithmeticExprCode(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_BITWISE_AND,
                                &expIOperands[1],
                                &maskROperand,
                                &tempROperand);
                status = clGenGenericCode2(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_MAX,
                                &expIOperands[0],
                                &expROperands[0],
                                &expROperands[1]);
            }
        }

        status = clGenArithmeticExprCode(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_SUB,
                        &expIOperands[1],
                        &toRcpROperand,
                        &expROperands[0]);

        /*To scale the p0 */
        status = clGenGenericCode2(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_MAX,
                        &expIOperands[0],
                        &expROperands[1],
                        &minROperand);

        /* scale back the distance*/
        status = clGenArithmeticExprCode(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_SUB,
                        &expIOperands[1],
                        &toRcpROperand,
                        &expROperands[0]);

        expROperands[0].dataType.elementType = clvTYPE_FLOAT;
        expROperands[1].dataType.elementType = clvTYPE_FLOAT;

        status = clGenArithmeticExprCode(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_MUL,
                        &intermIOperands[0],
                        &OperandsParameters[0].rOperands[0],
                        &expROperands[0]);


        /* dot t1, t0, t0 */
        clsIOPERAND_New(Compiler, &intermIOperands[1], clmGenCodeDataType(T_FLOAT));

        status = clGenGenericCode2(Compiler,
                    PolynaryExpr->exprBase.base.lineNo,
                    PolynaryExpr->exprBase.base.stringNo,
                    clvOPCODE_DOT,
                    &intermIOperands[1],
                    &intermROperands[0],
                    &intermROperands[0]);

        if (gcmIS_ERROR(status)) return status;

        /* sqrt result, t1 */
        clsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);

        status = clGenGenericCode1(Compiler,
                    PolynaryExpr->exprBase.base.lineNo,
                    PolynaryExpr->exprBase.base.stringNo,
                    clvOPCODE_SQRT,
                    &intermIOperands[1],
                    &intermROperands[1]);

        status = clGenArithmeticExprCode(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_MUL,
                        IOperand,
                        &intermROperands[1],
                        &expROperands[1]);

        if (gcmIS_ERROR(status)) return status;
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenFastLengthCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS       status;
    clsIOPERAND     intermIOperand;
    clsROPERAND     intermROperand;

    gcmHEADER();

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    if(clmGEN_CODE_IsScalarDataType(OperandsParameters[0].dataTypes[0].def)) /*Scalar case */
    {
        /* abs result, x */
        status = clGenGenericCode1(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   clvOPCODE_ABS,
                                   IOperand,
                                   &OperandsParameters[0].rOperands[0]);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }
    else
    {
        /* dot t0, x, x */
        clsIOPERAND_New(Compiler, &intermIOperand, clmGenCodeDataType(T_FLOAT));

        status = clGenGenericCode2(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   clvOPCODE_DOT,
                                   &intermIOperand,
                                   &OperandsParameters[0].rOperands[0],
                                   &OperandsParameters[0].rOperands[0]);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

        /* sqrt result, t0 */
        clsROPERAND_InitializeUsingIOperand(&intermROperand, &intermIOperand);

        status = clGenGenericCode1(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   clvOPCODE_SQRT,
                                   IOperand,
                                   &intermROperand);

        if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_GenDistanceCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS        status;
    clsIOPERAND        intermIOperands[2];
    clsROPERAND        intermROperands[2];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    /* sub t0, p0, p1 */
    clsIOPERAND_New(Compiler, &intermIOperands[0], OperandsParameters[0].dataTypes[0].def);
    clsROPERAND_InitializeUsingIOperand(&intermROperands[0], &intermIOperands[0]);

    status = clGenArithmeticExprCode(Compiler,
                    PolynaryExpr->exprBase.base.lineNo,
                    PolynaryExpr->exprBase.base.stringNo,
                    clvOPCODE_SUB,
                    &intermIOperands[0],
                    &OperandsParameters[0].rOperands[0],
                    &OperandsParameters[1].rOperands[0]);
    if (gcmIS_ERROR(status)) return status;


    intermROperands[1] = OperandsParameters[0].rOperands[0];
    OperandsParameters[0].rOperands[0] = intermROperands[0];

    status = _GenLengthCode(Compiler,
                                CodeGenerator,
                                PolynaryExpr,
                                1,
                                OperandsParameters,
                                IOperand);
    if (gcmIS_ERROR(status)) return status;

    OperandsParameters[0].rOperands[0] = intermROperands[1];

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenFastDistanceCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS        status;
    clsIOPERAND        intermIOperands[2];
    clsROPERAND        intermROperands[2];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    /* sub t0, p0, p1 */
    clsIOPERAND_New(Compiler, &intermIOperands[0], OperandsParameters[0].dataTypes[0].def);
    clsROPERAND_InitializeUsingIOperand(&intermROperands[0], &intermIOperands[0]);

    status = clGenArithmeticExprCode(Compiler,
                    PolynaryExpr->exprBase.base.lineNo,
                    PolynaryExpr->exprBase.base.stringNo,
                    clvOPCODE_SUB,
                    &intermIOperands[0],
                    &OperandsParameters[0].rOperands[0],
                    &OperandsParameters[1].rOperands[0]);
    if (gcmIS_ERROR(status)) return status;


    intermROperands[1] = OperandsParameters[0].rOperands[0];
    OperandsParameters[0].rOperands[0] = intermROperands[0];

    status = _GenFastLengthCode(Compiler,
                                    CodeGenerator,
                                    PolynaryExpr,
                                    1,
                                    OperandsParameters,
                                    IOperand);
    if (gcmIS_ERROR(status)) return status;

    OperandsParameters[0].rOperands[0] = intermROperands[1];

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenDotCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = clGenDotCode(Compiler,
                  PolynaryExpr->exprBase.base.lineNo,
                  PolynaryExpr->exprBase.base.stringNo,
                  IOperand,
                  &OperandsParameters[0].rOperands[0],
                  &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenCrossCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = clGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_CROSS,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenNormalizeCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;
    gctUINT8 vectorSize = clmGEN_CODE_vectorSize_GET(OperandsParameters[0].dataTypes[0].def), i;
    clsROPERAND    intermROperands[4+1], tempROperand, expROperand, floatOneROperand, zeroROperand, unsignROperand, signROperand, denormROperand, neg1ROperand, negInfROperand;
    clsIOPERAND intermIOperands[4+1];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    clsROPERAND_InitializeIntOrIVecConstant(&expROperand,
                      clmGenCodeDataType(T_UINT),
                      (gctINT) 0x7F800000);
    clsROPERAND_InitializeIntOrIVecConstant(&negInfROperand,
                      clmGenCodeDataType(T_UINT),
                      (gctINT) 0xfF800000);

    clsROPERAND_InitializeIntOrIVecConstant(&floatOneROperand,
                      clmGenCodeDataType(T_UINT),
                      (gctINT) 0x3F800000);
   clsROPERAND_InitializeIntOrIVecConstant(&neg1ROperand,
                      clmGenCodeDataType(T_UINT),
                      (gctINT) 0xBF800000);
    clsROPERAND_InitializeIntOrIVecConstant(&zeroROperand,
                      clmGenCodeDataType(T_UINT),
                      (gctINT) 0x00000000);
    clsROPERAND_InitializeIntOrIVecConstant(&unsignROperand,
                              clmGenCodeDataType(T_UINT),
                              (gctUINT) 0x7FFFFFFF);

    clsROPERAND_InitializeIntOrIVecConstant(&signROperand,
                              clmGenCodeDataType(T_UINT),
                              (gctUINT) 0x80000000);

    clsROPERAND_InitializeIntOrIVecConstant(&denormROperand,
                              clmGenCodeDataType(T_UINT),
                              (gctUINT) 0x007fffff);


    if(clmGEN_CODE_IsScalarDataType(OperandsParameters[0].dataTypes[0].def)){ /*Scalar case */
       clsIOPERAND_New(Compiler, &intermIOperands[0], clmGenCodeDataType(T_UINT));
       clsROPERAND_InitializeUsingIOperand(&intermROperands[0], &intermIOperands[0]);

        status = clGenArithmeticExprCode(Compiler,
                    PolynaryExpr->exprBase.base.lineNo,
                    PolynaryExpr->exprBase.base.stringNo,
                    clvOPCODE_BITWISE_AND,
                    &intermIOperands[0],
                    &unsignROperand,
                    &OperandsParameters[0].rOperands[0]);

        clmGEN_CODE_IF(Compiler, /*IF_N */
             CodeGenerator,
             PolynaryExpr->exprBase.base.lineNo,
             PolynaryExpr->exprBase.base.stringNo,
             &intermROperands[0],
             clvCONDITION_GREATER_THAN,
             &expROperand);

        /* Input component gets Nan involve, return to Nan */
        status = clGenGenericCode1(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_ASSIGN,
                                IOperand,
                                &OperandsParameters[0].rOperands[0]);

        if (gcmIS_ERROR(status)) return status;

        clmGEN_CODE_ELSE(Compiler, /*IF_N */
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo);
        /*Normal case, return +/-1.0 */

        clmGEN_CODE_IF(Compiler, /*IF_D */
             CodeGenerator,
             PolynaryExpr->exprBase.base.lineNo,
             PolynaryExpr->exprBase.base.stringNo,
             &intermROperands[0],
             clvCONDITION_LESS_THAN_EQUAL,
             &denormROperand);

        /* Input component Denormal, return zero */
        status = clGenGenericCode1(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_ASSIGN,
                                IOperand,
                                &zeroROperand);

        if (gcmIS_ERROR(status)) return status;

        clmGEN_CODE_ELSE(Compiler, /*IF_D */
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo);
        /*Normal case, return +/-1.0 */

        status = clGenArithmeticExprCode(Compiler,
                    PolynaryExpr->exprBase.base.lineNo,
                    PolynaryExpr->exprBase.base.stringNo,
                    clvOPCODE_BITWISE_AND,
                    &intermIOperands[0],
                    &signROperand,
                    &OperandsParameters[0].rOperands[0]);

        {
            cltELEMENT_TYPE copyTypeI = IOperand->dataType.elementType;
            IOperand->dataType.elementType = clvTYPE_INT;

            status = clGenArithmeticExprCode(Compiler,
                    PolynaryExpr->exprBase.base.lineNo,
                    PolynaryExpr->exprBase.base.stringNo,
                    clvOPCODE_BITWISE_OR,
                    IOperand,
                    &intermROperands[0],
                    &floatOneROperand);
            IOperand->dataType.elementType = copyTypeI;

        }


        clmGEN_CODE_ENDIF(Compiler, /*IF_D */
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo);


        clmGEN_CODE_ENDIF(Compiler, /*IF_N */
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo);


        if (gcmIS_ERROR(status)) return status;
    }
    else{
        /*Vector case, in order to avoid under/overflow, we should divide the maxValue */
        for(i = 0; i < 3; i++){
            clsIOPERAND_New(Compiler, &intermIOperands[i], OperandsParameters[0].dataTypes[0].def);
            intermIOperands[i].dataType.elementType = clvTYPE_UINT;
            clsROPERAND_InitializeUsingIOperand(&intermROperands[i], &intermIOperands[i]);
        }
        clsIOPERAND_New(Compiler, &intermIOperands[i], clmGenCodeDataType(T_UINT));
        clsROPERAND_InitializeUsingIOperand(&intermROperands[i], &intermIOperands[i]);

        /*Get out the sign*/
        status = clGenArithmeticExprCode(Compiler,
                    PolynaryExpr->exprBase.base.lineNo,
                    PolynaryExpr->exprBase.base.stringNo,
                    clvOPCODE_BITWISE_AND,
                    &intermIOperands[0],
                    &unsignROperand,
                    &OperandsParameters[0].rOperands[0]);

        for(i = 0; i<vectorSize; i++){ /*Get maximum of (|x|, |y|, |z|, |w|) */
            clmROPERAND_vectorComponent_GET(&tempROperand, &intermROperands[0], i);
            if(i == 0){
                status = clGenGenericCode1(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_ASSIGN,
                                &intermIOperands[3],
                                &tempROperand);
            }
            else
                status = clGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_MAX,
                            &intermIOperands[3],
                            &intermROperands[3],
                            &tempROperand);

        }

        clmGEN_CODE_IF(Compiler, /*IF_0 */
             CodeGenerator,
             PolynaryExpr->exprBase.base.lineNo,
             PolynaryExpr->exprBase.base.stringNo,
             &intermROperands[3],
             clvCONDITION_LESS_THAN_EQUAL,
             &denormROperand);

        /* Input denormal output zero */
        status = clGenGenericCode1(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_ASSIGN,
                                IOperand,
                                &zeroROperand);

        if (gcmIS_ERROR(status)) return status;

        clmGEN_CODE_ELSE(Compiler,  /*IF_0 */
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo);

        /*Unzero input */

        clmGEN_CODE_IF(Compiler, /*IF_1 */
             CodeGenerator,
             PolynaryExpr->exprBase.base.lineNo,
             PolynaryExpr->exprBase.base.stringNo,
             &intermROperands[3],
             clvCONDITION_GREATER_THAN,
             &expROperand);

        /* Input component gets Nan involve, return Nan */
        status = clGenGenericCode1(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_ASSIGN,
                                IOperand,
                                &unsignROperand);

        if (gcmIS_ERROR(status)) return status;

        clmGEN_CODE_ELSE(Compiler, /*IF_1 */
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo);


        clmGEN_CODE_IF(Compiler, /*IF_2 */
             CodeGenerator,
             PolynaryExpr->exprBase.base.lineNo,
             PolynaryExpr->exprBase.base.stringNo,
             &intermROperands[3],
             clvCONDITION_EQUAL,
             &expROperand);

        /* Input component gets Inf involve, inf->1.0, -Inf->-1.0, else->0.0 */
        status = clGenGenericCode2(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_EQUAL,
                                &intermIOperands[1],
                                &expROperand,
                                &OperandsParameters[0].rOperands[0]);

        status = clGenArithmeticExprCode(Compiler,
                    PolynaryExpr->exprBase.base.lineNo,
                    PolynaryExpr->exprBase.base.stringNo,
                    clvOPCODE_BITWISE_AND,
                    &intermIOperands[1],
                    &floatOneROperand,
                    &intermROperands[1]);
        if (gcmIS_ERROR(status)) return status;

        status = clGenGenericCode2(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_EQUAL,
                                &intermIOperands[2],
                                &negInfROperand,
                                &OperandsParameters[0].rOperands[0]);

        status = clGenArithmeticExprCode(Compiler,
                    PolynaryExpr->exprBase.base.lineNo,
                    PolynaryExpr->exprBase.base.stringNo,
                    clvOPCODE_BITWISE_AND,
                    &intermIOperands[2],
                    &neg1ROperand,
                    &intermROperands[2]);

        /*Now, r1 = 1, 0, -1, depends component = Inf, Finite, -Inf */
        status = clGenArithmeticExprCode(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_ADD,
                                &intermIOperands[1],
                                &intermROperands[1],
                                &intermROperands[2]);

        clmGEN_CODE_ELSE(Compiler, /*IF_2 */
                       CodeGenerator,
                       PolynaryExpr->exprBase.base.lineNo,
                       PolynaryExpr->exprBase.base.stringNo);

        /*Normal case, get maximum Exp part, then 1/MaxExp to scale the vector */

        status = clGenArithmeticExprCode(Compiler,
                    PolynaryExpr->exprBase.base.lineNo,
                    PolynaryExpr->exprBase.base.stringNo,
                    clvOPCODE_BITWISE_AND,
                    &intermIOperands[3],
                    &expROperand,
                    &intermROperands[3]);

        status = clGenArithmeticExprCode(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_SUB,
                                &intermIOperands[3],
                                &expROperand,
                                &intermROperands[3]);


        /*scale the vector, at range of 0~3.999*/
        intermIOperands[1].dataType.elementType = clvTYPE_FLOAT;
        status = clGenArithmeticExprCode(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_MUL,
                                &intermIOperands[1],
                                &OperandsParameters[0].rOperands[0],
                                &intermROperands[3]);


        clmGEN_CODE_ENDIF(Compiler, /*IF_2 */
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo);

        intermROperands[1].dataType.elementType = clvTYPE_FLOAT;
        status = clGenGenericCode1(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_NORMALIZE,
                            IOperand,
                            &intermROperands[1]);
        if (gcmIS_ERROR(status)) return status;

        clmGEN_CODE_ENDIF(Compiler, /*IF_1 */
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo);


        clmGEN_CODE_ENDIF(Compiler, /*IF_0 */
                     CodeGenerator,
                     PolynaryExpr->exprBase.base.lineNo,
                     PolynaryExpr->exprBase.base.stringNo);
    }

    return gcvSTATUS_OK;
OnError:
    return status;
}

static gceSTATUS
_GenFastNormalizeCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS   status;

    gcmHEADER();

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = clGenGenericCode1(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               clvOPCODE_NORMALIZE,
                               IOperand,
                               &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) { gcmFOOTER(); return status; }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_GenFaceForwardCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;
    clsIOPERAND    intermIOperand;
    clsROPERAND    intermROperand;
    clsLOPERAND    intermLOperand;
    clsSELECTION_CONTEXT    selectionContext;
    clsROPERAND    zeroROperand;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    /* dot t0, Nref (operand2), I (operand1) */
    clsIOPERAND_New(Compiler, &intermIOperand, clmGenCodeDataType(T_FLOAT));

    status = clGenGenericCode2(Compiler,
                PolynaryExpr->exprBase.base.lineNo,
                PolynaryExpr->exprBase.base.stringNo,
                clvOPCODE_DOT,
                &intermIOperand,
                &OperandsParameters[2].rOperands[0],
                &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) return status;

    /* The selection begin*/
    status = clDefineSelectionBegin(Compiler,
                    CodeGenerator,
                    gcvTRUE,
                    &selectionContext);

    if (gcmIS_ERROR(status)) return status;

    /* The condition part: t0 < 0.0 */
    clsROPERAND_InitializeUsingIOperand(&intermROperand, &intermIOperand);
    clsROPERAND_InitializeFloatOrVecOrMatConstant(&zeroROperand,
                              clmGenCodeDataType(T_FLOAT),
                              (gctFLOAT)0.0);

    status = clGenSelectionCompareConditionCode(Compiler,
                        CodeGenerator,
                        &selectionContext,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvCONDITION_LESS_THAN,
                        &intermROperand,
                        &zeroROperand);
    if (gcmIS_ERROR(status)) return status;

    /* The true part */
    status = clDefineSelectionTrueOperandBegin(
                                            Compiler,
                                            CodeGenerator,
                                            &selectionContext);

    if (gcmIS_ERROR(status)) return status;

    /* mov result, N (operand0) */
    clsLOPERAND_InitializeUsingIOperand(&intermLOperand, IOperand);

    status = clGenAssignCode(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            &intermLOperand,
                            &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) return status;


    status = clDefineSelectionTrueOperandEnd(
                                            Compiler,
                                            CodeGenerator,
                                            &selectionContext,
                                            gcvFALSE);

    if (gcmIS_ERROR(status)) return status;

    /* The false part */
    status = clDefineSelectionFalseOperandBegin(
                                            Compiler,
                                            CodeGenerator,
                                            &selectionContext);

    if (gcmIS_ERROR(status)) return status;

    /* sub result, 0.0, N (operand0) */
    status = clGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_SUB,
                                    IOperand,
                                    &zeroROperand,
                                    &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionFalseOperandEnd(
                                            Compiler,
                                            CodeGenerator,
                                            &selectionContext);

    if (gcmIS_ERROR(status)) return status;

    /* The selection end */
    status = clDefineSelectionEnd(
                                Compiler,
                                CodeGenerator,
                                &selectionContext);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenReflectCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS        status;
    clsIOPERAND        intermIOperands[3];
    clsROPERAND        intermROperands[3];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    /* dot t0, N (operand1), I (operand0) */
    clsIOPERAND_New(Compiler, &intermIOperands[0], clmGenCodeDataType(T_FLOAT));

    status = clGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_DOT,
                            &intermIOperands[0],
                            &OperandsParameters[1].rOperands[0],
                            &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) return status;

    /* add t1, t0, t0    ==> mul t1, 2.0, t0 */
    clsIOPERAND_New(Compiler, &intermIOperands[1], clmGenCodeDataType(T_FLOAT));
    clsROPERAND_InitializeUsingIOperand(&intermROperands[0], &intermIOperands[0]);

    status = clGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_ADD,
                                    &intermIOperands[1],
                                    &intermROperands[0],
                                    &intermROperands[0]);

    if (gcmIS_ERROR(status)) return status;

    /* mul t2, t1, N (operand1) */
    clsIOPERAND_New(Compiler, &intermIOperands[2], OperandsParameters[1].dataTypes[0].def);
    clsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);

    status = clGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_MUL,
                                    &intermIOperands[2],
                                    &intermROperands[1],
                                    &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) return status;

    /* sub result, I (operand0), t2 */
    clsROPERAND_InitializeUsingIOperand(&intermROperands[2], &intermIOperands[2]);

    status = clGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_SUB,
                                    IOperand,
                                    &OperandsParameters[0].rOperands[0],
                                    &intermROperands[2]);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenRefractCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS                status;
    clsIOPERAND                intermIOperands[11];
    clsROPERAND                intermROperands[11];
    clsLOPERAND                intermLOperand;
    clsSELECTION_CONTEXT    selectionContext;
    clsROPERAND                oneROperand, zeroROperand;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    clsROPERAND_InitializeFloatOrVecOrMatConstant(&oneROperand,
                              clmGenCodeDataType(T_FLOAT),
                              (gctFLOAT)1.0);

    clsROPERAND_InitializeFloatOrVecOrMatConstant(&zeroROperand,
                              clmGenCodeDataType(T_FLOAT),
                              (gctFLOAT)0.0);

    /* dot t0, N (operand1), I (operand0) */
    clsIOPERAND_New(Compiler, &intermIOperands[0], clmGenCodeDataType(T_FLOAT));

    status = clGenGenericCode2(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_DOT,
                                &intermIOperands[0],
                                &OperandsParameters[1].rOperands[0],
                                &OperandsParameters[0].rOperands[0]);

    if (gcmIS_ERROR(status)) return status;

    /* mul t1, t0, t0 */
    clsIOPERAND_New(Compiler, &intermIOperands[1], clmGenCodeDataType(T_FLOAT));
    clsROPERAND_InitializeUsingIOperand(&intermROperands[0], &intermIOperands[0]);

    status = clGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_MUL,
                                    &intermIOperands[1],
                                    &intermROperands[0],
                                    &intermROperands[0]);

    if (gcmIS_ERROR(status)) return status;

    /* sub t2, 1.0, t1 */
    clsIOPERAND_New(Compiler, &intermIOperands[2], clmGenCodeDataType(T_FLOAT));
    clsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);

    status = clGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_SUB,
                                    &intermIOperands[2],
                                    &oneROperand,
                                    &intermROperands[1]);

    if (gcmIS_ERROR(status)) return status;

    /* mul t3, eta (operand2), eta (operand2) */
    clsIOPERAND_New(Compiler, &intermIOperands[3], clmGenCodeDataType(T_FLOAT));

    status = clGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_MUL,
                                    &intermIOperands[3],
                                    &OperandsParameters[2].rOperands[0],
                                    &OperandsParameters[2].rOperands[0]);

    if (gcmIS_ERROR(status)) return status;

    /* mul t4, t3, t2 */
    clsIOPERAND_New(Compiler, &intermIOperands[4], clmGenCodeDataType(T_FLOAT));
    clsROPERAND_InitializeUsingIOperand(&intermROperands[3], &intermIOperands[3]);
    clsROPERAND_InitializeUsingIOperand(&intermROperands[2], &intermIOperands[2]);

    status = clGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_MUL,
                                    &intermIOperands[4],
                                    &intermROperands[3],
                                    &intermROperands[2]);

    if (gcmIS_ERROR(status)) return status;

    /* sub t5, 1.0, t4 */
    clsIOPERAND_New(Compiler, &intermIOperands[5], clmGenCodeDataType(T_FLOAT));
    clsROPERAND_InitializeUsingIOperand(&intermROperands[4], &intermIOperands[4]);

    status = clGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_SUB,
                                    &intermIOperands[5],
                                    &oneROperand,
                                    &intermROperands[4]);

    if (gcmIS_ERROR(status)) return status;

    /* The selection begin*/
    status = clDefineSelectionBegin(
                                    Compiler,
                                    CodeGenerator,
                                    gcvTRUE,
                                    &selectionContext);

    if (gcmIS_ERROR(status)) return status;

    /* The condition part: t5 < 0.0 */
    clsROPERAND_InitializeUsingIOperand(&intermROperands[5], &intermIOperands[5]);

    status = clGenSelectionCompareConditionCode(Compiler,
                            CodeGenerator,
                            &selectionContext,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvCONDITION_LESS_THAN,
                            &intermROperands[5],
                            &zeroROperand);
    if (gcmIS_ERROR(status)) return status;

    /* The true part */
    status = clDefineSelectionTrueOperandBegin(Compiler,
                           CodeGenerator,
                           &selectionContext);
    if (gcmIS_ERROR(status)) return status;

    /* mov result, 0.0 */
    clsLOPERAND_InitializeUsingIOperand(&intermLOperand, IOperand);
    clsROPERAND_InitializeFloatOrVecOrMatConstant(&zeroROperand,
                              IOperand->dataType,
                              (gctFLOAT)0.0);

    status = clGenAssignCode(Compiler,
                PolynaryExpr->exprBase.base.lineNo,
                PolynaryExpr->exprBase.base.stringNo,
                &intermLOperand,
                &zeroROperand);

    if (gcmIS_ERROR(status)) return status;




    status = clDefineSelectionTrueOperandEnd(Compiler,
                         CodeGenerator,
                         &selectionContext,
                         gcvFALSE);

    if (gcmIS_ERROR(status)) return status;

    /* The false part */
    status = clDefineSelectionFalseOperandBegin(Compiler,
                            CodeGenerator,
                            &selectionContext);
    if (gcmIS_ERROR(status)) return status;

    /* mul t6, eta (operand2), I (operand0) */
    clsIOPERAND_New(Compiler, &intermIOperands[6], OperandsParameters[0].dataTypes[0].def);

    status = clGenArithmeticExprCode(Compiler,
                    PolynaryExpr->exprBase.base.lineNo,
                    PolynaryExpr->exprBase.base.stringNo,
                    clvOPCODE_MUL,
                    &intermIOperands[6],
                    &OperandsParameters[2].rOperands[0],
                    &OperandsParameters[0].rOperands[0]);
    if (gcmIS_ERROR(status)) return status;

    /* mul t7, eta (operand2), t0 */
    clsIOPERAND_New(Compiler, &intermIOperands[7], clmGenCodeDataType(T_FLOAT));
    status = clGenArithmeticExprCode(Compiler,
                    PolynaryExpr->exprBase.base.lineNo,
                    PolynaryExpr->exprBase.base.stringNo,
                    clvOPCODE_MUL,
                    &intermIOperands[7],
                    &OperandsParameters[2].rOperands[0],
                    &intermROperands[0]);
    if (gcmIS_ERROR(status)) return status;

    /* sqrt t8, t5 */
    clsIOPERAND_New(Compiler, &intermIOperands[8], clmGenCodeDataType(T_FLOAT));
    status = clGenGenericCode1(Compiler,
                PolynaryExpr->exprBase.base.lineNo,
                PolynaryExpr->exprBase.base.stringNo,
                clvOPCODE_SQRT,
                &intermIOperands[8],
                &intermROperands[5]);
    if (gcmIS_ERROR(status)) return status;

    /* add t9, t7, t8 */
    clsIOPERAND_New(Compiler, &intermIOperands[9], clmGenCodeDataType(T_FLOAT));
    clsROPERAND_InitializeUsingIOperand(&intermROperands[7], &intermIOperands[7]);
    clsROPERAND_InitializeUsingIOperand(&intermROperands[8], &intermIOperands[8]);

    status = clGenArithmeticExprCode(Compiler,
                    PolynaryExpr->exprBase.base.lineNo,
                    PolynaryExpr->exprBase.base.stringNo,
                    clvOPCODE_ADD,
                    &intermIOperands[9],
                    &intermROperands[7],
                    &intermROperands[8]);
    if (gcmIS_ERROR(status)) return status;

    /* mul t10, t9, N (operand1) */
    clsIOPERAND_New(Compiler, &intermIOperands[10], OperandsParameters[1].dataTypes[0].def);
    clsROPERAND_InitializeUsingIOperand(&intermROperands[9], &intermIOperands[9]);

    status = clGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_MUL,
                                    &intermIOperands[10],
                                    &intermROperands[9],
                                    &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) return status;

    /* sub result, t6, t10 */
    clsROPERAND_InitializeUsingIOperand(&intermROperands[6], &intermIOperands[6]);
    clsROPERAND_InitializeUsingIOperand(&intermROperands[10], &intermIOperands[10]);

    status = clGenArithmeticExprCode(
                                    Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_SUB,
                                    IOperand,
                                    &intermROperands[6],
                                    &intermROperands[10]);

    status = clDefineSelectionFalseOperandEnd(
                                            Compiler,
                                            CodeGenerator,
                                            &selectionContext);

    if (gcmIS_ERROR(status)) return status;

    /* The selection end */
    status = clDefineSelectionEnd(
                                Compiler,
                                CodeGenerator,
                                &selectionContext);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenMatrixCompMultCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS        status;
    gctUINT            i;
    clsIOPERAND        columnIOperand;
    clsROPERAND        columnROperand0, columnROperand1;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    for (i = 0; i < gcGetMatrixDataTypeColumnCount(IOperand->dataType); i++)
    {
        clsIOPERAND_InitializeAsMatrixColumn(
                                            &columnIOperand,
                                            IOperand,
                                            i);

        clsROPERAND_InitializeAsMatrixColumn(
                                            &columnROperand0,
                                            &OperandsParameters[0].rOperands[0],
                                            i);

        clsROPERAND_InitializeAsMatrixColumn(
                                            &columnROperand1,
                                            &OperandsParameters[1].rOperands[0],
                                            i);

        status = clGenArithmeticExprCode(
                                        Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        clvOPCODE_MUL,
                                        &columnIOperand,
                                        &columnROperand0,
                                        &columnROperand1);

        if (gcmIS_ERROR(status)) return status;
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenLessThanCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = clGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_LESS_THAN,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenLessThanEqualCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = clGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_LESS_THAN_EQUAL,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenGreaterThanCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = clGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_GREATER_THAN,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenGreaterThanEqualCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = clGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_GREATER_THAN_EQUAL,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenEqualCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = clGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_EQUAL,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenNotEqualCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

/*    status = clGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_NOT_EQUAL,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);
*/
    {
        clsROPERAND tempROperand, zeroROperand;
       clsROPERAND_InitializeIntOrIVecConstant(&zeroROperand,
                   clmGenCodeDataType(T_INT),
                   (gctUINT)0);
       clsROPERAND_InitializeUsingIOperand(&tempROperand, IOperand);

        status = clGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_EQUAL,
                            IOperand,
                            &OperandsParameters[0].rOperands[0],
                            &OperandsParameters[1].rOperands[0]);

        status = clGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_EQUAL,
                            IOperand,
                            &tempROperand,
                            &zeroROperand);

    }
    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenLessGreaterCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;
    clsIOPERAND                intermIOperands[2];
    clsROPERAND                intermROperands[2];
    int i;
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    for(i = 0; i < 2; i++){
       clsIOPERAND_New(Compiler, &intermIOperands[i], clmGenCodeDataType(T_INT));
       clsROPERAND_InitializeUsingIOperand(&intermROperands[i], &intermIOperands[i]);
       status = clGenGenericCode2(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_LESS_THAN,
                                &intermIOperands[i],
                                &OperandsParameters[i].rOperands[0],
                                &OperandsParameters[1-i].rOperands[0]);
        if (gcmIS_ERROR(status)) return status;
    }

    status = clGenArithmeticExprCode(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_BITWISE_OR,
                        IOperand,
                        &intermROperands[0],
                        &intermROperands[1]);
    return gcvSTATUS_OK;
}

static gceSTATUS
_GenFiniteCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;
    clsIOPERAND intermIOperands[2];
    clsROPERAND    infROperand, unsignROperand, intermROperands[2];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    clsROPERAND_InitializeIntOrIVecConstant(&infROperand,
                              clmGenCodeDataType(T_UINT),
                              (gctUINT) 0x7F800000);
     clsROPERAND_InitializeIntOrIVecConstant(&unsignROperand,
                              clmGenCodeDataType(T_UINT),
                              (gctUINT) 0x7FFFFFFF);

   clsIOPERAND_New(Compiler, &intermIOperands[0], clmGenCodeDataType(T_UINT));
   clsROPERAND_InitializeUsingIOperand(&intermROperands[0], &intermIOperands[0]);

    status = clGenArithmeticExprCode(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_BITWISE_AND,
                        &intermIOperands[0],
                        &unsignROperand,
                        &OperandsParameters[0].rOperands[0]);

    status = clGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_LESS_THAN,
                            IOperand,
                            &intermROperands[0],
                            &infROperand);
    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenIsNormalCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;
    clsIOPERAND intermIOperands[2];
    clsROPERAND    infROperand, unsignROperand, minFloatROperand, intermROperands[2];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    clsROPERAND_InitializeIntOrIVecConstant(&infROperand,
                              clmGenCodeDataType(T_UINT),
                              (gctUINT) 0x7F800000);
     clsROPERAND_InitializeIntOrIVecConstant(&unsignROperand,
                              clmGenCodeDataType(T_UINT),
                              (gctUINT) 0x7FFFFFFF);

     clsROPERAND_InitializeIntOrIVecConstant(&minFloatROperand,
                              clmGenCodeDataType(T_UINT),
                              (gctUINT) 0x00800000);

   clsIOPERAND_New(Compiler, &intermIOperands[0], clmGenCodeDataType(T_UINT));
   clsROPERAND_InitializeUsingIOperand(&intermROperands[0], &intermIOperands[0]);
   clsIOPERAND_New(Compiler, &intermIOperands[1], clmGenCodeDataType(T_UINT));
   clsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);

    status = clGenArithmeticExprCode(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_BITWISE_AND,
                        &intermIOperands[0],
                        &unsignROperand,
                        &OperandsParameters[0].rOperands[0]);

    status = clGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_LESS_THAN,
                            &intermIOperands[1],
                            &intermROperands[0],
                            &infROperand);
    if (gcmIS_ERROR(status)) return status;

    status = clGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_GREATER_THAN_EQUAL,
                            &intermIOperands[0],
                            &intermROperands[0],
                            &minFloatROperand);
    if (gcmIS_ERROR(status)) return status;

    status = clGenArithmeticExprCode(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_BITWISE_AND,
                        IOperand,
                        &intermROperands[0],
                        &intermROperands[1]);


    return gcvSTATUS_OK;
}

static gceSTATUS
_GenOrderedCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;
    clsIOPERAND                intermIOperands[2];
    clsROPERAND                intermROperands[2];
    int i;
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    for(i = 0; i < 2; i++){
       clsIOPERAND_New(Compiler, &intermIOperands[i], clmGenCodeDataType(T_INT));
       clsROPERAND_InitializeUsingIOperand(&intermROperands[i], &intermIOperands[i]);
       status = clGenGenericCode2(
                                Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                clvOPCODE_EQUAL,
                                &intermIOperands[i],
                                &OperandsParameters[i].rOperands[0],
                                &OperandsParameters[i].rOperands[0]);
        if (gcmIS_ERROR(status)) return status;
    }

    status = clGenArithmeticExprCode(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_BITWISE_AND,
                        IOperand,
                        &intermROperands[0],
                        &intermROperands[1]);
    return gcvSTATUS_OK;
}

static gceSTATUS
_GenUnOrderedCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    {
        clsROPERAND tempROperand, zeroROperand;
       clsROPERAND_InitializeIntOrIVecConstant(&zeroROperand,
                   clmGenCodeDataType(T_INT),
                   (gctUINT)0);
       clsROPERAND_InitializeUsingIOperand(&tempROperand, IOperand);
        _GenOrderedCode(
            Compiler,
            CodeGenerator,
            PolynaryExpr,
            OperandCount,
            OperandsParameters,
            IOperand
            );
        status = clGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_EQUAL,
                            IOperand,
                            &tempROperand,
                            &zeroROperand);

    }
    return status;
}

static gceSTATUS
_GenSignBitCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;
    clsROPERAND    rshftROperand;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    clsROPERAND_InitializeIntOrIVecConstant(&rshftROperand,
                              clmGenCodeDataType(T_UINT),
                              31);
       {
          cltELEMENT_TYPE copyTypeI = IOperand->dataType.elementType;
          /*For vector, signbit(-2.0, 1.0) = {-1, 0}, for scalar, signbit(-2.0) = 1 */
          if(clmGEN_CODE_IsVectorDataType(OperandsParameters[0].dataTypes[0].def)){
                IOperand->dataType.elementType = clvTYPE_INT;
          }
          else{
              IOperand->dataType.elementType = clvTYPE_UINT;
          }
           status = clGenShiftExprCode(Compiler,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    clvOPCODE_RSHIFT,
                                    IOperand,
                                    &OperandsParameters[0].rOperands[0],
                                    &rshftROperand);
          IOperand->dataType.elementType = copyTypeI;
      }
    return gcvSTATUS_OK;
}


static gceSTATUS
_GenIsNanCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;
    clsIOPERAND intermIOperands[2];
    clsROPERAND    infROperand, unsignROperand, intermROperands[2];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    clsROPERAND_InitializeIntOrIVecConstant(&infROperand,
                              clmGenCodeDataType(T_UINT),
                              (gctUINT) 0x7F800000);
     clsROPERAND_InitializeIntOrIVecConstant(&unsignROperand,
                              clmGenCodeDataType(T_UINT),
                              (gctUINT) 0x7FFFFFFF);

   clsIOPERAND_New(Compiler, &intermIOperands[0], clmGenCodeDataType(T_UINT));
   clsROPERAND_InitializeUsingIOperand(&intermROperands[0], &intermIOperands[0]);

    status = clGenArithmeticExprCode(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_BITWISE_AND,
                        &intermIOperands[0],
                        &unsignROperand,
                        &OperandsParameters[0].rOperands[0]);

    status = clGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_GREATER_THAN,
                            IOperand,
                            &intermROperands[0],
                            &infROperand);
    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

static gceSTATUS
_GenIsInfCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;
    clsIOPERAND intermIOperands[2];
    clsROPERAND    infROperand, unsignROperand, intermROperands[2];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    clsROPERAND_InitializeIntOrIVecConstant(&infROperand,
                              clmGenCodeDataType(T_UINT),
                              (gctUINT) 0x7F800000);
     clsROPERAND_InitializeIntOrIVecConstant(&unsignROperand,
                              clmGenCodeDataType(T_UINT),
                              (gctUINT) 0x7FFFFFFF);

   clsIOPERAND_New(Compiler, &intermIOperands[0], clmGenCodeDataType(T_UINT));
   clsROPERAND_InitializeUsingIOperand(&intermROperands[0], &intermIOperands[0]);

    status = clGenArithmeticExprCode(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_BITWISE_AND,
                        &intermIOperands[0],
                        &unsignROperand,
                        &OperandsParameters[0].rOperands[0]);

    status = clGenGenericCode2(
                            Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_EQUAL,
                            IOperand,
                            &intermROperands[0],
                            &infROperand);
    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}


/*klc: - use SET.Z and SET.NZ instead */
static gceSTATUS
_GenSelectCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status = gcvSTATUS_OK;
    clsROPERAND zeroROperand, lshiftROperand, rshiftROperand;
    clsIOPERAND                intermIOperands[3];
    clsROPERAND                intermROperands[3];
    cltELEMENT_TYPE copyType = OperandsParameters[2].rOperands[0].dataType.elementType;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);

    clsROPERAND_InitializeIntOrIVecConstant(&zeroROperand,
                                            clmGenCodeDataType(T_INT),
                                            (gctUINT)0);
    if(clmGEN_CODE_IsScalarDataType(OperandsParameters[2].dataTypes[0].def) ) /*CL vector select */
    { /*Scalar type */
        if(OperandsParameters[2].rOperands[0].dataType.elementType == clvTYPE_FLOAT)
            OperandsParameters[2].rOperands[0].dataType.elementType = clvTYPE_INT; /* Use integer comparison*/

        clmGEN_CODE_IF(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo,
                         &OperandsParameters[2].rOperands[0],
                         clvCONDITION_EQUAL,
                         &zeroROperand);

        if(IOperand) {
           gcmONERROR(clGenGenericCode1(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_ASSIGN,
                            IOperand,
                            &OperandsParameters[0].rOperands[0]));

            /* The false part, "!==0" */
            clmGEN_CODE_ELSE(Compiler,
                           CodeGenerator,
                           PolynaryExpr->exprBase.base.lineNo,
                           PolynaryExpr->exprBase.base.stringNo);

            gcmONERROR(clGenGenericCode1(Compiler,
                             PolynaryExpr->exprBase.base.lineNo,
                             PolynaryExpr->exprBase.base.stringNo,
                             clvOPCODE_ASSIGN,
                             IOperand,
                             &OperandsParameters[1].rOperands[0]));
        }

        clmGEN_CODE_ENDIF(Compiler,
                         CodeGenerator,
                         PolynaryExpr->exprBase.base.lineNo,
                         PolynaryExpr->exprBase.base.stringNo);
   }
   else {
        cltELEMENT_TYPE elementType;

        if(clmIsElementTypeHighPrecision(OperandsParameters[2].dataTypes[0].def.elementType)) {
             elementType = clvTYPE_LONG;
             clsROPERAND_InitializeIntOrIVecConstant(&rshiftROperand,
                                                     clmGenCodeDataType(T_INT),
                                                     (gctUINT)63);
        }
        else {
             elementType = clvTYPE_INT;
             clsROPERAND_InitializeIntOrIVecConstant(&rshiftROperand,
                                                     clmGenCodeDataType(T_INT),
                                                     (gctUINT)31);
        }

        /*ideally, if we have clGenGenericCode3, then use select function, perfect*/
        /*Avoid branch for different component, we get sign(c), r1 = min(sign(c), 0) */
        /* return X + r1*X - r1*Y                                                   */

        /* Always use integer operation, avoid Nan, Inf, denorm*/
        OperandsParameters[2].rOperands[0].dataType.elementType = elementType;
        clsIOPERAND_New(Compiler, &intermIOperands[0], OperandsParameters[0].dataTypes[0].def);
        intermIOperands[0].dataType.elementType = elementType;
        clsIOPERAND_New(Compiler, &intermIOperands[1], OperandsParameters[0].dataTypes[0].def);
        intermIOperands[1].dataType.elementType = elementType;
        clsIOPERAND_New(Compiler, &intermIOperands[2], OperandsParameters[0].dataTypes[0].def);
        intermIOperands[2].dataType.elementType = elementType;

        clsROPERAND_InitializeUsingIOperand(&intermROperands[0], &intermIOperands[0]);
        clsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);
        clsROPERAND_InitializeUsingIOperand(&intermROperands[2], &intermIOperands[2]);

        switch(copyType) {
        case clvTYPE_FLOAT:
        case clvTYPE_UINT:
        case clvTYPE_INT:
        case clvTYPE_SHORT:
        case clvTYPE_CHAR:
        case clvTYPE_LONG:
        case clvTYPE_ULONG:
             gcmONERROR(clGenShiftExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_RSHIFT,
                                           &intermIOperands[1],
                                           &OperandsParameters[2].rOperands[0],
                                           &rshiftROperand));
             break;

        case clvTYPE_USHORT:
             clsROPERAND_InitializeIntOrIVecConstant(&lshiftROperand,
                                                     clmGenCodeDataType(T_INT),
                                                     (gctUINT)16);

             gcmONERROR(clGenShiftExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_LSHIFT,
                                           &intermIOperands[0],
                                           &OperandsParameters[2].rOperands[0],
                                           &lshiftROperand));

             gcmONERROR(clGenShiftExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_RSHIFT,
                                           &intermIOperands[1],
                                           &intermROperands[0],
                                           &rshiftROperand));
             break;

        case clvTYPE_UCHAR:
             clsROPERAND_InitializeIntOrIVecConstant(&lshiftROperand,
                                                     clmGenCodeDataType(T_USHORT),
                                                     (gctUINT)24);
             gcmONERROR(clGenShiftExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_LSHIFT,
                                           &intermIOperands[0],
                                           &OperandsParameters[2].rOperands[0],
                                           &lshiftROperand));

             gcmONERROR(clGenShiftExprCode(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_RSHIFT,
                                           &intermIOperands[1],
                                           &intermROperands[0],
                                           &rshiftROperand));
             break;
        default:
             break;
        }

        /*now r1 = 0 or 0xffffffff */
        gcmONERROR(clGenGenericCode1(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_BITWISE_NOT,
                                     &intermIOperands[0],
                                     &intermROperands[1]));

        {
             cltELEMENT_TYPE copyTypeR = OperandsParameters[0].rOperands[0].dataType.elementType;
             OperandsParameters[0].rOperands[0].dataType.elementType = elementType;
             gcmONERROR(clGenArithmeticExprCode(Compiler,
                                                PolynaryExpr->exprBase.base.lineNo,
                                                PolynaryExpr->exprBase.base.stringNo,
                                                clvOPCODE_BITWISE_AND,
                                                &intermIOperands[2],
                                                &intermROperands[0],
                                                &OperandsParameters[0].rOperands[0]));

             OperandsParameters[0].rOperands[0].dataType.elementType = copyTypeR;

             copyTypeR = OperandsParameters[1].rOperands[0].dataType.elementType;
             OperandsParameters[1].rOperands[0].dataType.elementType = elementType;
             gcmONERROR(clGenArithmeticExprCode(Compiler,
                                                PolynaryExpr->exprBase.base.lineNo,
                                                PolynaryExpr->exprBase.base.stringNo,
                                                clvOPCODE_BITWISE_AND,
                                                &intermIOperands[0],
                                                &intermROperands[1],
                                                &OperandsParameters[1].rOperands[0]));
             OperandsParameters[1].rOperands[0].dataType.elementType = copyTypeR;
        }

        if(IOperand)
        {
             cltELEMENT_TYPE copyTypeI = IOperand->dataType.elementType;
             IOperand->dataType.elementType = elementType;
             gcmONERROR(clGenArithmeticExprCode(Compiler,
                                                PolynaryExpr->exprBase.base.lineNo,
                                                PolynaryExpr->exprBase.base.stringNo,
                                                clvOPCODE_ADD,
                                                IOperand,
                                                &intermROperands[2],
                                                &intermROperands[0]));
             IOperand->dataType.elementType = copyTypeI;
        }
   }

OnError:
   OperandsParameters[2].rOperands[0].dataType.elementType = copyType;
   return status;
}

static gceSTATUS
_GenBitSelectCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;
    clsIOPERAND                intermIOperands[3];
    clsROPERAND                intermROperands[3];
    cltELEMENT_TYPE copyType2 = OperandsParameters[2].rOperands[0].dataType.elementType;
    cltELEMENT_TYPE elementType;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    if(clmIsElementTypeHighPrecision(copyType2)) {
        elementType = clvTYPE_LONG;
    }
    else {
        elementType = clvTYPE_INT;
    }

    OperandsParameters[2].rOperands[0].dataType.elementType = elementType;
    clsIOPERAND_New(Compiler, &intermIOperands[0], OperandsParameters[0].dataTypes[0].def);
    intermIOperands[0].dataType.elementType = elementType;
    clsIOPERAND_New(Compiler, &intermIOperands[1], OperandsParameters[0].dataTypes[0].def);
    intermIOperands[1].dataType.elementType = elementType;
    clsIOPERAND_New(Compiler, &intermIOperands[2], OperandsParameters[0].dataTypes[0].def);
    intermIOperands[2].dataType.elementType = elementType;

    clsROPERAND_InitializeUsingIOperand(&intermROperands[0], &intermIOperands[0]);
    clsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);
    clsROPERAND_InitializeUsingIOperand(&intermROperands[2], &intermIOperands[2]);

    status = clGenGenericCode1(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               clvOPCODE_BITWISE_NOT,
                               &intermIOperands[0],
                               &OperandsParameters[2].rOperands[0]);

   {
       cltELEMENT_TYPE copyTypeR = OperandsParameters[0].rOperands[0].dataType.elementType;
       OperandsParameters[0].rOperands[0].dataType.elementType = elementType;
       status = clGenArithmeticExprCode(Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        clvOPCODE_BITWISE_AND,
                                        &intermIOperands[2],
                                        &intermROperands[0],
                                        &OperandsParameters[0].rOperands[0]);
       OperandsParameters[0].rOperands[0].dataType.elementType = copyTypeR;

       copyTypeR = OperandsParameters[1].rOperands[0].dataType.elementType;
       OperandsParameters[1].rOperands[0].dataType.elementType = elementType;
       status = clGenArithmeticExprCode(Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        clvOPCODE_BITWISE_AND,
                                        &intermIOperands[0],
                                        &OperandsParameters[1].rOperands[0],
                                        &OperandsParameters[2].rOperands[0]);
       OperandsParameters[1].rOperands[0].dataType.elementType = copyTypeR;
   }

   {
       cltELEMENT_TYPE copyTypeI = IOperand->dataType.elementType;

       IOperand->dataType.elementType = elementType;
       status = clGenArithmeticExprCode(Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        clvOPCODE_ADD,
                                        &intermIOperands[1],
                                        &intermROperands[2],
                                        &intermROperands[0]);
       status = clGenGenericCode1(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  clvOPCODE_ASSIGN,
                                  IOperand,
                                  &intermROperands[1]);
       IOperand->dataType.elementType = copyTypeI;
   }

   OperandsParameters[2].rOperands[0].dataType.elementType = copyType2;
   return status;
}

static gceSTATUS
_GenShuffleCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;
    clsIOPERAND     intermIOperands[20];
    clsIOPERAND     indexIOperand, intermIndexIOperand, intermMaskIOperand;
    clsROPERAND     intermIndexROperand, intermMaskROperand;
    clsROPERAND     intermROperands[20], maskROperand, *maskComponent;
    clsROPERAND     constantOneROperand;
    gctREG_INDEX    tempRegIndex;
    clsLOPERAND tempLOperand, destLOperands[16];
    gctUINT8 vectorSize0 = clmGEN_CODE_IsScalarDataType(OperandsParameters[0].dataTypes[0].def) ? 1: clmGEN_CODE_vectorSize_GET(OperandsParameters[0].dataTypes[0].def);
    gctUINT8 vectorSize1 = clmGEN_CODE_IsScalarDataType(OperandsParameters[1].dataTypes[0].def) ? 1: clmGEN_CODE_vectorSize_GET(OperandsParameters[1].dataTypes[0].def);
    gctUINT8 i, vSizeMax = vectorSize0>vectorSize1 ? vectorSize0 : vectorSize1;
    clsGEN_CODE_DATA_TYPE operandType;
    gctSIZE_T    binaryDataTypeRegSize;
    gctSIZE_T    regOffset;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    operandType = gcGetComponentDataType(OperandsParameters[0].dataTypes[0].def);
    if(IOperand->dataType.elementType == clvTYPE_FLOAT)
    {
        operandType.elementType = clvTYPE_UINT;
    }

    clsROPERAND_InitializeIntOrIVecConstant(&maskROperand,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) (vectorSize0 - 1) );
    clsLOPERAND_InitializeUsingIOperand(&tempLOperand, IOperand);
    tempRegIndex = clNewTempRegs(Compiler, vSizeMax, operandType.elementType);
    binaryDataTypeRegSize    = gcGetDataTypeRegSize(operandType);
    regOffset = _clmGetTempRegIndexOffset(binaryDataTypeRegSize,
                                          operandType.elementType);

    status = clNewVariable(Compiler,
                           0,
                           0,
                           "_GenShuffleCode$tempArray",
                           clvQUALIFIER_NONE,
                           clvQUALIFIER_NONE,
                           clvSTORAGE_QUALIFIER_NONE,
                           operandType,
                           vSizeMax,
                           tempRegIndex,
                           gcvNULL);

    clsROPERAND_InitializeTempReg(&intermROperands[17],
                                  clvQUALIFIER_NONE,
                                  operandType,
                                  tempRegIndex);

    intermROperands[17].arrayIndex.mode = clvINDEX_REG;
    clsIOPERAND_New(Compiler, &intermIndexIOperand, clmGenCodeDataType(T_UINT));
    if(clmIsElementTypeHighPrecision(operandType.elementType))
    {
        clsROPERAND_InitializeIntOrIVecConstant(&constantOneROperand,
                                                clmGenCodeDataType(T_INT),
                                                (gctINT)1);

        clsIOPERAND_New(Compiler, &indexIOperand, clmGenCodeDataType(T_UINT));
        clsROPERAND_InitializeUsingIOperand(&intermIndexROperand, &intermIndexIOperand);
        intermROperands[17].arrayIndex.u.indexRegIndex = indexIOperand.tempRegIndex;
    }
    else
    {
        intermROperands[17].arrayIndex.u.indexRegIndex = intermIndexIOperand.tempRegIndex;
    }

    if(clmIsElementTypeHighPrecision(OperandsParameters[1].rOperands[0].dataType.elementType))
    {
        clsIOPERAND_New(Compiler, &intermMaskIOperand, clmGenCodeDataType(T_UINT));
        clsROPERAND_InitializeUsingIOperand(&intermMaskROperand, &intermMaskIOperand);
        maskComponent = &intermMaskROperand;
    }
    else
    {
        maskComponent = &intermROperands[16];
    }

    for(i = 0; i<vSizeMax; i++){
        gctREG_INDEX regIndex;

        regIndex = tempRegIndex + (gctREG_INDEX)(regOffset * i);
        clsIOPERAND_Initialize(&intermIOperands[i],
                               operandType,
                               regIndex);
        clsROPERAND_InitializeUsingIOperand(&intermROperands[i], &intermIOperands[i]);
    }

    /*Settle the vector to index temp registers */
    for(i = 0; i<vectorSize0; i++){
        clmROPERAND_vectorComponent_GET(&intermROperands[16], &OperandsParameters[0].rOperands[0], i);
        status = clGenGenericCode1(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_ASSIGN,
                        &intermIOperands[i],
                        &intermROperands[16]);
    }

    for(i = 0; i<vectorSize1; i++){
       clmROPERAND_vectorComponent_GET(&intermROperands[16], &OperandsParameters[1].rOperands[0], i);
       if(clmIsElementTypeHighPrecision(OperandsParameters[1].rOperands[0].dataType.elementType))
       {
           status = clGenGenericCode1(Compiler,
                                      PolynaryExpr->exprBase.base.lineNo,
                                      PolynaryExpr->exprBase.base.stringNo,
                                      clvOPCODE_LONGLO,
                                      &intermMaskIOperand,
                                      &intermROperands[16]);
           if (gcmIS_ERROR(status)) return status;
       }

       /*Get the effective LSB */
       status = clGenArithmeticExprCode(Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        clvOPCODE_BITWISE_AND,
                                        &intermIndexIOperand,
                                        maskComponent,
                                        &maskROperand);
       if (gcmIS_ERROR(status)) return status;

       if(clmIsElementTypeHighPrecision(operandType.elementType))
       {
           status = clGenGenericCode2(Compiler,
                                      PolynaryExpr->exprBase.base.lineNo,
                                      PolynaryExpr->exprBase.base.stringNo,
                                      clvOPCODE_LSHIFT,
                                      &indexIOperand,
                                      &intermIndexROperand,
                                      &constantOneROperand);
           if (gcmIS_ERROR(status)) return status;
       }

       clmLOPERAND_vectorComponent_GET(&destLOperands[i], &tempLOperand, i);
       /*integer move. Otherwise, it may impact previous index data type, when IOperand data type is float */
       destLOperands[i].dataType.elementType = operandType.elementType;
       status = clGenAssignCode(Compiler,
                                PolynaryExpr->exprBase.base.lineNo,
                                PolynaryExpr->exprBase.base.stringNo,
                                &destLOperands[i],
                                &intermROperands[17]);
       if (gcmIS_ERROR(status)) return status;
    }

    return status;
}

static gceSTATUS
_GenShufflePtrCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status;
    clsIOPERAND srcIOperand[1];
    clsIOPERAND maskIOperand[1];
    clsROPERAND src[1];
    clsROPERAND mask[1];
    clsROPERAND addressOffset[1];
    cloIR_BASE srcArgument;
    cloIR_BASE maskArgument;
    cloIR_EXPR srcExpr;
    cloIR_EXPR maskExpr;
    clsGEN_CODE_DATA_TYPE srcType;
    clsGEN_CODE_DATA_TYPE maskType;

    clsIOPERAND     intermIOperands[40];
    clsROPERAND     intermROperands[40], maskROperand;
    gctREG_INDEX    tempRegIndex;
    clsLOPERAND tempLOperand, destLOperands[16];
    gctUINT8 vectorSize0;
    gctUINT8 vectorSize1;
    gctUINT8 i, vSizeMax;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = cloIR_SET_GetMember(Compiler,
                                     PolynaryExpr->operands,
                                     1,
                                     &srcArgument);
    if (gcmIS_ERROR(status)) return status;

    srcExpr = (cloIR_EXPR)srcArgument;
    srcType = clmGenCodeDataType(srcExpr->decl.dataType->type);

    status = cloIR_SET_GetMember(Compiler,
                                     PolynaryExpr->operands,
                                     2,
                                     &maskArgument);
    if (gcmIS_ERROR(status)) return status;

    maskExpr = (cloIR_EXPR)maskArgument;
    maskType = clmGenCodeDataType(maskExpr->decl.dataType->type);

    clsIOPERAND_New(Compiler, srcIOperand, srcType);
    clsIOPERAND_New(Compiler, maskIOperand, maskType);
    clsROPERAND_InitializeIntOrIVecConstant(addressOffset,
                                                clmGenCodeDataType(T_UINT),
                                                OperandsParameters[0].dataTypes[0].byteOffset);
    status = clGenGenericCode2(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   clvOPCODE_LOAD,
                                   srcIOperand,
                                   &OperandsParameters[0].rOperands[0],
                                   addressOffset);
    if (gcmIS_ERROR(status)) return status;

    clsROPERAND_InitializeIntOrIVecConstant(addressOffset,
                                                clmGenCodeDataType(T_UINT),
                                                OperandsParameters[1].dataTypes[0].byteOffset);
    status = clGenGenericCode2(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   clvOPCODE_LOAD,
                                   maskIOperand,
                                   &OperandsParameters[1].rOperands[0],
                                   addressOffset);
    if (gcmIS_ERROR(status)) return status;

    clsROPERAND_InitializeUsingIOperand(src, srcIOperand);
    clsROPERAND_InitializeUsingIOperand(mask, maskIOperand);

    vectorSize0 = clmGEN_CODE_IsScalarDataType(srcType) ? 1: clmGEN_CODE_vectorSize_GET(srcType);
    vectorSize1 = clmGEN_CODE_IsScalarDataType(maskType) ? 1: clmGEN_CODE_vectorSize_GET(maskType);
    vSizeMax = vectorSize0>vectorSize1 ? vectorSize0 : vectorSize1;
    clsROPERAND_InitializeIntOrIVecConstant(&maskROperand,
                                                clmGenCodeDataType(T_UINT),
                                                (gctUINT) (vectorSize0 - 1) );

    clsLOPERAND_InitializeUsingIOperand(&tempLOperand, IOperand);

    tempRegIndex = clNewTempRegs(Compiler, vSizeMax, clvTYPE_UINT);
    status = clNewVariable(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               "_GenShufflePtrCode$tempArray",
                               clvQUALIFIER_NONE,
                               clvQUALIFIER_NONE,
                               clvSTORAGE_QUALIFIER_NONE,
                               clmGenCodeDataType(T_UINT),
                               vSizeMax,
                               tempRegIndex,
                               gcvNULL);
    clsROPERAND_InitializeTempReg(&intermROperands[17],
                                      clvQUALIFIER_NONE,
                                      clmGenCodeDataType(T_UINT),
                                      tempRegIndex);

    intermROperands[17].arrayIndex.mode = clvINDEX_REG;
    for(i = 0; i<vSizeMax; i++){
        clsIOPERAND_Initialize(&intermIOperands[i], clmGenCodeDataType(T_UINT), tempRegIndex + (gctREG_INDEX)i);
        clsIOPERAND_New(Compiler, &intermIOperands[20+i], clmGenCodeDataType(T_UINT));
    }

    for(i = 0; i<vSizeMax; i++){
        clsROPERAND_InitializeUsingIOperand(&intermROperands[i], &intermIOperands[i]);
        clsROPERAND_InitializeUsingIOperand(&intermROperands[i+20], &intermIOperands[i+20]);
    }

    /*Settle the vector to index temp registers */
    for(i = 0; i<vectorSize0; i++){
        clmROPERAND_vectorComponent_GET(&intermROperands[16], src, i);
        status = clGenGenericCode1(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_ASSIGN,
                        &intermIOperands[i],
                        &intermROperands[16]);
    }
    for(i = 0; i<vectorSize1; i++){
       clmROPERAND_vectorComponent_GET(&intermROperands[16], mask, i);
        /*Get the effect LSB */
       status = clGenArithmeticExprCode(Compiler,
                    PolynaryExpr->exprBase.base.lineNo,
                    PolynaryExpr->exprBase.base.stringNo,
                    clvOPCODE_BITWISE_AND,
                    &intermIOperands[20+i],
                    &intermROperands[16],
                    &maskROperand);

       intermROperands[17].arrayIndex.u.indexRegIndex = intermIOperands[20+i].tempRegIndex;


       clmLOPERAND_vectorComponent_GET(&destLOperands[i], &tempLOperand, i);
       /*integer move. Otherwise, it may impact previous index data type, when IOperand data type is float */
       destLOperands[i].dataType.elementType = clvTYPE_UINT;
       status = clGenAssignCode(
                Compiler,
                PolynaryExpr->exprBase.base.lineNo,
                PolynaryExpr->exprBase.base.stringNo,
                &destLOperands[i],
                &intermROperands[17]);
       if (gcmIS_ERROR(status)) return status;

    }

    return status;
}

static gceSTATUS
_GenShufflePtr1Code(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status;
    clsIOPERAND srcIOperand[1];
    clsROPERAND src[1];
    clsROPERAND mask[1];
    clsROPERAND addressOffset[1];
    cloIR_BASE srcArgument;
    cloIR_BASE maskArgument;
    cloIR_EXPR srcExpr;
    cloIR_EXPR maskExpr;
    clsGEN_CODE_DATA_TYPE srcType;
    clsGEN_CODE_DATA_TYPE maskType;
    gctSIZE_T    binaryDataTypeRegSize;
    gctSIZE_T    regOffset;

    clsIOPERAND     intermIOperands[20];
    clsIOPERAND     indexIOperand, intermIndexIOperand, intermMaskIOperand;
    clsROPERAND     intermIndexROperand, intermMaskROperand;
    clsROPERAND     intermROperands[20], maskROperand, *maskComponent;
    clsROPERAND     constantOneROperand;
    gctREG_INDEX    tempRegIndex;
    clsLOPERAND tempLOperand, destLOperands[16];
    gctUINT8 vectorSize0;
    gctUINT8 vectorSize1;
    gctUINT8 i, vSizeMax;
    clsGEN_CODE_DATA_TYPE operandType;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = cloIR_SET_GetMember(Compiler,
                                     PolynaryExpr->operands,
                                     1,
                                     &srcArgument);
    if (gcmIS_ERROR(status)) return status;

    srcExpr = (cloIR_EXPR)srcArgument;
    srcType = clmGenCodeDataType(srcExpr->decl.dataType->type);

    status = cloIR_SET_GetMember(Compiler,
                                     PolynaryExpr->operands,
                                     2,
                                     &maskArgument);
    if (gcmIS_ERROR(status)) return status;

    maskExpr = (cloIR_EXPR)maskArgument;
    maskType = clmGenCodeDataType(maskExpr->decl.dataType->type);

    clsIOPERAND_New(Compiler, srcIOperand, srcType);
    clsROPERAND_InitializeIntOrIVecConstant(addressOffset,
                                                clmGenCodeDataType(T_UINT),
                                                OperandsParameters[0].dataTypes[0].byteOffset);
    status = clGenGenericCode2(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   clvOPCODE_LOAD,
                                   srcIOperand,
                                   &OperandsParameters[0].rOperands[0],
                                   addressOffset);
    if (gcmIS_ERROR(status)) return status;

    clsROPERAND_InitializeUsingIOperand(src, srcIOperand);
    mask[0] = OperandsParameters[1].rOperands[0];

    vectorSize0 = clmGEN_CODE_IsScalarDataType(srcType) ? 1: clmGEN_CODE_vectorSize_GET(srcType);
    vectorSize1 = clmGEN_CODE_IsScalarDataType(maskType) ? 1: clmGEN_CODE_vectorSize_GET(maskType);
    vSizeMax = vectorSize0>vectorSize1 ? vectorSize0 : vectorSize1;

    operandType = gcGetComponentDataType(src->dataType);
    if(IOperand->dataType.elementType == clvTYPE_FLOAT)
    {
        operandType.elementType = clvTYPE_UINT;
    }
    clsROPERAND_InitializeIntOrIVecConstant(&maskROperand,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) (vectorSize0 - 1) );

    clsLOPERAND_InitializeUsingIOperand(&tempLOperand, IOperand);

    tempRegIndex = clNewTempRegs(Compiler, vSizeMax, operandType.elementType);
    binaryDataTypeRegSize    = gcGetDataTypeRegSize(operandType);
    regOffset = _clmGetTempRegIndexOffset(binaryDataTypeRegSize,
                                          operandType.elementType);
    status = clNewVariable(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               "_GenShufflePtr1Code$tempArray",
                               clvQUALIFIER_NONE,
                               clvQUALIFIER_NONE,
                               clvSTORAGE_QUALIFIER_NONE,
                               operandType,
                               vSizeMax,
                               tempRegIndex,
                               gcvNULL);
    clsROPERAND_InitializeTempReg(&intermROperands[17],
                                  clvQUALIFIER_NONE,
                                  operandType,
                                  tempRegIndex);

    intermROperands[17].arrayIndex.mode = clvINDEX_REG;
    clsIOPERAND_New(Compiler, &intermIndexIOperand, clmGenCodeDataType(T_UINT));
    if(clmIsElementTypeHighPrecision(operandType.elementType))
    {
        clsROPERAND_InitializeIntOrIVecConstant(&constantOneROperand,
                                                clmGenCodeDataType(T_INT),
                                                (gctINT)1);

        clsIOPERAND_New(Compiler, &indexIOperand, clmGenCodeDataType(T_UINT));
        clsROPERAND_InitializeUsingIOperand(&intermIndexROperand, &intermIndexIOperand);
        intermROperands[17].arrayIndex.u.indexRegIndex = indexIOperand.tempRegIndex;
    }
    else
    {
        intermROperands[17].arrayIndex.u.indexRegIndex = intermIndexIOperand.tempRegIndex;
    }

    if(clmIsElementTypeHighPrecision(OperandsParameters[1].rOperands[0].dataType.elementType))
    {
        clsIOPERAND_New(Compiler, &intermMaskIOperand, clmGenCodeDataType(T_UINT));
        clsROPERAND_InitializeUsingIOperand(&intermMaskROperand, &intermMaskIOperand);
        maskComponent = &intermMaskROperand;
    }
    else
    {
        maskComponent = &intermROperands[16];
    }

    for(i = 0; i<vSizeMax; i++){
        gctREG_INDEX regIndex;

        regIndex = tempRegIndex + (gctREG_INDEX)(regOffset * i);
        clsIOPERAND_Initialize(&intermIOperands[i],
                               operandType,
                               regIndex);
    }

    for(i = 0; i<vSizeMax; i++){
        clsROPERAND_InitializeUsingIOperand(&intermROperands[i], &intermIOperands[i]);
    }

    /*Settle the vector to index temp registers */
    for(i = 0; i<vectorSize0; i++){
        clmROPERAND_vectorComponent_GET(&intermROperands[16], src, i);
        status = clGenGenericCode1(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_ASSIGN,
                        &intermIOperands[i],
                        &intermROperands[16]);
    }
    for(i = 0; i<vectorSize1; i++){
       clmROPERAND_vectorComponent_GET(&intermROperands[16], mask, i);
       if(clmIsElementTypeHighPrecision(mask->dataType.elementType))
       {
           status = clGenGenericCode1(Compiler,
                                      PolynaryExpr->exprBase.base.lineNo,
                                      PolynaryExpr->exprBase.base.stringNo,
                                      clvOPCODE_LONGLO,
                                      &intermMaskIOperand,
                                      &intermROperands[16]);
           if (gcmIS_ERROR(status)) return status;
       }
        /*Get the effect LSB */
       status = clGenArithmeticExprCode(Compiler,
                    PolynaryExpr->exprBase.base.lineNo,
                    PolynaryExpr->exprBase.base.stringNo,
                    clvOPCODE_BITWISE_AND,
                    &intermIndexIOperand,
                    maskComponent,
                    &maskROperand);

       if(clmIsElementTypeHighPrecision(operandType.elementType))
       {
           status = clGenGenericCode2(Compiler,
                                      PolynaryExpr->exprBase.base.lineNo,
                                      PolynaryExpr->exprBase.base.stringNo,
                                      clvOPCODE_LSHIFT,
                                      &indexIOperand,
                                      &intermIndexROperand,
                                      &constantOneROperand);
           if (gcmIS_ERROR(status)) return status;
       }

       clmLOPERAND_vectorComponent_GET(&destLOperands[i], &tempLOperand, i);
       /*integer move. Otherwise, it may impact previous index data type, when IOperand data type is float */
       destLOperands[i].dataType.elementType = operandType.elementType;
       status = clGenAssignCode(
                Compiler,
                PolynaryExpr->exprBase.base.lineNo,
                PolynaryExpr->exprBase.base.stringNo,
                &destLOperands[i],
                &intermROperands[17]);
       if (gcmIS_ERROR(status)) return status;

    }

    return status;
}

static gceSTATUS
_GenShuffle2Code(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;
    clsIOPERAND     intermIOperands[60];
    clsIOPERAND     indexIOperand, intermIndexIOperand, intermMaskIOperand;
    clsROPERAND     intermIndexROperand, intermMaskROperand;
    clsROPERAND     intermROperands[60], maskROperand, *maskComponent;
    gctREG_INDEX    tempRegIndex;
    clsROPERAND     constantOneROperand;
    clsGEN_CODE_DATA_TYPE componentDataType;
    clsLOPERAND tempLOperand, destLOperands[16];
    gctUINT8 vectorSize0 = clmGEN_CODE_IsScalarDataType(OperandsParameters[0].dataTypes[0].def) ? 1: clmGEN_CODE_vectorSize_GET(OperandsParameters[0].dataTypes[0].def);
    gctUINT8 vectorSize1 = clmGEN_CODE_IsScalarDataType(OperandsParameters[2].dataTypes[0].def) ? 1: clmGEN_CODE_vectorSize_GET(OperandsParameters[2].dataTypes[0].def);
    gctUINT8 i;
    gctSIZE_T    binaryDataTypeRegSize;
    gctSIZE_T    regOffset;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    clsROPERAND_InitializeIntOrIVecConstant(&maskROperand,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT) (2*vectorSize0 - 1) );

    clsLOPERAND_InitializeUsingIOperand(&tempLOperand, IOperand);
    componentDataType = gcGetVectorComponentDataType(OperandsParameters[0].dataTypes[0].def),
    tempRegIndex = clNewTempRegs(Compiler, 2*vectorSize0, componentDataType.elementType);
    binaryDataTypeRegSize    = gcGetDataTypeRegSize(componentDataType);
    regOffset = _clmGetTempRegIndexOffset(binaryDataTypeRegSize,
                                          componentDataType.elementType);

    status = clNewVariable(Compiler,
                           PolynaryExpr->exprBase.base.lineNo,
                           PolynaryExpr->exprBase.base.stringNo,
                           "_GenShuffle2Code$tempArray",
                           clvQUALIFIER_NONE,
                           clvQUALIFIER_NONE,
                           clvSTORAGE_QUALIFIER_NONE,
                           componentDataType,
                           2*vectorSize0,
                           tempRegIndex,
                           gcvNULL);
    clsROPERAND_InitializeTempReg(&intermROperands[59],
                                  clvQUALIFIER_NONE,
                                  componentDataType,
                                  tempRegIndex);

    intermROperands[59].arrayIndex.mode = clvINDEX_REG;
    clsIOPERAND_New(Compiler, &intermIndexIOperand, clmGenCodeDataType(T_UINT));
    if(clmIsElementTypeHighPrecision(componentDataType.elementType))
    {
        clsROPERAND_InitializeIntOrIVecConstant(&constantOneROperand,
                                                clmGenCodeDataType(T_INT),
                                                (gctINT)1);

        clsIOPERAND_New(Compiler, &indexIOperand, clmGenCodeDataType(T_UINT));
        clsROPERAND_InitializeUsingIOperand(&intermIndexROperand, &intermIndexIOperand);
        intermROperands[59].arrayIndex.u.indexRegIndex = indexIOperand.tempRegIndex;
    }
    else
    {
        intermROperands[59].arrayIndex.u.indexRegIndex = intermIndexIOperand.tempRegIndex;
    }

    if(clmIsElementTypeHighPrecision(OperandsParameters[2].rOperands[0].dataType.elementType))
    {
        clsIOPERAND_New(Compiler, &intermMaskIOperand, clmGenCodeDataType(T_UINT));
        clsROPERAND_InitializeUsingIOperand(&intermMaskROperand, &intermMaskIOperand);
        maskComponent = &intermMaskROperand;
    }
    else
    {
        maskComponent = &intermROperands[58];
    }

    for(i = 0; i<2*vectorSize0; i++){
        gctREG_INDEX regIndex;

        regIndex = tempRegIndex + (gctREG_INDEX)(regOffset * i);
        clsIOPERAND_Initialize(&intermIOperands[i], componentDataType, regIndex);
        clsROPERAND_InitializeUsingIOperand(&intermROperands[i], &intermIOperands[i]);
    }

    /*Settle the vector to index temp registers */
    for(i = 0; i<vectorSize0; i++){
        clmROPERAND_vectorComponent_GET(&intermROperands[58], &OperandsParameters[0].rOperands[0], i);
        status = clGenGenericCode1(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   clvOPCODE_ASSIGN,
                                   &intermIOperands[i],
                                   &intermROperands[58]);
        clmROPERAND_vectorComponent_GET(&intermROperands[58], &OperandsParameters[1].rOperands[0], i);
        status = clGenGenericCode1(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   clvOPCODE_ASSIGN,
                                   &intermIOperands[i+vectorSize0],
                                   &intermROperands[58]);
    }
    for(i = 0; i<vectorSize1; i++){
       clmROPERAND_vectorComponent_GET(&intermROperands[58], &OperandsParameters[2].rOperands[0], i);
       if(clmIsElementTypeHighPrecision(OperandsParameters[2].rOperands[0].dataType.elementType))
       {
           status = clGenGenericCode1(Compiler,
                                      PolynaryExpr->exprBase.base.lineNo,
                                      PolynaryExpr->exprBase.base.stringNo,
                                      clvOPCODE_LONGLO,
                                      &intermMaskIOperand,
                                      &intermROperands[58]);
           if (gcmIS_ERROR(status)) return status;
       }
        /*Get the effect LSB */
       status = clGenArithmeticExprCode(Compiler,
                                        PolynaryExpr->exprBase.base.lineNo,
                                        PolynaryExpr->exprBase.base.stringNo,
                                        clvOPCODE_BITWISE_AND,
                                        &intermIndexIOperand,
                                        maskComponent,
                                        &maskROperand);
       if (gcmIS_ERROR(status)) return status;

       if(clmIsElementTypeHighPrecision(componentDataType.elementType))
       {
           status = clGenGenericCode2(Compiler,
                                      PolynaryExpr->exprBase.base.lineNo,
                                      PolynaryExpr->exprBase.base.stringNo,
                                      clvOPCODE_LSHIFT,
                                      &indexIOperand,
                                      &intermIndexROperand,
                                      &constantOneROperand);
           if (gcmIS_ERROR(status)) return status;
       }

       clmLOPERAND_vectorComponent_GET(&destLOperands[i], &tempLOperand, i);
       status = clGenAssignCode(Compiler,
                    PolynaryExpr->exprBase.base.lineNo,
                    PolynaryExpr->exprBase.base.stringNo,
                    &destLOperands[i],
                    &intermROperands[59]);
       if (gcmIS_ERROR(status)) return status;
    }

    return status;
}

static gceSTATUS
_GenShuffle2PtrCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status;
    clsIOPERAND src1IOperand[1];
    clsIOPERAND src2IOperand[1];
    clsIOPERAND maskIOperand[1];
    clsROPERAND src1[1];
    clsROPERAND src2[1];
    clsROPERAND mask[1];
    clsROPERAND addressOffset[1];
    cloIR_BASE src1Argument;
    cloIR_BASE maskArgument;
    cloIR_EXPR src1Expr;
    cloIR_EXPR maskExpr;
    clsGEN_CODE_DATA_TYPE src1Type;
    clsGEN_CODE_DATA_TYPE maskType;
    clsIOPERAND     intermIOperands[80];
    clsROPERAND     intermROperands[80], maskROperand;
    gctREG_INDEX    tempRegIndex;
    clsGEN_CODE_DATA_TYPE componentDataType;
    clsLOPERAND tempLOperand, destLOperands[16];
    gctUINT8 vectorSize0;
    gctUINT8 vectorSize1;
    gctUINT8 i;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = cloIR_SET_GetMember(Compiler,
                                     PolynaryExpr->operands,
                                     1,
                                     &src1Argument);
    if (gcmIS_ERROR(status)) return status;

    src1Expr = (cloIR_EXPR)src1Argument;
    src1Type = clmGenCodeDataType(src1Expr->decl.dataType->type);

    status = cloIR_SET_GetMember(Compiler,
                                     PolynaryExpr->operands,
                                     3,
                                     &maskArgument);
    if (gcmIS_ERROR(status)) return status;

    maskExpr = (cloIR_EXPR)maskArgument;
    maskType = clmGenCodeDataType(maskExpr->decl.dataType->type);

    clsIOPERAND_New(Compiler, src1IOperand, src1Type);
    clsIOPERAND_New(Compiler, src2IOperand, src1Type);
    clsIOPERAND_New(Compiler, maskIOperand, maskType);
    clsROPERAND_InitializeIntOrIVecConstant(addressOffset,
                                                clmGenCodeDataType(T_UINT),
                                                OperandsParameters[0].dataTypes[0].byteOffset);
    status = clGenGenericCode2(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   clvOPCODE_LOAD,
                                   src1IOperand,
                                   &OperandsParameters[0].rOperands[0],
                                   addressOffset);
    if (gcmIS_ERROR(status)) return status;

    clsROPERAND_InitializeIntOrIVecConstant(addressOffset,
                                                clmGenCodeDataType(T_UINT),
                                                OperandsParameters[1].dataTypes[0].byteOffset);
    status = clGenGenericCode2(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   clvOPCODE_LOAD,
                                   src2IOperand,
                                   &OperandsParameters[1].rOperands[0],
                                   addressOffset);
    if (gcmIS_ERROR(status)) return status;

    clsROPERAND_InitializeIntOrIVecConstant(addressOffset,
                                                clmGenCodeDataType(T_UINT),
                                                OperandsParameters[2].dataTypes[0].byteOffset);
    status = clGenGenericCode2(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   clvOPCODE_LOAD,
                                   maskIOperand,
                                   &OperandsParameters[2].rOperands[0],
                                   addressOffset);
    if (gcmIS_ERROR(status)) return status;

    clsROPERAND_InitializeUsingIOperand(src1, src1IOperand);
    clsROPERAND_InitializeUsingIOperand(src2, src2IOperand);
    clsROPERAND_InitializeUsingIOperand(mask, maskIOperand);

    vectorSize0 = clmGEN_CODE_IsScalarDataType(src1Type) ? 1: clmGEN_CODE_vectorSize_GET(src1Type);
    vectorSize1 = clmGEN_CODE_IsScalarDataType(maskType) ? 1: clmGEN_CODE_vectorSize_GET(maskType);
    clsROPERAND_InitializeIntOrIVecConstant(&maskROperand,
                                                clmGenCodeDataType(T_UINT),
                                                (gctUINT) (2*vectorSize0 - 1) );

    clsLOPERAND_InitializeUsingIOperand(&tempLOperand, IOperand);
    componentDataType = gcGetVectorComponentDataType(src1Type),
    tempRegIndex = clNewTempRegs(Compiler, 2*vectorSize0, componentDataType.elementType);
    status = clNewVariable(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               "_GenShuffle2PtrCode$tempArray",
                               clvQUALIFIER_NONE,
                               clvQUALIFIER_NONE,
                               clvSTORAGE_QUALIFIER_NONE,
                               componentDataType,
                               2*vectorSize0,
                               tempRegIndex,
                               gcvNULL);
    clsROPERAND_InitializeTempReg(&intermROperands[79],
                                      clvQUALIFIER_NONE,
                                      componentDataType,
                                      tempRegIndex);

    intermROperands[79].arrayIndex.mode = clvINDEX_REG;

    for(i = 0; i<2*vectorSize0; i++){
        clsIOPERAND_Initialize(&intermIOperands[i], componentDataType, tempRegIndex + (gctREG_INDEX)i);
        clsROPERAND_InitializeUsingIOperand(&intermROperands[i], &intermIOperands[i]);

    }

    for(i = 0; i<vectorSize1; i++){
        clsIOPERAND_New(Compiler, &intermIOperands[40+i], clmGenCodeDataType(T_UINT));
        clsROPERAND_InitializeUsingIOperand(&intermROperands[i+40], &intermIOperands[i+40]);
    }

    /*Settle the vector to index temp registers */
    for(i = 0; i<vectorSize0; i++){
        clmROPERAND_vectorComponent_GET(&intermROperands[78], src1, i);
        status = clGenGenericCode1(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_ASSIGN,
                                           &intermIOperands[i],
                                           &intermROperands[78]);
        clmROPERAND_vectorComponent_GET(&intermROperands[78], src2, i);
        status = clGenGenericCode1(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_ASSIGN,
                                           &intermIOperands[i+vectorSize0],
                                           &intermROperands[78]);
    }
    for(i = 0; i<vectorSize1; i++){
       clmROPERAND_vectorComponent_GET(&intermROperands[78], mask, i);
        /*Get the effect LSB */
       status = clGenArithmeticExprCode(Compiler,
                                            PolynaryExpr->exprBase.base.lineNo,
                                            PolynaryExpr->exprBase.base.stringNo,
                                            clvOPCODE_BITWISE_AND,
                                            &intermIOperands[40+i],
                                            &intermROperands[78],
                                            &maskROperand);

       intermROperands[79].arrayIndex.u.indexRegIndex = intermIOperands[40+i].tempRegIndex;

       clmLOPERAND_vectorComponent_GET(&destLOperands[i], &tempLOperand, i);
       status = clGenAssignCode(Compiler,
                    PolynaryExpr->exprBase.base.lineNo,
                    PolynaryExpr->exprBase.base.stringNo,
                    &destLOperands[i],
                    &intermROperands[79]);
       if (gcmIS_ERROR(status)) return status;
    }

    return status;
}

static gceSTATUS
_GenShuffle2Ptr1Code(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status;
    clsIOPERAND src1IOperand[1];
    clsIOPERAND src2IOperand[1];
    clsROPERAND src1[1];
    clsROPERAND src2[1];
    clsROPERAND mask[1];
    clsROPERAND addressOffset[1];
    cloIR_BASE src1Argument;
    cloIR_BASE maskArgument;
    cloIR_EXPR src1Expr;
    cloIR_EXPR maskExpr;
    clsGEN_CODE_DATA_TYPE src1Type;
    clsGEN_CODE_DATA_TYPE maskType;
    gctSIZE_T    binaryDataTypeRegSize;
    gctSIZE_T    regOffset;
    clsIOPERAND     intermIOperands[60];
    clsIOPERAND     indexIOperand, intermIndexIOperand, intermMaskIOperand;
    clsROPERAND     intermIndexROperand, intermMaskROperand;
    clsROPERAND     intermROperands[60], maskROperand, *maskComponent;
    clsROPERAND     constantOneROperand;
    gctREG_INDEX    tempRegIndex;
    clsGEN_CODE_DATA_TYPE componentDataType;
    clsLOPERAND tempLOperand, destLOperands[16];
    gctUINT8 vectorSize0;
    gctUINT8 vectorSize1;
    gctUINT8 i;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 3);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    status = cloIR_SET_GetMember(Compiler,
                                     PolynaryExpr->operands,
                                     1,
                                     &src1Argument);
    if (gcmIS_ERROR(status)) return status;

    src1Expr = (cloIR_EXPR)src1Argument;
    src1Type = clmGenCodeDataType(src1Expr->decl.dataType->type);

    status = cloIR_SET_GetMember(Compiler,
                                     PolynaryExpr->operands,
                                     3,
                                     &maskArgument);
    if (gcmIS_ERROR(status)) return status;

    maskExpr = (cloIR_EXPR)maskArgument;
    maskType = clmGenCodeDataType(maskExpr->decl.dataType->type);

    clsIOPERAND_New(Compiler, src1IOperand, src1Type);
    clsIOPERAND_New(Compiler, src2IOperand, src1Type);
    clsROPERAND_InitializeIntOrIVecConstant(addressOffset,
                                                clmGenCodeDataType(T_UINT),
                                                OperandsParameters[0].dataTypes[0].byteOffset);
    status = clGenGenericCode2(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   clvOPCODE_LOAD,
                                   src1IOperand,
                                   &OperandsParameters[0].rOperands[0],
                                   addressOffset);
    if (gcmIS_ERROR(status)) return status;

    clsROPERAND_InitializeIntOrIVecConstant(addressOffset,
                                                clmGenCodeDataType(T_UINT),
                                                OperandsParameters[1].dataTypes[0].byteOffset);
    status = clGenGenericCode2(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   clvOPCODE_LOAD,
                                   src2IOperand,
                                   &OperandsParameters[1].rOperands[0],
                                   addressOffset);
    if (gcmIS_ERROR(status)) return status;

    clsROPERAND_InitializeUsingIOperand(src1, src1IOperand);
    clsROPERAND_InitializeUsingIOperand(src2, src2IOperand);

    mask[0] = OperandsParameters[2].rOperands[0];
    vectorSize0 = clmGEN_CODE_IsScalarDataType(src1Type) ? 1: clmGEN_CODE_vectorSize_GET(src1Type);
    vectorSize1 = clmGEN_CODE_IsScalarDataType(maskType) ? 1: clmGEN_CODE_vectorSize_GET(maskType);
    clsROPERAND_InitializeIntOrIVecConstant(&maskROperand,
                                                clmGenCodeDataType(T_UINT),
                                                (gctUINT) (2*vectorSize0 - 1) );

    clsLOPERAND_InitializeUsingIOperand(&tempLOperand, IOperand);
    componentDataType = gcGetVectorComponentDataType(src1Type),
    tempRegIndex = clNewTempRegs(Compiler, 2*vectorSize0, componentDataType.elementType);
    binaryDataTypeRegSize    = gcGetDataTypeRegSize(componentDataType);
    regOffset = _clmGetTempRegIndexOffset(binaryDataTypeRegSize,
                                          componentDataType.elementType);
    status = clNewVariable(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               "_GenShuffle2Ptr1Code$tempArray",
                               clvQUALIFIER_NONE,
                               clvQUALIFIER_NONE,
                               clvSTORAGE_QUALIFIER_NONE,
                               componentDataType,
                               2*vectorSize0,
                               tempRegIndex,
                               gcvNULL);
    clsROPERAND_InitializeTempReg(&intermROperands[59],
                                      clvQUALIFIER_NONE,
                                      componentDataType,
                                      tempRegIndex);

    intermROperands[59].arrayIndex.mode = clvINDEX_REG;
    clsIOPERAND_New(Compiler, &intermIndexIOperand, clmGenCodeDataType(T_UINT));
    if(clmIsElementTypeHighPrecision(componentDataType.elementType))
    {
        clsROPERAND_InitializeIntOrIVecConstant(&constantOneROperand,
                                                clmGenCodeDataType(T_INT),
                                                (gctINT)1);

        clsIOPERAND_New(Compiler, &indexIOperand, clmGenCodeDataType(T_UINT));
        clsROPERAND_InitializeUsingIOperand(&intermIndexROperand, &intermIndexIOperand);
        intermROperands[59].arrayIndex.u.indexRegIndex = indexIOperand.tempRegIndex;
    }
    else
    {
        intermROperands[59].arrayIndex.u.indexRegIndex = intermIndexIOperand.tempRegIndex;
    }

    if(clmIsElementTypeHighPrecision(OperandsParameters[2].rOperands[0].dataType.elementType))
    {
        clsIOPERAND_New(Compiler, &intermMaskIOperand, clmGenCodeDataType(T_UINT));
        clsROPERAND_InitializeUsingIOperand(&intermMaskROperand, &intermMaskIOperand);
        maskComponent = &intermMaskROperand;
    }
    else
    {
        maskComponent = &intermROperands[58];
    }

    for(i = 0; i<2*vectorSize0; i++){
        gctREG_INDEX regIndex;

        regIndex = tempRegIndex + (gctREG_INDEX)(regOffset * i);
        clsIOPERAND_Initialize(&intermIOperands[i], componentDataType, regIndex);
        clsROPERAND_InitializeUsingIOperand(&intermROperands[i], &intermIOperands[i]);
    }

    /*Settle the vector to index temp registers */
    for(i = 0; i<vectorSize0; i++){
        clmROPERAND_vectorComponent_GET(&intermROperands[58], src1, i);
        status = clGenGenericCode1(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_ASSIGN,
                                           &intermIOperands[i],
                                           &intermROperands[58]);
        clmROPERAND_vectorComponent_GET(&intermROperands[58], src2, i);
        status = clGenGenericCode1(Compiler,
                                           PolynaryExpr->exprBase.base.lineNo,
                                           PolynaryExpr->exprBase.base.stringNo,
                                           clvOPCODE_ASSIGN,
                                           &intermIOperands[i+vectorSize0],
                                           &intermROperands[58]);
    }
    for(i = 0; i<vectorSize1; i++){
       clmROPERAND_vectorComponent_GET(&intermROperands[58], mask, i);
       if(clmIsElementTypeHighPrecision(mask->dataType.elementType))
       {
           status = clGenGenericCode1(Compiler,
                                      PolynaryExpr->exprBase.base.lineNo,
                                      PolynaryExpr->exprBase.base.stringNo,
                                      clvOPCODE_LONGLO,
                                      &intermMaskIOperand,
                                      &intermROperands[58]);
           if (gcmIS_ERROR(status)) return status;
       }
        /*Get the effect LSB */
       status = clGenArithmeticExprCode(Compiler,
                                            PolynaryExpr->exprBase.base.lineNo,
                                            PolynaryExpr->exprBase.base.stringNo,
                                            clvOPCODE_BITWISE_AND,
                                            &intermIndexIOperand,
                                            maskComponent,
                                            &maskROperand);
       if (gcmIS_ERROR(status)) return status;

       if(clmIsElementTypeHighPrecision(componentDataType.elementType))
       {
           status = clGenGenericCode2(Compiler,
                                      PolynaryExpr->exprBase.base.lineNo,
                                      PolynaryExpr->exprBase.base.stringNo,
                                      clvOPCODE_LSHIFT,
                                      &indexIOperand,
                                      &intermIndexROperand,
                                      &constantOneROperand);
           if (gcmIS_ERROR(status)) return status;
       }

       clmLOPERAND_vectorComponent_GET(&destLOperands[i], &tempLOperand, i);
       status = clGenAssignCode(Compiler,
                    PolynaryExpr->exprBase.base.lineNo,
                    PolynaryExpr->exprBase.base.stringNo,
                    &destLOperands[i],
                    &intermROperands[59]);
       if (gcmIS_ERROR(status)) return status;
    }

    return status;
}

static gceSTATUS
_GenAnyAllCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS   status;
    clsROPERAND rshiftROperand;
    clsIOPERAND intermIOperands[3];
    clsROPERAND intermROperands[3];
    clsROPERAND rOperandBuf[1];
    clsROPERAND *rOperand;
    clsGEN_CODE_DATA_TYPE dataType;
    gctUINT8 i, vSize;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 1);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    vSize = clmGEN_CODE_IsVectorDataType(OperandsParameters[0].dataTypes[0].def) == 0? 1: clmGEN_CODE_vectorSize_GET(OperandsParameters[0].dataTypes[0].def);

    dataType = OperandsParameters[0].dataTypes[0].def;
    rOperand = &OperandsParameters[0].rOperands[0];
    if(clmIsElementTypeHighPrecision(dataType.elementType)) {
        clsIOPERAND iOperand[1];

        if(dataType.elementType == clvTYPE_LONG) {
            dataType.elementType = clvTYPE_INT;
        }
        else {
            dataType.elementType = clvTYPE_UINT;
        }

        clsIOPERAND_New(Compiler, iOperand, dataType);
        status = clGenGenericCode1(Compiler,
                                   PolynaryExpr->exprBase.base.lineNo,
                                   PolynaryExpr->exprBase.base.stringNo,
                                   clvOPCODE_LONGHI,
                                   iOperand,
                                   rOperand);
        if (gcmIS_ERROR(status)) return status;

        clsROPERAND_InitializeUsingIOperand(rOperandBuf, iOperand);
        rOperand = rOperandBuf;
    }

    for(i = 0; i<3; i++){
        clsIOPERAND_New(Compiler, &intermIOperands[i], dataType);
        intermIOperands[i].dataType.elementType = clvTYPE_UINT;
        clsROPERAND_InitializeUsingIOperand(&intermROperands[i], &intermIOperands[i]);
    }
    clsROPERAND_InitializeIntOrIVecConstant(&rshiftROperand,
                                            clmGenCodeDataType(T_UINT),
                                            (gctUINT)31);

    if(vSize == 1){
       cltELEMENT_TYPE copyElementType = IOperand->dataType.elementType;
       IOperand->dataType.elementType = clvTYPE_UINT;
       status = clGenShiftExprCode(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_RSHIFT,
                        IOperand,
                        rOperand,
                        &rshiftROperand);
       IOperand->dataType.elementType = copyElementType;
    }
    else{
       cleOPCODE andOrOp = strstr(PolynaryExpr->funcName->symbol, "all") ? clvOPCODE_BITWISE_AND: clvOPCODE_BITWISE_OR;
       status = clGenShiftExprCode(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_RSHIFT,
                        &intermIOperands[0],
                        rOperand,
                        &rshiftROperand);
       clsROPERAND_InitializeUsingIOperand(&intermROperands[2], IOperand);
       for(i = 0; i<vSize; i++){
           clmROPERAND_vectorComponent_GET(&intermROperands[1], &intermROperands[0], i);
           if(i == 0){
                status = clGenGenericCode1(Compiler,
                            PolynaryExpr->exprBase.base.lineNo,
                            PolynaryExpr->exprBase.base.stringNo,
                            clvOPCODE_ASSIGN,
                            IOperand,
                            &intermROperands[1]);
           }
           else{
                status = clGenBitwiseExprCode(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        andOrOp,
                        IOperand,
                        &intermROperands[2],
                        &intermROperands[1]);
           }
       }
    }

    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}


static gceSTATUS
_GenAsyncCopyCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;
    gctUINT elementSize;
    cloIR_EXPR memOperand = gcvNULL;
    clsROPERAND oneROperand, zeroROperand, elementSizeROperand;
    clsIOPERAND intermIOperands[4];
    clsROPERAND intermROperands[4];
    clsLOPERAND lOperand[1];
    clsSELECTION_CONTEXT selectionContextLoopBack;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 4);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    FOR_EACH_DLINK_NODE(&PolynaryExpr->operands->members, struct _cloIR_EXPR, memOperand) {
        if(memOperand != gcvNULL) {
            break;
        }
    }

    elementSize = clsDECL_GetPointedToByteSize(&memOperand->decl);

    clsROPERAND_InitializeIntOrIVecConstant(&oneROperand, clmGenCodeDataType(T_UINT), 0x00000001);
    clsROPERAND_InitializeIntOrIVecConstant(&zeroROperand, clmGenCodeDataType(T_UINT), 0x00000000);
    clsROPERAND_InitializeIntOrIVecConstant(&elementSizeROperand, clmGenCodeDataType(T_UINT), elementSize);

    clsIOPERAND_New(Compiler, &intermIOperands[0], clmGenCodeDataType(T_UINT));
    clsROPERAND_InitializeUsingIOperand(&intermROperands[0], &intermIOperands[0]);

    clsIOPERAND_New(Compiler, &intermIOperands[1], clmGenCodeDataType(T_UINT));
    clsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);

    clsIOPERAND_New(Compiler, &intermIOperands[2], clmGenCodeDataType(T_UCHAR));
    clsROPERAND_InitializeUsingIOperand(&intermROperands[2], &intermIOperands[2]);

    clsIOPERAND_New(Compiler, &intermIOperands[3], clmGenCodeDataType(T_UINT));
    clsROPERAND_InitializeUsingIOperand(&intermROperands[3], &intermIOperands[3]);

    clsLOPERAND_InitializeUsingROperand(lOperand, &OperandsParameters[0].rOperands[0]);

    /* r0 = elementSize * n */
    status = clGenArithmeticExprCode(
                        Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_MUL,
                        &intermIOperands[0],
                        &OperandsParameters[2].rOperands[0],
                        &elementSizeROperand);
    if (gcmIS_ERROR(status)) return status;

    /* r1 = 0 */
    status = clGenGenericCode1(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_ASSIGN,
                        &intermIOperands[1],
                        &zeroROperand);
    if (gcmIS_ERROR(status)) return status;


    status = clDefineSelectionBegin(Compiler,
                                    CodeGenerator,
                                    gcvTRUE,
                                    &selectionContextLoopBack);
    if (gcmIS_ERROR(status)) return status;

    /* Loop ends here, jump back */
    status = clDefineSelectionFalseOperandBegin(Compiler,
                                            CodeGenerator,
                                            &selectionContextLoopBack);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionFalseOperandEnd(Compiler,
                                    CodeGenerator,
                                    &selectionContextLoopBack);
    if (gcmIS_ERROR(status)) return status;

    /* r2 = src[r1] */
    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_LOAD,
                                 &intermIOperands[2],
                                 &OperandsParameters[1].rOperands[0],
                                 &intermROperands[1]));
    /* dst[r1] = r2 */
    gcmONERROR(clGenStoreCode(Compiler,
                              PolynaryExpr->exprBase.base.lineNo,
                              PolynaryExpr->exprBase.base.stringNo,
                              &intermROperands[2],
                              lOperand,
                              clmGenCodeDataType(T_UCHAR),
                              &intermROperands[1]));
    /* r3 = r1 + 1 */
    status = clGenArithmeticExprCode(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_ADD,
                        &intermIOperands[3],
                        &intermROperands[1],
                        &oneROperand);
    if (gcmIS_ERROR(status)) return status;

    /* r1 = r3 */
    status = clGenGenericCode1(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_ASSIGN,
                        &intermIOperands[1],
                        &intermROperands[3]);
    if (gcmIS_ERROR(status)) return status;

    /* r1 >= elementSize * n ? */
    status = clGenSelectionCompareConditionCode(Compiler,
                                                CodeGenerator,
                                                &selectionContextLoopBack,
                                                PolynaryExpr->exprBase.base.lineNo,
                                                PolynaryExpr->exprBase.base.stringNo,
                                                clvCONDITION_GREATER_THAN_EQUAL,
                                                &intermROperands[1],
                                                &intermROperands[0]);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionTrueOperandBegin(Compiler,
                                            CodeGenerator,
                                            &selectionContextLoopBack);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionTrueOperandEnd(Compiler,
                                    CodeGenerator,
                                    &selectionContextLoopBack,
                                    gcvFALSE);

    status = clDefineSelectionEnd(Compiler,
                                CodeGenerator,
                                &selectionContextLoopBack);
    if (gcmIS_ERROR(status)) return status;

    /* return input parameter event as output */
    status = clGenGenericCode1(Compiler,
                PolynaryExpr->exprBase.base.lineNo,
                PolynaryExpr->exprBase.base.stringNo,
                clvOPCODE_ASSIGN,
                IOperand,
                &OperandsParameters[3].rOperands[0]);
    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    return status;
}

static gceSTATUS
_GenAsyncCopyStridedCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;
    gctUINT elementSize;
    cloIR_EXPR memOperand = gcvNULL;
    clsROPERAND oneROperand, zeroROperand, elementSizeROperand;
    clsIOPERAND intermIOperands[9];
    clsROPERAND intermROperands[9];
    clsLOPERAND lOperand[1];
    clsSELECTION_CONTEXT selectionContextLoopBack, selectionContextOuterLoopBack;
    gctBOOL isSrcStride = gcvFALSE;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 5);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    FOR_EACH_DLINK_NODE(&PolynaryExpr->operands->members, struct _cloIR_EXPR, memOperand) {
        if(memOperand != gcvNULL) {
            break;
        }
    }

    elementSize = clsDECL_GetPointedToByteSize(&memOperand->decl);

    if(memOperand->decl.dataType[0].addrSpaceQualifier == clvQUALIFIER_LOCAL) {
        isSrcStride = gcvTRUE;
    }

    clsROPERAND_InitializeIntOrIVecConstant(&oneROperand, clmGenCodeDataType(T_UINT), 0x00000001);
    clsROPERAND_InitializeIntOrIVecConstant(&zeroROperand, clmGenCodeDataType(T_UINT), 0x00000000);
    clsROPERAND_InitializeIntOrIVecConstant(&elementSizeROperand, clmGenCodeDataType(T_UINT), elementSize);

    clsIOPERAND_New(Compiler, &intermIOperands[0], clmGenCodeDataType(T_UINT));
    clsROPERAND_InitializeUsingIOperand(&intermROperands[0], &intermIOperands[0]);

    clsIOPERAND_New(Compiler, &intermIOperands[1], clmGenCodeDataType(T_UINT));
    clsROPERAND_InitializeUsingIOperand(&intermROperands[1], &intermIOperands[1]);

    clsIOPERAND_New(Compiler, &intermIOperands[2], clmGenCodeDataType(T_UCHAR));
    clsROPERAND_InitializeUsingIOperand(&intermROperands[2], &intermIOperands[2]);

    clsIOPERAND_New(Compiler, &intermIOperands[3], clmGenCodeDataType(T_UINT));
    clsROPERAND_InitializeUsingIOperand(&intermROperands[3], &intermIOperands[3]);

    clsIOPERAND_New(Compiler, &intermIOperands[4], clmGenCodeDataType(T_UINT));
    clsROPERAND_InitializeUsingIOperand(&intermROperands[4], &intermIOperands[4]);

    clsIOPERAND_New(Compiler, &intermIOperands[5], clmGenCodeDataType(T_UINT));
    clsROPERAND_InitializeUsingIOperand(&intermROperands[5], &intermIOperands[5]);

    clsIOPERAND_New(Compiler, &intermIOperands[6], clmGenCodeDataType(T_UINT));
    clsROPERAND_InitializeUsingIOperand(&intermROperands[6], &intermIOperands[6]);

    clsIOPERAND_New(Compiler, &intermIOperands[7], clmGenCodeDataType(T_UINT));
    clsROPERAND_InitializeUsingIOperand(&intermROperands[7], &intermIOperands[7]);

    clsIOPERAND_New(Compiler, &intermIOperands[8], clmGenCodeDataType(T_UINT));
    clsROPERAND_InitializeUsingIOperand(&intermROperands[8], &intermIOperands[8]);

    clsLOPERAND_InitializeUsingROperand(lOperand, &OperandsParameters[0].rOperands[0]);

    /* r4 = 0 */
    status = clGenGenericCode1(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_ASSIGN,
                        &intermIOperands[4],
                        &zeroROperand);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionBegin(Compiler,
                                    CodeGenerator,
                                    gcvTRUE,
                                    &selectionContextOuterLoopBack);
    if (gcmIS_ERROR(status)) return status;

    /* Outer loop ends here, jump back */
    status = clDefineSelectionFalseOperandBegin(Compiler,
                                            CodeGenerator,
                                            &selectionContextOuterLoopBack);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionFalseOperandEnd(Compiler,
                                    CodeGenerator,
                                    &selectionContextOuterLoopBack);
    if (gcmIS_ERROR(status)) return status;

    /* r7 = elementSize * r4 */
    status = clGenArithmeticExprCode(
                        Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_MUL,
                        &intermIOperands[7],
                        &intermROperands[4],
                        &elementSizeROperand);
    if (gcmIS_ERROR(status)) return status;

    /* r8 = elementSize * r4 * stride */
    status = clGenArithmeticExprCode(
                        Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_MUL,
                        &intermIOperands[8],
                        &OperandsParameters[3].rOperands[0],
                        &intermROperands[7]);
    if (gcmIS_ERROR(status)) return status;

    /* r1 = 0 */
    status = clGenGenericCode1(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_ASSIGN,
                        &intermIOperands[1],
                        &zeroROperand);
    if (gcmIS_ERROR(status)) return status;


    status = clDefineSelectionBegin(Compiler,
                                    CodeGenerator,
                                    gcvTRUE,
                                    &selectionContextLoopBack);
    if (gcmIS_ERROR(status)) return status;

    /* Loop ends here, jump back */
    status = clDefineSelectionFalseOperandBegin(Compiler,
                                            CodeGenerator,
                                            &selectionContextLoopBack);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionFalseOperandEnd(Compiler,
                                    CodeGenerator,
                                    &selectionContextLoopBack);
    if (gcmIS_ERROR(status)) return status;

    /* r5 = r7 + r1 */
    status = clGenArithmeticExprCode(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_ADD,
                        &intermIOperands[5],
                        &intermROperands[1],
                        &intermROperands[7]);
    if (gcmIS_ERROR(status)) return status;

    /* r6 = r8 + r1 */
    status = clGenArithmeticExprCode(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_ADD,
                        &intermIOperands[6],
                        &intermROperands[1],
                        &intermROperands[8]);
    if (gcmIS_ERROR(status)) return status;

    if (isSrcStride) {

        /* r2 = src[r6] */
        gcmONERROR(clGenGenericCode2(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_LOAD,
                                     &intermIOperands[2],
                                     &OperandsParameters[1].rOperands[0],
                                     &intermROperands[6]));
        /* dst[r5] = r2 */
        gcmONERROR(clGenStoreCode(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  &intermROperands[2],
                                  lOperand,
                                  clmGenCodeDataType(T_UCHAR),
                                  &intermROperands[5]));
    } else {

        /* r2 = src[r5] */
        gcmONERROR(clGenGenericCode2(Compiler,
                                     PolynaryExpr->exprBase.base.lineNo,
                                     PolynaryExpr->exprBase.base.stringNo,
                                     clvOPCODE_LOAD,
                                     &intermIOperands[2],
                                     &OperandsParameters[1].rOperands[0],
                                     &intermROperands[5]));
        /* dst[r6] = r2 */
        gcmONERROR(clGenStoreCode(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  &intermROperands[2],
                                  lOperand,
                                  clmGenCodeDataType(T_UCHAR),
                                  &intermROperands[6]));
    }

    /* r3 = r1 + 1 */
    status = clGenArithmeticExprCode(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_ADD,
                        &intermIOperands[3],
                        &intermROperands[1],
                        &oneROperand);
    if (gcmIS_ERROR(status)) return status;

    /* r1 = r3 */
    status = clGenGenericCode1(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_ASSIGN,
                        &intermIOperands[1],
                        &intermROperands[3]);
    if (gcmIS_ERROR(status)) return status;

    /* r1 >= elementSize ? */
    status = clGenSelectionCompareConditionCode(Compiler,
                                                CodeGenerator,
                                                &selectionContextLoopBack,
                                                PolynaryExpr->exprBase.base.lineNo,
                                                PolynaryExpr->exprBase.base.stringNo,
                                                clvCONDITION_GREATER_THAN_EQUAL,
                                                &intermROperands[1],
                                                &elementSizeROperand);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionTrueOperandBegin(Compiler,
                                            CodeGenerator,
                                            &selectionContextLoopBack);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionTrueOperandEnd(Compiler,
                                    CodeGenerator,
                                    &selectionContextLoopBack,
                                    gcvFALSE);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionEnd(Compiler,
                                CodeGenerator,
                                &selectionContextLoopBack);
    if (gcmIS_ERROR(status)) return status;

    /* r3 = r4 + 1 */
    status = clGenArithmeticExprCode(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_ADD,
                        &intermIOperands[3],
                        &intermROperands[4],
                        &oneROperand);
    if (gcmIS_ERROR(status)) return status;

    /* r4 = r3 */
    status = clGenGenericCode1(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        clvOPCODE_ASSIGN,
                        &intermIOperands[4],
                        &intermROperands[3]);
    if (gcmIS_ERROR(status)) return status;

    /* r4 >= n ? */
    status = clGenSelectionCompareConditionCode(Compiler,
                                                CodeGenerator,
                                                &selectionContextOuterLoopBack,
                                                PolynaryExpr->exprBase.base.lineNo,
                                                PolynaryExpr->exprBase.base.stringNo,
                                                clvCONDITION_GREATER_THAN_EQUAL,
                                                &intermROperands[4],
                                                &OperandsParameters[2].rOperands[0]);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionTrueOperandBegin(Compiler,
                                            CodeGenerator,
                                            &selectionContextOuterLoopBack);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionTrueOperandEnd(Compiler,
                                    CodeGenerator,
                                    &selectionContextOuterLoopBack,
                                    gcvFALSE);
    if (gcmIS_ERROR(status)) return status;

    status = clDefineSelectionEnd(Compiler,
                                CodeGenerator,
                                &selectionContextOuterLoopBack);
    if (gcmIS_ERROR(status)) return status;

    /* return input parameter event as output */
    status = clGenGenericCode1(Compiler,
                PolynaryExpr->exprBase.base.lineNo,
                PolynaryExpr->exprBase.base.stringNo,
                clvOPCODE_ASSIGN,
                IOperand,
                &OperandsParameters[4].rOperands[0]);
    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    return status;
}

static gceSTATUS
_GenWaitGroupEventsCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);

    /* Nothing to do, all built-in functions are synchronous */
    return gcvSTATUS_OK;
}

static gceSTATUS
_GenPrefetchCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandCount == 2);
    gcmASSERT(OperandsParameters);

    /* Nothing to do, no need to prefetch into global cache */
    return gcvSTATUS_OK;
}

static gceSTATUS
_GenAtomCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS    status;
    cltPOOL_STRING  functionName;
    cleOPCODE   opcode = clvOPCODE_INVALID;
    clsROPERAND *cmpOperand = gcvNULL;
    clsROPERAND *valOperand = gcvNULL;
    clsROPERAND constOne[1];

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandsParameters);
    gcmASSERT(IOperand);

    if(!cldSupportOpenCLAtomicExtension || !CodeGenerator->supportAtomic) {
       gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvREPORT_ERROR,
                                       "atomic function \"%s\" not supported",
                                       PolynaryExpr->funcName->symbol));
       return gcvSTATUS_INVALID_DATA;
    }

    functionName = PolynaryExpr->funcName->symbol;
    if ((strcmp(functionName, "atomic_add") == 0) ||
        (strcmp(functionName, "atom_add") == 0))
    {
        gcmASSERT(OperandCount == 2);
        opcode = clvOPCODE_ATOMADD;
        valOperand = &OperandsParameters[1].rOperands[0];
    }
    else if ((strcmp(functionName, "atomic_sub") == 0) ||
             (strcmp(functionName, "atom_sub") == 0))
    {
        gcmASSERT(OperandCount == 2);
        opcode = clvOPCODE_ATOMSUB;
        valOperand = &OperandsParameters[1].rOperands[0];
    }
    else if ((strcmp(functionName, "atomic_inc") == 0) ||
             (strcmp(functionName, "atom_inc") == 0))
    {
        gcmASSERT(OperandCount == 1);
        opcode = clvOPCODE_ATOMADD;
        clsROPERAND_InitializeScalarConstant(constOne,
                                             clmGenCodeDataType(T_INT),
                                             int,
                                             1);
        valOperand = constOne;
    }
    else if ((strcmp(functionName, "atomic_dec") == 0) ||
             (strcmp(functionName, "atom_dec") == 0))
    {
        gcmASSERT(OperandCount == 1);
        opcode = clvOPCODE_ATOMSUB;
        clsROPERAND_InitializeScalarConstant(constOne,
                                             clmGenCodeDataType(T_INT),
                                             int,
                                             1);
        valOperand = constOne;
    }
    else if ((strcmp(functionName, "atomic_xchg") == 0) ||
             (strcmp(functionName, "atom_xchg") == 0))
    {
        gcmASSERT(OperandCount == 2);
        opcode = clvOPCODE_ATOMXCHG;
        valOperand = &OperandsParameters[1].rOperands[0];
    }
    else if ((strcmp(functionName, "atomic_cmpxchg") == 0) ||
             (strcmp(functionName, "atom_cmpxchg") == 0))
    {
        gcmASSERT(OperandCount == 3);
        opcode = clvOPCODE_ATOMCMPXCHG;
        cmpOperand = &OperandsParameters[1].rOperands[0];
        valOperand = &OperandsParameters[2].rOperands[0];
    }
    else if ((strcmp(functionName, "atomic_min") == 0) ||
             (strcmp(functionName, "atom_min") == 0))
    {
        gcmASSERT(OperandCount == 2);
        opcode = clvOPCODE_ATOMMIN;
        valOperand = &OperandsParameters[1].rOperands[0];
    }
    else if ((strcmp(functionName, "atomic_max") == 0) ||
             (strcmp(functionName, "atom_max") == 0))
    {
        gcmASSERT(OperandCount == 2);
        opcode = clvOPCODE_ATOMMAX;
        valOperand = &OperandsParameters[1].rOperands[0];
    }
    else if ((strcmp(functionName, "atomic_or") == 0) ||
             (strcmp(functionName, "atom_or") == 0))
    {
        gcmASSERT(OperandCount == 2);
        opcode = clvOPCODE_ATOMOR;
        valOperand = &OperandsParameters[1].rOperands[0];
    }
    else if ((strcmp(functionName, "atomic_and") == 0) ||
             (strcmp(functionName, "atom_and") == 0))
    {
        gcmASSERT(OperandCount == 2);
        opcode = clvOPCODE_ATOMAND;
        valOperand = &OperandsParameters[1].rOperands[0];
    }
    else if ((strcmp(functionName, "atomic_xor") == 0) ||
             (strcmp(functionName, "atom_xor") == 0))
    {
        gcmASSERT(OperandCount == 2);
        opcode = clvOPCODE_ATOMXOR;
        valOperand = &OperandsParameters[1].rOperands[0];
    }

    status = clGenAtomicCode(Compiler,
                             PolynaryExpr->exprBase.base.lineNo,
                             PolynaryExpr->exprBase.base.stringNo,
                             opcode,
                             IOperand,
                             &OperandsParameters[0].rOperands[0],
                             cmpOperand,
                             valOperand);
    if (gcmIS_ERROR(status)) return status;

    return gcvSTATUS_OK;
}

#if _SUPPORT_PRINTF_BUILTIN
#define _cldPrintfConversionSpecifiers "diouxXfFeEgGaAcsp"
#define _cldDigits                     "0123456789"

static gctBOOL
_IsConversionSpecValid(
    IN gctSTRING StartPtr,
    IN gctSTRING EndPtr,
    OUT gctUINT *VectorSize,
    OUT gctCHAR *Length
    )
{
    gctCHAR chr;
    gctBOOL flagDisallowed = gcvFALSE;
    gctBOOL minusDisallowed = gcvFALSE;
    gctBOOL plusDisallowed = gcvFALSE;
    gctBOOL spaceDisallowed = gcvFALSE;
    gctBOOL zeroDisallowed = gcvFALSE;
    gctBOOL sharpDisallowed = gcvFALSE;
    gctBOOL fieldWidthDisallowed = gcvFALSE;
    gctBOOL vectorSpecifierDisallowed = gcvFALSE;
    gctBOOL lengthModifierDisallowed = gcvFALSE;
    gctUINT vectorSize = 0;
    gctCHAR length[2] = {'\0', '\0'};

    while (StartPtr < EndPtr) {
         chr = *StartPtr++;
         switch(chr) {
         case '-':
         case '+':
         case ' ':
         case '#':
         case '0':
             if(flagDisallowed) return gcvFALSE;
             switch(chr) {
             case '-':
                 if(minusDisallowed) return gcvFALSE;
                 minusDisallowed = gcvTRUE;
                 break;

             case '+':
                 if(plusDisallowed) return gcvFALSE;
                 plusDisallowed = gcvTRUE;
                 break;

             case ' ':
                 if(spaceDisallowed) return gcvFALSE;
                 spaceDisallowed = gcvTRUE;
                 break;

             case '#':
                 if(sharpDisallowed) return gcvFALSE;
                 sharpDisallowed = gcvTRUE;
                 break;

             case '0':
                 if(zeroDisallowed) return gcvFALSE;
                 zeroDisallowed = gcvTRUE;
                 break;
             }
             break;

         case 'v':
             if(vectorSpecifierDisallowed) return gcvFALSE;
             if (StartPtr < EndPtr) {
                 chr = *StartPtr++;
                 switch(chr) {
                 case '1':
                     chr = *StartPtr++;
                     if(StartPtr < EndPtr) {
                         if(chr != '6') return gcvFALSE;
                     }
                     else return gcvFALSE;
                     vectorSize = 16;
                     break;

                 case '2':
                     vectorSize = 2;
                     break;

                 case '4':
                     vectorSize = 4;
                     break;

                 case '8':
                     vectorSize = 8;
                     break;

                 default:
                     return gcvFALSE;
                 }
             }
             else return gcvFALSE;
             vectorSpecifierDisallowed = gcvTRUE;
             flagDisallowed = gcvTRUE;
             fieldWidthDisallowed = gcvTRUE;
             break;

         case 'h':
             if(lengthModifierDisallowed) return gcvFALSE;
             length[0] = 'h';
             switch (*StartPtr) {
             case 'h':
                 length[1] = 'h';
                 StartPtr++;
                 break;

             case 'l':
                 if(vectorSpecifierDisallowed) {  /* hl applies to vector specifier appeared only */
                     length[1] = 'l';
                     StartPtr++;
                 }
                 else return gcvFALSE;
                 break;

             default:
                 break;
             }
             vectorSpecifierDisallowed = gcvTRUE;
             lengthModifierDisallowed  = gcvTRUE;
             flagDisallowed = gcvTRUE;
             fieldWidthDisallowed = gcvTRUE;
             break;

         case 'l':
             if(lengthModifierDisallowed) return gcvFALSE;
             length[0] = 'l';
             vectorSpecifierDisallowed = gcvTRUE;
             lengthModifierDisallowed  = gcvTRUE;
             flagDisallowed = gcvTRUE;
             fieldWidthDisallowed = gcvTRUE;
             break;

         default:
             if(fieldWidthDisallowed) return gcvFALSE;
             else {
                 gctBOOL hasFieldWidth = gcvFALSE;
                 gctBOOL hasPrecision = gcvFALSE;
                 gctBOOL more;
                 gctSTRING matchPtr;
                 do {
                     matchPtr = (gctSTRING) _cldDigits;
                     more = gcvFALSE;
                     while(*matchPtr) {
                         if(chr == *matchPtr++) {
                             more = gcvTRUE;
                             hasFieldWidth = gcvTRUE;
                             break;
                         }
                     }
                     if(more) {
                         if(StartPtr < EndPtr) {
                             chr = *StartPtr++;
                         }
                         else break;
                     }
                     else {
                         break;
                     }
                 } while(gcvTRUE);

                 /* check for precision */
                 if(chr == '.') {
                     while(StartPtr < EndPtr) {
                         chr = *StartPtr;
                         matchPtr = (gctSTRING) _cldDigits;
                         more = gcvFALSE;
                         while(*matchPtr) {
                             if(chr == *matchPtr++) {
                                 more = gcvTRUE;
                                 hasPrecision = gcvTRUE;
                                 break;
                             }
                         }
                         if(!more) break;
                         StartPtr++;
                     }
                     if(!(hasFieldWidth || hasPrecision)) {
                         return gcvFALSE;
                     }
                 }
             }
             flagDisallowed = gcvTRUE;
             fieldWidthDisallowed = gcvTRUE;
             break;
         }
    }

    *VectorSize = vectorSize;
    Length[0] = length[0];
    Length[1] = length[1];
    return gcvTRUE;
}

static gctBOOL
_CheckVectorLength(
    IN clsDATA_TYPE *ArgType,
    IN gctUINT  VectorSize,
    IN gctCHAR  *Length
    )
{
    gctBOOL matched = gcvFALSE;
    cltELEMENT_TYPE  elementType;

    elementType = clmDATA_TYPE_elementType_GET(ArgType);

    if(VectorSize == clmDATA_TYPE_vectorSize_NOCHECK_GET(ArgType)) {
       switch(Length[0]) {
       case 'h':
           switch(Length[1]) {
           case '\0':
               if(elementType == clvTYPE_SHORT ||
                  elementType == clvTYPE_USHORT ||
                  (VectorSize > 1 &&
                   elementType == clvTYPE_HALF)) {
                   matched = gcvTRUE;
               }
               break;

           case 'h':
               if(clmIsElementTypeChar(elementType)) {
                   matched = gcvTRUE;
               }
               break;

           case 'l':
               if(VectorSize > 1 &&
                 (elementType == clvTYPE_INT ||
                  elementType == clvTYPE_UINT ||
                  elementType == clvTYPE_FLOAT)) {
                   matched = gcvTRUE;
               }
               break;

           default:
               break;
           }
           break;

       case 'l':
           if(clmDATA_TYPE_IsHighPrecision(ArgType)) {
               matched = gcvTRUE;
           }
           break;

       default:
           matched = gcvTRUE;
           break;
       }
    }

    return matched;
}


static clsGEN_CODE_DATA_TYPE
_PromoteType(
    IN clsGEN_CODE_DATA_TYPE DataType
    )
{
    if(clmGEN_CODE_IsHighPrecisionDataType(DataType)) return DataType;
    else {
        clsGEN_CODE_DATA_TYPE baseType;
        cltELEMENT_TYPE elementType;

        elementType = clmGEN_CODE_elementType_GET(DataType);

        if(clmIsElementTypeSigned(elementType)) {
            baseType = clmGenCodeDataType(T_INT);
        }
        else if(clmIsElementTypeUnsigned(elementType)) {
            baseType = clmGenCodeDataType(T_UINT);
        }
        else { /* float assumed */
            baseType = clmGenCodeDataType(T_FLOAT);
        }
        if(gcIsScalarDataType(DataType)) {
            return baseType;
        }
        else {
            return gcConvScalarToVectorDataType(baseType,
                                                gcGetDataTypeComponentCount(DataType));
        }
    }
}

static gceSTATUS
_GenPrintfCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    clsNAME *constantVar;
    gctINT formatStringOffset;
    gctINT operandsNeeded = 0;
    gctSTRING formatString;
    gctSTRING bufPtr;
    gctSTRING buffer = gcvNULL;
    gctINT i;
    clsROPERAND constantROperand[1];
    clsROPERAND offsetOperand[1];
    clsLOPERAND tempLOperand[1];
    clsROPERAND tempROperand[1];
    clsROPERAND printfMaxEnd[1];
    clsIOPERAND tempIOperand[1];
    clsLOPERAND printfStart[1];
    clsROPERAND constantOneROperand[1];
    clsROPERAND constantZeroROperand[1];
    clsROPERAND *highPrecisionFlag;
    gctINT printfBufferOffset;
    gctLABEL endLabel;
    cloIR_EXPR arg;

    /* Verify the arguments. */
    clmVERIFY_OBJECT(Compiler, clvOBJ_COMPILER);
    clmVERIFY_IR_OBJECT(PolynaryExpr, clvIR_POLYNARY_EXPR);
    gcmASSERT(OperandsParameters);
    gcmASSERT(OperandCount != 0);
    gcmASSERT(IOperand);

    if(!clmHasRightLanguageVersion(Compiler, _cldCL1Dot2)) {
       gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                       PolynaryExpr->exprBase.base.lineNo,
                                       PolynaryExpr->exprBase.base.stringNo,
                                       clvREPORT_ERROR,
                                       "printf function \"%s\" not supported",
                                       PolynaryExpr->funcName->symbol));
       return gcvSTATUS_INVALID_DATA;
    }

    cloCOMPILER_SetNeedPrintfMemory(Compiler);

    /* do check arguments */
    arg = slsDLINK_NODE_Next(&PolynaryExpr->operands->members, struct _cloIR_EXPR);

    formatStringOffset = OperandsParameters[0].dataTypes[0].savedByteOffset;

    status = cloCOMPILER_FindConstantVariable(Compiler,
                                              formatStringOffset,
                                              &constantVar);
    if (gcmIS_ERROR(status)) return status;

    formatStringOffset -= clmNAME_VariableMemoryOffset_NOCHECK_GET(constantVar);
    if(clmDECL_IsUnderlyingStructOrUnion(&constantVar->decl)) {
        formatString = constantVar->u.variableInfo.u.constant->buffer + formatStringOffset;
    }
    else {
        cltELEMENT_TYPE elementType;
        gctPOINTER pointer;

        elementType = constantVar->decl.dataType->elementType;
        gcmASSERT(elementType == clvTYPE_CHAR || elementType == clvTYPE_UCHAR);
        status = cloCOMPILER_Allocate(Compiler,
                                      constantVar->u.variableInfo.u.constant->valueCount - formatStringOffset + 1,
                                      &pointer);
        if (gcmIS_ERROR(status)) return status;
        buffer = pointer;
        bufPtr = buffer;
        for(i = formatStringOffset; i < (gctINT)constantVar->u.variableInfo.u.constant->valueCount; i++) {
            *bufPtr++ = (gctCHAR)constantVar->u.variableInfo.u.constant->values[i].intValue;
        }
        *bufPtr = '\0';
        formatString = buffer;
    }

    bufPtr = formatString;
    while(*bufPtr) {
        gctCHAR c = *bufPtr++;

        if(c == '%') {
            if(*bufPtr == '%') {
                bufPtr++;
                continue;
            }
            else { /*indentify the conversion specification */
                gctSTRING startPtr = bufPtr;
                gctBOOL found = gcvFALSE;
                gctSTRING matchPtr;
                gctUINT vectorSize = 0;
                gctCHAR length[2];

                while(*bufPtr) {
                    c = *bufPtr++;
                    matchPtr = (gctSTRING) _cldPrintfConversionSpecifiers;
                    while(*matchPtr) {
                        if(c != *matchPtr++) continue;
                        /* TO DO - Analysis the conversion specification for error */
                        if(_IsConversionSpecValid(startPtr,
                                                  bufPtr - 1,
                                                  &vectorSize,
                                                  length)) {
                            found = gcvTRUE;
                            operandsNeeded++;
                        }
                        goto END_FIND;
                    }
                }
END_FIND:
                if(!found) {
                    gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                    PolynaryExpr->exprBase.base.lineNo,
                                                    PolynaryExpr->exprBase.base.stringNo,
                                                    clvREPORT_ERROR,
                                                    "error in printf function conversion specification \"%s\"",
                                                    formatString));
                    status = gcvSTATUS_INVALID_DATA;
                    gcmONERROR(gcvSTATUS_INVALID_DATA);
                }
                else if((gctINT)OperandCount <= operandsNeeded) {
                    gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                    PolynaryExpr->exprBase.base.lineNo,
                                                    PolynaryExpr->exprBase.base.stringNo,
                                                    clvREPORT_ERROR,
                                                    "insufficient arguments for the printf function format string \"%s\"",
                                                    formatString));
                    status = gcvSTATUS_INVALID_DATA;
                    gcmONERROR(gcvSTATUS_INVALID_DATA);
                }
                else {
                    cltELEMENT_TYPE elementType;
                    gctBOOL matched = gcvFALSE;

                    arg = slsDLINK_NODE_Next((slsDLINK_NODE *)arg, struct _cloIR_EXPR);
                    gcmASSERT((slsDLINK_NODE *)arg != &PolynaryExpr->operands->members);

                    elementType = clmDATA_TYPE_elementType_GET(arg->decl.dataType);
                    switch(c) {
                    case 'd':
                    case 'i':
                        if((clmIsElementTypeSigned(elementType) ||
                           (cloIR_OBJECT_GetType(&arg->base) == clvIR_CONSTANT &&
                            clmIsElementTypeUnsigned(elementType))) &&
                           _CheckVectorLength(arg->decl.dataType, vectorSize, length)) {
                            matched = gcvTRUE;
                        }
                        break;

                    case 'o':
                    case 'u':
                    case 'x':
                    case 'X':
                        if((clmIsElementTypeUnsigned(elementType) ||
                           (cloIR_OBJECT_GetType(&arg->base) == clvIR_CONSTANT &&
                            clmIsElementTypeSigned(elementType))) &&
                           _CheckVectorLength(arg->decl.dataType, vectorSize, length)) {
                            matched = gcvTRUE;
                        }
                        break;

                    case 'f':
                    case 'F':
                    case 'e':
                    case 'E':
                    case 'g':
                    case 'G':
                    case 'a':
                    case 'A':
                        if(clmIsElementTypeFloating(elementType) &&
                           _CheckVectorLength(arg->decl.dataType, vectorSize, length)) {
                            matched = gcvTRUE;
                        }
                        break;

                    case 'c':
                        if(vectorSize > 1) {
                            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                            PolynaryExpr->exprBase.base.lineNo,
                                                            PolynaryExpr->exprBase.base.stringNo,
                                                            clvREPORT_ERROR,
                                                            "invalid use of vector specifier on 'c' conversion specifier in format string \"%s\"",
                                                            formatString));
                            status = gcvSTATUS_INVALID_DATA;
                            gcmONERROR(gcvSTATUS_INVALID_DATA);

                        }
                        else if(clmIsElementTypeInteger(elementType)) {
                            matched = gcvTRUE;
                        }
                        break;

                    case 's':
                        if(vectorSize > 1) {
                            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                            PolynaryExpr->exprBase.base.lineNo,
                                                            PolynaryExpr->exprBase.base.stringNo,
                                                            clvREPORT_ERROR,
                                                            "invalid use of vector specifier on 's' conversin specifier in format string \"%s\"",
                                                            formatString));
                            status = gcvSTATUS_INVALID_DATA;
                            gcmONERROR(gcvSTATUS_INVALID_DATA);

                        }
                        else if(clmDECL_IsPointerType(&arg->decl) &&
                                clmIsElementTypeChar(elementType)) {
                            matched = gcvTRUE;
                        }
                        else if(cloIR_OBJECT_GetType(&arg->base) == clvIR_CONSTANT &&
                                clmIsElementTypeInteger(elementType)) {
                            cloIR_CONSTANT constant;
                            constant = (cloIR_CONSTANT)(&arg->base);

                            if(constant->values[0].intValue == 0) {
                                clsROPERAND *rOperand;
                                rOperand = &OperandsParameters[operandsNeeded].rOperands[0];
                                gcmASSERT(!rOperand->isReg &&
                                          rOperand->u.constant.valueCount == 1);
                                rOperand->u.constant.values[0].uintValue = (gctUINT)~0;
                                matched = gcvTRUE;
                            }
                        }
                        break;

                    case 'p':
                        if(vectorSize > 1) {
                            gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                            PolynaryExpr->exprBase.base.lineNo,
                                                            PolynaryExpr->exprBase.base.stringNo,
                                                            clvREPORT_ERROR,
                                                            "invalid use of vector specifier on 'p' conversin specifier in format string \"%s\"",
                                                            formatString));
                            status = gcvSTATUS_INVALID_DATA;
                            gcmONERROR(gcvSTATUS_INVALID_DATA);

                        }
                        else if(clmDECL_IsPointerType(&arg->decl)) {
                            matched = gcvTRUE;
                        }
                        break;

                    default:
                        gcmASSERT(0);
                        break;
                    }
                    if(!matched) {
                        gcmVERIFY_OK(cloCOMPILER_Report(Compiler,
                                                        PolynaryExpr->exprBase.base.lineNo,
                                                        PolynaryExpr->exprBase.base.stringNo,
                                                        clvREPORT_ERROR,
                                                        "argument data type mismatch with corresponding conversion specifier in the printf function format string \"%s\"",
                                                        formatString));
                        status = gcvSTATUS_INVALID_DATA;
                        gcmONERROR(gcvSTATUS_INVALID_DATA);
                    }
                }
            }
        }
    }

    printfBufferOffset = 4;
    for(i = 1; i < (operandsNeeded + 1); i++) {
        gctINT componentCount;

        componentCount = gcGetDataTypeTargetComponentCount(OperandsParameters[i].dataTypes[0].def);
        if(clmGEN_CODE_IsHighPrecisionDataType(OperandsParameters[i].dataTypes[0].def)) {
            printfBufferOffset += componentCount * 8 + 4;
        }
        else printfBufferOffset += componentCount * 4 + 4;
    }

    clsIOPERAND_Initialize(tempIOperand,
                           clmGenCodeDataType(T_UINT),
                           cldPrintfStartMemoryAddressRegIndex);

    clsROPERAND_InitializeIntOrIVecConstant(constantROperand,
                                            clmGenCodeDataType(T_UINT),
                                            printfBufferOffset);

    clsROPERAND_InitializeUsingIOperand(tempROperand, tempIOperand);

    gcmONERROR(clGenGenericCode2(Compiler,
                                 PolynaryExpr->exprBase.base.lineNo,
                                 PolynaryExpr->exprBase.base.stringNo,
                                 clvOPCODE_ADD,
                                 tempIOperand,
                                 tempROperand,
                                 constantROperand));

    clsLOPERAND_InitializeUsingIOperand(printfStart, tempIOperand);

    clsROPERAND_InitializeIntOrIVecConstant(constantROperand,
                                            clmGenCodeDataType(T_INT),
                                            -1);
    clsLOPERAND_InitializeUsingIOperand(tempLOperand, IOperand);
    gcmONERROR(clGenAssignCode(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               tempLOperand,
                               constantROperand));

    endLabel = clNewLabel(Compiler);
    clsROPERAND_InitializeTempReg(printfMaxEnd,
                                  clvQUALIFIER_NONE,
                                  clmGenCodeDataType(T_UINT),
                                  cldPrintfEndMemoryAddressRegIndex);
    gcmONERROR(clGenCompareJumpCode(Compiler,
                                    CodeGenerator,
                                    PolynaryExpr->exprBase.base.lineNo,
                                    PolynaryExpr->exprBase.base.stringNo,
                                    endLabel,
                                    gcvTRUE,
                                    clvCONDITION_LESS_THAN,
                                    printfMaxEnd,
                                    tempROperand));

    clsROPERAND_InitializeIntOrIVecConstant(constantROperand,
                                            clmGenCodeDataType(T_INT),
                                            OperandsParameters[0].dataTypes[0].savedByteOffset);
    printfBufferOffset = -printfBufferOffset;
    clsROPERAND_InitializeIntOrIVecConstant(offsetOperand,
                                            clmGenCodeDataType(T_INT),
                                            printfBufferOffset);
    gcmONERROR(clGenStoreCode(Compiler,
                              PolynaryExpr->exprBase.base.lineNo,
                              PolynaryExpr->exprBase.base.stringNo,
                              constantROperand,
                              printfStart,
                              clmGenCodeDataType(T_INT),
                              offsetOperand));

    clsROPERAND_InitializeIntOrIVecConstant(constantOneROperand,
                                            clmGenCodeDataType(T_INT),
                                            (gctINT)1);

    clsROPERAND_InitializeIntOrIVecConstant(constantZeroROperand,
                                            clmGenCodeDataType(T_INT),
                                            (gctINT)0);
    printfBufferOffset += 4;
    arg = slsDLINK_NODE_Next(&PolynaryExpr->operands->members, struct _cloIR_EXPR);
    for(i = 1; i < (operandsNeeded + 1); i++) {
        clsROPERAND *rOperand;

        highPrecisionFlag = constantZeroROperand;
        if(clmGEN_CODE_IsHighPrecisionDataType(OperandsParameters[i].dataTypes[0].def)) {
            highPrecisionFlag = constantOneROperand;
        }
        clsROPERAND_InitializeIntOrIVecConstant(offsetOperand,
                                                clmGenCodeDataType(T_INT),
                                                printfBufferOffset);
        gcmONERROR(clGenStoreCode(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  highPrecisionFlag,
                                  printfStart,
                                  clmGenCodeDataType(T_INT),
                                  offsetOperand));

        printfBufferOffset += 4;

        arg = slsDLINK_NODE_Next((slsDLINK_NODE *)arg, struct _cloIR_EXPR);
        gcmASSERT((slsDLINK_NODE *)arg != &PolynaryExpr->operands->members);

        if(clmDECL_IsPointerType(&arg->decl) &&
           clmIsElementTypeChar(arg->decl.dataType->elementType)) {
            gctINT literalStringOffset;

            literalStringOffset = OperandsParameters[i].dataTypes[0].savedByteOffset;

            status = cloCOMPILER_FindConstantVariable(Compiler,
                                                      literalStringOffset,
                                                      &constantVar);
            if (gcmIS_ERROR(status)) return status;

            gcmASSERT(literalStringOffset >= clmNAME_VariableMemoryOffset_NOCHECK_GET(constantVar));

            clsROPERAND_InitializeIntOrIVecConstant(constantROperand,
                                                    clmGenCodeDataType(T_INT),
                                                    literalStringOffset);
            rOperand = constantROperand;
        }
        else {
            rOperand = &OperandsParameters[i].rOperands[0];
        }
        clsROPERAND_InitializeIntOrIVecConstant(offsetOperand,
                                                clmGenCodeDataType(T_INT),
                                                printfBufferOffset);
        gcmONERROR(clGenStoreCode(Compiler,
                                  PolynaryExpr->exprBase.base.lineNo,
                                  PolynaryExpr->exprBase.base.stringNo,
                                  rOperand,
                                  printfStart,
                                  _PromoteType(OperandsParameters[i].dataTypes[0].def),
                                  offsetOperand));

        printfBufferOffset += gcGetDataTypeTargetComponentCount(OperandsParameters[i].dataTypes[0].def) *
                              (highPrecisionFlag == constantOneROperand ? 8 : 4);
    }

    clsROPERAND_InitializeIntOrIVecConstant(constantROperand,
                                            clmGenCodeDataType(T_INT),
                                            0);

    clsLOPERAND_InitializeUsingIOperand(tempLOperand, IOperand);
    gcmONERROR(clGenAssignCode(Compiler,
                               PolynaryExpr->exprBase.base.lineNo,
                               PolynaryExpr->exprBase.base.stringNo,
                               tempLOperand,
                               constantROperand));
    status = clSetLabel(Compiler,
                        PolynaryExpr->exprBase.base.lineNo,
                        PolynaryExpr->exprBase.base.stringNo,
                        endLabel);
    if (gcmIS_ERROR(status)) return status;


OnError:
    if(buffer) {
       gcmVERIFY_OK(cloCOMPILER_Free(Compiler, buffer));
    }

    return status;
}
#endif

#endif /* __gc_cl_built_ins_common_h_ */
