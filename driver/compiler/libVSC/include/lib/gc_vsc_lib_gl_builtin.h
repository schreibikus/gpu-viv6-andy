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


#ifndef __gc_vsc_gl_builtin_lib_h_
#define __gc_vsc_gl_builtin_lib_h_

#define __BUILTIN_SHADER_LENGTH__ (65535 * 8)

extern gctSTRING gcLibFunc_Extension;
extern gctSTRING gcLibFunc_Extension_For_GL;
extern gctSTRING gcLibFunc_Extension_For_TexMS2DArray;
extern gctSTRING gcLibFunc_Extension_For_CubeMapArray;
extern gctSTRING gcLibFunc_Extension_For_TextureBuffer;
extern gctSTRING gcLibFunc_Extension_For_MSShading;
extern gctSTRING gcLibFunc_TextureBufferSize_For_OES;
extern gctSTRING gcLibFunc_TextureBufferSize_For_VK;
extern gctSTRING gcLibFunc_MathMacros;
extern gctSTRING gcLibFunc_BuiltinHeader;
extern gctSTRING gcLibFunc_BlendEquationHeader;
extern gctSTRING gcLibASIN_ACOS_Funcs_Common;
extern gctSTRING gcLibASIN_Funcs_halti0;
extern gctSTRING gcLibASIN_Funcs;
extern gctSTRING gcLibACOS_Funcs;
extern gctSTRING gcLibATAN_Funcs;
extern gctSTRING gcLibATAN2_Funcs;
extern gctSTRING gcLibASIN_ACOS_Funcs_halti2;
extern gctSTRING gcLibACOSH_Funcs;
extern gctSTRING gcLibTAN_Funcs_Halti;
extern gctSTRING gcLibTAN_Funcs_Common;
extern gctSTRING gcLibATAN_Funcs_halti2;
extern gctSTRING gcLibATAN2_Funcs_halti2;
extern gctSTRING gcLibASIN_ACOS_Funcs_halti5_fmaSupported;
extern gctSTRING gcLibASIN_ACOS_Funcs_halti5;
extern gctSTRING gcLibATAN_Funcs_halti5_fmaSupported;
extern gctSTRING gcLibATAN_Funcs_halti5;
extern gctSTRING gcLibATAN2_Funcs_halti5_fmaSupported;
extern gctSTRING gcLibATAN2_Funcs_halti5;
extern gctSTRING gcLibConvF32ToF16_Funcs;
extern gctSTRING gcLibQuantizeToF16_Funcs;
extern gctSTRING gcLibFindLSB_Func_1;
extern gctSTRING gcLibFindLSB_Func_2;
extern gctSTRING gcLibFindLSB_Func_3;
extern gctSTRING gcLibFindLSB_Func_4;
extern gctSTRING gcLibFindLSB_Func_5;
extern gctSTRING gcLibFindLSB_Func_6;
extern gctSTRING gcLibFindLSB_Func_7;
extern gctSTRING gcLibFindLSB_Func_8;
extern gctSTRING gcLibFindLSB_Func_1_hati4;
extern gctSTRING gcLibFindLSB_Func_2_hati4;
extern gctSTRING gcLibFindLSB_Func_3_hati4;
extern gctSTRING gcLibFindLSB_Func_4_hati4;
extern gctSTRING gcLibFindLSB_Func_5_hati4;
extern gctSTRING gcLibFindLSB_Func_6_hati4;
extern gctSTRING gcLibFindLSB_Func_7_hati4;
extern gctSTRING gcLibFindLSB_Func_8_hati4;
extern gctSTRING gcLibFindMSB_Func_1;
extern gctSTRING gcLibFindMSB_Func_2;
extern gctSTRING gcLibFindMSB_Func_3;
extern gctSTRING gcLibFindMSB_Func_4;
extern gctSTRING gcLibFindMSB_Func_5;
extern gctSTRING gcLibFindMSB_Func_6;
extern gctSTRING gcLibFindMSB_Func_7;
extern gctSTRING gcLibFindMSB_Func_8;
extern gctSTRING gcLibFindMSB_Func_1_hati4;
extern gctSTRING gcLibFindMSB_Func_2_hati4;
extern gctSTRING gcLibFindMSB_Func_3_hati4;
extern gctSTRING gcLibFindMSB_Func_4_hati4;
extern gctSTRING gcLibFindMSB_Func_5_hati4;
extern gctSTRING gcLibFindMSB_Func_6_hati4;
extern gctSTRING gcLibFindMSB_Func_7_hati4;
extern gctSTRING gcLibFindMSB_Func_8_hati4;
extern gctSTRING gcLibBitfieldReverse_Func_1;
extern gctSTRING gcLibBitfieldReverse_Func_2;
extern gctSTRING gcLibBitfieldReverse_Func_3;
extern gctSTRING gcLibBitfieldReverse_Func_4;
extern gctSTRING gcLibBitfieldReverse_Func_5;
extern gctSTRING gcLibBitfieldReverse_Func_6;
extern gctSTRING gcLibBitfieldReverse_Func_7;
extern gctSTRING gcLibBitfieldReverse_Func_8;
extern gctSTRING gcLibBitfieldReverse_Func_1_hati4;
extern gctSTRING gcLibBitfieldReverse_Func_2_hati4;
extern gctSTRING gcLibBitfieldReverse_Func_3_hati4;
extern gctSTRING gcLibBitfieldReverse_Func_4_hati4;
extern gctSTRING gcLibBitfieldReverse_Func_5_hati4;
extern gctSTRING gcLibBitfieldReverse_Func_6_hati4;
extern gctSTRING gcLibBitfieldReverse_Func_7_hati4;
extern gctSTRING gcLibBitfieldReverse_Func_8_hati4;
extern gctSTRING gcLibBitfieldExtract_Func;
extern gctSTRING gcLibBitfieldExtract_Func_halti4;
extern gctSTRING gcLibBitfieldInsert_Func;
extern gctSTRING gcLibBitfieldInsert_Func_halti4;
extern gctSTRING gcLibUaddCarry_Func;
extern gctSTRING gcLibUaddCarry_Func_hati4;
extern gctSTRING gcLibUaddCarry_Func_VK_hati4;
extern gctSTRING gcLibLDEXP_Func;
extern gctSTRING gcLibFREXP_Func;
extern gctSTRING gcLibFREXPSTRUCT_Func;
extern gctSTRING gcLibUsubBorrow_Func;
extern gctSTRING gcLibUsubBorrow_Func_VK;
extern gctSTRING gcLibPack_Func;
extern gctSTRING gcLibUnpack_Func;
extern gctSTRING gcLibUmulExtended_Func;
extern gctSTRING gcLibImulExtended_Func;
extern gctSTRING gcLibUmulExtended_Func_VK;
extern gctSTRING gcLibImulExtended_Func_VK;
extern gctSTRING gcLibMODF_Func;
extern gctSTRING gcLibFMA_Func_fmaNotSupported;
extern gctSTRING gcLibFMA_Func_fmaSupported;
extern gctSTRING gcLibREFLECT_Func_float;
extern gctSTRING gcLibREFLECT_Func_vec2;
extern gctSTRING gcLibREFLECT_Func_vec3;
extern gctSTRING gcLibREFLECT_Func_vec4;
extern gctSTRING gcLibREFLECT_Func_float_fmaSupported;
extern gctSTRING gcLibREFLECT_Func_vec2_fmaSupported;
extern gctSTRING gcLibREFLECT_Func_vec3_fmaSupported;
extern gctSTRING gcLibREFLECT_Func_vec4_fmaSupported;
extern gctSTRING gcLibMatrixTranspose_Func;
extern gctSTRING gcLibNMin_Func;
extern gctSTRING gcLibNMax_Func;
extern gctSTRING gcLibNClamp_Func;
extern gctSTRING gcLibTextureSize_Func_1;
extern gctSTRING gcLibTextureSize_Func_2;
extern gctSTRING gcLibTextureSize_Func_3;
extern gctSTRING gcLibTextureSize_Func_4;
extern gctSTRING gcLibTextureSize_Func_5;
extern gctSTRING gcLibTextureSize_Func_6;
extern gctSTRING gcLibTextureSize_Func_7;
extern gctSTRING gcLibTextureSize_Func_8;
extern gctSTRING gcLibTextureSize_Func_9;
extern gctSTRING gcLibTextureSize_Func_10;
extern gctSTRING gcLibTextureSize_Func_11;
extern gctSTRING gcLibTextureSize_Func_12;
extern gctSTRING gcLibTextureSize_Func_13;
extern gctSTRING gcLibTextureSize_Func_14;
extern gctSTRING gcLibTextureSize_Func_15;
extern gctSTRING gcLibTextureSize_Func_16;
extern gctSTRING gcLibTextureSize_Func_17;
extern gctSTRING gcLibTextureSize_Func_18;
extern gctSTRING gcLibTextureSize_Func_19;
extern gctSTRING gcLibTextureSize_Func_20;
extern gctSTRING gcLibTextureSize_Func_21;
extern gctSTRING gcLibTextureSize_Func_22;
extern gctSTRING gcLibTextureSize_Func_23;
extern gctSTRING gcLibTextureSize_Func_24;
extern gctSTRING gcLibTextureSize_Func_25;
extern gctSTRING gcLibTextureSize_Func_26;
extern gctSTRING gcLibTextureSize_Func_27;
extern gctSTRING gcLibTextureSize_Func_28;
extern gctSTRING gcLibTextureSize_Func_29;
extern gctSTRING gcLibTextureCommon_Func;
extern gctSTRING gcLibTextureGatherCommon_Func_1;
extern gctSTRING gcLibTextureGather_Func_1;
extern gctSTRING gcLibTextureGather_Func_2;
extern gctSTRING gcLibTextureGather_Func_3;
extern gctSTRING gcLibTextureGather_Func_4;
extern gctSTRING gcLibTextureGather_Func_5;
extern gctSTRING gcLibTextureGather_Func_6;
extern gctSTRING gcLibTextureGather_Func_7;
extern gctSTRING gcLibTextureGather_Func_8;
extern gctSTRING gcLibTextureGather_Func_9;
extern gctSTRING gcLibTextureGather_Func_10;
extern gctSTRING gcLibTextureGather_Func_11;
extern gctSTRING gcLibTextureGather_Func_12;
extern gctSTRING gcLibTextureGather_Func_13;
extern gctSTRING gcLibTextureGather_Func_14;
extern gctSTRING gcLibTextureGather_Func_15;
extern gctSTRING gcLibTextureGather_Func_16;
extern gctSTRING gcLibTextureGather_Func_17;
extern gctSTRING gcLibTextureGather_Func_18;
extern gctSTRING gcLibTextureGather_Func_19;
extern gctSTRING gcLibTextureGather_Func_20;
extern gctSTRING gcLibTextureGather_Func_21;
extern gctSTRING gcLibTextureGather_Func_22;
extern gctSTRING gcLibTextureGather_Func_23;
extern gctSTRING gcLibTextureGather_Func_24;
extern gctSTRING gcLibTextureGather_Func_25;
extern gctSTRING gcLibTextureGather_Func_26;
extern gctSTRING gcLibTextureGather_Func_27;
extern gctSTRING gcLibTextureGather_Func_28;
extern gctSTRING gcLibTextureGather_Func_2_0;
extern gctSTRING gcLibTextureGather_Func_2_1;
extern gctSTRING gcLibTextureGather_Func_2_2;
extern gctSTRING gcLibTextureGather_Func_2_3;
extern gctSTRING gcLibTextureGather_Func_2_4;
extern gctSTRING gcLibTextureGather_Func_2_5;
extern gctSTRING gcLibTextureGather_Func_2_6;
extern gctSTRING gcLibTextureGather_Func_2_7;
extern gctSTRING gcLibTextureGather_Func_2_8;
extern gctSTRING gcLibTextureGather_Func_2_9;
extern gctSTRING gcLibTextureGather_Func_2_10;
extern gctSTRING gcLibTextureGather_Func_2_11;
extern gctSTRING gcLibTextureGather_Func_2_12;
extern gctSTRING gcLibTextureGather_Func_2_13;
extern gctSTRING gcLibTextureGather_Func_2_14;
extern gctSTRING gcLibTextureGather_Func_2_15;
extern gctSTRING gcLibTextureGather_Func_2_16;
extern gctSTRING gcLibTextureGather_Func_2_17;
extern gctSTRING gcLibTextureGather_Func_2_18;
extern gctSTRING gcLibTextureGather_Func_2_19;
extern gctSTRING gcLibTextureGather_Func_2_20;
extern gctSTRING gcLibTextureGather_Func_2_21;
extern gctSTRING gcLibTextureGatherOffset_Func_1;
extern gctSTRING gcLibTextureGatherOffset_Func_2;
extern gctSTRING gcLibTextureGatherOffset_Func_3;
extern gctSTRING gcLibTextureGatherOffset_Func_4;
extern gctSTRING gcLibTextureGatherOffset_Func_5;
extern gctSTRING gcLibTextureGatherOffset_Func_6;
extern gctSTRING gcLibTextureGatherOffset_Func_7;
extern gctSTRING gcLibTextureGatherOffset_Func_8;
extern gctSTRING gcLibTextureGatherOffset_Func_9;
extern gctSTRING gcLibTextureGatherOffset_Func_10;
extern gctSTRING gcLibTextureGatherOffset_Func_11;
extern gctSTRING gcLibTextureGatherOffset_Func_12;
extern gctSTRING gcLibTextureGatherOffset_Func_13;
extern gctSTRING gcLibTextureGatherOffset_Func_14;
extern gctSTRING gcLibTextureGatherOffsets_Func_1;
extern gctSTRING gcLibTextureGatherOffsets_Func_2;
extern gctSTRING gcLibTextureGatherOffsets_Func_3;
extern gctSTRING gcLibTextureGatherOffsets_Func_4;
extern gctSTRING gcLibTextureGatherOffsets_Func_5;
extern gctSTRING gcLibTextureGatherOffsets_Func_6;
extern gctSTRING gcLibTextureGatherOffsets_Func_7;
extern gctSTRING gcLibTextureGatherOffsets_Func_8;
extern gctSTRING gcLibTextureGatherOffsets_Func_9;
extern gctSTRING gcLibTextureGatherOffsets_Func_10;
extern gctSTRING gcLibTextureGatherOffsets_Func_11;
extern gctSTRING gcLibTextureGatherOffsets_Func_12;
extern gctSTRING gcLibTextureGatherOffsets_Func_13;
extern gctSTRING gcLibTextureGatherOffsets_Func_14;
extern gctSTRING gcLibTexelFetchForMSAA_Func_1;
extern gctSTRING gcLibTexelFetchForMSAA_Func_2;
extern gctSTRING gcLibTexelFetchForMSAA_Func_3;
extern gctSTRING gcLibTexelFetchForMSAA_Func_4;
extern gctSTRING gcLibTexelFetchForMSAA_Func_5;
extern gctSTRING gcLibTexelFetchForMSAA_Func_6;
extern gctSTRING gcLibTexelFetchForMSAA_Func_2_1;
extern gctSTRING gcLibTexelFetchForMSAA_Func_2_2;
extern gctSTRING gcLibTexelFetchForMSAA_Func_2_3;
extern gctSTRING gcLibTexelFetchForMSAA_Func_2_4;
extern gctSTRING gcLibTexelFetchForMSAA_Func_2_5;
extern gctSTRING gcLibTexelFetchForMSAA_Func_2_6;
extern gctSTRING gcLibImageSize;
extern gctSTRING gcLibImageSize_halti4;
extern gctSTRING gcLibImageSize_2D_float;
extern gctSTRING gcLibImageSize_3D_float;
extern gctSTRING gcLibImageSize_CUBE_float;
extern gctSTRING gcLibImageSize_2DArray_float;
extern gctSTRING gcLibImageSize_2D_int;
extern gctSTRING gcLibImageSize_3D_int;
extern gctSTRING gcLibImageSize_CUBE_int;
extern gctSTRING gcLibImageSize_2DArray_int;
extern gctSTRING gcLibImageSize_2D_uint;
extern gctSTRING gcLibImageSize_3D_uint ;
extern gctSTRING gcLibImageSize_CUBE_uint ;
extern gctSTRING gcLibImageSize_2DArray_uint ;
extern gctSTRING gcLibImageSize_CubeArray_float ;
extern gctSTRING gcLibImageSize_CubeArray_int ;
extern gctSTRING gcLibImageSize_CubeArray_uint ;
extern gctSTRING gcLibImageSize_Buffer_float ;
extern gctSTRING gcLibImageSize_Buffer_int ;
extern gctSTRING gcLibImageSize_Buffer_uint ;
extern gctSTRING gcLibImageAddr;
extern gctSTRING gcLibImageAddr_halti4;
extern gctSTRING gcLibImageAddr_intrinsic;
extern gctSTRING gcLibImageSwizzle;
extern gctSTRING gcLibImageStoreSwizzle;
/* imageLoad from 1D. */
extern gctSTRING gcLibImageLoad_1D_float;
extern gctSTRING gcLibImageLoad_1D_float_rgba8;
extern gctSTRING gcLibImageLoad_1D_float_rgba8_snorm;
extern gctSTRING gcLibImageLoad_1D_float_rgba32f;
extern gctSTRING gcLibImageLoad_1D_float_r32f;
extern gctSTRING gcLibImageLoad_1D_float_r32i;
extern gctSTRING gcLibImageLoad_1D_float_r32ui;
extern gctSTRING gcLibImageLoad_1D_int;
extern gctSTRING gcLibImageLoad_1D_int_rgba8i;
extern gctSTRING gcLibImageLoad_1D_int_rgba32i;
extern gctSTRING gcLibImageLoad_1D_int_r32i;
extern gctSTRING gcLibImageLoad_1D_uint;
extern gctSTRING gcLibImageLoad_1D_uint_rgba8ui;
extern gctSTRING gcLibImageLoad_1D_uint_rgba32ui;
extern gctSTRING gcLibImageLoad_1D_uint_r32ui;
/* imageLoad from 2D. */
extern gctSTRING gcLibImageLoad_2D_float;
extern gctSTRING gcLibImageLoad_2D_float_rgba8;
extern gctSTRING gcLibImageLoad_2D_float_rgba8_snorm;
extern gctSTRING gcLibImageLoad_2D_float_rg8;
extern gctSTRING gcLibImageLoad_2D_float_r8;
extern gctSTRING gcLibImageLoad_2D_float_rgba32f;
extern gctSTRING gcLibImageLoad_2D_float_rg32f;
extern gctSTRING gcLibImageLoad_2D_float_rg16f;
extern gctSTRING gcLibImageLoad_2D_float_r16f;
extern gctSTRING gcLibImageLoad_2D_float_r32f;
extern gctSTRING gcLibImageLoad_2D_float_r5g6b5_unorm_pack16;
extern gctSTRING gcLibImageLoad_2D_float_abgr8_unorm_pack32;
extern gctSTRING gcLibImageLoad_2D_float_a2r10g10b10_unorm_pack32;
extern gctSTRING gcLibImageLoad_2D_uint_abgr8ui_pack32;
extern gctSTRING gcLibImageLoad_2D_int_abgr8i_pack32;
extern gctSTRING gcLibImageLoad_2D_int;
extern gctSTRING gcLibImageLoad_2D_int_rgba8i;
extern gctSTRING gcLibImageLoad_2D_int_rg8i;
extern gctSTRING gcLibImageLoad_2D_int_r8i;
extern gctSTRING gcLibImageLoad_2D_int_rgba32i;
extern gctSTRING gcLibImageLoad_2D_int_rg32i;
extern gctSTRING gcLibImageLoad_2D_int_r32i;
extern gctSTRING gcLibImageLoad_2D_int_rg16i;
extern gctSTRING gcLibImageLoad_2D_int_r16i;
extern gctSTRING gcLibImageLoad_2D_uint;
extern gctSTRING gcLibImageLoad_2D_uint_rgba8ui;
extern gctSTRING gcLibImageLoad_2D_uint_rg8ui;
extern gctSTRING gcLibImageLoad_2D_uint_r8ui;
extern gctSTRING gcLibImageLoad_2D_uint_rgba32ui;
extern gctSTRING gcLibImageLoad_2D_uint_rg16ui;
extern gctSTRING gcLibImageLoad_2D_uint_r16ui;
extern gctSTRING gcLibImageLoad_2D_uint_rg32ui;
extern gctSTRING gcLibImageLoad_2D_uint_r32ui;
extern gctSTRING gcLibImageLoad_3Dcommon;
extern gctSTRING gcLibImageLoad_3D;
extern gctSTRING gcLibImageLoad_cube;
extern gctSTRING gcLibImageLoad_1DArray;
extern gctSTRING gcLibImageLoad_2DArray;
extern gctSTRING gcLibImageLoad_CubeArray;
extern gctSTRING gcLibImageLoad_Buffer_float;
extern gctSTRING gcLibImageLoad_Buffer_float_rgba8;
extern gctSTRING gcLibImageLoad_Buffer_float_rgba8_snorm;
extern gctSTRING gcLibImageLoad_Buffer_float_rgba32f;
extern gctSTRING gcLibImageLoad_Buffer_float_r32f;
extern gctSTRING gcLibImageLoad_Buffer_int;
extern gctSTRING gcLibImageLoad_Buffer_int_rgba8i;
extern gctSTRING gcLibImageLoad_Buffer_int_rgba32i;
extern gctSTRING gcLibImageLoad_Buffer_int_r32i;
extern gctSTRING gcLibImageLoad_Buffer_uint;
extern gctSTRING gcLibImageLoad_Buffer_uint_rgba8ui;
extern gctSTRING gcLibImageLoad_Buffer_uint_rgba32ui;
extern gctSTRING gcLibImageLoad_Buffer_uint_r32ui;
extern gctSTRING gcLibImageLoad_1D_float_hati4;
extern gctSTRING gcLibImageLoad_1D_float_1_hati4;
extern gctSTRING gcLibImageLoad_1D_int_hati4;
extern gctSTRING gcLibImageLoad_1D_int_1_hati4;
extern gctSTRING gcLibImageLoad_1D_uint_hati4;
extern gctSTRING gcLibImageLoad_1D_uint_1_hati4;
extern gctSTRING gcLibImageLoad_1D_array_float_hati4;
extern gctSTRING gcLibImageLoad_1D_array_float_1_hati4;
extern gctSTRING gcLibImageLoad_1D_array_int_hati4;
extern gctSTRING gcLibImageLoad_1D_array_int_1_hati4;
extern gctSTRING gcLibImageLoad_1D_array_uint_hati4;
extern gctSTRING gcLibImageLoad_1D_array_uint_1_hati4;
extern gctSTRING gcLibImageLoad_2D_float_hati4;
extern gctSTRING gcLibImageLoad_2D_float_1_hati4;
extern gctSTRING gcLibImageLoad_2D_int_hati4;
extern gctSTRING gcLibImageLoad_2D_int_1_hati4;
extern gctSTRING gcLibImageLoad_2D_uint_hati4;
extern gctSTRING gcLibImageLoad_2D_uint_1_hati4;
/* imageLoad for image3D. */
extern gctSTRING gcLibImageLoad_3D_float_hati4;
extern gctSTRING gcLibImageLoad_3D_float_1_hati4;
extern gctSTRING gcLibImageLoad_3D_int_hati4;
extern gctSTRING gcLibImageLoad_3D_int_1_hati4;
extern gctSTRING gcLibImageLoad_3D_uint_hati4;
extern gctSTRING gcLibImageLoad_3D_uint_1_hati4;
/* imageLoad for imageCube. */
extern gctSTRING gcLibImageLoad_cube_float_hati4;
extern gctSTRING gcLibImageLoad_cube_float_1_hati4;
extern gctSTRING gcLibImageLoad_cube_int_hati4;
extern gctSTRING gcLibImageLoad_cube_int_1_hati4;
extern gctSTRING gcLibImageLoad_cube_uint_hati4;
extern gctSTRING gcLibImageLoad_cube_uint_1_hati4;
/* imageLoad for image2DArray. */
extern gctSTRING gcLibImageLoad_2DArray_float_hati4;
extern gctSTRING gcLibImageLoad_2DArray_float_1_hati4;
extern gctSTRING gcLibImageLoad_2DArray_int_hati4;
extern gctSTRING gcLibImageLoad_2DArray_int_1_hati4;
extern gctSTRING gcLibImageLoad_2DArray_uint_hati4;
extern gctSTRING gcLibImageLoad_2DArray_uint_1_hati4;
/* imageLoad for imageCubeArray. */
extern gctSTRING gcLibImageLoad_CubeArray_float_img_access;
extern gctSTRING gcLibImageLoad_CubeArray_float_1_img_access;
extern gctSTRING gcLibImageLoad_CubeArray_int_img_access;
extern gctSTRING gcLibImageLoad_CubeArray_int_1_img_access;
extern gctSTRING gcLibImageLoad_CubeArray_uint_img_access;
extern gctSTRING gcLibImageLoad_CubeArray_uint_1_img_access;
/* imageLoad for imageBuffer. */
extern gctSTRING gcLibImageLoad_Buffer_float_img_access;
extern gctSTRING gcLibImageLoad_Buffer_int_img_access;
extern gctSTRING gcLibImageLoad_Buffer_uint_img_access;

/* texelFetch for sampler2D. */
extern gctSTRING gcLibTexelFetch_Sampler2D;
extern gctSTRING gcLibTexelFetch_Sampler2D_halti4;

/* texelFetch for sampler2DArray. */
extern gctSTRING gcLibTexelFetch_Sampler2DArray;
extern gctSTRING gcLibTexelFetch_Sampler2DArray_halti4;

/* texelFetch for sampler2DMS. */
extern gctSTRING gcLibTexelFetch_Sampler2DMS_halti4;

/* texelFetch for sampler2DMSArray. */
extern gctSTRING gcLibTexelFetch_Sampler2DMSArray_halti4;

/* texelFetch for sampler3D. */
extern gctSTRING gcLibTexelFetch_Sampler3D;
extern gctSTRING gcLibTexelFetch_Sampler3D_halti4;

/* texelFetch for samplerBufferr. */
extern gctSTRING gcLibTexelFetch_SamplerBuffer;
extern gctSTRING gcLibTexelFetch_SamplerBuffer_halti4;

/* getLod. */
extern gctSTRING gcLibGetLod;

/* imageStore for image1D. */
extern gctSTRING gcLibImageStore_1D_float_rgba32f;
extern gctSTRING gcLibImageStore_1D_float;
extern gctSTRING gcLibImageStore_1D_float_r32f;
extern gctSTRING gcLibImageStore_1D_float_r32i;
extern gctSTRING gcLibImageStore_1D_float_r32ui;
extern gctSTRING gcLibImageStore_1D_float_rgba8;
extern gctSTRING gcLibImageStore_1D_float_rgba8_snorm;
extern gctSTRING gcLibImageStore_1D_int_rgba32i;
extern gctSTRING gcLibImageStore_1D_int;
extern gctSTRING gcLibImageStore_1D_int_rgba8i;
extern gctSTRING gcLibImageStore_1D_int_r32i;
extern gctSTRING gcLibImageStore_1D_uint_rgba32ui;
extern gctSTRING gcLibImageStore_1D_uint;
extern gctSTRING gcLibImageStore_1D_uint_rgba8ui;
extern gctSTRING gcLibImageStore_1D_uint_r32ui;

/* imageStore for image2D. */
extern gctSTRING gcLibImageStore_2D_float_rgba32f;
extern gctSTRING gcLibImageStore_2D_float;
extern gctSTRING gcLibImageStore_2D_float_rg32f;
extern gctSTRING gcLibImageStore_2D_float_rg16f;
extern gctSTRING gcLibImageStore_2D_float_r16f;
extern gctSTRING gcLibImageStore_2D_float_r32f;
extern gctSTRING gcLibImageStore_2D_float_r5g6b5_unorm_pack16;
extern gctSTRING gcLibImageStore_2D_float_abgr8_unorm_pack32;
extern gctSTRING gcLibImageStore_2D_float_a2r10g10b10_unorm_pack32;
extern gctSTRING gcLibImageStore_2D_uint_abgr8ui_pack32;
extern gctSTRING gcLibImageStore_2D_int_abgr8i_pack32;
extern gctSTRING gcLibImageStore_2D_float_rgba8;
extern gctSTRING gcLibImageStore_2D_float_rgba8_snorm;
extern gctSTRING gcLibImageStore_2D_float_rg8;
extern gctSTRING gcLibImageStore_2D_float_r8;
extern gctSTRING gcLibImageStore_2D_int_rgba32i;
extern gctSTRING gcLibImageStore_2D_int_rg16i;
extern gctSTRING gcLibImageStore_2D_int_r16i;
extern gctSTRING gcLibImageStore_2D_int;
extern gctSTRING gcLibImageStore_2D_int_rgba8i;
extern gctSTRING gcLibImageStore_2D_int_rg8i;
extern gctSTRING gcLibImageStore_2D_int_r8i;
extern gctSTRING gcLibImageStore_2D_int_r32i;
extern gctSTRING gcLibImageStore_2D_int_rg32i;
extern gctSTRING gcLibImageStore_2D_uint_rgba32ui;
extern gctSTRING gcLibImageStore_2D_uint;
extern gctSTRING gcLibImageStore_2D_uint_rgba8ui;
extern gctSTRING gcLibImageStore_2D_uint_rg8ui;
extern gctSTRING gcLibImageStore_2D_uint_r8ui;
extern gctSTRING gcLibImageStore_2D_uint_rg16ui;
extern gctSTRING gcLibImageStore_2D_uint_r16ui;
extern gctSTRING gcLibImageStore_2D_uint_rg32ui;
extern gctSTRING gcLibImageStore_2D_uint_r32ui;

extern gctSTRING gcLibImageStore_3Dcommon;
extern gctSTRING gcLibImageStore_3D;
extern gctSTRING gcLibImageStore_cube;
extern gctSTRING gcLibImageStore_1DArray;
extern gctSTRING gcLibImageStore_2DArray;
extern gctSTRING gcLibImageStore_CubeArray;
/* imageStore for imageBuffer. */
extern gctSTRING gcLibImageStore_Buffer_float_rgba32f;
extern gctSTRING gcLibImageStore_Buffer_float;
extern gctSTRING gcLibImageStore_Buffer_float_r32f;
extern gctSTRING gcLibImageStore_Buffer_float_rgba8;
extern gctSTRING gcLibImageStore_Buffer_float_rgba8_snorm;
extern gctSTRING gcLibImageStore_Buffer_int_rgba32i;
extern gctSTRING gcLibImageStore_Buffer_int;
extern gctSTRING gcLibImageStore_Buffer_int_rgba8i;
extern gctSTRING gcLibImageStore_Buffer_int_r32i;
extern gctSTRING gcLibImageStore_Buffer_uint_rgba32ui;
extern gctSTRING gcLibImageStore_Buffer_uint;
extern gctSTRING gcLibImageStore_Buffer_uint_rgba8ui;
extern gctSTRING gcLibImageStore_Buffer_uint_r32ui;
/* imageStore for image1D. */
extern gctSTRING gcLibImageStore_1D_float_hati4;
extern gctSTRING gcLibImageStore_1D_float_1_hati4;
extern gctSTRING gcLibImageStore_1D_int_hati4;
extern gctSTRING gcLibImageStore_1D_int_1_hati4;
extern gctSTRING gcLibImageStore_1D_uint_hati4;
extern gctSTRING gcLibImageStore_1D_uint_1_hati4;
extern gctSTRING gcLibImageStore_1D_array_float_hati4;
extern gctSTRING gcLibImageStore_1D_array_float_1_hati4;
extern gctSTRING gcLibImageStore_1D_array_int_hati4;
extern gctSTRING gcLibImageStore_1D_array_int_1_hati4;
extern gctSTRING gcLibImageStore_1D_array_uint_hati4;
extern gctSTRING gcLibImageStore_1D_array_uint_1_hati4;
extern gctSTRING gcLibImageStore_2D_float_hati4;
extern gctSTRING gcLibImageStore_2D_float_1_hati4;
extern gctSTRING gcLibImageStore_2D_int_hati4;
extern gctSTRING gcLibImageStore_2D_int_1_hati4;
extern gctSTRING gcLibImageStore_2D_uint_hati4;
extern gctSTRING gcLibImageStore_2D_uint_1_hati4;
/* imageStore for image3D. */
extern gctSTRING gcLibImageStore_3D_float_hati4;
extern gctSTRING gcLibImageStore_3D_float_1_hati4;
extern gctSTRING gcLibImageStore_3D_int_hati4;
extern gctSTRING gcLibImageStore_3D_int_1_hati4;
extern gctSTRING gcLibImageStore_3D_uint_hati4;
extern gctSTRING gcLibImageStore_3D_uint_1_hati4;
/* imageStore for imageCube. */
extern gctSTRING gcLibImageStore_cube_float_hati4;
extern gctSTRING gcLibImageStore_cube_float_1_hati4;
extern gctSTRING gcLibImageStore_cube_int_hati4;
extern gctSTRING gcLibImageStore_cube_int_1_hati4;
extern gctSTRING gcLibImageStore_cube_uint_hati4;
extern gctSTRING gcLibImageStore_cube_uint_1_hati4;
/* imageStore for image2DArray. */
extern gctSTRING gcLibImageStore_2DArray_float_hati4;
extern gctSTRING gcLibImageStore_2DArray_float_1_hati4;
extern gctSTRING gcLibImageStore_2DArray_int_hati4;
extern gctSTRING gcLibImageStore_2DArray_int_1_hati4;
extern gctSTRING gcLibImageStore_2DArray_uint_hati4;
extern gctSTRING gcLibImageStore_2DArray_uint_1_hati4;
/* imageStore for imageCubeArray. */
extern gctSTRING gcLibImageStore_CubeArray_float_img_access;
extern gctSTRING gcLibImageStore_CubeArray_float_1_img_access;
extern gctSTRING gcLibImageStore_CubeArray_int_img_access;
extern gctSTRING gcLibImageStore_CubeArray_int_1_img_access;
extern gctSTRING gcLibImageStore_CubeArray_uint_img_access;
extern gctSTRING gcLibImageStore_CubeArray_uint_1_img_access;
/* imageStore for imageBuffer. */
extern gctSTRING gcLibImageStore_Buffer_float_img_access;
extern gctSTRING gcLibImageStore_Buffer_int_img_access;
extern gctSTRING gcLibImageStore_Buffer_uint_img_access;
extern gctSTRING gcLibImageAtomicAdd_2D_int;
extern gctSTRING gcLibImageAtomicAdd_2D_uint;
extern gctSTRING gcLibImageAtomicAdd_3D_int;
extern gctSTRING gcLibImageAtomicAdd_3D_uint;
extern gctSTRING gcLibImageAtomicAdd_CUBE_int;
extern gctSTRING gcLibImageAtomicAdd_CUBE_uint;
extern gctSTRING gcLibImageAtomicAdd_2DARRAY_int;
extern gctSTRING gcLibImageAtomicAdd_2DARRAY_uint;
extern gctSTRING gcLibImageAtomicAdd_buffer_int;
extern gctSTRING gcLibImageAtomicAdd_buffer_uint;
extern gctSTRING gcLibImageAtomicMin_2D_int;
extern gctSTRING gcLibImageAtomicMin_2D_uint;
extern gctSTRING gcLibImageAtomicMin_3D_int;
extern gctSTRING gcLibImageAtomicMin_3D_uint;
extern gctSTRING gcLibImageAtomicMin_CUBE_int;
extern gctSTRING gcLibImageAtomicMin_CUBE_uint;
extern gctSTRING gcLibImageAtomicMin_2DARRAY_int;
extern gctSTRING gcLibImageAtomicMin_2DARRAY_uint;
extern gctSTRING gcLibImageAtomicMin_buffer_int;
extern gctSTRING gcLibImageAtomicMin_buffer_uint;
extern gctSTRING gcLibImageAtomicMax_2D_int;
extern gctSTRING gcLibImageAtomicMax_2D_uint;
extern gctSTRING gcLibImageAtomicMax_3D_int;
extern gctSTRING gcLibImageAtomicMax_3D_uint;
extern gctSTRING gcLibImageAtomicMax_CUBE_int;
extern gctSTRING gcLibImageAtomicMax_CUBE_uint;
extern gctSTRING gcLibImageAtomicMax_2DARRAY_int;
extern gctSTRING gcLibImageAtomicMax_2DARRAY_uint;
extern gctSTRING gcLibImageAtomicMax_buffer_int;
extern gctSTRING gcLibImageAtomicMax_buffer_uint;
extern gctSTRING gcLibImageAtomicAnd_2D_int;
extern gctSTRING gcLibImageAtomicAnd_2D_uint;
extern gctSTRING gcLibImageAtomicAnd_3D_int;
extern gctSTRING gcLibImageAtomicAnd_3D_uint;
extern gctSTRING gcLibImageAtomicAnd_CUBE_int;
extern gctSTRING gcLibImageAtomicAnd_CUBE_uint;
extern gctSTRING gcLibImageAtomicAnd_2DARRAY_int;
extern gctSTRING gcLibImageAtomicAnd_2DARRAY_uint;
extern gctSTRING gcLibImageAtomicAnd_buffer_int;
extern gctSTRING gcLibImageAtomicAnd_buffer_uint;
extern gctSTRING gcLibImageAtomicOr_2D_int;
extern gctSTRING gcLibImageAtomicOr_2D_uint;
extern gctSTRING gcLibImageAtomicOr_3D_int;
extern gctSTRING gcLibImageAtomicOr_3D_uint;
extern gctSTRING gcLibImageAtomicOr_CUBE_int;
extern gctSTRING gcLibImageAtomicOr_CUBE_uint;
extern gctSTRING gcLibImageAtomicOr_2DARRAY_int;
extern gctSTRING gcLibImageAtomicOr_2DARRAY_uint;
extern gctSTRING gcLibImageAtomicOr_buffer_int;
extern gctSTRING gcLibImageAtomicOr_buffer_uint;
extern gctSTRING gcLibImageAtomicXor_2D_int;
extern gctSTRING gcLibImageAtomicXor_2D_uint;
extern gctSTRING gcLibImageAtomicXor_3D_int;
extern gctSTRING gcLibImageAtomicXor_3D_uint;
extern gctSTRING gcLibImageAtomicXor_CUBE_int;
extern gctSTRING gcLibImageAtomicXor_CUBE_uint;
extern gctSTRING gcLibImageAtomicXor_2DARRAY_int;
extern gctSTRING gcLibImageAtomicXor_2DARRAY_uint;
extern gctSTRING gcLibImageAtomicXor_buffer_int;
extern gctSTRING gcLibImageAtomicXor_buffer_uint;
extern gctSTRING gcLibImageAtomicXchg_2D_int;
extern gctSTRING gcLibImageAtomicXchg_2D_uint;
extern gctSTRING gcLibImageAtomicXchg_2D_float;
extern gctSTRING gcLibImageAtomicXchg_3D_int;
extern gctSTRING gcLibImageAtomicXchg_3D_uint;
extern gctSTRING gcLibImageAtomicXchg_3D_float;
extern gctSTRING gcLibImageAtomicXchg_CUBE_int;
extern gctSTRING gcLibImageAtomicXchg_CUBE_uint;
extern gctSTRING gcLibImageAtomicXchg_CUBE_float;
extern gctSTRING gcLibImageAtomicXchg_2DARRAY_int;
extern gctSTRING gcLibImageAtomicXchg_2DARRAY_uint;
extern gctSTRING gcLibImageAtomicXchg_2DARRAY_float;
extern gctSTRING gcLibImageAtomicXchg_buffer_int;
extern gctSTRING gcLibImageAtomicXchg_buffer_uint;
extern gctSTRING gcLibImageAtomicXchg_buffer_float;
extern gctSTRING gcLibImageAtomicCmpXchg_2D_int;
extern gctSTRING gcLibImageAtomicCmpXchg_2D_uint;
extern gctSTRING gcLibImageAtomicCmpXchg_3D_int;
extern gctSTRING gcLibImageAtomicCmpXchg_3D_uint;
extern gctSTRING gcLibImageAtomicCmpXchg_CUBE_int;
extern gctSTRING gcLibImageAtomicCmpXchg_CUBE_uint;
extern gctSTRING gcLibImageAtomicCmpXchg_2DARRAY_int;
extern gctSTRING gcLibImageAtomicCmpXchg_2DARRAY_uint;
extern gctSTRING gcLibImageAtomicCmpXchg_buffer_int;
extern gctSTRING gcLibImageAtomicCmpXchg_buffer_uint;
extern gctSTRING gcLibImageAtomicAdd_2D_int_hati4;
extern gctSTRING gcLibImageAtomicAdd_2D_uint_hati4;
extern gctSTRING gcLibImageAtomicAdd_3D_int_hati4;
extern gctSTRING gcLibImageAtomicAdd_3D_uint_hati4;
extern gctSTRING gcLibImageAtomicAdd_CUBE_int_hati4;
extern gctSTRING gcLibImageAtomicAdd_CUBE_uint_hati4;
extern gctSTRING gcLibImageAtomicAdd_2DARRAY_int_hati4;
extern gctSTRING gcLibImageAtomicAdd_2DARRAY_uint_hati4;
extern gctSTRING gcLibImageAtomicAdd_buffer_int_img_access;
extern gctSTRING gcLibImageAtomicAdd_buffer_uint_img_access;
extern gctSTRING gcLibImageAtomicMin_2D_int_hati4;
extern gctSTRING gcLibImageAtomicMin_2D_uint_hati4;
extern gctSTRING gcLibImageAtomicMin_3D_int_hati4;
extern gctSTRING gcLibImageAtomicMin_3D_uint_hati4;
extern gctSTRING gcLibImageAtomicMin_CUBE_int_hati4;
extern gctSTRING gcLibImageAtomicMin_CUBE_uint_hati4;
extern gctSTRING gcLibImageAtomicMin_2DARRAY_int_hati4;
extern gctSTRING gcLibImageAtomicMin_2DARRAY_uint_hati4;
extern gctSTRING gcLibImageAtomicMin_buffer_int_img_access;
extern gctSTRING gcLibImageAtomicMin_buffer_uint_img_access;
extern gctSTRING gcLibImageAtomicMax_2D_int_hati4;
extern gctSTRING gcLibImageAtomicMax_2D_uint_hati4;
extern gctSTRING gcLibImageAtomicMax_3D_int_hati4;
extern gctSTRING gcLibImageAtomicMax_3D_uint_hati4;
extern gctSTRING gcLibImageAtomicMax_CUBE_int_hati4;
extern gctSTRING gcLibImageAtomicMax_CUBE_uint_hati4;
extern gctSTRING gcLibImageAtomicMax_2DARRAY_int_hati4;
extern gctSTRING gcLibImageAtomicMax_2DARRAY_uint_hati4;
extern gctSTRING gcLibImageAtomicMax_buffer_int_img_access;
extern gctSTRING gcLibImageAtomicMax_buffer_uint_img_access;
extern gctSTRING gcLibImageAtomicAnd_2D_int_hati4;
extern gctSTRING gcLibImageAtomicAnd_2D_uint_hati4;
extern gctSTRING gcLibImageAtomicAnd_3D_int_hati4;
extern gctSTRING gcLibImageAtomicAnd_3D_uint_hati4;
extern gctSTRING gcLibImageAtomicAnd_CUBE_int_hati4;
extern gctSTRING gcLibImageAtomicAnd_CUBE_uint_hati4;
extern gctSTRING gcLibImageAtomicAnd_2DARRAY_int_hati4;
extern gctSTRING gcLibImageAtomicAnd_2DARRAY_uint_hati4;
extern gctSTRING gcLibImageAtomicAnd_buffer_int_img_access;
extern gctSTRING gcLibImageAtomicAnd_buffer_uint_img_access;
extern gctSTRING gcLibImageAtomicOr_2D_int_hati4;
extern gctSTRING gcLibImageAtomicOr_2D_uint_hati4;
extern gctSTRING gcLibImageAtomicOr_3D_int_hati4;
extern gctSTRING gcLibImageAtomicOr_3D_uint_hati4;
extern gctSTRING gcLibImageAtomicOr_CUBE_int_hati4;
extern gctSTRING gcLibImageAtomicOr_CUBE_uint_hati4;
extern gctSTRING gcLibImageAtomicOr_2DARRAY_int_hati4;
extern gctSTRING gcLibImageAtomicOr_2DARRAY_uint_hati4;
extern gctSTRING gcLibImageAtomicOr_buffer_int_img_access;
extern gctSTRING gcLibImageAtomicOr_buffer_uint_img_access;
extern gctSTRING gcLibImageAtomicXor_2D_int_hati4;
extern gctSTRING gcLibImageAtomicXor_2D_uint_hati4;
extern gctSTRING gcLibImageAtomicXor_3D_int_hati4;
extern gctSTRING gcLibImageAtomicXor_3D_uint_hati4;
extern gctSTRING gcLibImageAtomicXor_CUBE_int_hati4;
extern gctSTRING gcLibImageAtomicXor_CUBE_uint_hati4;
extern gctSTRING gcLibImageAtomicXor_2DARRAY_int_hati4;
extern gctSTRING gcLibImageAtomicXor_2DARRAY_uint_hati4;
extern gctSTRING gcLibImageAtomicXor_buffer_int_img_access;
extern gctSTRING gcLibImageAtomicXor_buffer_uint_img_access;
extern gctSTRING gcLibImageAtomicXchg_2D_int_hati4;
extern gctSTRING gcLibImageAtomicXchg_2D_uint_hati4;
extern gctSTRING gcLibImageAtomicXchg_2D_float_hati4;
extern gctSTRING gcLibImageAtomicXchg_3D_int_hati4;
extern gctSTRING gcLibImageAtomicXchg_3D_uint_hati4;
extern gctSTRING gcLibImageAtomicXchg_3D_float_hati4;
extern gctSTRING gcLibImageAtomicXchg_CUBE_int_hati4;
extern gctSTRING gcLibImageAtomicXchg_CUBE_uint_hati4;
extern gctSTRING gcLibImageAtomicXchg_CUBE_float_hati4;
extern gctSTRING gcLibImageAtomicXchg_2DARRAY_int_hati4;
extern gctSTRING gcLibImageAtomicXchg_2DARRAY_uint_hati4;
extern gctSTRING gcLibImageAtomicXchg_2DARRAY_float_hati4;
extern gctSTRING gcLibImageAtomicXchg_buffer_int_img_access;
extern gctSTRING gcLibImageAtomicXchg_buffer_uint_img_access;
extern gctSTRING gcLibImageAtomicXchg_buffer_float_img_access;
extern gctSTRING gcLibImageAtomicCmpXchg_2D_int_hati4;
extern gctSTRING gcLibImageAtomicCmpXchg_2D_uint_hati4;
extern gctSTRING gcLibImageAtomicCmpXchg_3D_int_hati4;
extern gctSTRING gcLibImageAtomicCmpXchg_3D_uint_hati4;
extern gctSTRING gcLibImageAtomicCmpXchg_CUBE_int_hati4;
extern gctSTRING gcLibImageAtomicCmpXchg_CUBE_uint_hati4;
extern gctSTRING gcLibImageAtomicCmpXchg_2DARRAY_int_hati4;
extern gctSTRING gcLibImageAtomicCmpXchg_2DARRAY_uint_hati4;
extern gctSTRING gcLibImageAtomicCmpXchg_buffer_int_img_access;
extern gctSTRING gcLibImageAtomicCmpXchg_buffer_uint_img_access;
/* image query function. */
extern gctSTRING gcLibImageQuery_halti4;
/* blend equation functions. */
extern gctSTRING gcLibBlendEquation_Multiply;
extern gctSTRING gcLibBlendEquation_Screen;
extern gctSTRING gcLibBlendEquation_Overlay;
extern gctSTRING gcLibBlendEquation_Darken;
extern gctSTRING gcLibBlendEquation_Lighten;
extern gctSTRING gcLibBlendEquation_Hardlight;
extern gctSTRING gcLibBlendEquation_Difference;
extern gctSTRING gcLibBlendEquation_Exclusion;
extern gctSTRING gcLibBlendEquation_Colordodge;
extern gctSTRING gcLibBlendEquation_Colorburn;
extern gctSTRING gcLibBlendEquation_Softlight;
extern gctSTRING gcLibBlendEquation_HSL_HUE;
extern gctSTRING gcLibBlendEquation_HSL_SATURATION;
extern gctSTRING gcLibBlendEquation_HSL_COLOR;
extern gctSTRING gcLibBlendEquation_HSL_LUMINOSITY;
extern gctSTRING gcLibBlendEquation_ALL;
extern gctSTRING gcLibBlendEquation_Colordodge_hati4;
extern gctSTRING gcLibBlendEquation_Colorburn_hati4;
extern gctSTRING gcLibBlendEquation_Softlight_hati4;
extern gctSTRING gcLibBlendEquation_HSL_HUE_hati4;
extern gctSTRING gcLibBlendEquation_HSL_SATURATION_hati4;
extern gctSTRING gcLibBlendEquation_HSL_COLOR_hati4;
extern gctSTRING gcLibBlendEquation_HSL_LUMINOSITY_hati4;
extern gctSTRING gcLibBlendEquation_ALL_hati4;
extern gctSTRING gcLib_3instMixFunc;
extern gctSTRING gcLib_2instMixFunc;
extern gctSTRING gcLibCommon_Func;
extern gctSTRING gcLibInterpolateCommon;
extern gctSTRING gcLibInterpolateAtCentroid_float;
extern gctSTRING gcLibInterpolateAtCentroid_vec2;
extern gctSTRING gcLibInterpolateAtCentroid_vec3;
extern gctSTRING gcLibInterpolateAtCentroid_vec4;
extern gctSTRING gcLibInterpolateAtSample_float;
extern gctSTRING gcLibInterpolateAtSample_vec2;
extern gctSTRING gcLibInterpolateAtSample_vec3;
extern gctSTRING gcLibInterpolateAtSample_vec4;
extern gctSTRING gcLibInterpolateAtOffset_float;
extern gctSTRING gcLibInterpolateAtOffset_vec2;
extern gctSTRING gcLibInterpolateAtOffset_vec3;
extern gctSTRING gcLibInterpolateAtOffset_vec4;
/* texld for sampler1DArray */
extern gctSTRING gcLibTexLd_sampler_1d_array;
extern gctSTRING gcLibTexLd_sampler_1d_array_lod;
extern gctSTRING gcLibTexLd_sampler_1d_array_bias;
/* texld for sampler2DArray */
extern gctSTRING gcLibTexLd_sampler_2d_array;
extern gctSTRING gcLibTexLd_sampler_2d_array_lod;
extern gctSTRING gcLibTexLd_sampler_2d_array_bias;
extern gctSTRING gcGLLibGetLocalID;
extern gctSTRING gcGLLib_AtomcmpxchgPatch_Func_core1_Str;
extern gctSTRING gcGLLib_AtomcmpxchgPatch_Func_core2_Str;
extern gctSTRING gcGLLib_AtomcmpxchgPatch_Func_core4_Str;
extern gctSTRING gcGLLib_AtomcmpxchgPatch_Func_core8_Str;
#endif /* __gc_vsc_gl_builtin_lib_h_ */

