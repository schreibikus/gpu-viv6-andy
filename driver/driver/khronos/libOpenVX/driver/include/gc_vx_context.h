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


#ifndef __GC_VX_CONTEXT_H__
#define __GC_VX_CONTEXT_H__

EXTERN_C_BEGIN

/*
** Misc
*/
VX_INTERNAL_API vx_bool vxIsValidImportType(vx_enum type);

VX_INTERNAL_API vx_bool vxIsValidBorderMode(vx_enum mode);

/*
** DataType APIs
*/
VX_INTERNAL_API vx_size vxDataType_GetSize(vx_type_e type);

VX_INTERNAL_API vx_bool vxDataType_IsValid(vx_enum type);

typedef void VX_INTERNAL_CALLBACK_API (* vx_object_destructor_f)(vx_reference ref);

VX_INTERNAL_API vx_object_destructor_f vxDataType_GetDestructor(vx_type_e type);

VX_INTERNAL_API vx_bool vxDataType_IsStatic(vx_type_e type);

/*
** Context APIs
*/
VX_INTERNAL_API vx_error vxoContext_GetErrorObject(vx_context context, vx_status status);

VX_INTERNAL_API vx_bool vxoContext_IsValid(vx_context context);

VX_INTERNAL_API vx_context vxoContext_GetFromReference(vx_reference ref);

VX_INTERNAL_API vx_bool vxoContext_AddObject(vx_context context, vx_reference ref);

VX_INTERNAL_API vx_bool vxoContext_RemoveObject(vx_context context, vx_reference ref);

VX_INTERNAL_API vx_status vxContext_LoadKernels(vx_context context, vx_string module);

VX_INTERNAL_API vx_bool vxoContext_AddAccessor(
        vx_context context, vx_size size, vx_enum usage, vx_ptr ptr, vx_reference ref, OUT vx_uint32_ptr indexPtr);

VX_INTERNAL_API void vxoContext_RemoveAccessor(vx_context context, vx_uint32 index);

VX_INTERNAL_API vx_bool vxoContext_SearchAccessor(vx_context context, vx_ptr ptr, OUT vx_uint32_ptr indexPtr);

EXTERN_C_END

#endif /* __GC_VX_CONTEXT_H__ */
