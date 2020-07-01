/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_cl_built_ins_intrinsic_h_
#define __gc_cl_built_ins_intrinsic_h_

/******************************************
struct _clsINTRINCSIC_BUILTIN_FUNCTION
{
    cleEXTENSION    extension;
    gctCONST_STRING symbol;
    gceINTRINSICS_KIND intrinsicKind;
    gctCONST_STRING    nameInLibrary;
    gctINT          returnType;
    gctUINT         paramCount;
    gctINT          paramTypes[clmMAX_BUILT_IN_PARAMETER_COUNT];
    gctUINT8        ptrLevels[clmMAX_BUILT_IN_PARAMETER_COUNT];
    gctUINT8        typeConvertible[clmMAX_BUILT_IN_PARAMETER_COUNT];
    gctBOOL         isInline;
    gctBOOL         hasWriteArg;
    gctBOOL         passArgByRef;
    gctBOOL         hasVarArg;
}
*************************************************/

static clsINTRINSIC_BUILTIN_FUNCTION IntrinsicBuiltinFunctions[] =
{
    /* Intrinsic builtin functions */

#if IMPL_TRIG_OR_POW_FUNCS_AS_INTRINSICS
    {clvEXTENSION_NONE,  "sin",              gceINTRIN_source, "_viv_sin_float",      T_FLOAT,  1, {T_FLOAT},    {0}, {0}, 1},
    {clvEXTENSION_NONE,  "sin",              gceINTRIN_source, "_viv_sin_float2",     T_FLOAT2, 1, {T_FLOAT2},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "sin",              gceINTRIN_source, "_viv_sin_float3",     T_FLOAT3, 1, {T_FLOAT3},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "sin",              gceINTRIN_source, "_viv_sin_float4",     T_FLOAT4, 1, {T_FLOAT4},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "sin",              gceINTRIN_source, "_viv_sin_float8",     T_FLOAT8, 1, {T_FLOAT8},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "sin",              gceINTRIN_source, "_viv_sin_float16",    T_FLOAT16, 1, {T_FLOAT16},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "cos",              gceINTRIN_source, "_viv_cos_float",      T_FLOAT,  1, {T_FLOAT},    {0}, {0}, 1},
    {clvEXTENSION_NONE,  "cos",              gceINTRIN_source, "_viv_cos_float2",     T_FLOAT2, 1, {T_FLOAT2},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "cos",              gceINTRIN_source, "_viv_cos_float3",     T_FLOAT3, 1, {T_FLOAT3},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "cos",              gceINTRIN_source, "_viv_cos_float4",     T_FLOAT4, 1, {T_FLOAT4},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "cos",              gceINTRIN_source, "_viv_cos_float8",     T_FLOAT8, 1, {T_FLOAT8},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "cos",              gceINTRIN_source, "_viv_cos_float16",    T_FLOAT16, 1, {T_FLOAT16},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "tan",              gceINTRIN_source, "_viv_tan_float",      T_FLOAT,  1, {T_FLOAT},    {0}, {0}, 1},
    {clvEXTENSION_NONE,  "tan",              gceINTRIN_source, "_viv_tan_float2",     T_FLOAT2, 1, {T_FLOAT2},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "tan",              gceINTRIN_source, "_viv_tan_float3",     T_FLOAT3, 1, {T_FLOAT3},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "tan",              gceINTRIN_source, "_viv_tan_float4",     T_FLOAT4, 1, {T_FLOAT4},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "tan",              gceINTRIN_source, "_viv_tan_float8",     T_FLOAT8, 1, {T_FLOAT8},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "tan",              gceINTRIN_source, "_viv_tan_float16",    T_FLOAT16, 1, {T_FLOAT16},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "pow",              gceINTRIN_source, "_viv_pow_float",      T_FLOAT,  2, {T_FLOAT, T_FLOAT},    {0}, {0}, 1},
    {clvEXTENSION_NONE,  "pow",              gceINTRIN_source, "_viv_pow_float2",     T_FLOAT2, 2, {T_FLOAT2, T_FLOAT2},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "pow",              gceINTRIN_source, "_viv_pow_float3",     T_FLOAT3, 2, {T_FLOAT3, T_FLOAT3},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "pow",              gceINTRIN_source, "_viv_pow_float4",     T_FLOAT4, 2, {T_FLOAT4, T_FLOAT4},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "pow",              gceINTRIN_source, "_viv_pow_float8",     T_FLOAT8, 2, {T_FLOAT8, T_FLOAT8},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "pow",              gceINTRIN_source, "_viv_pow_float16",    T_FLOAT16, 2, {T_FLOAT16, T_FLOAT16},  {0}, {0}, 1},
#endif
    {clvEXTENSION_NONE,  "asin",              gceINTRIN_source, "_viv_asin_float",      T_FLOAT,  1, {T_FLOAT},    {0}, {0}, 1},
    {clvEXTENSION_NONE,  "asin",              gceINTRIN_source, "_viv_asin_float2",     T_FLOAT2, 1, {T_FLOAT2},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "asin",              gceINTRIN_source, "_viv_asin_float3",     T_FLOAT3, 1, {T_FLOAT3},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "asin",              gceINTRIN_source, "_viv_asin_float4",     T_FLOAT4, 1, {T_FLOAT4},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "asin",              gceINTRIN_source, "_viv_asin_float8",     T_FLOAT8, 1, {T_FLOAT8},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "asin",              gceINTRIN_source, "_viv_asin_float16",    T_FLOAT16, 1, {T_FLOAT16},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "acos",              gceINTRIN_source, "_viv_acos_float",      T_FLOAT,  1, {T_FLOAT},    {0}, {0}, 1},
    {clvEXTENSION_NONE,  "acos",              gceINTRIN_source, "_viv_acos_float2",     T_FLOAT2, 1, {T_FLOAT2},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "acos",              gceINTRIN_source, "_viv_acos_float3",     T_FLOAT3, 1, {T_FLOAT3},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "acos",              gceINTRIN_source, "_viv_acos_float4",     T_FLOAT4, 1, {T_FLOAT4},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "acos",              gceINTRIN_source, "_viv_acos_float8",     T_FLOAT8, 1, {T_FLOAT8},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "acos",              gceINTRIN_source, "_viv_acos_float16",    T_FLOAT16, 1, {T_FLOAT16},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "atan",              gceINTRIN_source, "_viv_atan_float",      T_FLOAT,  1, {T_FLOAT},    {0}, {0}, 1},
    {clvEXTENSION_NONE,  "atan",              gceINTRIN_source, "_viv_atan_float2",     T_FLOAT2, 1, {T_FLOAT2},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "atan",              gceINTRIN_source, "_viv_atan_float3",     T_FLOAT3, 1, {T_FLOAT3},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "atan",              gceINTRIN_source, "_viv_atan_float4",     T_FLOAT4, 1, {T_FLOAT4},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "atan",              gceINTRIN_source, "_viv_atan_float8",     T_FLOAT8, 1, {T_FLOAT8},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "atan",              gceINTRIN_source, "_viv_atan_float16",    T_FLOAT16, 1, {T_FLOAT16},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "atan2",             gceINTRIN_source, "_viv_atan2_float",     T_FLOAT,  2, {T_FLOAT, T_FLOAT},    {0}, {0}, 1},
    {clvEXTENSION_NONE,  "atan2",             gceINTRIN_source, "_viv_atan2_float2",    T_FLOAT2, 2, {T_FLOAT2, T_FLOAT2},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "atan2",             gceINTRIN_source, "_viv_atan2_float3",    T_FLOAT3, 2, {T_FLOAT3, T_FLOAT3},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "atan2",             gceINTRIN_source, "_viv_atan2_float4",    T_FLOAT4, 2, {T_FLOAT4, T_FLOAT4},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "atan2",             gceINTRIN_source, "_viv_atan2_float8",    T_FLOAT8, 2, {T_FLOAT8, T_FLOAT8},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "atan2",             gceINTRIN_source, "_viv_atan2_float16",   T_FLOAT16, 2, {T_FLOAT16, T_FLOAT16},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "mad_sat",           gceINTRIN_source, "_viv_madsat_long",   T_LONG,  3, {T_LONG,  T_LONG,  T_LONG},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "mad_sat",           gceINTRIN_source, "_viv_madsat_ulong",  T_ULONG, 3, {T_ULONG, T_ULONG, T_ULONG}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "mad_sat",           gceINTRIN_source, "_viv_madsat_long2",   T_LONG2,  3, {T_LONG2,  T_LONG2,  T_LONG2},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "mad_sat",           gceINTRIN_source, "_viv_madsat_ulong2",  T_ULONG2, 3, {T_ULONG2, T_ULONG2, T_ULONG2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "mad_sat",           gceINTRIN_source, "_viv_madsat_long3",   T_LONG3,  3, {T_LONG3,  T_LONG3,  T_LONG3},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "mad_sat",           gceINTRIN_source, "_viv_madsat_ulong3",  T_ULONG3, 3, {T_ULONG3, T_ULONG3, T_ULONG3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "mad_sat",           gceINTRIN_source, "_viv_madsat_long4",   T_LONG4,  3, {T_LONG4,  T_LONG4,  T_LONG4},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "mad_sat",           gceINTRIN_source, "_viv_madsat_ulong4",  T_ULONG4, 3, {T_ULONG4, T_ULONG4, T_ULONG4}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "mad_sat",           gceINTRIN_source, "_viv_madsat_long8",   T_LONG8,  3, {T_LONG8,  T_LONG8,  T_LONG8},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "mad_sat",           gceINTRIN_source, "_viv_madsat_ulong8",  T_ULONG8, 3, {T_ULONG8, T_ULONG8, T_ULONG8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "mad_sat",           gceINTRIN_source, "_viv_madsat_long16",   T_LONG16,  3, {T_LONG16,  T_LONG16,  T_LONG16},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "mad_sat",           gceINTRIN_source, "_viv_madsat_ulong16",  T_ULONG16, 3, {T_ULONG16, T_ULONG16, T_ULONG16}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_mul_long",     gceINTRIN_source, "_viv_mul_long",      T_LONG,  2, {T_LONG,  T_LONG},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_mul_ulong",    gceINTRIN_source, "_viv_mul_ulong",     T_ULONG, 2, {T_ULONG, T_ULONG}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_mulhi_long",   gceINTRIN_source, "_viv_mulhi_long",    T_LONG,  2, {T_LONG,  T_LONG},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_mulhi_ulong",  gceINTRIN_source, "_viv_mulhi_ulong",   T_ULONG, 2, {T_ULONG, T_ULONG}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "nextafter",         gceINTRIN_source, "_viv_nextafter",     T_FLOAT, 2, {T_FLOAT, T_FLOAT}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_image_query_width_image2d_t",   gceINTRIN_source,   "_viv_image_query_width_image2d_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_image_query_width_image3d_t",   gceINTRIN_source,   "_viv_image_query_width_image3d_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_image_query_width_image1d_t",   gceINTRIN_source,   "_viv_image_query_width_image1d_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_image_query_width_image2d_array_t",   gceINTRIN_source,   "_viv_image_query_width_image2d_array_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_image_query_width_image1d_array_t",   gceINTRIN_source,   "_viv_image_query_width_image1d_array_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_image_query_width_imageld_buffer_t",  gceINTRIN_source,   "_viv_image_query_width_image1d_buufer_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_image_query_height_image2d_t",  gceINTRIN_source,   "_viv_image_query_height_image2d_t",  T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_image_query_height_image3d_t",  gceINTRIN_source,   "_viv_image_query_height_image3d_t",  T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_image_query_height_image2d_array_t",  gceINTRIN_source,   "_viv_image_query_height_image2d_array_t",  T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_image_query_depth_image3d_t",   gceINTRIN_source,   "_viv_image_query_depth_image3d_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_image_query_format_image2d_t",   gceINTRIN_source,   "_viv_image_query_format_image2d_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_image_query_format_image3d_t",   gceINTRIN_source,   "_viv_image_query_format_image3d_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_image_query_format_image1d_t",   gceINTRIN_source,   "_viv_image_query_format_image1d_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_image_query_format_image2d_array_t",   gceINTRIN_source,   "_viv_image_query_format_image2d_array_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_image_query_format_image1d_array_t",   gceINTRIN_source,   "_viv_image_query_format_image1d_array_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_image_query_format_image1d_buffer_t",  gceINTRIN_source,   "_viv_image_query_format_image1d_buffer_t",  T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_image_query_order_image2d_t",   gceINTRIN_source,   "_viv_image_query_order_image2d_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_image_query_order_image3d_t",   gceINTRIN_source,   "_viv_image_query_order_image3d_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_image_query_order_image1d_t",   gceINTRIN_source,   "_viv_image_query_order_image1d_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_image_query_order_image2d_array_t",   gceINTRIN_source,   "_viv_image_query_order_image2d_arrary_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_image_query_order_image1d_array_t",   gceINTRIN_source,   "_viv_image_query_order_image1d_array_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_image_query_order_image1d_buffer_t",  gceINTRIN_source,   "_viv_image_query_order_image1d_buffer_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_image_query_size_image2d_t",   gceINTRIN_source,   "_viv_image_query_size_image2d_t",   T_INT2, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_image_query_size_image3d_t",   gceINTRIN_source,   "_viv_image_query_size_image3d_t",   T_INT4, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_image_query_size_image1d_t",   gceINTRIN_source,   "_viv_image_query_size_image1d_t",   T_INT2, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_image_query_array_size_image1d_array_t",   gceINTRIN_source,   "_viv_image_query_array_size_image1d_array_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_image_query_array_size_image2d_array_t",   gceINTRIN_source,   "_viv_image_query_array_size_image2d_array_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,    "vstore_half_rtz",      gceINTRIN_source,  "_viv_vstore_half_rtz", T_VOID, 3, {T_FLOAT, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore_half2_rtz",     gceINTRIN_source,  "_viv_vstore_half2_rtz", T_VOID, 3, {T_FLOAT2, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore_half3_rtz",     gceINTRIN_source,  "_viv_vstore_half3_rtz", T_VOID, 3, {T_FLOAT3, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore_half4_rtz",     gceINTRIN_source,  "_viv_vstore_half4_rtz", T_VOID, 3, {T_FLOAT4, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore_half8_rtz",     gceINTRIN_source,  "_viv_vstore_half8_rtz", T_VOID, 3, {T_FLOAT8, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore_half16_rtz",    gceINTRIN_source,  "_viv_vstore_half16_rtz", T_VOID, 3, {T_FLOAT16, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},

    {clvEXTENSION_NONE,    "vstore_half_rtp",      gceINTRIN_source, "_viv_vstore_half_rtp", T_VOID, 3, {T_FLOAT, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore_half2_rtp",     gceINTRIN_source, "_viv_vstore_half2_rtp", T_VOID, 3, {T_FLOAT2, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore_half3_rtp",     gceINTRIN_source, "_viv_vstore_half3_rtp", T_VOID, 3, {T_FLOAT3, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore_half4_rtp",     gceINTRIN_source, "_viv_vstore_half4_rtp", T_VOID, 3, {T_FLOAT4, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore_half8_rtp",     gceINTRIN_source, "_viv_vstore_half8_rtp", T_VOID, 3, {T_FLOAT8, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore_half16_rtp",    gceINTRIN_source, "_viv_vstore_half16_rtp", T_VOID, 3, {T_FLOAT16, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},

    {clvEXTENSION_NONE,    "vstore_half_rtn",      gceINTRIN_source, "_viv_vstore_half_rtn", T_VOID, 3, {T_FLOAT, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore_half2_rtn",     gceINTRIN_source, "_viv_vstore_half2_rtn", T_VOID, 3, {T_FLOAT2, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore_half3_rtn",     gceINTRIN_source, "_viv_vstore_half3_rtn", T_VOID, 3, {T_FLOAT3, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore_half4_rtn",     gceINTRIN_source, "_viv_vstore_half4_rtn", T_VOID, 3, {T_FLOAT4, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore_half8_rtn",     gceINTRIN_source, "_viv_vstore_half8_rtn", T_VOID, 3, {T_FLOAT8, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstore_half16_rtn",    gceINTRIN_source, "_viv_vstore_half16_rtn", T_VOID, 3, {T_FLOAT16, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},

    {clvEXTENSION_NONE,    "vstorea_half_rtz",      gceINTRIN_source,  "_viv_vstorea_half_rtz", T_VOID, 3, {T_FLOAT, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstorea_half2_rtz",     gceINTRIN_source,  "_viv_vstorea_half2_rtz", T_VOID, 3, {T_FLOAT2, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstorea_half3_rtz",     gceINTRIN_source,  "_viv_vstorea_half3_rtz", T_VOID, 3, {T_FLOAT3, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstorea_half4_rtz",     gceINTRIN_source,  "_viv_vstorea_half4_rtz", T_VOID, 3, {T_FLOAT4, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstorea_half8_rtz",     gceINTRIN_source,  "_viv_vstorea_half8_rtz", T_VOID, 3, {T_FLOAT8, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstorea_half16_rtz",    gceINTRIN_source,  "_viv_vstorea_half16_rtz", T_VOID, 3, {T_FLOAT16, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},

    {clvEXTENSION_NONE,    "vstorea_half_rtp",      gceINTRIN_source, "_viv_vstorea_half_rtp", T_VOID, 3, {T_FLOAT, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstorea_half2_rtp",     gceINTRIN_source, "_viv_vstorea_half2_rtp", T_VOID, 3, {T_FLOAT2, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstorea_half3_rtp",     gceINTRIN_source, "_viv_vstorea_half3_rtp", T_VOID, 3, {T_FLOAT3, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstorea_half4_rtp",     gceINTRIN_source, "_viv_vstorea_half4_rtp", T_VOID, 3, {T_FLOAT4, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstorea_half8_rtp",     gceINTRIN_source, "_viv_vstorea_half8_rtp", T_VOID, 3, {T_FLOAT8, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstorea_half16_rtp",    gceINTRIN_source, "_viv_vstorea_half16_rtp", T_VOID, 3, {T_FLOAT16, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},

    {clvEXTENSION_NONE,    "vstorea_half_rtn",      gceINTRIN_source, "_viv_vstorea_half_rtn", T_VOID, 3, {T_FLOAT, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstorea_half2_rtn",     gceINTRIN_source, "_viv_vstorea_half2_rtn", T_VOID, 3, {T_FLOAT2, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstorea_half3_rtn",     gceINTRIN_source, "_viv_vstorea_half3_rtn", T_VOID, 3, {T_FLOAT3, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstorea_half4_rtn",     gceINTRIN_source, "_viv_vstorea_half4_rtn", T_VOID, 3, {T_FLOAT4, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstorea_half8_rtn",     gceINTRIN_source, "_viv_vstorea_half8_rtn", T_VOID, 3, {T_FLOAT8, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
    {clvEXTENSION_NONE,    "vstorea_half16_rtn",    gceINTRIN_source, "_viv_vstorea_half16_rtn", T_VOID, 3, {T_FLOAT16, T_SIZE_T, T_HALF}, {0, 0, 1}, {0, 1, 0}, 1, 1},
};

#define _cldIntrinsicBuiltinFunctionCount (sizeof(IntrinsicBuiltinFunctions) / sizeof(clsINTRINSIC_BUILTIN_FUNCTION))

#endif /* __gc_cl_built_ins_intrinsics_h_ */
