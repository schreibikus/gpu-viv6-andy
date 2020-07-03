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


    /* input. */
    {
        {slvEXTENSION1_EXT_GEOMETRY_SHADER},
        "gl_in",
        "#In",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_IN_IO_BLOCK,
        T_IO_BLOCK,
        -1,
        slvSTORAGE_QUALIFIER_IN_IO_BLOCK,
        PerVertexVariables,
        "gl_PerVertex",
        2,
        gcvFALSE
    },
    {
        {slvEXTENSION1_EXT_GEOMETRY_SHADER},
        "gl_in.gl_Position",
        "#In_Position",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_IN_IO_BLOCK_MEMBER,
        T_VEC4,
        0,
        slvSTORAGE_QUALIFIER_IN_IO_BLOCK_MEMBER,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE
    },
    {
        {slvEXTENSION1_EXT_GEOMETRY_POINT_SIZE},
        "gl_in.gl_PointSize",
        "#In_PointSize",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_IN_IO_BLOCK_MEMBER,
        T_FLOAT,
        0,
        slvSTORAGE_QUALIFIER_IN_IO_BLOCK_MEMBER,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE
    },
    {
        {slvEXTENSION1_EXT_GEOMETRY_SHADER, slvEXTENSION2_GL_GEOMETRY_SHADER4},
        "gl_PrimitiveIDIn",
        "#PrimitiveIDIn",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_VARYING_IN,
        T_INT,
        0,
        slvSTORAGE_QUALIFIER_VARYING_IN,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE
    },
    {
        {slvEXTENSION1_EXT_GEOMETRY_SHADER},
        "gl_InvocationID",
        "#InvocationID",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_VARYING_IN,
        T_INT,
        0,
        slvSTORAGE_QUALIFIER_VARYING_IN,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE
    },
    /* output. */
    {
        {slvEXTENSION1_EXT_GEOMETRY_SHADER},
        gcvNULL,
        "#Out",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_OUT_IO_BLOCK,
        T_IO_BLOCK,
        0,
        slvSTORAGE_QUALIFIER_OUT_IO_BLOCK,
        PerVertexVariables,
        "gl_PerVertex",
        2,
        gcvFALSE
    },
    {
        {slvEXTENSION1_EXT_GEOMETRY_SHADER, slvEXTENSION2_GL_GEOMETRY_SHADER4},
        "gl_Position",
        "#Position",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_VARYING_OUT,
        T_VEC4,
        0,
        slvSTORAGE_QUALIFIER_VARYING_OUT,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE
    },
    {
        {slvEXTENSION1_EXT_GEOMETRY_POINT_SIZE, slvEXTENSION2_GL_GEOMETRY_SHADER4},
        "gl_PointSize",
        "#PointSize",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_VARYING_OUT,
        T_FLOAT,
        0,
        slvSTORAGE_QUALIFIER_VARYING_OUT,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE
    },
    {
        {slvEXTENSION1_EXT_GEOMETRY_SHADER, slvEXTENSION2_GL_GEOMETRY_SHADER4},
        "gl_PrimitiveID",
        "#PrimitiveID",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_VARYING_OUT,
        T_INT,
        0,
        slvSTORAGE_QUALIFIER_VARYING_OUT,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE
    },
    {
        {slvEXTENSION1_EXT_GEOMETRY_SHADER, slvEXTENSION2_GL_GEOMETRY_SHADER4},
        "gl_Layer",
        "#Layer",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_VARYING_OUT,
        T_INT,
        0,
        slvSTORAGE_QUALIFIER_VARYING_OUT,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE
    },
    /* Uniform */
    {
        {slvEXTENSION1_NONE, slvEXTENSION2_GL_GEOMETRY_SHADER4},
        "gl_VerticesIn",
        "#GsVerticesIn",
        slvPRECISION_QUALIFIER_HIGH,
        slvSTORAGE_QUALIFIER_UNIFORM,
        T_INT,
        0,
        slvSTORAGE_QUALIFIER_UNIFORM,
        gcvNULL,
        gcvNULL,
        0,
        gcvFALSE},