/****************************************************************************
*
*    Copyright (c) 2005 - 2016 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "vir/lower/gc_vsc_vir_ml_2_ll.h"

#include "vir/lower/gc_vsc_vir_lower_common_func.h"

static gctBOOL
_hasNEW_SIN_COS_LOG_DIV(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return ((VIR_PatternLowerContext *)Context)->hwCfg->hwFeatureFlags.hasNewSinCosLogDiv;
}

static gctBOOL
_SetUIntFloatFour(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.uValue = 0xf0000004,

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_UINT32,
        imm0);

    return gcvTRUE;
}

static gctBOOL
_hasNot32IntDIV(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return gcvTRUE;
}

static VIR_PatternMatchInst _divPatInst0[] = {
    { VIR_OP_DIV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _hasNEW_SIN_COS_LOG_DIV, VIR_Lower_IsFloatOpcode }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _divRepInst0[] = {
    { VIR_OP_PRE_DIV, 0, VIR_PATTERN_TEMP_TYPE_XY, { -1, 2, 3, 0 }, { 0 } },
    { VIR_OP_MUL, 0, 0, {  1, -1, -1, 0 }, { 0, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleY } },
};

static VIR_Pattern _divPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_div, 0) },
    { VIR_PATN_FLAG_NONE }
};

/*
** MOD, temp(1), temp(2), temp(3)
** -->
** DIV, temp(4), temp(2), temp(3)
** FLOOR, temp(4), temp(4)
** MAD, temp(1), -temp(3),temp(4), temp(2)
*/
static VIR_PatternMatchInst _modPatInst0[] = {
    { VIR_OP_MOD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { VIR_Lower_IsDstFloat }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _modRepInst0[] = {
    { VIR_OP_DIV, 0, 0, { -1, 2, 3, 0 }, { 0 } },
    { VIR_OP_FLOOR, 0, 0, { -1, -1, 0, 0 }, { 0 } },
    { VIR_OP_MAD, 0, 0, { 1, 3, -1, 2 }, { 0, VIR_Lower_SetOpndNeg, 0, 0 } },
};

static VIR_PatternMatchInst _modPatInst1[] = {
    { VIR_OP_MOD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _hasNot32IntDIV, VIR_Lower_IsIntOpcode, VIR_Lower_IsDstInt32, VIR_Lower_IsDstSigned }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _modRepInst1[] = {
    { VIR_OP_SIGN, 0, 0, { -1, 3, 0, 0 }, { 0 } },
    { VIR_OP_SIGN, 0, 0, { -2, 2, 0, 0 }, { 0 } },
    { VIR_OP_MUL, 0, 0, { -1, -1, -2, 0 }, { 0 } },
    { VIR_OP_ABS, 0, 0, { -3, 2, 0, 0 }, { VIR_Lower_SetOpndUINT32 } },
    { VIR_OP_ABS, 0, 0, { -4, 3, 0, 0 }, { VIR_Lower_SetOpndUINT32 } },
    { VIR_OP_JMPC, VIR_COP_LESS_OR_EQUAL, 0, { 0, -4, 0, 0 }, { 0, 0, VIR_Lower_SetUIntOne } },
    { VIR_OP_AQ_I2F, 0, 0, { -5, -4, 0, 0 }, { VIR_Lower_SetOpndFloat } },
    { VIR_OP_ADD, 0, 0, { -6, -5, 0, 0 }, { VIR_Lower_SetOpndUINT32, 0, _SetUIntFloatFour } },
    { VIR_OP_RCP, 0, 0, { -7, -6, 0, 0 }, { VIR_Lower_SetOpndFloat } },
    { VIR_OP_AQ_F2I, 0, 0, { -8, -7, 0, 0 }, { VIR_Lower_SetOpndUINT32 } },
    { VIR_OP_SUB, 0, 0, { -9, 0, -4, 0 }, { VIR_Lower_SetOpndUINT32, VIR_Lower_SetIntZero} },
    { VIR_OP_MUL, 0, 0, { -10, -9, -8, 0 }, { VIR_Lower_SetOpndUINT32 } },
    { VIR_OP_MULHI, 0, 0, { -11, -10, -8, 0 }, { VIR_Lower_SetOpndUINT32 } },
    { VIR_OP_ADD, 0, 0, { -12, -11, -8, 0 }, { VIR_Lower_SetOpndUINT32 } },
    { VIR_OP_MUL, 0, 0, { -13, -12, -4, 0 }, { VIR_Lower_SetOpndUINT32 } },
    { VIR_OP_JMPC, VIR_COP_GREATER, 0, { 0, -13, -9, 0 }, { 0 } },
    { VIR_OP_ADD, 0, 0, { -12, -12, 0, 0 }, { VIR_Lower_SetOpndUINT32, 0, VIR_Lower_SetUIntOne} },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg2 } },
    { VIR_OP_MULHI, 0, 0, { -13, -12, -3, 0 }, { VIR_Lower_SetOpndUINT32 } },
    { VIR_OP_MUL, 0, 0, { -14, -13, -9, 0 }, { VIR_Lower_SetOpndUINT32 } },
    { VIR_OP_ADD, 0, 0, { -14, -3, -14, 0 }, { VIR_Lower_SetOpndUINT32 } },
    { VIR_OP_JMPC, VIR_COP_LESS, 0, { 0, -14, -4, 0 }, { 0 } },
    { VIR_OP_ADD, 0, 0, { 1, -13, 0, 0 }, { 0, 0, VIR_Lower_SetUIntOne} },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { 0 } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg3 } },
    { VIR_OP_MOV, 0, 0, { 1, -13, 0, 0 }, { 0 } },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { 0 } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg22 } },
    { VIR_OP_MOV, 0, 0, { 1, -3, 0, 0 }, { 0 } },
    { VIR_OP_LABEL, 0, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg3_6 } },
    { VIR_OP_MUL, 0, 0, { 1, -1, 1, 0 }, { 0 } },
    { VIR_OP_MUL, 0, 0, { 1, 3, 1, 0 }, { 0 } },
    { VIR_OP_SUB, 0, 0, { 1, 2, 1, 0 }, { 0 } },
};

static VIR_PatternMatchInst _modPatInst2[] = {
    { VIR_OP_MOD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _hasNot32IntDIV, VIR_Lower_IsIntOpcode, VIR_Lower_IsDstInt32 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _modRepInst2[] = {
    { VIR_OP_JMPC, VIR_COP_LESS_OR_EQUAL, 0, { 0, 3, 0, 0 }, { 0, 0, VIR_Lower_SetUIntOne } },
    { VIR_OP_AQ_I2F, 0, 0, { -5, 3, 0, 0 }, { VIR_Lower_SetOpndFloat } },
    { VIR_OP_ADD, 0, 0, { -6, -5, 0, 0 }, { VIR_Lower_SetOpndUINT32, 0, _SetUIntFloatFour } },
    { VIR_OP_RCP, 0, 0, { -7, -6, 0, 0 }, { VIR_Lower_SetOpndFloat } },
    { VIR_OP_AQ_F2I, 0, 0, { -8, -7, 0, 0 }, { VIR_Lower_SetOpndUINT32 } },
    { VIR_OP_SUB, 0, 0, { -9, 0, 3, 0 }, { VIR_Lower_SetOpndUINT32, VIR_Lower_SetIntZero} },
    { VIR_OP_MUL, 0, 0, { -10, -9, -8, 0 }, { VIR_Lower_SetOpndUINT32 } },
    { VIR_OP_MULHI, 0, 0, { -11, -10, -8, 0 }, { VIR_Lower_SetOpndUINT32 } },
    { VIR_OP_ADD, 0, 0, { -12, -11, -8, 0 }, { VIR_Lower_SetOpndUINT32 } },
    { VIR_OP_MUL, 0, 0, { -13, -12, 3, 0 }, { VIR_Lower_SetOpndUINT32 } },
    { VIR_OP_JMPC, VIR_COP_GREATER, 0, { 0, -13, -9, 0 }, { 0 } },
    { VIR_OP_ADD, 0, 0, { -12, -12, 0, 0 }, { VIR_Lower_SetOpndUINT32, 0, VIR_Lower_SetUIntOne} },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg2 } },
    { VIR_OP_MULHI, 0, 0, { -13, -12, 2, 0 }, { VIR_Lower_SetOpndUINT32 } },
    { VIR_OP_MUL, 0, 0, { -14, -13, -9, 0 }, { VIR_Lower_SetOpndUINT32 } },
    { VIR_OP_ADD, 0, 0, { -14, 2, -14, 0 }, { VIR_Lower_SetOpndUINT32 } },
    { VIR_OP_JMPC, VIR_COP_LESS, 0, { 0, -14, 3, 0 }, { 0 } },
    { VIR_OP_ADD, 0, 0, { 1, -13, 0, 0 }, { 0, 0, VIR_Lower_SetUIntOne} },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { 0 } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg3 } },
    { VIR_OP_MOV, 0, 0, { 1, -13, 0, 0 }, { 0 } },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { 0 } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg22 } },
    { VIR_OP_MOV, 0, 0, { 1, 2, 0, 0 }, { 0 } },
    { VIR_OP_LABEL, 0, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg3_6 } },
    { VIR_OP_MUL, 0, 0, { 1, 3, 1, 0 }, { 0 } },
    { VIR_OP_SUB, 0, 0, { 1, 2, 1, 0 }, { 0 } },
};

static VIR_PatternMatchInst _modPatInst3[] = {
    { VIR_OP_MOD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _modRepInst3[] = {
    { VIR_OP_AQ_IMOD, 0, 0, { 1, 2, 3, 0 }, { 0 } },
};

static VIR_Pattern _modPattern[] = {
    { VIR_PATN_FLAG_RECURSIVE_SCAN, CODEPATTERN(_mod, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_mod, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_mod, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_mod, 3) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _sinpiPatInst0[] = {
    { VIR_OP_SINPI, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _hasNEW_SIN_COS_LOG_DIV }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _sinpiRepInst0[] = {
    { VIR_OP_SINPI, 0, VIR_PATTERN_TEMP_TYPE_XY, { -1, 2, 0, 0 }, { 0 } },
    { VIR_OP_MUL, 0, 0, {  1, -1, -1, 0 }, { 0, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleY } },
};

static VIR_Pattern _sinpiPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_sinpi, 0) },
    { VIR_PATN_FLAG_NONE }
};


static VIR_PatternMatchInst _cospiPatInst0[] = {
    { VIR_OP_COSPI, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _hasNEW_SIN_COS_LOG_DIV }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _cospiRepInst0[] = {
    { VIR_OP_COSPI, 0, VIR_PATTERN_TEMP_TYPE_XY, { -1, 2, 0, 0 }, { 0 } },
    { VIR_OP_MUL, 0, 0, {  1, -1, -1, 0 }, { 0, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleY } },
};

static VIR_Pattern _cospiPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_cospi, 0) },
    { VIR_PATN_FLAG_NONE }
};


static VIR_PatternMatchInst _logPatInst0[] = {
    { VIR_OP_PRE_LOG2, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _hasNEW_SIN_COS_LOG_DIV }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _logRepInst0[] = {
    { VIR_OP_PRE_LOG2, 0, VIR_PATTERN_TEMP_TYPE_XY, { -1, 2, 0, 0 }, { 0 } },
    { VIR_OP_MUL, 0, 0, {  1, -1, -1, 0 }, { 0, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleY } },
};

static VIR_Pattern _logPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_log, 0) },
    { VIR_PATN_FLAG_NONE }
};


/*
        MAX 1, 2, 3
            select.lt 1, 2, 3, 2, 0
*/
static VIR_PatternMatchInst _maxPatInst0[] = {
    { VIR_OP_MAX, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { VIR_Lower_IsNotCLShader, VIR_Lower_IsIntOpcode }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _maxRepInst0[] = {
    { VIR_OP_AQ_SELECT, VIR_COP_LESS, 0, {  1, 2, 3, 2 }, { 0 } },
};

/*
        MAX 1, 2, 3
            select.lt -1, 2, 3, 2, 0
            add       1, -1, zero

        new chip's select doesn't flush denorm to zero, add zero to flush to zero
*/
static VIR_PatternMatchInst _maxPatInst1[] = {
    { VIR_OP_MAX, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _maxRepInst1[] = {
    { VIR_OP_AQ_SELECT, VIR_COP_LESS, 0, {  -1, 2, 3, 2 }, { 0 } },
    { VIR_OP_ADD, 0, 0, {  1, -1, 0, 0 }, { 0, 0, VIR_Lower_SetZero } },
};

VIR_Pattern _maxPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_max, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_max, 1) },
    { VIR_PATN_FLAG_NONE }
};


/*
        MIN 1, 2, 3
            select.lt 1, 2, 3, 2, 0
*/
static VIR_PatternMatchInst _minPatInst0[] = {
    { VIR_OP_MIN, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { VIR_Lower_IsNotCLShader, VIR_Lower_IsIntOpcode }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _minRepInst0[] = {
    { VIR_OP_AQ_SELECT, VIR_COP_GREATER, 0, {  1, 2, 3, 2 }, { 0 } },
};

/*
        MIN 1, 2, 3
            select.lt -1, 2, 3, 2, 0
            add       1, -1, zero

        new chip's select doesn't flush denorm to zero, add zero to flush to zero
*/
static VIR_PatternMatchInst _minPatInst1[] = {
    { VIR_OP_MIN, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _minRepInst1[] = {
    { VIR_OP_AQ_SELECT, VIR_COP_GREATER, 0, { -1, 2, 3, 2 }, { 0 } },
    { VIR_OP_ADD, 0, 0, {  1, -1, 0, 0 }, { 0, 0, VIR_Lower_SetZero } },
};

VIR_Pattern _minPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_min, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_min, 1) },
    { VIR_PATN_FLAG_NONE }
};


static VIR_Pattern*
_GetLowerPatternPhaseMachine(
    IN VIR_PatternContext      *Context,
    IN VIR_Instruction         *Inst
    )
{
    switch(VIR_Inst_GetOpcode(Inst))
    {
    case VIR_OP_DIV:
        return _divPattern;
    case VIR_OP_MOD:
        return _modPattern;
    case VIR_OP_PRE_LOG2:
        return _logPattern;
    case VIR_OP_SINPI:
        return _sinpiPattern;
    case VIR_OP_COSPI:
        return _cospiPattern;
    case VIR_OP_MAX:
        return _maxPattern;
    case VIR_OP_MIN:
        return _minPattern;
    default:
        break;
    }
    return gcvNULL;
}

static gctBOOL
_CmpInstuction(
    IN VIR_PatternContext   *Context,
    IN VIR_PatternMatchInst *Inst0,
    IN VIR_Instruction      *Inst1
    )
{
    return Inst0->opcode == Inst1->_opcode;
}

VSC_ErrCode
VIR_Lower_MiddleLevel_To_LowLevel_Machine(
    IN  VIR_Shader              *Shader,
    IN  VSC_HW_CONFIG           *HwCfg,
    IN  VIR_PatternLowerContext *Context
    )
{
    VSC_ErrCode errCode  = VSC_ERR_NONE;

    VIR_PatternContext_Initialize(&Context->header, Shader, VIR_PATN_CONTEXT_FLAG_NONE, _GetLowerPatternPhaseMachine, _CmpInstuction, 512);

    errCode = VIR_Pattern_Transform((VIR_PatternContext *)Context);
    CHECK_ERROR(errCode, "VIR_Lower_MiddleLevel_To_LowLevel_Machine failed.");

    VIR_PatternContext_Finalize(&Context->header, 512);

    return errCode;
}


