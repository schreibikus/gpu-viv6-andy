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


#ifndef __gc_vsc_vir_ml_2_ll_h_
#define __gc_vsc_vir_ml_2_ll_h_

#include "gc_vsc.h"
#include "vir/lower/gc_vsc_vir_pattern.h"

BEGIN_EXTERN_C()

#ifndef M_PI
#   define M_PI 3.14159265358979323846f
#endif

VSC_ErrCode
VIR_Lower_MiddleLevel_To_LowLevel_Pre(
    IN VSC_SH_PASS_WORKER* pPassWorker
    );
DECLARE_QUERY_PASS_PROP(VIR_Lower_MiddleLevel_To_LowLevel_Pre);
DECLARE_SH_NECESSITY_CHECK(VIR_Lower_MiddleLevel_To_LowLevel_Pre);

VSC_ErrCode
VIR_Lower_MiddleLevel_To_LowLevel_Post(
    IN VSC_SH_PASS_WORKER* pPassWorker
    );
DECLARE_QUERY_PASS_PROP(VIR_Lower_MiddleLevel_To_LowLevel_Post);
DECLARE_SH_NECESSITY_CHECK(VIR_Lower_MiddleLevel_To_LowLevel_Post);

END_EXTERN_C()
#endif

