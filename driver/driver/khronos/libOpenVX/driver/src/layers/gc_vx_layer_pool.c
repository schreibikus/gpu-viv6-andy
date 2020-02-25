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


#include <gc_vx_common.h>
#include <gc_vx_nn_wb.h>
#include <layers/gc_vx_layer_pool.h>


vx_status vxnneExecuteSWPooling(struct _vxnne_operation_s *operation)
{
    vxnne_pooling_operation           poolingOperation   = (vxnne_pooling_operation)operation;

    vx_tensor inputs  = (vx_tensor)poolingOperation->inputs;
    vx_tensor outputs = (vx_tensor)poolingOperation->outputs;

    vx_enum  poolType_v = poolingOperation->pool_type;
    vx_uint32 poolSizeX_v = poolingOperation->pool_size_x;
    vx_uint32 poolSizeY_v = poolingOperation->pool_size_y;
    vx_uint32 poolPadXLeft_v = poolingOperation->pool_pad_x_left;
    vx_uint32 poolPadXRight_v = poolingOperation->pool_pad_x_right;
    vx_uint32 poolPadYTop_v = poolingOperation->pool_pad_y_top;
    vx_uint32 poolPadYBottom_v = poolingOperation->pool_pad_y_bottom;
    vx_enum rounding_v = poolingOperation->rounding;

    vx_status status = VX_SUCCESS;

    gctPOINTER inputsBaseLogicalAddr = VX_NULL;
    gctPOINTER outputsBaseLogicalAddr = VX_NULL;

    vx_int32 inputs_width, inputs_height, depth, outputs_width, outputs_height, out_w, out_h, batch;
    vx_uint32 stride_x, stride_y;
    vx_int32 type;

    vxoTensor_GetTensorViewMemory(inputs, &inputsBaseLogicalAddr, VX_NULL);
    vxoTensor_GetTensorViewMemory(outputs, &outputsBaseLogicalAddr, VX_NULL);

    inputs_width   = TENSOR_SIZE_INDEX(inputs, 0);
    inputs_height  = TENSOR_SIZE_INDEX(inputs, 1);
    depth          = TENSOR_SIZE_INDEX(inputs, 2);
    batch          = TENSOR_SIZE_INDEX(inputs, 3);
    outputs_width  = TENSOR_SIZE_INDEX(outputs, 0);
    outputs_height = TENSOR_SIZE_INDEX(outputs, 1);

    switch (poolType_v)
    {
    case VX_NN_POOLING_MAX:
        type = 1;
        break;
    case VX_NN_POOLING_AVG:
    case VX_NN_POOLING_AVG_ANDROID:
        type = 2;
        break;
    default:
        vxError("not support this pool type");
        return VX_ERROR_INVALID_PARAMETERS;
    }

    if (outputs_width == 1 && outputs_height == 1)
    {
        stride_x = 1;
        stride_y = 1;
    }
    else if (outputs_width == 1)
    {
        stride_x = 1;
        stride_y = vxoNNExternsionConvlutionRound((vx_float32)(inputs_height + poolPadYTop_v + poolPadYBottom_v - poolSizeY_v) / (outputs_height - 1), rounding_v);
    }
    else if(outputs_height == 1)
    {
        stride_x = vxoNNExternsionConvlutionRound((vx_float32)(inputs_width + poolPadXLeft_v + poolPadXRight_v - poolSizeX_v) / (outputs_width - 1), rounding_v);
        stride_y = 1;
    }
    else
    {
        /* Calculate stride = (w + 2*pad - weight)/(output_w - 1) */
        stride_x = vxoNNExternsionConvlutionRound((vx_float32)(inputs_width + poolPadXLeft_v + poolPadXRight_v - poolSizeX_v) / (outputs_width - 1), rounding_v);
        stride_y = vxoNNExternsionConvlutionRound((vx_float32)(inputs_height + poolPadYTop_v + poolPadYBottom_v - poolSizeY_v) / (outputs_height - 1), rounding_v);
    }

    status = vxnnePoolingCpu((vx_uint8_ptr)inputsBaseLogicalAddr,
                             TENSOR_POS(inputs),
                             type,
                             (vx_type_e)TENSOR_DATA_TYPE(inputs),
                             inputs_width,
                             inputs_height,
                             batch,
                             depth,
                             &out_w,
                             &out_h,
                             stride_x,
                             stride_y,
                             poolSizeX_v,
                             poolSizeY_v,
                             poolPadXLeft_v,
                             poolPadXRight_v,
                             poolPadYTop_v,
                             poolPadYBottom_v,
                             rounding_v,
                             (vx_uint8_ptr)outputsBaseLogicalAddr,
                             TENSOR_POS(outputs),
                             TENSOR_ROUNDING_MODE(outputs),
                             (vx_type_e)TENSOR_DATA_TYPE(outputs),
                             (vx_type_e)TENSOR_QUANT_TYPE(inputs),
                             (vx_type_e)TENSOR_QUANT_TYPE(outputs),
                             TENSOR_TF_ZEROPOINT(inputs),
                             TENSOR_TF_SCALE(inputs),
                             TENSOR_TF_ZEROPOINT(outputs),
                             TENSOR_TF_SCALE(outputs));
    gcmASSERT((out_w == outputs_width) && (out_h == outputs_height));

    /* make release build happy */
    outputs_height = outputs_height;

    return status;

}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNPoolingLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNPoolingLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNPoolingLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    if (index != vxmLENGTH_OF(nn_PoolingLayer_params) - 1) return VX_ERROR_INVALID_PARAMETERS;

    ptr->type                 = VX_TYPE_TENSOR;

    return VX_SUCCESS;
}

vx_status vxnnePoolingOperation_Deinitialize(vxnne_operation_s *operation)
{
    vxnne_pooling_operation pooling_operation = (vxnne_pooling_operation)operation;

    if (pooling_operation->weights_biases != VX_NULL)
    {
        vxReleaseWeightsBiasesParameter(&pooling_operation->weights_biases);
    }

    vxnneOperation_Deinitialize(operation);
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxnnePoolingInitializer(
    vx_node node,
    char* name,
    vx_tensor inputs,
    vx_scalar pool_type_s,
    vx_scalar pool_size_x_s,
    vx_scalar pool_size_y_s,
    vx_uint32 pool_pad_x_left,
    vx_uint32 pool_pad_x_right,
    vx_uint32 pool_pad_y_top,
    vx_uint32 pool_pad_y_bottom,
    vx_scalar rounding_s,
    vx_enum padMode,
    vx_scalar padConst,
    vx_tensor outputs
    )
{
    vx_status  status                        = VX_SUCCESS;

    vx_enum poolTypeValue                    = pool_type_s->value->e;
    vx_uint32 poolSizeXValue                 = pool_size_x_s->value->u32;
    vx_uint32 poolSizeYValue                 = pool_size_y_s->value->u32;
    vx_enum roundingValue                    = rounding_s->value->e;
    vx_enum inputdata_format                 = TENSOR_DATA_TYPE(inputs);
    vx_enum outputdata_format                = TENSOR_DATA_TYPE(outputs);
    vx_bool dataFormat_AvgPool_flag[6]       = {vx_false_e};
    vx_bool avgPool_flag                     = vx_false_e;
    vx_bool avgPool_BF_flag                  = vx_false_e;
    vx_bool enable_tf_quantize               = vx_false_e;
    vx_bool enable_int16_sh                  = vx_false_e;
    vx_context context                       = vxGetContext((vx_reference)node);

    vx_uint32 inputsWidth, inputsHeight, outputsWidth, outputsHeight;
    vx_int32  inputsDepth, outputsDepth;
    vxnne_pooling_layer  poolingLayer = gcvNULL;
    vx_uint32  stride = 0;
    vx_uint32 totalSize = 0;
    vx_uint32 maxAllocateSize = 256 * 1024 * 1024; /* set max allocate size because fpga out of memory when using nn do avg pooling, max value is 256M */
    vx_uint32 batchCount;


    inputsWidth   = TENSOR_SIZE_INDEX(inputs, 0);
    inputsHeight  = TENSOR_SIZE_INDEX(inputs, 1);
    inputsDepth   = TENSOR_SIZE_INDEX(inputs, 2);
    batchCount    = TENSOR_SIZE_INDEX(inputs, 3);
    outputsWidth  = TENSOR_VIEW_SIZE_INDEX(outputs, 0);
    outputsHeight = TENSOR_VIEW_SIZE_INDEX(outputs, 1);
    outputsDepth  = TENSOR_VIEW_SIZE_INDEX(outputs, 2);

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }
    gcoOS_Allocate(gcvNULL, sizeof(vxnne_pooling_layer_s), (gctPOINTER*)&poolingLayer);
    if (!poolingLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        return status;
    }

    gcoOS_ZeroMemory(poolingLayer, sizeof(vxnne_pooling_layer_s));

    vxnneLayer_Initialize(&poolingLayer->base,
                          name,
                          node,
                          vxmOPERATION_COUNT(poolingLayer),
                          poolingLayer->operations,
                          vxnneLayer_Deinitialize);


    if (outputsWidth == 1)
    {
        stride = 1;
    }
    else
    {
        stride = vxoNNExternsionConvlutionRound((vx_float32)(inputsWidth + pool_pad_x_left + pool_pad_x_right - poolSizeXValue) / (outputsWidth - 1), roundingValue);
    }

    totalSize = poolSizeXValue * poolSizeYValue * inputsDepth * outputsDepth * (vx_uint32)vxDataType_GetSize((vx_type_e)TENSOR_DATA_TYPE(inputs)) + outputsDepth * sizeof(vx_float32);

    if(context->evisNoInst.supportEVIS)
    {
        dataFormat_AvgPool_flag[0] = (vx_bool)(inputdata_format == VX_TYPE_INT8 && outputdata_format == VX_TYPE_FLOAT16);
        dataFormat_AvgPool_flag[1] = (vx_bool)(inputdata_format == VX_TYPE_FLOAT16 && outputdata_format == VX_TYPE_INT8);
        dataFormat_AvgPool_flag[2] = (vx_bool)(inputdata_format == VX_TYPE_INT8 && outputdata_format == VX_TYPE_INT8);
        dataFormat_AvgPool_flag[3] = (vx_bool)(inputdata_format == VX_TYPE_FLOAT16 && outputdata_format == VX_TYPE_FLOAT16);
        dataFormat_AvgPool_flag[4] = (vx_bool)(inputdata_format == VX_TYPE_INT16);
        dataFormat_AvgPool_flag[5] = (vx_bool)(inputdata_format == VX_TYPE_UINT8 );
        avgPool_BF_flag            = (vx_bool)(inputdata_format == VX_TYPE_BFLOAT16 && outputdata_format == VX_TYPE_BFLOAT16);
    }
    else
    {
        dataFormat_AvgPool_flag[0] = vx_false_e;
        dataFormat_AvgPool_flag[1] = vx_false_e;
        dataFormat_AvgPool_flag[2] = vx_false_e;
        dataFormat_AvgPool_flag[3] = (vx_bool)((inputdata_format == VX_TYPE_FLOAT16 && outputdata_format == VX_TYPE_FLOAT16) ||
                                               (inputdata_format == VX_TYPE_FLOAT32 && outputdata_format == VX_TYPE_FLOAT32));
        dataFormat_AvgPool_flag[4] = vx_false_e;
        dataFormat_AvgPool_flag[5] = (vx_bool)(inputdata_format == VX_TYPE_UINT8 );
    }

   enable_tf_quantize           =  (vx_bool)(dataFormat_AvgPool_flag[5] );

   enable_int16_sh              = (vx_bool)(dataFormat_AvgPool_flag[4]);

   avgPool_flag   = (vx_bool)(((dataFormat_AvgPool_flag[0])
                            ||  (dataFormat_AvgPool_flag[1])
                            ||  (dataFormat_AvgPool_flag[2])
                            ||  (dataFormat_AvgPool_flag[3])
                            ||  (enable_tf_quantize == vx_true_e)
                            ||  (enable_int16_sh == vx_true_e))
                            &&  (poolTypeValue == VX_NN_POOLING_AVG || poolTypeValue == VX_NN_POOLING_AVG_ANDROID));

    /* if the needed total size is larger than maxAllocateSize, do pooling with CPU version. maybe need implement avg pooling with shader */
    if (vxnneIsNNSupportFormat(context, inputs, VX_NULL, outputs) &&
        (poolTypeValue == VX_NN_POOLING_AVG) &&
        (stride == 1) &&
        (totalSize <= maxAllocateSize) &&
        (avgPool_flag == vx_false_e) &&
        (avgPool_BF_flag == vx_false_e))
    {
        status = vxnneOperation_Initialize(&poolingLayer->pooling_nne_operation.base,
                                           &poolingLayer->base,
                                           VXNNE_OPERATION_TARGET_NN,
                                           VXNNE_OPERATOR_POOLING,
                                           VX_NULL,
                                           vxnnePoolingOperation_Deinitialize,
                                           batchCount,
                                           NNE_COMMAND_SIZE);
        if (status != VX_SUCCESS) goto exit;

        vxnneLayer_SetOperation(
            &poolingLayer->base,
            &poolingLayer->pooling_nne_operation.base,
            0);

        poolingLayer->pooling_nne_operation.inputs            = inputs;
        poolingLayer->pooling_nne_operation.pool_type         = poolTypeValue;
        poolingLayer->pooling_nne_operation.pool_size_x       = poolSizeXValue;
        poolingLayer->pooling_nne_operation.pool_size_y       = poolSizeYValue;
        poolingLayer->pooling_nne_operation.pool_pad_x_left   = pool_pad_x_left;
        poolingLayer->pooling_nne_operation.pool_pad_x_right  = pool_pad_x_right;
        poolingLayer->pooling_nne_operation.pool_pad_y_top    = pool_pad_y_top;
        poolingLayer->pooling_nne_operation.pool_pad_y_bottom = pool_pad_y_bottom;
        poolingLayer->pooling_nne_operation.rounding          = roundingValue;
        poolingLayer->pooling_nne_operation.outputs           = outputs;

        vxnneOperation_AddReference(&poolingLayer->pooling_nne_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&poolingLayer->pooling_nne_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);

        /* prepare data for nne */
        {
            vx_int8 *weightData = VX_NULL;
            vx_float32 *biasData = VX_NULL;
            vx_uint32 i, j;
            vx_int32 w, z;
            vx_int8 *weightsValuePtr;
            vx_int8 *zerosValuePtr;
            vx_uint32 weightItemCount;
            vx_int8 *pWeightData;
            vx_float32 *pBiasData;
            vx_tensor weights = VX_NULL;
            vx_tensor biases = VX_NULL;
            vx_context context = VX_NULL;
            vx_uint32 numWeightDims = 4, numBiasDims = 1;
            vx_uint32 weightSize[4] = {poolSizeXValue, poolSizeYValue, inputsDepth, outputsDepth};
            vx_uint32 weightStrideSize[4];
            vx_uint32 biasSize[4] = {outputsDepth};
            vx_uint32 biasStrideSize[1];
            vx_tensor_addressing weightUserAddr = NULL;
            vx_tensor_addressing biasUserAddr = NULL;
            vx_weights_biases_parameter weights_biases = VX_NULL;
            vx_type_e inputDataFormat = (vx_type_e)TENSOR_DATA_TYPE(inputs);
            vx_type_e weightDataFormat = inputDataFormat;
            vx_type_e biasDataFormat = (weightDataFormat == VX_TYPE_INT8 || weightDataFormat == VX_TYPE_UINT8) ? VX_TYPE_INT32 : VX_TYPE_FLOAT32;
            vx_int32 weightItemSize = vxnneGetTypeSize(weightDataFormat);
            vx_int32 biasItemSize = vxnneGetTypeSize(biasDataFormat);
            vx_int8 weightFixPointPos, biasFixPointPos;
            vx_enum weightRoundingMode;

            context = vxGetContext((vx_reference)node);
            if (context == VX_NULL)
            {
                vxError("vxGetContext fail at function %s line %d", __FUNCTION__, __LINE__);
                goto exit;
            }

            /* generate special weight and bias data for average pooling. create weightsBiasesParameters from this specail weight and bias */
            weightsValuePtr = (vx_int8*)vxAllocateAndZeroMemory(poolSizeXValue * poolSizeYValue * weightItemSize);
            zerosValuePtr = (vx_int8*)vxAllocate(poolSizeXValue * poolSizeYValue * weightItemSize);

            weightData = (vx_int8*)vxAllocate(poolSizeXValue * poolSizeYValue * inputsDepth * outputsDepth * weightItemSize);
            biasData = (vx_float32*)vxAllocate(outputsDepth * biasItemSize);

            if (weightsValuePtr == NULL || zerosValuePtr == NULL || weightData == NULL || biasData == NULL)
            {
                status = VX_ERROR_NO_MEMORY;
                vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
                if (weightsValuePtr != VX_NULL) vxFree(weightsValuePtr);
                if (zerosValuePtr != VX_NULL) vxFree(zerosValuePtr);
                if (weightData != VX_NULL) vxFree(weightData);
                if (biasData != VX_NULL) vxFree(biasData);
                goto exit;
            }

            weightItemCount = poolSizeXValue * poolSizeYValue;

            if (weightDataFormat == VX_TYPE_UINT8 && TENSOR_DATA_TYPE(outputs) == VX_TYPE_UINT8 && TENSOR_QUANT_TYPE(inputs) == VX_QUANT_AFFINE_SCALE)
            {
                vx_tensor_create_params_t  tensor_create_params;
                vx_weights_biases_parameter_optimizations_t opt = {0};
                vx_float32 scale = 1.0f / (vx_float32)(255 * weightItemCount);

                opt.inputZeroPoint = TENSOR_TF_ZEROPOINT(inputs);
                opt.zrl = -1;
                opt.outputFormat = TENSOR_DATA_TYPE(outputs);

                gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
                tensor_create_params.num_of_dims = numWeightDims;
                tensor_create_params.sizes = weightSize;
                tensor_create_params.data_format = weightDataFormat;
                tensor_create_params.quant_format = VX_QUANT_AFFINE_SCALE;
                tensor_create_params.quant_data.affine.scale = scale;
                tensor_create_params.quant_data.affine.zeroPoint = 0;

                weights = vxoTensor_CreateTensor(context, node->graph, &tensor_create_params, vx_false_e);
                if (weights == NULL)
                {
                    status = VX_ERROR_NO_MEMORY;
                    vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
                    if (weightsValuePtr != VX_NULL) vxFree(weightsValuePtr);
                    if (zerosValuePtr != VX_NULL) vxFree(zerosValuePtr);
                    if (weightData != VX_NULL) vxFree(weightData);
                    if (biasData != VX_NULL) vxFree(biasData);
                    goto exit;
                }

                weightRoundingMode = TENSOR_ROUNDING_MODE(weights);

                for (j = 0; j < weightItemCount; j++)
                {
                    vxnneSaveDataQuant(weightDataFormat, j, 1.0f/weightItemCount, weightsValuePtr, 0, scale, weightRoundingMode);
                    vxnneSaveDataQuant(weightDataFormat, j, 0.0f, zerosValuePtr, 0, scale, weightRoundingMode);
                }

                pWeightData = weightData;
                pBiasData = biasData;

                for (w = 0; w < outputsDepth; w++)
                {
                    for (z = 0; z < inputsDepth; z++)
                    {
                        if (w == z)
                        {
                            memcpy(pWeightData, weightsValuePtr, weightItemCount * weightItemSize);
                        }
                        else
                        {
                            memcpy(pWeightData, zerosValuePtr, weightItemCount * weightItemSize);
                        }
                        pWeightData += weightItemCount * weightItemSize;
                    }
                    *pBiasData++ = 0.0f;
                }

                weightStrideSize[0] = weightItemSize;
                for (i = 1; i < numWeightDims; i++)
                {
                    weightStrideSize[i] =  weightStrideSize[i-1] * weightSize[i-1];
                }

                weightUserAddr = vxCreateTensorAddressing(context, weightSize, weightStrideSize, (vx_uint8)numWeightDims);

                vxoCopyTensorPatch(weights, VX_NULL, weightUserAddr, weightData, VX_WRITE_ONLY,0);

                biasStrideSize[0] = biasItemSize;

                gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
                tensor_create_params.num_of_dims = numBiasDims;
                tensor_create_params.sizes = biasSize;
                tensor_create_params.data_format = biasDataFormat;
                tensor_create_params.quant_format = VX_QUANT_AFFINE_SCALE;
                tensor_create_params.quant_data.affine.scale = scale * TENSOR_TF_SCALE(inputs) / TENSOR_TF_SCALE(outputs);;
                tensor_create_params.quant_data.affine.zeroPoint = 0;

                biases = vxoTensor_CreateTensor(context, node->graph, &tensor_create_params, vx_false_e);

                biasUserAddr = vxCreateTensorAddressing(context, biasSize, biasStrideSize, (vx_uint8)numBiasDims);

                vxoCopyTensorPatch(biases, VX_NULL, biasUserAddr, biasData, VX_WRITE_ONLY,0);

                weights_biases = vxoCreateWeightsBiasesParameterFromTensors(context,
                                                                       VX_NN_CONVOLUTION_LAYER,
                                                                       (vx_uint32*)(TENSOR_SIZES(inputs)),
                                                                       inputs->dimCount,
                                                                       inputs->dimCount,
                                                                       pool_pad_x_left,
                                                                       pool_pad_x_right,
                                                                       pool_pad_y_top,
                                                                       pool_pad_y_bottom,
                                                                       0,
                                                                       0,
                                                                       0,
                                                                       0,
                                                                       roundingValue,
                                                                       (vx_uint32*)(TENSOR_SIZES(outputs)),
                                                                       VX_NULL,
                                                                       &opt,
                                                                       TENSOR_DATA_TYPE(weights),
                                                                       0,
                                                                       VX_TENSOR_RANK_WHCN,
                                                                       weights,
                                                                       biases,
                                                                       VX_NULL,
                                                                       vx_false_e,
                                                                       vx_false_e);
            }
            else
            {
                vx_tensor_create_params_t tensor_create_params;

                weightFixPointPos = (vx_int8)(8 - gcoMATH_Ceiling(gcoMATH_Log(1.0f/weightItemCount) + 1));
                biasFixPointPos = weightFixPointPos + TENSOR_POS(inputs);

                gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
                tensor_create_params.num_of_dims = numWeightDims;
                tensor_create_params.sizes = weightSize;
                tensor_create_params.data_format = weightDataFormat;
                tensor_create_params.quant_format = VX_QUANT_DYNAMIC_FIXED_POINT;
                tensor_create_params.quant_data.dfp.fixed_point_pos = weightFixPointPos;

                weights = vxoTensor_CreateTensor(context, node->graph, &tensor_create_params, vx_false_e);

                weightRoundingMode = TENSOR_ROUNDING_MODE(weights);

                for (j = 0; j < weightItemCount; j++)
                {
                    vxnneSaveData(weightDataFormat, j, 1.0f/weightItemCount, weightsValuePtr, weightFixPointPos, weightRoundingMode);
                    vxnneSaveData(weightDataFormat, j, 0.0f, zerosValuePtr, weightFixPointPos, weightRoundingMode);
                }

                pWeightData = weightData;
                pBiasData = biasData;

                for (w = 0; w < outputsDepth; w++)
                {
                    for (z = 0; z < inputsDepth; z++)
                    {
                        if (w == z)
                        {
                            memcpy(pWeightData, weightsValuePtr, weightItemCount * weightItemSize);
                        }
                        else
                        {
                            memcpy(pWeightData, zerosValuePtr, weightItemCount * weightItemSize);
                        }
                        pWeightData += weightItemCount * weightItemSize;
                    }
                    *pBiasData++ = 0.0f;
                }

                weightStrideSize[0] = weightItemSize;
                for (i = 1; i < numWeightDims; i++)
                {
                    weightStrideSize[i] =  weightStrideSize[i-1] * weightSize[i-1];
                }

                weightUserAddr = vxCreateTensorAddressing(context, weightSize, weightStrideSize, (vx_uint8)numWeightDims);

                vxoCopyTensorPatch(weights, VX_NULL, weightUserAddr, weightData, VX_WRITE_ONLY,0);

                biasStrideSize[0] = biasItemSize;

                gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
                tensor_create_params.num_of_dims = numBiasDims;
                tensor_create_params.sizes = biasSize;
                tensor_create_params.data_format = biasDataFormat;
                tensor_create_params.quant_format = VX_QUANT_DYNAMIC_FIXED_POINT;
                tensor_create_params.quant_data.dfp.fixed_point_pos = biasFixPointPos;

                biases = vxoTensor_CreateTensor(context, node->graph, &tensor_create_params, vx_false_e);
                biasUserAddr = vxCreateTensorAddressing(context, biasSize, biasStrideSize, (vx_uint8)numBiasDims);

                vxoCopyTensorPatch(biases, VX_NULL, biasUserAddr, biasData, VX_WRITE_ONLY,0);

                weights_biases = vxoCreateWeightsBiasesParameterFromTensors(context,
                                                                       VX_NN_CONVOLUTION_LAYER,
                                                                       (vx_uint32*)(TENSOR_SIZES(inputs)),
                                                                       inputs->dimCount,
                                                                       inputs->dimCount,
                                                                       pool_pad_x_left,
                                                                       pool_pad_x_right,
                                                                       pool_pad_y_top,
                                                                       pool_pad_y_bottom,
                                                                       0,
                                                                       0,
                                                                       0,
                                                                       0,
                                                                       roundingValue,
                                                                       (vx_uint32*)(TENSOR_SIZES(outputs)),
                                                                       VX_NULL,
                                                                       VX_NULL,
                                                                       TENSOR_DATA_TYPE(weights),
                                                                       0,
                                                                       VX_TENSOR_RANK_WHCN,
                                                                       weights,
                                                                       biases,
                                                                       VX_NULL,
                                                                       vx_false_e,
                                                                       vx_false_e);
            }

            if (weightsValuePtr != VX_NULL)
                vxFree(weightsValuePtr);

            if (zerosValuePtr != VX_NULL)
                vxFree(zerosValuePtr);

            if (weightData != VX_NULL)
                vxFree(weightData);

            if (biasData != VX_NULL)
                vxFree(biasData);

            {
                vx_op_param_s conv = {0};

                conv.pad_x_left = pool_pad_x_left;
                conv.pad_x_right = pool_pad_x_right;
                conv.pad_y_top = pool_pad_y_top;
                conv.pad_y_bottom = pool_pad_y_bottom;
                conv.pad_mode = VX_PAD_CONSTANT;
                conv.pad_const = 0,
                conv.pool_type = VIV_NN_POOLING_NON;
                conv.conv_rounding_type = roundingValue;
                conv.enable_relu = vx_false_e;
                conv.pool_size_x = conv.pool_size_y = 0;
                /* fill in cmd buffer */
                memcpy(&poolingLayer->pooling_nne_operation.base.parameter, &conv, sizeof(vx_op_param_s));
            }

            vxoCompressNNFirstTime(context, weights_biases, outputs);

            poolingLayer->pooling_nne_operation.weights_biases = weights_biases;

            if (weights != VX_NULL)
            {
                vxoTensor_ReleaseTensor(&weights);
            }

            if (biases != VX_NULL)
            {
                vxoTensor_ReleaseTensor(&biases);
            }

            if (weightUserAddr != VX_NULL)
            {
                vxReleaseTensorAddressing(&weightUserAddr);
            }

            if (biasUserAddr != VX_NULL)
            {
                vxReleaseTensorAddressing(&biasUserAddr);
            }
        }
    }
    else
    {
        vx_bool isTpSupportFormat = vxnneIsTPSupportFormat(context, inputs, VX_NULL, outputs);
        vx_bool isStride1Support = ((stride == 1) && gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_TP_MAX_POOLING_STRIDE1) && poolSizeXValue <= 3) ? vx_true_e : vx_false_e;
        vx_bool isStride2Support = ((stride == 2) && (stride == poolSizeXValue || stride == poolSizeXValue-1)) ? vx_true_e : vx_false_e;
        vx_bool isPoolSizeSupport = (poolSizeXValue == 1 && !pool_pad_x_left && !pool_pad_y_top && TENSOR_VIEW_SIZE_INDEX(inputs, 0) % stride == 0 && TENSOR_VIEW_SIZE_INDEX(inputs, 1) % stride == 0) ? vx_true_e : vx_false_e;

        /* stride!=2 is not supported yet */
        if ((poolTypeValue == VX_NN_POOLING_MAX) &&
            isTpSupportFormat &&
            vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP_MAX_POOLING) &&
            (isStride1Support || isStride2Support || isPoolSizeSupport) &&
            (poolSizeXValue <= 64) &&
            (avgPool_BF_flag == vx_false_e) &&
            (poolSizeXValue < 4 &&  poolSizeYValue < 4))
        {
            vx_op_param_s conv = {0};

            status = vxnneOperation_Initialize(&poolingLayer->pooling_tp_operation.base,
                                               &poolingLayer->base,
                                               VXNNE_OPERATION_TARGET_TP,
                                               VXNNE_OPERATOR_POOLING,
                                               VX_NULL,
                                               VX_NULL,
                                               batchCount,
                                               0);
            if (status != VX_SUCCESS) goto exit;

            conv.pad_x_left   = pool_pad_x_left;
            conv.pad_x_right  = pool_pad_x_right;
            conv.pad_y_top    = pool_pad_y_top;
            conv.pad_y_bottom = pool_pad_y_bottom;
            conv.pool_size_x  = poolSizeXValue;
            conv.pool_size_y  = poolSizeYValue;
            conv.pool_stride  = stride;
            conv.enable_relu  = vx_false_e;
            conv.conv_rounding_type = roundingValue;
            conv.pad_mode = VX_PAD_REPLICATE;
            conv.pad_const = 0;
            conv.tpType = TP_MAX_POOLING;
            conv.other_ref = gcvNULL;
            conv.data_buff = gcvNULL;

            memcpy(&poolingLayer->pooling_tp_operation.base.parameter, &conv, sizeof(vx_op_param_s));

            vxnneLayer_SetOperation(
                &poolingLayer->base,
                &poolingLayer->pooling_tp_operation.base,
                0);

            poolingLayer->pooling_tp_operation.input  = inputs;
            poolingLayer->pooling_tp_operation.output = outputs;

            vxnneOperation_AddReference(&poolingLayer->pooling_tp_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&poolingLayer->pooling_tp_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
        }
        else
        {
            vx_uint32  operation_idx           = 0;
            vx_uint32  numTmpTensor            = 0;
            vx_bool kernel_MaxPool_flag[5]     = {vx_false_e};
            vx_bool dataformat_MaxPool_flag[6] = {vx_false_e};
            vx_bool maxPool_flag               = vx_false_e;
            vx_bool enable_L2Pool_SH           = vx_false_e;
            vx_bool generic_flag               = vx_false_e;
            vx_bool maxPool_BF_flag            = vx_false_e;
            vx_bool enable_outputALU           = checkOutputTensorDoAlu(inputs, outputs);
            vx_bool enable_downSampleSH        = (vx_bool)(stride == 2 && poolSizeXValue == 1 && poolSizeYValue == 1 && (!enable_outputALU));
            vx_bool enable_tf_avgPool          = (vx_bool)(pool_pad_x_left || pool_pad_x_right ||pool_pad_y_top || pool_pad_y_bottom);

            kernel_MaxPool_flag[0]             = (vx_bool)(stride == 2 && poolSizeXValue == 3 && pool_pad_x_left == 1);
            kernel_MaxPool_flag[1]             = (vx_bool)(stride == 2 && poolSizeXValue == 2 && pool_pad_x_left == 0);
            kernel_MaxPool_flag[2]             = (vx_bool)(stride == 2 && poolSizeXValue == 3 && pool_pad_x_left == 0);
            kernel_MaxPool_flag[3]             = (vx_bool)(stride == 1 && poolSizeXValue == 3 && pool_pad_x_left == 1);
            kernel_MaxPool_flag[4]             = (vx_bool)(stride == 1 && poolSizeXValue == 2 && poolSizeYValue == 2 && pool_pad_x_left == 0 && pool_pad_y_top == 0);

            if(context->evisNoInst.supportEVIS)
            {
                dataformat_MaxPool_flag[0]         = (vx_bool)((inputdata_format == VX_TYPE_FLOAT16 || inputdata_format == VX_TYPE_INT8)
                                                            && (outputdata_format == VX_TYPE_FLOAT16 || outputdata_format == VX_TYPE_INT8));
                dataformat_MaxPool_flag[1]         = (vx_bool)(inputdata_format == VX_TYPE_INT16 && outputdata_format == VX_TYPE_INT16);
                dataformat_MaxPool_flag[2]         = (vx_bool)(inputdata_format == VX_TYPE_UINT8 && outputdata_format == VX_TYPE_UINT8);
                dataformat_MaxPool_flag[3]         = (vx_bool)(inputdata_format == VX_TYPE_FLOAT16 && outputdata_format == VX_TYPE_FLOAT16);
                dataformat_MaxPool_flag[4]         = (vx_bool)(inputdata_format == VX_TYPE_INT8 && outputdata_format == VX_TYPE_INT8);
                dataformat_MaxPool_flag[5]         = (vx_bool)(inputdata_format == VX_TYPE_INT16 && outputdata_format == VX_TYPE_INT16);
                enable_L2Pool_SH                   = (vx_bool)(((inputdata_format == VX_TYPE_BFLOAT16 && outputdata_format == VX_TYPE_BFLOAT16)
                                                            || ((inputdata_format == VX_TYPE_FLOAT16  || inputdata_format == VX_TYPE_UINT8)
                                                            && (outputdata_format == VX_TYPE_FLOAT16  || outputdata_format == VX_TYPE_UINT8)))
                                                            && (poolTypeValue == VX_NN_POOLING_L2));
                maxPool_BF_flag                    = (vx_bool)(inputdata_format == VX_TYPE_BFLOAT16 && outputdata_format == VX_TYPE_BFLOAT16);
            }
            else
            {
                dataformat_MaxPool_flag[0]         = vx_false_e;
                dataformat_MaxPool_flag[1]         = vx_false_e;
                dataformat_MaxPool_flag[2]         = (vx_bool)(inputdata_format == VX_TYPE_UINT8 && outputdata_format == VX_TYPE_UINT8);
                dataformat_MaxPool_flag[3]         = (vx_bool)((inputdata_format == VX_TYPE_FLOAT16 && outputdata_format == VX_TYPE_FLOAT16) ||
                                                                (inputdata_format == VX_TYPE_FLOAT32 && outputdata_format == VX_TYPE_FLOAT32));
                dataformat_MaxPool_flag[4]         = vx_false_e;
                dataformat_MaxPool_flag[5]         = vx_false_e;
                enable_L2Pool_SH                   = (vx_bool)((dataformat_MaxPool_flag[2] || dataformat_MaxPool_flag[3]) && (poolTypeValue == VX_NN_POOLING_L2));
            }

            generic_flag                       = (vx_bool)(dataformat_MaxPool_flag[2] || dataformat_MaxPool_flag[3] || dataformat_MaxPool_flag[4] || dataformat_MaxPool_flag[5]);
            maxPool_flag                       = (vx_bool)((((kernel_MaxPool_flag[0] || kernel_MaxPool_flag[1] || kernel_MaxPool_flag[2] || kernel_MaxPool_flag[3]) && dataformat_MaxPool_flag[0])
                                               || (kernel_MaxPool_flag[3] && dataformat_MaxPool_flag[1])
                                               || ((kernel_MaxPool_flag[3] || kernel_MaxPool_flag[0]) && dataformat_MaxPool_flag[2])
                                               || (kernel_MaxPool_flag[4] && dataformat_MaxPool_flag[4])
                                               || generic_flag || maxPool_BF_flag)
                                               && (poolTypeValue == VX_NN_POOLING_MAX));

            enable_downSampleSH = enable_downSampleSH && (poolTypeValue == VX_NN_POOLING_MAX || poolTypeValue == VX_NN_POOLING_AVG) && (pool_pad_x_left == 0 && pool_pad_y_top == 0);

            enable_tf_avgPool  = enable_tf_avgPool && (avgPool_flag || avgPool_BF_flag) && (poolTypeValue == VX_NN_POOLING_AVG_ANDROID);

            /* tf pad avgpool, now only support fp16 to fp16 and u8 to u8*/
            enable_tf_avgPool  = enable_tf_avgPool && (dataFormat_AvgPool_flag[1] || dataFormat_AvgPool_flag[3] || dataFormat_AvgPool_flag[5] || avgPool_BF_flag);

            if ((avgPool_flag || maxPool_flag || enable_L2Pool_SH || enable_downSampleSH || avgPool_BF_flag)
               && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
            {
                vxnne_shader_executable shaderExecutable = NULL;
                vx_scalar stride_s = NULL;
                stride_s = vxCreateScalar(node->base.context, VX_TYPE_UINT32, &stride);
                if (!stride_s)
                {
                    status = VX_FAILURE;
                    goto exit;
                }

                if(node->base.context->evisNoInst.supportEVIS)
                {
                    if (enable_tf_avgPool)
                    {
                        vx_tensor mask          = NULL;
                        vx_uint32 sizes[4]      = {1, 1, 1, 1};
                        vx_uint32 mask_size     = 0;
                        vx_uint8  *maskData     = VX_NULL;
                        vx_tensor_create_params_t tensor_create_params;

                        sizes[0]            = TENSOR_VIEW_SIZE_INDEX(inputs, 0);
                        sizes[1]            = TENSOR_VIEW_SIZE_INDEX(inputs, 1);
                        sizes[2]            = TENSOR_VIEW_SIZE_INDEX(inputs, 2);
                        sizes[3]            = TENSOR_VIEW_SIZE_INDEX(inputs, 3);

                        gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
                        tensor_create_params.num_of_dims = TENSOR_DIM_NUM(inputs);
                        tensor_create_params.sizes = sizes;
                        tensor_create_params.data_format = VX_TYPE_UINT8;
                        tensor_create_params.quant_format = TENSOR_QUANT_TYPE(inputs);
                        if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
                        {
                            tensor_create_params.quant_data.dfp.fixed_point_pos = 0;
                        }
                        else
                        {
                            tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(inputs);
                            tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(inputs);
                        }
                        mask                = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_false_e);
                        if (vxoTensor_AllocateMemory(mask) != VX_SUCCESS)
                        {
                            vxError("vxoTensor_AllocateMemory fail at function %s, line %d", __FUNCTION__, __LINE__);
                            status = VX_ERROR_NO_MEMORY;
                            goto exit;
                        }
                        vxoTensor_GetTensorViewMemory(mask, (vx_ptr_ptr)&maskData, VX_NULL);
                        vxoTensor_GetTensorElementCount(mask, &mask_size);
                        memset(maskData, 1, mask_size);

                        shaderExecutable = vxnneGetTFAvgPoolingShaderExecutable(node->base.context, VXNNE_KERNEL_TF_AVGPOOLING, &node->kernelAttributes.borderMode, inputs, mask, stride_s, pool_size_x_s, pool_size_y_s, pool_pad_x_left, pool_pad_y_top, VX_NN_ACTIVATION_NONE, outputs);


                        poolingLayer->base.num_temp_tensors = 1;
                        poolingLayer->base.temp_tensors[0]  = mask;
                    }
                    else if (enable_downSampleSH)
                    {
                        vx_float32   scale_factor[2]    = {inputsWidth / (vx_float32)outputsWidth, inputsHeight / (vx_float32)outputsHeight};

                        if (scale_factor[0] == 2.0f && scale_factor[1] == 2.0f)
                            shaderExecutable = vxnneGetResizeNearestNeighborShaderExecutable(node->base.context, VXNNE_KERNEL_RESIZE_NEAREST_NEIGHBOR, &node->kernelAttributes.borderMode, inputs, VX_INTERPOLATION_NEAREST_NEIGHBOR, outputs);
                        else
                            shaderExecutable = vxnneGetMaxPoolingShaderExecutable(node->base.context, VXNNE_KERNEL_MAXPOOLING, &node->kernelAttributes.borderMode, inputs, pool_type_s, stride_s, pool_size_x_s, pool_size_y_s, pool_pad_x_left, pool_pad_y_top, rounding_s, VX_NN_ACTIVATION_NONE, outputs);

                    }
                    else if (maxPool_flag)
                        shaderExecutable = vxnneGetMaxPoolingShaderExecutable(node->base.context, VXNNE_KERNEL_MAXPOOLING, &node->kernelAttributes.borderMode,
                        inputs, pool_type_s, stride_s, pool_size_x_s, pool_size_y_s, pool_pad_x_left, pool_pad_y_top, rounding_s, VX_NN_ACTIVATION_NONE, outputs);
                    else if (enable_L2Pool_SH)
                        shaderExecutable = vxnneGetL2PoolingShaderExecutable(node->base.context, VXNNE_KERNEL_L2POOLING, &node->kernelAttributes.borderMode,
                        inputs, pool_type_s, stride_s, pool_size_x_s, pool_size_y_s, pool_pad_x_left, pool_pad_y_top, rounding_s, VX_NN_ACTIVATION_NONE, outputs);
                    else if(avgPool_flag && enable_tf_quantize)
                        shaderExecutable = vxnneGetAvgPooling_UInt8ShaderExecutable(node->base.context, VXNNE_KERNEL_AVGPOOLING_UINT8, &node->kernelAttributes.borderMode,
                        inputs, pool_type_s, stride_s, pool_size_x_s, pool_size_y_s, pool_pad_x_left, pool_pad_y_top, rounding_s, VX_NN_ACTIVATION_NONE, outputs);
                    else if(avgPool_flag && enable_int16_sh)
                        shaderExecutable = vxnneGetAvgPooling_Int16ShaderExecutable(node->base.context, VXNNE_KERNEL_AVGPOOLING_INT16, &node->kernelAttributes.borderMode,
                        inputs, pool_type_s, stride_s, pool_size_x_s, pool_size_y_s, pool_pad_x_left, pool_pad_y_top, rounding_s, VX_NN_ACTIVATION_NONE, outputs);
                    else
                        shaderExecutable = vxnneGetAvgPoolingShaderExecutable(node->base.context, VXNNE_KERNEL_AVGPOOLING, &node->kernelAttributes.borderMode,
                        inputs, pool_type_s, stride_s, pool_size_x_s, pool_size_y_s, pool_pad_x_left, pool_pad_y_top, rounding_s, outputs);
                }
                else
                {
                    vx_uint32 stride_y_value = vxoNNExternsionConvlutionRound((vx_float32)(inputsHeight + pool_pad_y_top + pool_pad_y_bottom - poolSizeYValue) / (outputsHeight - 1), roundingValue);
                    vx_scalar stride_y = NULL;

                    if (!maxPool_flag)
                        stride_y = vxCreateScalar(node->base.context, VX_TYPE_UINT32, &stride_y_value);

                    if(avgPool_flag)
                    {
                        vx_tensor tensorCopy       = NULL;
                        vx_bool enable_2d_img  = (vx_bool)(TENSOR_VIEW_SIZE_INDEX(inputs, 1) * TENSOR_VIEW_SIZE_INDEX(inputs, 2) < IMG_MAX_WIDTH);
                        vx_bool is_copy_tensor = ((inputdata_format == VX_TYPE_UINT8) && (3 == poolSizeXValue) && (3 == poolSizeYValue)
                            && (stride == 1 || stride == 2) && enable_2d_img && outputdata_format == VX_TYPE_UINT8
                            && (pool_pad_x_left == 1 || pool_pad_x_right == 1 || pool_pad_y_top == 1 || pool_pad_y_bottom == 1));

                        if (is_copy_tensor)
                        {
                            vx_tensor_create_params_t tensor_create_params;
                            vx_uint32 sizes[]       = {1, 1, 1, 1};
                            vx_uint32 inputWidth   = 0;
                            vx_uint32 copy_dims     = TENSOR_DIM_NUM(inputs);
                            gctPOINTER inputLogical = VX_NULL;
                            vx_uint8   inputZP      = (vx_uint8)TENSOR_TF_ZEROPOINT(inputs);
                            vx_uint32  copy_size     = 0;
                            vx_int32      padLeftv                       = 0;
                            vx_int32      padRightv                      = 0;
                            vx_int32      padTopv                        = 0;
                            vx_int32      padBottomv                     = 0;
                            vx_scalar     padLeft                        = VX_NULL;
                            vx_scalar     padTop                         = VX_NULL;
                            vx_scalar     padXLeft                       = VX_NULL;
                            vx_scalar     padYTop                        = VX_NULL;


                            sizes[0] = TENSOR_VIEW_SIZE_INDEX(inputs, 0);
                            sizes[1] = TENSOR_VIEW_SIZE_INDEX(inputs, 1);
                            sizes[2] = copy_dims > 2 ? TENSOR_VIEW_SIZE_INDEX(inputs, 2) : 1;
                            sizes[3] = copy_dims > 3 ? TENSOR_VIEW_SIZE_INDEX(inputs, 3) : 1;
                            sizes[0] = sizes[0] + pool_pad_x_left + pool_pad_x_right;
                            inputWidth = sizes[0];
                            sizes[1] = sizes[1] + pool_pad_y_top + pool_pad_y_bottom;
                            if (sizes[0] % 4 != 0)
                            {
                                sizes[0] = gcmALIGN(sizes[0], 4);
                            }
                            gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
                            tensor_create_params.num_of_dims = copy_dims;
                            tensor_create_params.sizes = sizes;
                            tensor_create_params.data_format = TENSOR_DATA_TYPE(inputs);
                            tensor_create_params.quant_format = TENSOR_QUANT_TYPE(inputs);
                            if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
                            {
                                tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(inputs);
                            }
                            else
                            {
                                tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(inputs);
                                tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(inputs);
                            }
                            tensorCopy = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_false_e);
                            if (tensorCopy == VX_NULL || vxoTensor_AllocateMemory(tensorCopy) != VX_SUCCESS)
                            {
                                vxError("vxoTensor_AllocateMemory fail at function %s, line %d", __FUNCTION__, __LINE__);
                                status = VX_ERROR_NO_MEMORY;
                                goto exit;
                            }
                            vxoTensor_GetTensorViewMemory(tensorCopy, &inputLogical, VX_NULL);
                            copy_size = sizes[0] * sizes[1] * sizes[2] * sizes[3];
                            gcoOS_MemFill(inputLogical, inputZP, copy_size);
                            padLeftv   = 0;
                            padRightv  = 0;
                            padTopv    = 0;
                            padBottomv = 0;
                            padLeft   = vxCreateScalar(context, VX_TYPE_INT32, &padLeftv);
                            padTop  = vxCreateScalar(context, VX_TYPE_INT32, &padTopv);
                            padXLeft    = vxCreateScalar(context, VX_TYPE_INT32, &pool_pad_x_left);
                            padYTop = vxCreateScalar(context, VX_TYPE_INT32, &pool_pad_y_top);

                            shaderExecutable = vxnneGPUTensorCopyROIShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_COPYROI,
                                &node->kernelAttributes.borderMode,
                                padLeft, padTop, padXLeft, padYTop, inputs, tensorCopy);
                            if (!shaderExecutable)
                            {
                                status = VX_FAILURE;
                                goto exit;
                            }

                            status = vxnneShaderOperation_Initialize(&poolingLayer->pooling_tensorcopy_sh_operation,
                                &poolingLayer->base,
                                VXNNE_OPERATOR_DEPTHWISE_CONV,
                                batchCount,
                                shaderExecutable);

                            if (status != VX_SUCCESS) goto exit;

                            vxnneLayer_SetOperation(
                                &poolingLayer->base,
                                &poolingLayer->pooling_tensorcopy_sh_operation.base,
                                operation_idx++);

                            vxnneOperation_AddReference(&poolingLayer->pooling_tensorcopy_sh_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
                            vxnneOperation_AddReference(&poolingLayer->pooling_tensorcopy_sh_operation.base, (vx_reference)tensorCopy, VXNNE_OPERATION_REFENRENCE_OUTPUT);


                            if(padLeft) vxReleaseScalar(&padLeft);
                            if(padTop) vxReleaseScalar(&padTop);
                            if(padXLeft) vxReleaseScalar(&padXLeft);
                            if(padYTop) vxReleaseScalar(&padYTop);
                            poolingLayer->base.temp_tensors[numTmpTensor++] = tensorCopy;
                            poolingLayer->base.num_temp_tensors = numTmpTensor;
                        }
                        else
                        {
                            tensorCopy = inputs;
                        }

                        shaderExecutable = vxnneGetGPUAvgPoolingShaderExecutable(
                        node->base.context, VXNNE_KERNEL_AVGPOOLING, &node->kernelAttributes.borderMode,
                        tensorCopy, pool_type_s, stride_s, stride_y, pool_size_x_s, pool_size_y_s, pool_pad_x_left,
                        pool_pad_y_top, pool_pad_x_right, pool_pad_y_bottom, rounding_s, is_copy_tensor,
                        TENSOR_VIEW_SIZE_INDEX(inputs, 0), TENSOR_VIEW_SIZE_INDEX(inputs, 1), outputs);
                    }
                    else if (enable_L2Pool_SH)
                    {
                        shaderExecutable = vxnneGetGPUL2PoolingShaderExecutable(node->base.context, VXNNE_KERNEL_L2POOLING, &node->kernelAttributes.borderMode,
                                        inputs, pool_type_s, stride_s, stride_y, pool_size_x_s, pool_size_y_s, pool_pad_x_left, pool_pad_y_top, pool_pad_x_right,
                                        pool_pad_y_bottom, rounding_s, outputs);
                    }
                    else if (maxPool_flag)
                    {
                        shaderExecutable = vxnneGetGPUMaxPoolingShaderExecutable(node->base.context, VXNNE_KERNEL_MAXPOOLING, &node->kernelAttributes.borderMode,
                                       inputs, pool_type_s, stride_s, stride_y_value, pool_size_x_s, pool_size_y_s, pool_pad_x_left, pool_pad_y_top, pool_pad_x_right,
                                       pool_pad_y_bottom,rounding_s, outputs);
                    }

                    if (!maxPool_flag)
                    {
                        if (stride_y) (vxReleaseScalar(&stride_y));
                    }
                }

                if (!shaderExecutable)
                {
                    status = VX_FAILURE;
                    vxReleaseScalar(&stride_s);
                    goto exit;
                }

                status = vxnneShaderOperation_Initialize(&poolingLayer->pooling_sh_operation,
                                                &poolingLayer->base,
                                                VXNNE_OPERATOR_POOLING,
                                                batchCount,
                                                shaderExecutable);

                if (status != VX_SUCCESS)
                {
                    vxReleaseScalar(&stride_s);
                    goto exit;
                }

                vxnneOperation_AddReference(&poolingLayer->pooling_sh_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&poolingLayer->pooling_sh_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);

                vxnneLayer_SetOperation(
                    &poolingLayer->base,
                    &poolingLayer->pooling_sh_operation.base,
                    operation_idx++);

                if (stride_s) (vxReleaseScalar(&stride_s));
            }
            else
            {
                status = vxnneOperation_Initialize(&poolingLayer->pooling_sw_operation.base,
                                                   &poolingLayer->base,
                                                   VXNNE_OPERATION_TARGET_SW,
                                                   VXNNE_OPERATOR_POOLING,
                                                   vxnneExecuteSWPooling,
                                                   VX_NULL,
                                                   batchCount,
                                                   0);

                vxnneLayer_SetOperation(
                    &poolingLayer->base,
                    &poolingLayer->pooling_sw_operation.base,
                    0);

                poolingLayer->pooling_sw_operation.inputs            = inputs;
                poolingLayer->pooling_sw_operation.pool_type         = pool_type_s->value->e;
                poolingLayer->pooling_sw_operation.pool_size_x       = pool_size_x_s->value->u32;
                poolingLayer->pooling_sw_operation.pool_size_y       = pool_size_y_s->value->u32;
                poolingLayer->pooling_sw_operation.pool_pad_x_left   = pool_pad_x_left;
                poolingLayer->pooling_sw_operation.pool_pad_x_right  = pool_pad_x_right;
                poolingLayer->pooling_sw_operation.pool_pad_y_top    = pool_pad_y_top;
                poolingLayer->pooling_sw_operation.pool_pad_y_bottom = pool_pad_y_bottom;
                poolingLayer->pooling_sw_operation.rounding          = rounding_s->value->e;
                poolingLayer->pooling_sw_operation.outputs           = outputs;

                vxnneOperation_AddReference(&poolingLayer->pooling_sw_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&poolingLayer->pooling_sw_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
            }
        }
    }

    node->layer = &poolingLayer->base;
    return status;

exit:
    if (poolingLayer) gcoOS_Free(gcvNULL, (gctPOINTER)poolingLayer);
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNPoolingLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_tensor  inputs                     = (vx_tensor)parameters[0];
    vx_scalar  poolTypeScalar             = (vx_scalar)parameters[1];
    vx_scalar  poolSizeXScalar            = (vx_scalar)parameters[2];
    vx_scalar  poolSizeYScalar            = (vx_scalar)parameters[3];
    vx_scalar  poolPadXScalar             = (vx_scalar)parameters[4];
    vx_scalar  poolPadYScalar             = (vx_scalar)parameters[5];
    vx_scalar  roundingScalar             = (vx_scalar)parameters[6];
    vx_tensor  outputs                    = (vx_tensor)parameters[7];

    vx_uint32 pad_x_left;
    vx_uint32 pad_x_right;
    vx_uint32 pad_y_top;
    vx_uint32 pad_y_bottom;

    vx_status  status                     = VX_SUCCESS;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    pad_x_left = pad_x_right = poolPadXScalar->value->u32;
    pad_y_top = pad_y_bottom = poolPadYScalar->value->u32;

    return status = vxnnePoolingInitializer(node,
                                            "PoolingLayer",
                                            inputs,
                                            poolTypeScalar,
                                            poolSizeXScalar,
                                            poolSizeYScalar,
                                            pad_x_left,
                                            pad_x_right,
                                            pad_y_top,
                                            pad_y_bottom,
                                            roundingScalar,
                                            VX_PAD_CONSTANT,
                                            VX_NULL,
                                            outputs);

}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNPoolingLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return VX_SUCCESS;
}


VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNPoolingLayer2(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNPoolingLayer2_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNPoolingLayer2_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    if (index != vxmLENGTH_OF(nn_PoolingLayer_params2) - 1) return VX_ERROR_INVALID_PARAMETERS;

    ptr->type                 = VX_TYPE_TENSOR;

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNPoolingLayer2_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
    vx_tensor  inputs                     = (vx_tensor)parameters[0];
    vx_scalar  poolTypeScalar             = (vx_scalar)parameters[1];
    vx_scalar  poolSizeXScalar            = (vx_scalar)parameters[2];
    vx_scalar  poolSizeYScalar            = (vx_scalar)parameters[3];
    vx_scalar  poolPadXLeftScalar         = (vx_scalar)parameters[4];
    vx_scalar  poolPadXRightScalar        = (vx_scalar)parameters[5];
    vx_scalar  poolPadYTopScalar          = (vx_scalar)parameters[6];
    vx_scalar  poolPadYBottomScalar       = (vx_scalar)parameters[7];
    vx_scalar  roundingScalar             = (vx_scalar)parameters[8];
    vx_tensor  outputs                    = (vx_tensor)parameters[9];

    vx_uint32 poolPadXLeft                = poolPadXLeftScalar->value->u32;
    vx_uint32 poolPadXRight               = poolPadXRightScalar->value->u32;
    vx_uint32 poolPadYTop                 = poolPadYTopScalar->value->u32;
    vx_uint32 poolPadYBottom              = poolPadYBottomScalar->value->u32;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return status = vxnnePoolingInitializer(node,
                                            "PoolingLayer2",
                                            inputs,
                                            poolTypeScalar,
                                            poolSizeXScalar,
                                            poolSizeYScalar,
                                            poolPadXLeft,
                                            poolPadXRight,
                                            poolPadYTop,
                                            poolPadYBottom,
                                            roundingScalar,
                                            VX_PAD_CONSTANT,
                                            VX_NULL,
                                            outputs);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNPoolingLayer2_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return VX_SUCCESS;
}
