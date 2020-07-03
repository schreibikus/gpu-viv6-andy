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


    /* uniform state */
    {
        {slvEXTENSION1_NONE},
        "gl_DepthRange.near",
        "#DepthRange.near",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_FLOAT,
        (gctUINT)-1,
        slvSTORAGE_QUALIFIER_UNIFORM,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE
    },
    {
        {slvEXTENSION1_NONE},
        "gl_DepthRange.far",
        "#DepthRange.far",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_FLOAT,
        (gctUINT)-1,
        slvSTORAGE_QUALIFIER_UNIFORM,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE
    },
    {
        {slvEXTENSION1_NONE},
        "gl_DepthRange.diff",
        "#DepthRange.diff",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_FLOAT,
        (gctUINT)-1,
        slvSTORAGE_QUALIFIER_UNIFORM,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE
    },
    {
        {slvEXTENSION1_NONE},
        "gl_DepthRange",
        "#DepthRange",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_STRUCT,
        (gctUINT)-1,
        slvSTORAGE_QUALIFIER_UNIFORM,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE
    },
    {
        {slvEXTENSION1_NONE,slvEXTENSION2_GL_ARB_COMPATIBILITY},
        "gl_LightSource",
        "#LightSource",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_STRUCT,
        8,
        slvSTORAGE_QUALIFIER_UNIFORM,
        LightSourceParameters,
        "gl_LightSourceParameters",
        12,
        gcvFALSE,
        updateForLights
    },
    {
        {slvEXTENSION1_NONE,slvEXTENSION2_GL_ARB_COMPATIBILITY},
        "gl_ModelViewProjectionMatrix",
        "#ff_MVP_Matrix",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_MAT4,
        0,
        slvSTORAGE_QUALIFIER_UNIFORM,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE
    },
    {
        {slvEXTENSION1_NONE,slvEXTENSION2_GL_ARB_COMPATIBILITY},
        "gl_ModelViewMatrix",
        "#ModelViewMatrix",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_MAT4,
        0,
        slvSTORAGE_QUALIFIER_UNIFORM,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE
    },
    {
        {slvEXTENSION1_NONE,slvEXTENSION2_GL_ARB_COMPATIBILITY},
        "gl_ProjectionMatrix",
        "#ProjectionMatrix",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_MAT4,
        0,
        slvSTORAGE_QUALIFIER_UNIFORM,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE},
    {
        {slvEXTENSION1_NONE,slvEXTENSION2_GL_ARB_COMPATIBILITY},
        "gl_NormalMatrix",
        "#NormalMatrix",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_MAT3,
        0,
        slvSTORAGE_QUALIFIER_UNIFORM,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE
    },
    {
        {slvEXTENSION1_NONE,slvEXTENSION2_GL_ARB_COMPATIBILITY},
        "gl_TextureMatrix",
        "#TextureMatrix",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_MAT4,
        8,
        slvSTORAGE_QUALIFIER_UNIFORM,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE,
        updateForTextureCoord
    },

    {
        {slvEXTENSION1_NONE,slvEXTENSION2_GL_ARB_COMPATIBILITY},
        "gl_ModelViewMatrixInverse",
        "#ModelViewMatrixInverse",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_MAT4,
        0,
        slvSTORAGE_QUALIFIER_UNIFORM,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE
    },
    {
        {slvEXTENSION1_NONE,slvEXTENSION2_GL_ARB_COMPATIBILITY},
        "gl_ProjectionMatrixInverse",
        "#ProjectionMatrixInverse",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_MAT4,
        0,
        slvSTORAGE_QUALIFIER_UNIFORM,
        gcvNULL, gcvNULL,
        0,
        gcvFALSE
    },
    {
        {slvEXTENSION1_NONE,slvEXTENSION2_GL_ARB_COMPATIBILITY},
        "gl_ModelViewProjectionMatrixInverse",
        "#ModelViewProjectionMatrixInverse",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_MAT4,
        0,
        slvSTORAGE_QUALIFIER_UNIFORM,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE
    },
    {
        {slvEXTENSION1_NONE,slvEXTENSION2_GL_ARB_COMPATIBILITY},
        "gl_TextureMatrixInverse",
        "#TextureMatrixInverse",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_MAT4,
        8,
        slvSTORAGE_QUALIFIER_UNIFORM,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE
    },

    {
        {slvEXTENSION1_NONE,slvEXTENSION2_GL_ARB_COMPATIBILITY},
        "gl_ModelViewMatrixTranspose",
        "#ModelViewMatrixTranspose",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_MAT4,
        0,
        slvSTORAGE_QUALIFIER_UNIFORM,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE
    },
    {
        {slvEXTENSION1_NONE,slvEXTENSION2_GL_ARB_COMPATIBILITY},
        "gl_ProjectionMatrixTranspose",
        "#ProjectionMatrixTranspose",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_MAT4,
        0,
        slvSTORAGE_QUALIFIER_UNIFORM,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE
    },
    {
        {slvEXTENSION1_NONE,slvEXTENSION2_GL_ARB_COMPATIBILITY},
        "gl_ModelViewProjectionMatrixTranspose",
        "#ModelViewProjectionMatrixTranspose",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_MAT4,
        0,
        slvSTORAGE_QUALIFIER_UNIFORM,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE
    },
    {
        {slvEXTENSION1_NONE,slvEXTENSION2_GL_ARB_COMPATIBILITY},
        "gl_TextureMatrixTranspose",
        "#TextureMatrixTranspose",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_MAT4,
        8,
        slvSTORAGE_QUALIFIER_UNIFORM,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE,
        updateForTextureCoord
    },

    {
        {slvEXTENSION1_NONE,slvEXTENSION2_GL_ARB_COMPATIBILITY},
        "gl_ModelViewMatrixInverseTranspose",
        "#ModelViewMatrixInverseTranspose",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_MAT4,
        0,
        slvSTORAGE_QUALIFIER_UNIFORM,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE
    },
    {
        {slvEXTENSION1_NONE,slvEXTENSION2_GL_ARB_COMPATIBILITY},
        "gl_ProjectionMatrixInverseTranspose",
        "#ProjectionMatrixInverseTranspose",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_MAT4,
        0,
        slvSTORAGE_QUALIFIER_UNIFORM,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE
    },
    {
        {slvEXTENSION1_NONE,slvEXTENSION2_GL_ARB_COMPATIBILITY},
        "gl_ModelViewProjectionMatrixInverseTranspose",
        "#ModelViewProjectionMatrixInverseTranspose",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_MAT4,
        0,
        slvSTORAGE_QUALIFIER_UNIFORM,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE
    },
    {
        {slvEXTENSION1_NONE,slvEXTENSION2_GL_ARB_COMPATIBILITY},
        "gl_TextureMatrixInverseTranspose",
        "#TextureMatrixInverseTranspose",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_MAT4,
        8,
        slvSTORAGE_QUALIFIER_UNIFORM,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE,
        updateForTextureCoord
    },

    {
        {slvEXTENSION1_NONE,slvEXTENSION2_GL_ARB_COMPATIBILITY},
        "gl_NormalScale",
        "#NormalScale",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_MAT4,
        0,
        slvSTORAGE_QUALIFIER_UNIFORM,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE
    },

    {
        {slvEXTENSION1_NONE,slvEXTENSION2_GL_ARB_COMPATIBILITY},
        "gl_ClipPlane",
        "#ClipPlane",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_MAT4,
        8,
        slvSTORAGE_QUALIFIER_UNIFORM,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE
    },
    {
        {slvEXTENSION1_NONE,slvEXTENSION2_GL_ARB_COMPATIBILITY},
        "gl_Point",
        "#Point",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_STRUCT,
        0,
        slvSTORAGE_QUALIFIER_UNIFORM,
        PointParameters,
        "gl_PointParameters",
        7,
        gcvFALSE
    },
    {
        {slvEXTENSION1_NONE,slvEXTENSION2_GL_ARB_COMPATIBILITY},
        "gl_FrontMaterial",
        "#FrontMaterial",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_STRUCT,
        0,
        slvSTORAGE_QUALIFIER_UNIFORM,
        FrontMaterialParameters,
        "gl_FrontMaterialParameters",
        5,
        gcvFALSE
    },
    {
        {slvEXTENSION1_NONE,slvEXTENSION2_GL_ARB_COMPATIBILITY},
        "gl_BackMaterial",
        "#BackMaterial",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_STRUCT,
        0,
        slvSTORAGE_QUALIFIER_UNIFORM,
        BackMaterialParameters,
        "gl_BackMaterialParameters",
        5,
        gcvFALSE
    },
    {
        {slvEXTENSION1_NONE,slvEXTENSION2_GL_ARB_COMPATIBILITY},
        "gl_LightModel",
        "#LightModel",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_STRUCT,
        0,
        slvSTORAGE_QUALIFIER_UNIFORM,
        LightModelParameters,
        "gl_LightModelParameters",
        1,
        gcvFALSE
    },
    {
        {slvEXTENSION1_NONE,slvEXTENSION2_GL_ARB_COMPATIBILITY},
        "gl_FrontLightModelProduct",
        "#FrontLightModelProduct",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_STRUCT,
        0,
        slvSTORAGE_QUALIFIER_UNIFORM,
        FrontLightModelProductParameters,
        "gl_FrontLightModelProductParameters",
        1,
        gcvFALSE
    },
    {
        {slvEXTENSION1_NONE,slvEXTENSION2_GL_ARB_COMPATIBILITY},
        "gl_BackLightModelProduct",
        "#BackLightModelProduct",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_STRUCT,
        0,
        slvSTORAGE_QUALIFIER_UNIFORM,
        BackLightModelProductParameters,
        "gl_BackLightModelProductParameters",
        1,
        gcvFALSE
    },
    {
        {slvEXTENSION1_NONE,slvEXTENSION2_GL_ARB_COMPATIBILITY},
        "gl_FrontLightProduct",
        "#FrontLightProduct",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_STRUCT,
        8,
        slvSTORAGE_QUALIFIER_UNIFORM,
        FrontLightProductParameters,
        "gl_FrontLightProductParameters",
        3,
        gcvFALSE,
        updateForLights
    },
    {
        {slvEXTENSION1_NONE,slvEXTENSION2_GL_ARB_COMPATIBILITY},
        "gl_BackLightProduct",
        "#BackLightProduct",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_STRUCT,
        8,
        slvSTORAGE_QUALIFIER_UNIFORM,
        BackLightProductParameters,
        "gl_BackLightProductParameters",
        3,
        gcvFALSE,
        updateForLights
    },
    {
        {slvEXTENSION1_NONE,slvEXTENSION2_GL_ARB_COMPATIBILITY},
        "gl_TextureEnvColor",
        "#TextureEnvColor",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_VEC4,
        2,
        slvSTORAGE_QUALIFIER_UNIFORM,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE
    },
    {
        {slvEXTENSION1_NONE,slvEXTENSION2_GL_ARB_COMPATIBILITY},
        "gl_EyePlaneS",
        "#EyePlaneS",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_VEC4,
        2,
        slvSTORAGE_QUALIFIER_UNIFORM,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE,
        updateForTextureCoord
    },
    {
        {slvEXTENSION1_NONE,slvEXTENSION2_GL_ARB_COMPATIBILITY},
        "gl_EyePlaneT",
        "#EyePlaneT",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_VEC4,
        2,
        slvSTORAGE_QUALIFIER_UNIFORM,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE,
        updateForTextureCoord
    },
    {
        {slvEXTENSION1_NONE,slvEXTENSION2_GL_ARB_COMPATIBILITY},
        "gl_EyePlaneR",
        "#EyePlaneR",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_VEC4,
        2,
        slvSTORAGE_QUALIFIER_UNIFORM,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE,
        updateForTextureCoord
    },
    {
        {slvEXTENSION1_NONE,slvEXTENSION2_GL_ARB_COMPATIBILITY},
        "gl_EyePlaneQ",
        "#EyePlaneQ",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_VEC4,
        2,
        slvSTORAGE_QUALIFIER_UNIFORM,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE,
        updateForTextureCoord
    },
     {
        {slvEXTENSION1_NONE,slvEXTENSION2_GL_ARB_COMPATIBILITY},
        "gl_ObjectPlaneS",
        "#ObjectPlaneS",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_VEC4,
        2,
        slvSTORAGE_QUALIFIER_UNIFORM,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE,
        updateForTextureCoord
    },
    {
        {slvEXTENSION1_NONE,slvEXTENSION2_GL_ARB_COMPATIBILITY},
        "gl_ObjectPlaneT",
        "#ObjectPlaneT",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_VEC4,
        2,
        slvSTORAGE_QUALIFIER_UNIFORM,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE,
        updateForTextureCoord
    },
    {
        {slvEXTENSION1_NONE,slvEXTENSION2_GL_ARB_COMPATIBILITY},
        "gl_ObjectPlaneR",
        "#ObjectPlaneR",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_VEC4,
        2,
        slvSTORAGE_QUALIFIER_UNIFORM,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE,
        updateForTextureCoord
    },
    {
        {slvEXTENSION1_NONE,slvEXTENSION2_GL_ARB_COMPATIBILITY},
        "gl_ObjectPlaneQ",
        "#ObjectPlaneQ",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_VEC4,
        2,
        slvSTORAGE_QUALIFIER_UNIFORM,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE,
        updateForTextureCoord
    },
    {
        {slvEXTENSION1_NONE,slvEXTENSION2_GL_ARB_COMPATIBILITY},
        "gl_Fog",
        "#Fog",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_STRUCT,
        0,
        slvSTORAGE_QUALIFIER_UNIFORM,
        FogParameters,
        "gl_FogParameters",
        5,
        gcvFALSE
    },