/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include <gc_vx_common.h>

VX_INTERNAL_API vx_error vxoError_Create(vx_context context, vx_status status)
{
    vx_error error = (vx_error)vxoReference_Create(context, VX_TYPE_ERROR, VX_REF_INTERNAL, &context->base);

    if (error == VX_NULL) return VX_NULL;

    error->status = status;

    return error;
}

VX_INTERNAL_API vx_status vxError_Release(vx_error_ptr errorPtr)
{
    return vxoReference_Release((vx_reference_ptr)errorPtr, VX_TYPE_ERROR, VX_REF_INTERNAL);
}

VX_INTERNAL_API vx_error_s *vxoError_GetErrorObject(vx_context_s *context, vx_status status)
{
    vx_error_s *error = NULL;
    vx_size i = 0ul;
    vxAcquireMutex(context->base.lock);
    for (i = 0ul; i < context->refCount; i++)
    {
        if (context->refTable[i] == NULL)
            continue;

        if (context->refTable[i]->type == VX_TYPE_ERROR)
        {
            error = (vx_error_s *)context->refTable[i];
            if (error->status == status)
            {
                break;
            }
            error = NULL;
        }
    }
    vxReleaseMutex(context->base.lock);
    return error;
}

