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


#include "gc_glff_precomp.h"

#define _GC_OBJ_ZONE gcdZONE_ES11_TRACE

/******************************************************************************\
*********************** Support Functions and Definitions **********************
\******************************************************************************/

static void _UpdateVectorFlags(
    glsVECTOR_PTR Variable
    )
{
    gcmHEADER_ARG("Variable=0x%x", Variable);

    Variable->zero3
        = ((Variable->value[0] == glvFLOATZERO)
        && (Variable->value[1] == glvFLOATZERO)
        && (Variable->value[2] == glvFLOATZERO));

    Variable->zero4
        = ((Variable->value[3] == glvFLOATZERO)
        && (Variable->zero3));

    Variable->one3
        = ((Variable->value[0] == glvFLOATONE)
        && (Variable->value[1] == glvFLOATONE)
        && (Variable->value[2] == glvFLOATONE));

    Variable->one4
        = ((Variable->value[3] == glvFLOATONE)
        && (Variable->one3));

    gcmFOOTER_NO();
}


/******************************************************************************\
***************** Value Conversion From/To OpenGL Enumerations *****************
\******************************************************************************/

/*******************************************************************************
**
**  glfConvertGLEnum
**
**  Converts OpenGL GL_xxx constants into intrernal enumerations.
**
**  INPUT:
**
**      Names
**      NameCount
**          Pointer and a count of the GL enumaration array.
**
**      Value
**      Type
**          GL enumeration value to be converted.
**
**  OUTPUT:
**
**      Result
**          Internal value corresponding to the GL enumeration.
*/

GLboolean glfConvertGLEnum(
    const GLenum* Names,
    GLint NameCount,
    const GLvoid* Value,
    gleTYPE Type,
    GLuint* Result
    )
{
    /* Convert the enum. */
    GLenum value = (Type == glvFLOAT)
        ? glmFLOAT2INT(* (GLfloat *) Value)
        :             (* (GLint   *) Value);

    /* Search the map for it. */
    GLint i;

    gcmHEADER_ARG("Names=0x%x NameCount=%d Value=0x%x Type=0x%04x Result=0x%x",
                    Names, NameCount, Value, Type, Result);

    for (i = 0; i < NameCount; i++)
    {
        if (Names[i] == value)
        {
            *Result = i;
            gcmFOOTER_ARG("return=%d", GL_TRUE);
            return GL_TRUE;
        }
    }

    gcmFOOTER_ARG("return=%d", GL_FALSE);

    /* Bad enum. */
    return GL_FALSE;
}

/*******************************************************************************
**
**  glfConvertGLboolean
**
**  Validates OpenGL GLboolean constants.
**
**  INPUT:
**
**      Value
**      Type
**          GL enumeration value to be converted.
**
**  OUTPUT:
**
**      Result
**          Internal value corresponding to the GL enumeration.
*/

GLboolean glfConvertGLboolean(
    const GLvoid* Value,
    gleTYPE Type,
    GLuint* Result
    )
{
    GLboolean result;
    static GLenum _BooleanNames[] =
    {
        GL_FALSE,
        GL_TRUE
    };

    gcmHEADER_ARG("Value=0x%x Type=0x%04x Result=0x%x", Value, Type, Result);

    result = glfConvertGLEnum(
        _BooleanNames,
        gcmCOUNTOF(_BooleanNames),
        Value,
        Type,
        Result
        );

    gcmFOOTER_ARG("return=%d", result);

    return result;
}


/******************************************************************************\
***************************** Value Type Converters ****************************
\******************************************************************************/

void glfGetFromBool(
    GLboolean Variable,
    GLvoid* Value,
    gleTYPE Type
    )
{
    gcmHEADER_ARG("Variable=%d Value=0x%x Type=0x%04x", Variable, Value, Type);
    glfGetFromBoolArray(&Variable, 1, Value, Type);
    gcmFOOTER_NO();
}

void glfGetFromBoolArray(
    const GLboolean* Variables,
    GLint Count,
    GLvoid* Value,
    gleTYPE Type
    )
{
    GLint i;
    gcmHEADER_ARG("Variable=%d Value=0x%x Type=0x%04x", Variables, Value, Type);

    for (i = 0; i < Count; i++)
    {
        switch (Type)
        {
        case glvBOOL:
            ((GLboolean *) Value) [i] = Variables[i];
            break;

        case glvINT:
            ((GLint *) Value) [i] = glmBOOL2INT(Variables[i]);
            break;

        case glvFIXED:
            ((GLfixed *) Value) [i] = glmBOOL2FIXED(Variables[i]);
            break;

        case glvFLOAT:
            ((GLfloat *) Value) [i] = glmBOOL2FLOAT(Variables[i]);
            break;

        default:
            gcmFATAL("glfGetFromIntArray: invalid type %d", Type);
        }
    }
    gcmFOOTER_NO();
}

void glfGetFromInt(
    GLint Variable,
    GLvoid* Value,
    gleTYPE Type
    )
{
    gcmHEADER_ARG("Variable=%d Value=0x%x Type=0x%04x", Variable, Value, Type);
    glfGetFromIntArray(&Variable, 1, Value, Type);
    gcmFOOTER_NO();
}

void glfGetFromIntArray(
    const GLint* Variables,
    GLint Count,
    GLvoid* Value,
    gleTYPE Type
    )
{
    GLint i;

    gcmHEADER_ARG("Variables=0x%x Count=%d Value=0x%x Type=0x%04x", Variables, Count, Value, Type);

    for (i = 0; i < Count; i++)
    {
        switch (Type)
        {
        case glvBOOL:
            ((GLboolean *) Value) [i] = glmINT2BOOL(Variables[i]);
            break;

        case glvINT:
            ((GLint *) Value) [i] = Variables[i];
            break;

        case glvFIXED:
            ((GLfixed *) Value) [i] = glmINT2FIXED(Variables[i]);
            break;

        case glvFLOAT:
            ((GLfloat *) Value) [i] = glmINT2FLOAT(Variables[i]);
            break;

        default:
            gcmFATAL("glfGetFromIntArray: invalid type %d", Type);
        }
    }
    gcmFOOTER_NO();
}

void glfGetFromFixed(
    GLfixed Variable,
    GLvoid* Value,
    gleTYPE Type
    )
{
    gcmHEADER_ARG("Variable=%d Value=0x%x Type=0x%04x", Variable, Value, Type);
    glfGetFromFixedArray(&Variable, 1, Value, Type);
    gcmFOOTER_NO();
}

void glfGetFromFixedArray(
    const GLfixed* Variables,
    GLint Count,
    GLvoid* Value,
    gleTYPE Type
    )
{
    GLint i;

    gcmHEADER_ARG("Variables=0x%x Count=%d Value=0x%x Type=0x%04x", Variables, Count, Value, Type);

    for (i = 0; i < Count; i++)
    {
        switch (Type)
        {
        case glvBOOL:
            ((GLboolean *) Value) [i] = glmFIXED2BOOL(Variables[i]);
            break;

        case glvINT:
            ((GLint *) Value) [i] = glmFIXED2INT(Variables[i]);
            break;

        case glvNORM:
            ((GLint *) Value) [i] = glmFIXED2NORM(Variables[i]);
            break;

        case glvFIXED:
            ((GLfixed *) Value) [i] = Variables[i];
            break;

        case glvFLOAT:
            ((GLfloat *) Value) [i] = glmFIXED2FLOAT(Variables[i]);
            break;

        default:
            gcmFATAL("glfGetFromFixedArray: invalid type %d", Type);
        }
    }
    gcmFOOTER_NO();
}

void glfGetFromFloat(
    GLfloat Variable,
    GLvoid* Value,
    gleTYPE Type
    )
{
    gcmHEADER_ARG("Variable=%f Value=0x%x Type=0x%04x", Variable, Value, Type);
    glfGetFromFloatArray(&Variable, 1, Value, Type);
    gcmFOOTER_NO();
}

void glfGetFromFloatArray(
    const GLfloat* Variables,
    GLint Count,
    GLvoid* Value,
    gleTYPE Type
    )
{
    GLint i;
    gcmHEADER_ARG("Variables=%f Count=%d Value=0x%x Type=0x%04x", Variables, Count, Value, Type);

    for (i = 0; i < Count; i++)
    {
        switch (Type)
        {
        case glvBOOL:
            ((GLboolean *) Value) [i] = glmFLOAT2BOOL(Variables[i]);
            break;

        case glvINT:
            ((GLint *) Value) [i] = glmFLOAT2INT(Variables[i]);
            break;

        case glvNORM:
            ((GLint *) Value) [i] = glmFLOAT2NORM(Variables[i]);
            break;

        case glvFIXED:
            ((GLfixed *) Value) [i] = glmFLOAT2FIXED(Variables[i]);
            break;

        case glvFLOAT:
            ((GLfloat *) Value) [i] = Variables[i];
            break;

        default:
            gcmFATAL("glfGetFromFloatArray: invalid type %d", Type);
        }
    }
    gcmFOOTER_NO();
}

void glfGetFloatFromFloatArray(
    const GLfloat* Variables,
    GLint Count,
    GLfloat* Value
    )
{
    GLint i;
    gcmHEADER_ARG("Variables=%f Count=%d Value=0x%x", Variables, Count, Value);
    for (i = 0; i < Count; i++)
    {
        Value[i] = Variables[i];
    }
    gcmFOOTER_NO();
}

void glfGetFromEnum(
    GLenum Variable,
    GLvoid* Value,
    gleTYPE Type
    )
{
    gcmHEADER_ARG("Variable=0x%04x Value=0x%x Type=0x%04x", Variable, Value, Type);
    glfGetFromEnumArray(&Variable, 1, Value, Type);
    gcmFOOTER_NO();
}

void glfGetFromEnumArray(
    const GLenum* Variables,
    GLint Count,
    GLvoid* Value,
    gleTYPE Type
    )
{
    GLint i;

    gcmHEADER_ARG("Variables=0x%x Count=%d Value=0x%x Type=0x%04x", Variables, Count, Value, Type);

    for (i = 0; i < Count; i++)
    {
        switch (Type)
        {
        case glvBOOL:
            ((GLboolean *) Value) [i] = glmINT2BOOL(Variables[i]);
            break;

        case glvINT:
            ((GLint *) Value) [i] = Variables[i];
            break;

        case glvFIXED:
            ((GLfixed *) Value) [i] = Variables[i];
            break;

        case glvFLOAT:
            ((GLfloat *) Value) [i] = glmINT2FLOAT(Variables[i]);
            break;

        default:
            gcmFATAL("glfGetFromEnumArray: invalid type %d", Type);
        }
    }
    gcmFOOTER_NO();
}

void glfGetFromVector3(
    const glsVECTOR_PTR Variable,
    GLvoid* Value,
    gleTYPE Type
    )
{
    GLint i;
    gcmHEADER_ARG("Variable=0x%x Value=0x%x Type=0x%04x",
                    Variable, Value, Type);

    switch (Type)
    {
    case glvBOOL:
        for (i = 0; i < 3; i++)
        {
            glfGetFromFloat(
                Variable->value[i],
                & ((GLboolean *) Value) [i],
                Type
                );
        }
        break;

    case glvINT:
        for (i = 0; i < 3; i++)
        {
            glfGetFromFloat(
                Variable->value[i],
                & ((GLint *) Value) [i],
                Type
                );
        }
        break;

    case glvFIXED:
        for (i = 0; i < 3; i++)
        {
            glfGetFromFloat(
                Variable->value[i],
                & ((GLfixed *) Value) [i],
                Type
                );
        }
        break;

    case glvFLOAT:
        for (i = 0; i < 3; i++)
        {
            glfGetFromFloat(
                Variable->value[i],
                & ((GLfloat *) Value) [i],
                Type
                );
        }
        break;

    default:
        gcmFATAL("glfGetFromVector3: invalid type %d", Type);
    }
    gcmFOOTER_NO();
}

void glfGetFloatFromVector3(
    const glsVECTOR_PTR Variable,
    GLfloat* Value
    )
{
    GLint i;
    gcmHEADER_ARG("Variable=0x%x Value=0x%x", Variable, Value);

    for (i = 0; i < 3; i++)
    {
        Value[i] = Variable->value[i];
    }
    gcmFOOTER_NO();
}


void glfGetFromVector4(
    const glsVECTOR_PTR Variable,
    GLvoid* Value,
    gleTYPE Type
    )
{
    GLuint i;
    gcmHEADER_ARG("Variable=0x%x Value=0x%x Type=0x%04x",
                    Variable, Value, Type);

    switch (Type)
    {
    case glvBOOL:
        for (i = 0; i < gcmCOUNTOF(Variable->value); i++)
        {
            glfGetFromFloat(
                Variable->value[i],
                & ((GLboolean *) Value) [i],
                Type
                );
        }
        break;

    case glvINT:
    case glvNORM:
        for (i = 0; i < gcmCOUNTOF(Variable->value); i++)
        {
            glfGetFromFloat(
                Variable->value[i],
                & ((GLint *) Value) [i],
                Type
                );
        }
        break;

    case glvFIXED:
        for (i = 0; i < gcmCOUNTOF(Variable->value); i++)
        {
            glfGetFromFloat(
                Variable->value[i],
                & ((GLfixed *) Value) [i],
                Type
                );
        }
        break;

    case glvFLOAT:
        for (i = 0; i < gcmCOUNTOF(Variable->value); i++)
        {
            glfGetFromFloat(
                Variable->value[i],
                & ((GLfloat *) Value) [i],
                Type
                );
        }
        break;

    default:
        gcmFATAL("glfGetFromVector4: invalid type %d", Type);
    }
    gcmFOOTER_NO();
}

void glfGetFloatFromVector4(
    const glsVECTOR_PTR Variable,
    GLfloat* Value
    )
{
    GLuint i;
    gcmHEADER_ARG("Variable=0x%x Value=0x%x", Variable, Value);
    for (i = 0; i < gcmCOUNTOF(Variable->value); i++)
    {
        Value[i] = Variable->value[i];
    }
    gcmFOOTER_NO();
}

void glfGetFromMatrix(
    const glsMATRIX_PTR Variable,
    GLvoid* Value,
    gleTYPE Type
    )
{
    GLuint i;
    gcmHEADER_ARG("Variable=0x%x Value=0x%x Type=0x%04x",
                    Variable, Value, Type);

    switch (Type)
    {
    case glvBOOL:
        for (i = 0; i < gcmCOUNTOF(Variable->value); i++)
        {
            glfGetFromFloat(
                Variable->value[i],
                & ((GLboolean *) Value) [i],
                Type
                );
        }
        break;

    case glvINT:
    case glvNORM:
        for (i = 0; i < gcmCOUNTOF(Variable->value); i++)
        {
            glfGetFromFloat(
                Variable->value[i],
                & ((GLint *) Value) [i],
                Type
                );
        }
        break;

    case glvFIXED:
        for (i = 0; i < gcmCOUNTOF(Variable->value); i++)
        {
            glfGetFromFloat(
                Variable->value[i],
                & ((GLfixed *) Value) [i],
                Type
                );
        }
        break;

    case glvFLOAT:
        for (i = 0; i < gcmCOUNTOF(Variable->value); i++)
        {
            glfGetFromFloat(
                Variable->value[i],
                & ((GLfloat *) Value) [i],
                Type
                );
        }
        break;

    default:
        gcmFATAL("glfGetFromMatrix: invalid type %d", Type);
    }
    gcmFOOTER_NO();
}

void glfGetFloatFromMatrix(
    const glsMATRIX_PTR Variable,
    GLfloat* Value
    )
{
    GLuint i;
    gcmHEADER_ARG("Variable=0x%x Value=0x%x", Variable, Value);
    for (i = 0; i < gcmCOUNTOF(Variable->value); i++)
    {
        Value[i] = Variable->value[i];
    }
    gcmFOOTER_NO();
}


/******************************************************************************\
**************************** Get Values From Raw Input *************************
\******************************************************************************/

GLfixed glfFixedFromRaw(
    const GLvoid* Variable,
    gleTYPE Type
    )
{
    GLfixed result=0;
    gcmHEADER_ARG("Variable=0x%x Type=0x%04x", Variable, Type);
    switch (Type)
    {
    case glvINT:
        result = glmINT2FIXED(* (GLint *) Variable);
        break;

    case glvFIXED:
        result = * (GLfixed *) Variable;
        break;

    case glvFLOAT:
        result = glmFLOAT2FIXED(* (GLfloat *) Variable);
        break;

    default:
        gcmFATAL("glfFixedFromRaw: invalid type %d", Type);
        break;
    }
    gcmFOOTER_ARG("return=0x%d08x", result);
    return result;
}

GLfloat glfFloatFromRaw(
    const GLvoid* Variable,
    gleTYPE Type
    )
{
    GLfloat result = 0.0;
    gcmHEADER_ARG("Variable=0x%x Type=0x%04x", Variable, Type);
    switch (Type)
    {
    case glvINT:
        result = glmINT2FLOAT(* (GLint *) Variable);
        break;
    case glvFIXED:
        result = glmFIXED2FLOAT(* (GLfixed *) Variable);
        break;
    case glvFLOAT:
        result = * (GLfloat *) Variable;
        break;

    default:
        gcmFATAL("glfFloatFromRaw: invalid type %d", Type);
        break;
    }
    gcmFOOTER_ARG("return=%f", result);
    return result;
}

/******************************************************************************\
***************************** Set Values To Vectors ****************************
\******************************************************************************/

void glfSetIntVector4(
    glsVECTOR_PTR Variable,
    GLint X,
    GLint Y,
    GLint Z,
    GLint W
    )
{
    gcmHEADER_ARG("Variable=0x%x X=%d Y=%d Z=%d W=%d", Variable, X, Y, Z, W);

    /* Set components. */
    Variable->value[0] = glmINT2FLOAT(X);
    Variable->value[1] = glmINT2FLOAT(Y);
    Variable->value[2] = glmINT2FLOAT(Z);
    Variable->value[3] = glmINT2FLOAT(W);

    /* Update special value flags. */
    _UpdateVectorFlags(Variable);

    gcmFOOTER_NO();
}

void glfSetFixedVector4(
    glsVECTOR_PTR Variable,
    GLfixed X,
    GLfixed Y,
    GLfixed Z,
    GLfixed W
    )
{
    gcmHEADER_ARG("Variable=0x%x X=0x%08x Y=0x%08x Z=0x%08x W=0x%08x", Variable, X, Y, Z, W);

    /* Set components. */
    Variable->value[0] = glmFIXED2FLOAT(X);
    Variable->value[1] = glmFIXED2FLOAT(Y);
    Variable->value[2] = glmFIXED2FLOAT(Z);
    Variable->value[3] = glmFIXED2FLOAT(W);

    /* Update special value flags. */
    _UpdateVectorFlags(Variable);

    gcmFOOTER_NO();
}

void glfSetFloatVector4(
    glsVECTOR_PTR Variable,
    GLfloat X,
    GLfloat Y,
    GLfloat Z,
    GLfloat W
    )
{
    gcmHEADER_ARG("Variable=0x%x X=%f Y=%f Z=%f W=%f", Variable, X, Y, Z, W);

    /* Set components. */
    Variable->value[0] = X;
    Variable->value[1] = Y;
    Variable->value[2] = Z;
    Variable->value[3] = W;

    /* Update special value flags. */
    _UpdateVectorFlags(Variable);

    gcmFOOTER_NO();
}

void glfSetVector3(
    glsVECTOR_PTR Variable,
    const GLfloat* Value
    )
{
    GLint i;

    gcmHEADER_ARG("Variable=0x%x Value=0x%x", Variable, Value);

    for (i = 0; i < 3; i++)
    {
        Variable->value[i] = Value[i];
    }

    /* Set the fourth component to zero. */
    Variable->value[3] = glvFLOATZERO;

    /* Update special value flags. */
    _UpdateVectorFlags(Variable);

    gcmFOOTER_NO();
}

void glfSetVector4(
    glsVECTOR_PTR Variable,
    const GLfloat* Value
    )
{
    GLuint i;
    gcmHEADER_ARG("Variable=0x%x Value=0x%x", Variable, Value);

    for (i = 0; i < gcmCOUNTOF(Variable->value); i++)
    {
        Variable->value[i] = Value[i];
    }

    /* Update special value flags. */
    _UpdateVectorFlags(Variable);

    gcmFOOTER_NO();
}

void glfSetClampedVector4(
    glsVECTOR_PTR Variable,
    const GLfloat* Value
    )
{
    GLuint i;
    gcmHEADER_ARG("Variable=0x%x Value=0x%x", Variable, Value);

    for (i = 0; i < gcmCOUNTOF(Variable->value); i++)
    {
        Variable->value[i]
            = glmFLOATCLAMP_0_TO_1(Value[i]);
    }

    /* Update special value flags. */
    _UpdateVectorFlags(Variable);

    gcmFOOTER_NO();
}

void glfSetHomogeneousVector4(
    glsVECTOR_PTR Variable,
    const GLfloat* Value
    )
{
    gcmHEADER_ARG("Variable=0x%x Value=0x%x", Variable, Value);

    /* Set the value. */

    if ((Value[3] != 0.0f) && (Value[3] != 1.0f))
    {
        Variable->value[0] = gcoMATH_Divide(Value[0], Value[3]);
        Variable->value[1] = gcoMATH_Divide(Value[1], Value[3]);
        Variable->value[2] = gcoMATH_Divide(Value[2], Value[3]);
        Variable->value[3] = glvFLOATONE;
    }
    else
    {
        Variable->value[0] = Value[0];
        Variable->value[1] = Value[1];
        Variable->value[2] = Value[2];
        Variable->value[3] = Value[3];
    }


    /* Update special value flags. */
    _UpdateVectorFlags(Variable);

    gcmFOOTER_NO();
}

/******************************************************************************\
*********************** Operations on Mutants and Vectors **********************
\******************************************************************************/

void glfCos(
    const GLfloat* Value,
    GLfloat* Result
    )
{
    GLfloat radians;
    GLfloat cosine;

    gcmHEADER_ARG("Mutant=0x%x Result=0x%x", Value, Result);

    /* Convert to randians. */
    radians = (*Value) * glvFLOATPIOVER180;

    /* Compute cos. */
    cosine = gcoMATH_Cosine(radians);

    /* Set the value. */
    *Result = cosine;

    gcmFOOTER_NO();
}

void glfAddVector4(
    const glsVECTOR_PTR Variable1,
    const glsVECTOR_PTR Variable2,
    glsVECTOR_PTR Result
    )
{
    GLint i;
    GLfloat vec1[4];
    GLfloat vec2[4];
    GLfloat result[4];

    gcmHEADER_ARG("Variable1=0x%x Variable2=0x%x Result=0x%x", Variable1, Variable2, Result);

    glfGetFloatFromVector4(Variable1, vec1);
    glfGetFloatFromVector4(Variable2, vec2);

    for (i = 0; i < 4; i++)
    {
        result[i] = vec1[i] + vec2[i];
    }

    glfSetVector4(Result, result);

    gcmFOOTER_NO();
}


void glfMulVector4(
    const glsVECTOR_PTR Variable1,
    const glsVECTOR_PTR Variable2,
    glsVECTOR_PTR Result
    )
{
    GLint i;
    GLfloat vec1[4];
    GLfloat vec2[4];
    GLfloat result[4];

    gcmHEADER_ARG("Variable1=0x%x Variable2=0x%x Result=0x%x", Variable1, Variable2, Result);

    glfGetFloatFromVector4(Variable1, vec1);
    glfGetFloatFromVector4(Variable2, vec2);

    for (i = 0; i < 4; i++)
    {
        result[i] = vec1[i] * vec2[i];
    }

    glfSetVector4(Result, result);

    gcmFOOTER_NO();
}

void glfNorm3Vector4(
    const glsVECTOR_PTR Vector,
    glsVECTOR_PTR Result
    )
{
    GLfloat squareSum;
    GLfloat norm;
    GLfloat x, y, z, w;

    gcmHEADER_ARG("Vector=0x%x Result=0x%x", Vector, Result);

    /* Dispatch base on the type. */

    /* Compute normal. */
    squareSum
        = Vector->value[0] * Vector->value[0]
        + Vector->value[1] * Vector->value[1]
        + Vector->value[2] * Vector->value[2];

    norm = (squareSum < 0)
        ? squareSum
        : gcoMATH_Divide(1.0f, gcoMATH_SquareRoot(squareSum));

    /* Multiply vector by normal. */
    x = Vector->value[0] * norm;
    y = Vector->value[1] * norm;
    z = Vector->value[2] * norm;
    w = 0.0f;

    /* Set the value. */
    glfSetFloatVector4(Result, x, y, z, w);

    gcmFOOTER_NO();
}

/* Normalize vector3 to vector4.                */
/* Using float to do intermedia calculation.    */
void glfNorm3Vector4f(
    const glsVECTOR_PTR Vector,
    glsVECTOR_PTR Result
    )
{
    GLfloat vec[4];
    GLfloat squareSum, norm;
    GLfloat x, y, z, w;

    gcmHEADER_ARG("Vector=0x%x Result=0x%x", Vector, Result);

    /* Dispatch base on the type. */
    /* Convert vector to float type. */
    glfGetFloatFromVector4(Vector, vec);

    /* Compute normal. */
    squareSum
        = vec[0] * vec[0]
        + vec[1] * vec[1]
        + vec[2] * vec[2];

    norm = (squareSum < 0)
        ? squareSum
        : gcoMATH_Divide(1.0f, gcoMATH_SquareRoot(squareSum));

    /* Multiply vector by normal. */
    x = vec[0] * norm;
    y = vec[1] * norm;
    z = vec[2] * norm;
    w = 0.0f;

    /* Set the value. */
    glfSetFloatVector4(Result, x, y, z, w);

    gcmFOOTER_NO();
}

void glfHomogeneousVector4(
    const glsVECTOR_PTR Vector,
    glsVECTOR_PTR Result
    )
{
    GLfloat x, y, z, w;

    gcmHEADER_ARG("Vector=0x%x Result=0x%x", Vector, Result);

    /* Dispatch base on the type. */
    /* Make vector homogeneous. */
    if ((Vector->value[3] != 0.0f) &&
        (Vector->value[3] != 1.0f))
    {
        x = gcoMATH_Divide(Vector->value[0], Vector->value[3]);
        y = gcoMATH_Divide(Vector->value[1], Vector->value[3]);
        z = gcoMATH_Divide(Vector->value[2], Vector->value[3]);
        w = 1.0f;
    }
    else
    {
        x = Vector->value[0];
        y = Vector->value[1];
        z = Vector->value[2];
        w = Vector->value[3];
    }

    /* Set the value. */
    glfSetFloatVector4(Result, x, y, z, w);

    gcmFOOTER_NO();
}


/******************************************************************************\
**************** Debug Printing for Mutants, Vectors and Matrices **************
\******************************************************************************/

#if gcmIS_DEBUG(gcdDEBUG_TRACE)

void glfPrintMatrix3x3(
    gctSTRING Name,
    glsMATRIX_PTR Matrix
    )
{
    gctUINT y;
    gctFLOAT values[4 * 4];

    gcmHEADER_ARG("Name=0x%x Matrix=0x%x", Name, Matrix);

    glfGetFloatFromMatrix(Matrix, values);

    gcmTRACE(
        gcvLEVEL_INFO,
        "%s:", Name
        );

    for (y = 0; y < 3; y += 1)
    {
        gcmTRACE(
            gcvLEVEL_INFO,
            "  %.6f, %.6f, %.6f",
            values[y + 0 * 4],
            values[y + 1 * 4],
            values[y + 2 * 4]
            );
    }
    gcmFOOTER_NO();
}

void glfPrintMatrix4x4(
    gctSTRING Name,
    glsMATRIX_PTR Matrix
    )
{
    gctUINT y;
    gctFLOAT values[4 * 4];

    gcmHEADER_ARG("Name=0x%x Matrix=0x%x", Name, Matrix);

    glfGetFloatFromMatrix(Matrix, values);

    gcmTRACE(
        gcvLEVEL_INFO,
        "%s:", Name
        );

    for (y = 0; y < 4; y += 1)
    {
        gcmTRACE(
            gcvLEVEL_INFO,
            "  %.6f, %.6f, %.6f, %.6f",
            values[y + 0 * 4],
            values[y + 1 * 4],
            values[y + 2 * 4],
            values[y + 3 * 4]
            );
    }
    gcmFOOTER_NO();
}

void glfPrintVector3(
    gctSTRING Name,
    glsVECTOR_PTR Vector
    )
{
    gctFLOAT values[3];

    gcmHEADER_ARG("Name=0x%x Vector=0x%x", Name, Vector);

    glfGetFloatFromVector3(Vector, values);

    gcmTRACE(
        gcvLEVEL_INFO,
        "%s: %.6f, %.6f, %.6f",
        Name, values[0], values[1], values[2]
        );

    gcmFOOTER_NO();
}

void glfPrintVector4(
    gctSTRING Name,
    glsVECTOR_PTR Vector
    )
{
    gctFLOAT values[4];

    gcmHEADER_ARG("Name=0x%x Vector=0x%x", Name, Vector);

    glfGetFloatFromVector4(Vector, values);

    gcmTRACE(
        gcvLEVEL_INFO,
        "%s: %.6f, %.6f, %.6f, %.6f",
        Name, values[0], values[1], values[2], values[3]
        );

    gcmFOOTER_NO();
}

#endif
