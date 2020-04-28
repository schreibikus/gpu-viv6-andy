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


#ifndef __gc_vsc_vir_misc_opts_h_
#define __gc_vsc_vir_misc_opts_h_

#include "gc_vsc.h"

BEGIN_EXTERN_C()

typedef struct _VSC_PRELL_PASS_DATA
{
    gctBOOL                     bHasFuncNeedToForceInline;
} VSC_PRELL_PASS_DATA;

VSC_ErrCode vscVIR_RemoveNop(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_RemoveNop);
DECLARE_SH_NECESSITY_CHECK(vscVIR_RemoveNop);

VSC_ErrCode vscVIR_RecordInstructionStatus(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_RecordInstructionStatus);
DECLARE_SH_NECESSITY_CHECK(vscVIR_RecordInstructionStatus);

VSC_ErrCode vscVIR_VX_ReplaceDest(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_VX_ReplaceDest);
DECLARE_SH_NECESSITY_CHECK(vscVIR_VX_ReplaceDest);

VSC_ErrCode vscVIR_PutScalarConstToImm(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_PutScalarConstToImm);
DECLARE_SH_NECESSITY_CHECK(vscVIR_PutScalarConstToImm);

VSC_ErrCode vscVIR_PutImmValueToUniform(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_PutImmValueToUniform);
DECLARE_SH_NECESSITY_CHECK(vscVIR_PutImmValueToUniform);

VSC_ErrCode vscVIR_CheckCstRegFileReadPortLimitation(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_CheckCstRegFileReadPortLimitation);
DECLARE_SH_NECESSITY_CHECK(vscVIR_CheckCstRegFileReadPortLimitation);

VSC_ErrCode vscVIR_CheckEvisInstSwizzleRestriction(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_CheckEvisInstSwizzleRestriction);
DECLARE_SH_NECESSITY_CHECK(vscVIR_CheckEvisInstSwizzleRestriction);

VSC_ErrCode vscVIR_CheckPosAndDepthConflict(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_CheckPosAndDepthConflict);
DECLARE_SH_NECESSITY_CHECK(vscVIR_CheckPosAndDepthConflict);

VSC_ErrCode vscVIR_ConvFrontFacing(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_ConvFrontFacing);
DECLARE_SH_NECESSITY_CHECK(vscVIR_ConvFrontFacing);

VSC_ErrCode vscVIR_CheckDual16able(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_CheckDual16able);
DECLARE_SH_NECESSITY_CHECK(vscVIR_CheckDual16able);

VSC_ErrCode vscVIR_AddOutOfBoundCheckSupport(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_AddOutOfBoundCheckSupport);
DECLARE_SH_NECESSITY_CHECK(vscVIR_AddOutOfBoundCheckSupport);

VSC_ErrCode vscVIR_AdjustPrecision(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_AdjustPrecision);
DECLARE_SH_NECESSITY_CHECK(vscVIR_AdjustPrecision);

VSC_ErrCode vscVIR_ConvertVirtualInstructions(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_ConvertVirtualInstructions);
DECLARE_SH_NECESSITY_CHECK(vscVIR_ConvertVirtualInstructions);

VSC_ErrCode vscVIR_PreprocessLLShader(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_PreprocessLLShader);
DECLARE_SH_NECESSITY_CHECK(vscVIR_PreprocessLLShader);

VSC_ErrCode vscVIR_CheckMustInlineFuncForML(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_CheckMustInlineFuncForML);
DECLARE_SH_NECESSITY_CHECK(vscVIR_CheckMustInlineFuncForML);

VSC_ErrCode vscVIR_CheckVariableUsage(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_CheckVariableUsage);
DECLARE_SH_NECESSITY_CHECK(vscVIR_CheckVariableUsage);

VSC_ErrCode vscVIR_FixTexldOffset(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_FixTexldOffset);
DECLARE_SH_NECESSITY_CHECK(vscVIR_FixTexldOffset);

VSC_ErrCode vscVIR_InitializeVariables(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_InitializeVariables);
DECLARE_SH_NECESSITY_CHECK(vscVIR_InitializeVariables);

VSC_ErrCode vscVIR_PreprocessMCShader(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_PreprocessMCShader);
DECLARE_SH_NECESSITY_CHECK(vscVIR_PreprocessMCShader);

VSC_ErrCode vscVIR_PreprocessCGShader(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_PreprocessCGShader);
DECLARE_SH_NECESSITY_CHECK(vscVIR_PreprocessCGShader);

VSC_ErrCode vscVIR_FixDynamicIdxDep(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_FixDynamicIdxDep);
DECLARE_SH_NECESSITY_CHECK(vscVIR_FixDynamicIdxDep);

VSC_ErrCode vscVIR_GenCombinedSampler(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_GenCombinedSampler);
DECLARE_SH_NECESSITY_CHECK(vscVIR_GenCombinedSampler);

VSC_ErrCode vscVIR_GenExternalAtomicCall(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_GenExternalAtomicCall);
DECLARE_SH_NECESSITY_CHECK(vscVIR_GenExternalAtomicCall);

VSC_ErrCode vscVIR_GenRobustBoundCheck(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_GenRobustBoundCheck);
DECLARE_SH_NECESSITY_CHECK(vscVIR_GenRobustBoundCheck);

VSC_ErrCode vscVIR_ClampPointSize(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_QUERY_PASS_PROP(vscVIR_ClampPointSize);
DECLARE_SH_NECESSITY_CHECK(vscVIR_ClampPointSize);

VSC_ErrCode vscVIR_CutDownWorkGroupSize(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_SH_NECESSITY_CHECK(vscVIR_CutDownWorkGroupSize);
DECLARE_QUERY_PASS_PROP(vscVIR_CutDownWorkGroupSize);

VSC_ErrCode vscVIR_ProcessImageFormatMismatch(VSC_SH_PASS_WORKER* pPassWorker);
DECLARE_SH_NECESSITY_CHECK(vscVIR_ProcessImageFormatMismatch);
DECLARE_QUERY_PASS_PROP(vscVIR_ProcessImageFormatMismatch);

END_EXTERN_C()

#endif /* __gc_vsc_vir_misc_opts_h_ */

