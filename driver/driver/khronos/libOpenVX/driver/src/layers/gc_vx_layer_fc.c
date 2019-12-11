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
#include <layers/gc_vx_layer_fc.h>
#include <ops/gc_vx_op_fc.h>


vx_status vxnneExecuteSWFullyConnected(struct _vxnne_operation_s *operation)
{
    vxnne_fully_connected_sw_operation           fullyConnectedOperation   = (vxnne_fully_connected_sw_operation)operation;

    vx_tensor inputs  = (vx_tensor)fullyConnectedOperation->inputs;
    vx_tensor weights = (vx_tensor)fullyConnectedOperation->weights;
    vx_tensor biases  = (vx_tensor)fullyConnectedOperation->biases;
    vx_tensor outputs = (vx_tensor)fullyConnectedOperation->outputs;
    gctPOINTER inputsBaseLogicalAddr = VX_NULL, outputsBaseLogicalAddr = VX_NULL;
    gctPOINTER weightsBaseLogicalAddr = VX_NULL, biasesBaseLogicalAddr = VX_NULL;
    vx_uint32 i = 0, j = 0, b = 0;
    vx_uint32 inputCount, outputCount;
    vx_float32 madValue, inputValue, weightValue, biasValue = 0.0f;
    vx_enum srcType, dstType, weightsType, biasesType, outputRoundingMode;
    vx_int8 inputFpPos = 0, weightFpPos = 0, biasFpPos = 0, outputFpPos = 0;
    vx_float32 result = 0.0f;
    vx_status status = VX_SUCCESS;
    vx_uint32 dims = TENSOR_VIEW_DIM_NUM(inputs);
    vx_uint32  width         = TENSOR_VIEW_SIZE_INDEX(inputs, 0);
    vx_uint32  height        = dims > 1 ? TENSOR_VIEW_SIZE_INDEX(inputs, 1) : 1;
    vx_uint32  depth         = dims > 2 ? TENSOR_VIEW_SIZE_INDEX(inputs, 2) : 1;
    vx_uint32  batch         = dims > 3 ? TENSOR_VIEW_SIZE_INDEX(inputs, 3) : 1;
    vx_uint32  batchCount    = 1;

    if (TENSOR_DIM_NUM(inputs) == 2)
    {
        batchCount = TENSOR_SIZE_INDEX(inputs, 1);
    }
    else if (TENSOR_DIM_NUM(inputs) == 4)
    {
        batchCount = TENSOR_SIZE_INDEX(inputs, 3);
    }

    srcType = TENSOR_DATA_TYPE(inputs);
    dstType = TENSOR_DATA_TYPE(outputs);
    weightsType = TENSOR_DATA_TYPE(weights);
    biasesType = TENSOR_DATA_TYPE(biases);

    vxoTensor_GetTensorViewMemory(inputs, &inputsBaseLogicalAddr, VX_NULL);
    vxoTensor_GetTensorViewMemory(outputs, &outputsBaseLogicalAddr, VX_NULL);
    vxoTensor_GetTensorViewMemory(weights, &weightsBaseLogicalAddr, VX_NULL);
    vxoTensor_GetTensorViewMemory(biases, &biasesBaseLogicalAddr, VX_NULL);

    inputCount = (vx_uint32)(width * height * depth * batch) / batchCount;
    dims          = TENSOR_VIEW_DIM_NUM(outputs);
    width         = TENSOR_VIEW_SIZE_INDEX(outputs, 0);
    height        = dims > 1 ? TENSOR_VIEW_SIZE_INDEX(outputs, 1) : 1;
    depth         = dims > 2 ? TENSOR_VIEW_SIZE_INDEX(outputs, 2) : 1;
    batch         = dims > 3 ? TENSOR_VIEW_SIZE_INDEX(outputs, 3) : 1;
    outputCount = (vx_uint32)(width * height * depth * batch) / batchCount;

    inputFpPos = TENSOR_POS(inputs);
    weightFpPos = TENSOR_POS(weights);
    biasFpPos = TENSOR_POS(biases);
    outputFpPos = TENSOR_POS(outputs);
    outputRoundingMode = TENSOR_ROUNDING_MODE(outputs);

    for (b = 0; b < batchCount; b ++)
    {
        for (i = 0; i < outputCount; i++)
        {
            madValue = 0.0;
            for (j = 0; j < inputCount; j++)
            {
                if (((srcType == VX_TYPE_FLOAT16) && (weightsType == VX_TYPE_FLOAT16) && (biasesType == VX_TYPE_FLOAT32)) ||
                    ((srcType == VX_TYPE_FLOAT32) && (weightsType == VX_TYPE_FLOAT32) && (biasesType ==  VX_TYPE_FLOAT32)) ||
                    ((srcType == VX_TYPE_INT8) && (weightsType == VX_TYPE_INT8) && (biasesType == VX_TYPE_INT32 || biasesType == VX_TYPE_FLOAT32)) ||
                    ((srcType == VX_TYPE_INT16) && (weightsType == VX_TYPE_INT16) && (biasesType == VX_TYPE_INT32 || biasesType == VX_TYPE_INT64 || biasesType == VX_TYPE_FLOAT32)))
                {
                    inputValue  = vxnneGetDataExt((vx_type_e)srcType, TENSOR_QUANT_TYPE(inputs), j + b * inputCount, (vx_uint8_ptr)inputsBaseLogicalAddr, inputFpPos, TENSOR_TF_ZEROPOINT(inputs), TENSOR_TF_SCALE(inputs));
                    weightValue = vxnneGetDataExt((vx_type_e)weightsType, TENSOR_QUANT_TYPE(weights), inputCount * i + j, (vx_uint8_ptr)weightsBaseLogicalAddr, weightFpPos, TENSOR_TF_ZEROPOINT(weights), TENSOR_TF_SCALE(weights));

                    madValue += inputValue * weightValue;
                }
                else if((srcType == VX_TYPE_UINT8) && (weightsType == VX_TYPE_UINT8) && (biasesType == VX_TYPE_INT32 || biasesType == VX_TYPE_FLOAT32) && TENSOR_QUANT_TYPE(inputs) == VX_QUANT_AFFINE_SCALE)
                {
                    inputValue  = vxnneGetDataQuant((vx_type_e)srcType, j + b * inputCount, (vx_uint8_ptr)inputsBaseLogicalAddr, TENSOR_TF_ZEROPOINT(inputs), TENSOR_TF_SCALE(inputs));
                    weightValue = vxnneGetDataQuant((vx_type_e)weightsType, inputCount * i + j, (vx_uint8_ptr)weightsBaseLogicalAddr, TENSOR_TF_ZEROPOINT(weights), TENSOR_TF_SCALE(weights));

                    madValue += inputValue * weightValue;
                }
                else
                {
                    /* other format not surpport now */
                    vxError("can't support this input data format\n");
                    gcmASSERT(0);
                }
            }

            if (biasesType == VX_TYPE_FLOAT32 || biasesType == VX_TYPE_INT32 || biasesType == VX_TYPE_INT64)
            {
                biasValue = vxnneGetDataExt((vx_type_e)biasesType, TENSOR_QUANT_TYPE(biases), i, (vx_uint8_ptr)biasesBaseLogicalAddr, biasFpPos, TENSOR_TF_ZEROPOINT(biases), TENSOR_TF_SCALE(biases));
            }
            else
            {
                vxError("can't support this bias data format\n");
                gcmASSERT(0);
            }

            result = madValue + biasValue;

            vxnneSaveDataExt((vx_type_e)dstType, TENSOR_QUANT_TYPE(outputs), i + b * outputCount, result, outputsBaseLogicalAddr, outputFpPos, TENSOR_TF_ZEROPOINT(outputs), TENSOR_TF_SCALE(outputs), outputRoundingMode);
        }
    }

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNFullyConnectedReluLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNFullyConnectedReluLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNFullyConnectedReluLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    if (index != vxmLENGTH_OF(nn_FullyConnectedReluLayer_params) - 1) return VX_ERROR_INVALID_PARAMETERS;

    ptr->type                 = VX_TYPE_TENSOR;

    return VX_SUCCESS;
}

vx_status vxoNNFullyConnectedLayerInitializer(
    vx_node node,
    vxnne_layer layer,
    vxnne_tp_operation tp_operation0,
    vxnne_tp_operation tp_operation1,
    vxnne_convolution_relu_pooling_operation nn_operation,
    vxnne_shader_operation sh_operation,
    vx_tensor inputs,
    vx_weights_biases_parameter weights_biases,
    vx_uint32 pad,
    vx_enum conv_rounding_type,
    vx_bool enable_relu,
    vx_int32_ptr count,
    vx_tensor outputs)
{
    vx_status status = VX_SUCCESS;
    vx_context context = vxGetContext((vx_reference)node);
    vx_uint32 i = 0, batchCount = 1;
    vx_uint32 phys;
    vx_bool aligned64 = vx_true_e;
    vx_bool   enable_shader               = vx_false_e;
    vx_bool   supportDataFormat0          = vx_false_e;
    vx_bool   supportDataFormat1          = vx_false_e;
    vx_bool   supportDataFormat2          = vx_false_e;
    vx_bool   supportDataFormat3          = vx_false_e;
    vx_enum   input_dataformat            = TENSOR_DATA_TYPE(inputs);
    vx_enum   weight_dataformat           = TENSOR_DATA_TYPE(weights_biases->wb_base->origWeight);
    vx_enum   bias_dataformat             = weights_biases->wb_base->origBias ? TENSOR_DATA_TYPE(weights_biases->wb_base->origBias) : VX_TYPE_INVALID;
    vx_enum   output_dataformat           = TENSOR_DATA_TYPE(outputs);
    vx_uint32 dims                        = TENSOR_VIEW_DIM_NUM(inputs);
    vx_uint32 width                       = TENSOR_VIEW_SIZE_INDEX(inputs, 0);
    vx_uint32 height                      = (dims > 1) ? TENSOR_VIEW_SIZE_INDEX(inputs, 1) : 1;
    vx_uint32 depth                       = (dims > 2) ? TENSOR_VIEW_SIZE_INDEX(inputs, 2) : 1;
    vx_uint32 inputDims                   = width * height * depth;
    vx_uint32 op_index = 0;

    supportDataFormat0 = (vx_bool)(input_dataformat == VX_TYPE_FLOAT16 && weight_dataformat == VX_TYPE_FLOAT16 && (bias_dataformat == VX_TYPE_INVALID || bias_dataformat == VX_TYPE_FLOAT32) && output_dataformat == VX_TYPE_FLOAT16);
    supportDataFormat1 = (vx_bool)(input_dataformat == VX_TYPE_INT8 && weight_dataformat == VX_TYPE_INT8 && (bias_dataformat == VX_TYPE_INVALID || bias_dataformat == VX_TYPE_INT32) && output_dataformat == VX_TYPE_INT8);
    supportDataFormat2 = (vx_bool)(input_dataformat == VX_TYPE_INT16 && weight_dataformat == VX_TYPE_INT16 && (bias_dataformat == VX_TYPE_INVALID || bias_dataformat == VX_TYPE_INT32) && output_dataformat == VX_TYPE_INT16);
    supportDataFormat3 = (vx_bool)(input_dataformat == VX_TYPE_UINT8 && weight_dataformat == VX_TYPE_UINT8 && (bias_dataformat == VX_TYPE_INVALID || bias_dataformat == VX_TYPE_INT32) && output_dataformat == VX_TYPE_UINT8);
    enable_shader      = (supportDataFormat0 || supportDataFormat1 || supportDataFormat2 || supportDataFormat3) && (inputDims < IMG_MAX_WIDTH);

    if (TENSOR_DIM_NUM(inputs) == 2)
    {
        batchCount = TENSOR_SIZE_INDEX(inputs, 1);
        if ((inputs->dims[0] != weights_biases->weights_sizes[2]) ||
            (outputs->dims[0] != weights_biases->weights_sizes[3]))
        {
            vxError("parameter is invalid at function %s, line %d\n", __FUNCTION__, __LINE__);
            vxmONERROR(VX_ERROR_INVALID_PARAMETERS);
        }
    }
    else if (TENSOR_DIM_NUM(inputs) == 4)
    {
        batchCount = TENSOR_SIZE_INDEX(inputs, 3);
        if (((inputs->dims[0] * inputs->dims[1] * inputs->dims[2]) != weights_biases->weights_sizes[2]) ||
            ((outputs->dimCount == 4) && (outputs->dims[2] !=weights_biases->weights_sizes[3])) ||
            ((outputs->dimCount == 2) && (outputs->dims[0] !=weights_biases->weights_sizes[3])))
        {
            vxError("parameter is invalid at function %s, line %d\n", __FUNCTION__, __LINE__);
            vxmONERROR(VX_ERROR_INVALID_PARAMETERS);
        }
    }

    /* TP needs 64-byte aligned inputs. */
    vxoTensor_GetTensorViewMemory(inputs, VX_NULL, &phys);
    if ((phys % 64) ||
        ((batchCount > 1) &&
         (inputs->strides[TENSOR_DIM_NUM(inputs) - 1] % 64)))
    {
        aligned64 = vx_false_e;
    }

    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP) &&
        vxnneIsTPSupportFormat(context, inputs, weights_biases, outputs) &&
        weights_biases->use_tp_fc &&
        WB_IS_NN_FC_BATCH_MODE(weights_biases) == vx_false_e &&
        aligned64)
    {
        vx_op_param conv = VX_NULL;
        vx_uint32 kzgroup = weights_biases->weights_sizes[2] % weights_biases->slice_array[0].kz_count == 0 ?
 weights_biases->weights_sizes[2] / weights_biases->slice_array[0].kz_count :
 weights_biases->weights_sizes[2] / weights_biases->slice_array[0].kz_count + 1;
        vx_uint32 zoffset = 0;

       vxmONERROR(vxnneOperation_Initialize(&tp_operation0->base,
                                            layer,
                                            VXNNE_OPERATION_TARGET_TP,
                                            VXNNE_OPERATOR_FULLYCONNECTED,
                                            VX_NULL,
                                            vxnneOperation_TP_Deinitialize,
                                            batchCount,
                                            0));

        node->layer = layer;

        conv = &tp_operation0->base.parameter;
        conv->pad_x_left = pad;
        conv->pad_y_top = pad;
        conv->pad_x_right = pad;
        conv->pad_y_bottom = pad;
        conv->pool_size_x = 0;
        conv->pool_size_y = 0;
        conv->pool_stride = 1;
        conv->enable_relu = enable_relu;
        conv->conv_rounding_type = 0;
        conv->pad_mode = VX_PAD_CONSTANT;
        conv->pad_const = 0;

        conv->tp_value = (vx_tp_value_cmd_s*)vxAllocateAndZeroMemory(weights_biases->slice_num * sizeof(vx_tp_value_cmd_s));

        if (kzgroup == 1)
        {
            conv->tpType = TP_SINGLE_FC;
            conv->other_ref = (vx_reference)weights_biases;
            conv->data_buff = gcvNULL;

            for (i = 0; i < weights_biases->slice_num; i++)
            {
                conv->tp_value[i].u32[0] = kzgroup;
                conv->tp_value[i].u32[1] = zoffset;
                zoffset += weights_biases->slice_array[i].z_count;
            }

            tp_operation0->input          = inputs;
            tp_operation0->weights_biases = weights_biases;
            tp_operation0->output         = outputs;

            vxnneOperation_AddReference(&tp_operation0->base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&tp_operation0->base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
            vxnneLayer_SetOperation(
                layer,
                &tp_operation0->base,
                (*count) ++);
        }
        else
        {
            vx_tensor tmpTensor;
            vx_uint32 size = kzgroup * weights_biases->weights_sizes[3] * TENSOR_DATA_SIZE(outputs);
            vx_uint32 kzoffset = 0, kzoffset2 = 0;

            vx_tensor_create_params_t tensor_create_params;

            gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
            tensor_create_params.num_of_dims = 1;
            tensor_create_params.sizes = &size;
            tensor_create_params.data_format = TENSOR_DATA_TYPE(outputs);
            tensor_create_params.quant_format = TENSOR_QUANT_TYPE(outputs);
            if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
            {
                tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(outputs);
            }
            else
            {
                tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(outputs);
                tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(outputs);
            }

            vxmONERROR(vxnneOperation_Initialize(&tp_operation1->base,
                                                 layer,
                                                 VXNNE_OPERATION_TARGET_TP,
                                                 VXNNE_OPERATOR_FULLYCONNECTED,
                                                 VX_NULL,
                                                 vxnneOperation_TP_Deinitialize,
                                                 batchCount,
                                                 0));

            tmpTensor = vxoTensor_CreateTensor(context, node->graph, &tensor_create_params, vx_true_e);
            if (tmpTensor == VX_NULL)
            {
                vxError("vxoTensor_CreateTensor fail at function %s, line %d", __FUNCTION__, __LINE__);
                vxmONERROR(VX_ERROR_NO_MEMORY);
            }

            conv->tpType = TP_SINGLE_FC;
            conv->other_ref = (vx_reference)weights_biases;
            conv->data_buff = gcvNULL;

            for (i = 0; i < weights_biases->slice_num; i++)
            {
                conv->tp_value[i].e32[0] = 0;
                conv->tp_value[i].u32[0] = kzgroup;
                conv->tp_value[i].u32[1] = zoffset;
                conv->tp_value[i].u32[2] = kzoffset;
                conv->tp_value[i].u32[3] = kzoffset2;

                if (i % kzgroup == kzgroup - 1)
                {
                    kzoffset = kzoffset2 = 0;
                    zoffset += weights_biases->slice_array[i].z_count;
                }
                else
                {
                    kzoffset += weights_biases->slice_array[i].kz_count;
                    kzoffset2 += weights_biases->weights_sizes[3];
                }
            }

            vxnneLayer_SetOperation(
                layer,
                &tp_operation0->base,
                (*count) ++);
            tp_operation0->weights_biases = weights_biases;
            tp_operation0->input          = inputs;
            tp_operation0->output         = tmpTensor;

            vxnneOperation_AddReference(&tp_operation0->base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&tp_operation0->base, (vx_reference)tmpTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT);

            conv = &tp_operation1->base.parameter;

            conv->pad_x_left = pad;
            conv->pad_y_top = pad;
            conv->pad_x_right = pad;
            conv->pad_y_bottom = pad;
            conv->pool_size_x = 0;
            conv->pool_size_y = 0;
            conv->pool_stride = 1;
            conv->enable_relu = enable_relu;
            conv->conv_rounding_type = 0;
            conv->pad_mode = VX_PAD_CONSTANT;
            conv->pad_const = 0;

            conv->tpType = TP_SINGLE_FC;
            conv->other_ref = gcvNULL;
            conv->data_buff = gcvNULL;
            conv->tp_value = (vx_tp_value_cmd_s*)vxAllocateAndZeroMemory(sizeof(vx_tp_value_cmd_s));
            conv->tp_value->e32[0] = 1;
            conv->tp_value->u32[0] = kzgroup;
            conv->tp_value->u32[1] = weights_biases->weights_sizes[3];

            vxnneLayer_SetOperation(
                layer,
                &tp_operation1->base,
                (*count) ++);
            tp_operation1->input  = tmpTensor;
            tp_operation1->output = outputs;

            vxnneOperation_AddReference(&tp_operation1->base, (vx_reference)tmpTensor, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&tp_operation1->base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);

            layer->num_temp_tensors = 1;
            layer->temp_tensors[0] = tmpTensor;
        }
    }
    else if (vxnneIsNNSupportFormat(context, inputs, weights_biases, outputs) &&
        (nn_operation != VX_NULL))
    {
        vx_op_param_s conv = {0};
        vx_tensor reshapeInput = VX_NULL, reshapeOutput = VX_NULL;
        vx_uint32 reshapeDimCount = 0;

        vxmONERROR(vxnneOperation_Initialize(&nn_operation->base,
                                             layer,
                                             VXNNE_OPERATION_TARGET_NN,
                                             VXNNE_OPERATOR_FULLYCONNECTED,
                                             VX_NULL,
                                             VX_NULL,
                                             batchCount,
                                             NNE_COMMAND_SIZE * weights_biases->slice_num));

        conv.pad_x_left = conv.pad_x_right = conv.pad_y_top = conv.pad_y_bottom = pad;
        conv.pad_mode = VX_PAD_CONSTANT;
        conv.pad_const = 0;
        conv.pool_type = VIV_NN_POOLING_NON;
        conv.pool_size_x = conv.pool_size_y = 0;
        conv.conv_rounding_type = conv_rounding_type;
        conv.enable_relu = enable_relu;

        vxnneLayer_SetOperation(
            layer,
            &nn_operation->base,
            (*count) ++);

        if ((inputs->dimCount == 1) ||
            (inputs->dimCount == 2) ||
            (((inputs->dimCount == 3) || (inputs->dimCount == 4)) && ((inputs->dims[0] == 1) || (inputs->dims[1] != 1))))
        {
            vx_int32 reshapeSize[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0};
            reshapeSize[0] = 1;
            reshapeSize[1] = 1;

            if ((inputs->dimCount == 1) || (inputs->dimCount == 2))
            {
                for (i = 2; i < inputs->dimCount + 2; i++)
                    reshapeSize[i] = inputs->dims[i-2];

                reshapeDimCount = inputs->dimCount + 2;
            }
            else if (((inputs->dimCount == 3) || (inputs->dimCount == 4)) && ((inputs->dims[0] == 1) || (inputs->dims[1] != 1)))
            {
                reshapeSize[2] = inputs->dims[0] * inputs->dims[1] * inputs->dims[2];
                reshapeDimCount = inputs->dimCount;
            }

            reshapeInput = vxoTensor_ReshapeTensor(inputs, reshapeSize, reshapeDimCount);

            vxmASSERT(reshapeInput != VX_NULL);

            nn_operation->inputs                   = reshapeInput;
            vxnneOperation_AddReference(&nn_operation->base, (vx_reference)reshapeInput, VXNNE_OPERATION_REFENRENCE_INPUT);
        }
        else
        {
            nn_operation->inputs                   = inputs;
            vxnneOperation_AddReference(&nn_operation->base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
        }

        if (outputs->dimCount == 1 || outputs->dimCount == 2)
        {
            vx_int32 reshapeSize[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0};
            reshapeSize[0] = 1;
            reshapeSize[1] = 1;

            for (i = 2; i < outputs->dimCount + 2; i++)
                reshapeSize[i] = outputs->dims[i-2];

            reshapeOutput = vxoTensor_ReshapeTensor(outputs, reshapeSize, outputs->dimCount + 2);

            vxmASSERT(reshapeOutput != VX_NULL);

            nn_operation->outputs                   = reshapeOutput;
            vxnneOperation_AddReference(&nn_operation->base, (vx_reference)reshapeOutput, VXNNE_OPERATION_REFENRENCE_OUTPUT);
        }
        else
        {
            nn_operation->outputs                  = outputs;
            vxnneOperation_AddReference(&nn_operation->base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
        }
        nn_operation->weights_biases           = weights_biases;
        nn_operation->pad_x_left               = pad;
        nn_operation->pad_x_right              = pad;
        nn_operation->pad_y_top                = pad;
        nn_operation->pad_y_bottom             = pad;
        nn_operation->down_scale_size_rounding = conv_rounding_type;
        nn_operation->enable_relu              = enable_relu;

        memcpy(&nn_operation->base.parameter, &conv, sizeof(vx_op_param_s));
    }
    else if (enable_shader && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
    {
        vxmONERROR(vxoFCOperationSH_Initialize(sh_operation,
                                               layer,
                                               inputs,
                                               1,
                                               weights_biases->wb_base->origWeight,
                                               weights_biases->wb_base->origBias,
                                               0,
                                               0,
                                               enable_relu,
                                               outputs,
                                               &op_index));
    }
    else
    {
         /* TODO: SW path. */
    }

    node->layer = layer;

OnError:
    return status;
}


VX_PRIVATE_API vx_status VX_CALLBACK vxoNNFullyConnectedReluLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_context                  context = vxGetContext((vx_reference)node);
    vx_tensor                   inputs = (vx_tensor)parameters[0];
    vx_weights_biases_parameter weights_biases = (vx_weights_biases_parameter)parameters[1]; // need modify
    vx_scalar                   pad_s = (vx_scalar)parameters[2];
    vx_scalar                   down_scale_size_rounding_s = (vx_scalar)parameters[6];
    vx_scalar                   enable_relu_s = (vx_scalar)parameters[7];
    vx_tensor                   outputs = (vx_tensor)parameters[8];

    vx_uint32                   pad;
    vx_enum                     conv_rounding_type;
    vx_bool                     enable_relu;
    vx_int32                    count = 0;

    vx_status                   status = VX_SUCCESS;
    vxnne_fully_connected_relu_layer  fullyConnectReluLayer = gcvNULL;

    if (!vxnneIsNNSupportFormat(context, inputs, weights_biases, outputs) &&
        !vxnneIsTPSupportFormat(context, inputs, weights_biases, outputs))
    {
        vxError("hw not support this format. function %s line %d", __FUNCTION__, __LINE__);
        status = VX_ERROR_NOT_SUPPORTED;
        goto exit;
    }


    if (TENSOR_DIM_NUM(inputs) == 2)
    {
        if ((inputs->dims[0] != weights_biases->weights_sizes[2]) ||
            (outputs->dims[0] != weights_biases->weights_sizes[3]))
        {
            vxError("parameter is invalid at function %s, line %d\n", __FUNCTION__, __LINE__);
            status = VX_ERROR_INVALID_PARAMETERS;
            goto exit;
        }
    }
    else if (TENSOR_DIM_NUM(inputs) == 4)
    {
        if (((inputs->dims[0] * inputs->dims[1] * inputs->dims[2]) != weights_biases->weights_sizes[2]) ||
            ((outputs->dimCount == 4) && (outputs->dims[2] !=weights_biases->weights_sizes[3])) ||
            ((outputs->dimCount == 2) && (outputs->dims[0] !=weights_biases->weights_sizes[3])))
        {
            vxError("parameter is invalid at function %s, line %d\n", __FUNCTION__, __LINE__);
            status = VX_ERROR_INVALID_PARAMETERS;
            goto exit;
        }
    }
    pad                  = pad_s->value->u32;
    conv_rounding_type   = down_scale_size_rounding_s->value->e;
    enable_relu          = enable_relu_s->value->b;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_fully_connected_relu_layer_s), (gctPOINTER*)&fullyConnectReluLayer);
    if (!fullyConnectReluLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        goto exit;
    }

    gcoOS_ZeroMemory(fullyConnectReluLayer, sizeof(vxnne_fully_connected_relu_layer_s));

    vxnneLayer_Initialize(&fullyConnectReluLayer->base,
                            "FullyConnectedReluLayer",
                            node,
                            vxmOPERATION_COUNT(fullyConnectReluLayer),
                            fullyConnectReluLayer->operations,
                            VX_NULL);

    status = vxoNNFullyConnectedLayerInitializer(
        node,
        &fullyConnectReluLayer->base,
        &fullyConnectReluLayer->fully_connected_TPoperation[0],
        &fullyConnectReluLayer->fully_connected_TPoperation[1],
        &fullyConnectReluLayer->fully_connected_relu_operation,
        &fullyConnectReluLayer->fully_connected_SHoperation,
        inputs,
        weights_biases,
        pad,
        conv_rounding_type,
        enable_relu,
        &count,
        outputs);

    if (status != VX_SUCCESS && fullyConnectReluLayer) gcoOS_Free(gcvNULL, (gctPOINTER)fullyConnectReluLayer);

exit:
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNFullyConnectedReluLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNFullyConnectedLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNFullyConnectedLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNFullyConnectedLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNFullyConnectedLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status  status                     = VX_SUCCESS;

    vx_tensor  inputs                     = (vx_tensor)parameters[0];
    vx_tensor  weights                    = (vx_tensor)parameters[1];
    vx_tensor  biases                     = (vx_tensor)parameters[2];
    vx_tensor  outputs                    = (vx_tensor)parameters[node->numParameters - 1];

    vx_bool   enable_shader               = vx_false_e;
    vx_bool   supportDataFormat0          = vx_false_e;
    vx_bool   supportDataFormat1          = vx_false_e;
    vx_bool   supportDataFormat2          = vx_false_e;
    vx_bool   supportDataFormat3          = vx_false_e;
    vx_enum   input_type                  = TENSOR_DATA_TYPE(inputs);
    vx_enum   weight_type                 = TENSOR_DATA_TYPE(weights);
    vx_enum   bias_type                   = biases ? TENSOR_DATA_TYPE(biases) : VX_TYPE_INVALID;
    vx_enum   output_type                 = TENSOR_DATA_TYPE(outputs);
    vx_uint32 dims                        = TENSOR_VIEW_DIM_NUM(inputs);
    vx_uint32 width                       = TENSOR_VIEW_SIZE_INDEX(inputs, 0);
    vx_uint32 height                      = (dims > 1) ? TENSOR_VIEW_SIZE_INDEX(inputs, 1) : 1;
    vx_uint32 depth                       = (dims > 2) ? TENSOR_VIEW_SIZE_INDEX(inputs, 2) : 1;
    vx_uint32 inputDims                   = dims > 2 ? width * height * depth : width;
    vx_uint32 batch                       = 1;
    vxnne_fully_connected_relu_layer  fullyConnectedLayer = gcvNULL;
    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    if (TENSOR_DIM_NUM(inputs) == 2)
    {
        batch = TENSOR_SIZE_INDEX(inputs, 1);
    }
    else if (TENSOR_DIM_NUM(inputs) == 4)
    {
        batch = TENSOR_SIZE_INDEX(inputs, 3);
    }

    if(node->base.context->evisNoInst.supportEVIS)
    {
        supportDataFormat0 = (vx_bool)(input_type == VX_TYPE_FLOAT16 && weight_type == VX_TYPE_FLOAT16 && (bias_type == VX_TYPE_INVALID || bias_type == VX_TYPE_FLOAT32) && output_type == VX_TYPE_FLOAT16);
        supportDataFormat1 = (vx_bool)(input_type == VX_TYPE_INT8 && weight_type == VX_TYPE_INT8 && (bias_type == VX_TYPE_INVALID || bias_type == VX_TYPE_INT32) && output_type == VX_TYPE_INT8);
        supportDataFormat2 = (vx_bool)(input_type == VX_TYPE_INT16 && weight_type == VX_TYPE_INT16
                             && (bias_type == VX_TYPE_INVALID || bias_type == VX_TYPE_INT32 || bias_type == VX_TYPE_INT64 || bias_type == VX_TYPE_INT16) && output_type == VX_TYPE_INT16);
        supportDataFormat3 = (vx_bool)(input_type == VX_TYPE_UINT8 && weight_type == VX_TYPE_UINT8
                             && (bias_type == VX_TYPE_INVALID || bias_type == VX_TYPE_INT32 || bias_type == VX_TYPE_UINT8) && output_type != VX_TYPE_FLOAT32);
        enable_shader      = (supportDataFormat0 || supportDataFormat1 || supportDataFormat2 || supportDataFormat3) && (inputDims < IMG_MAX_WIDTH);
    }
    else
    {
        supportDataFormat0 = (vx_bool)((input_type == VX_TYPE_FLOAT16 && weight_type == VX_TYPE_FLOAT16 && (bias_type == VX_TYPE_INVALID || bias_type == VX_TYPE_FLOAT32) && output_type == VX_TYPE_FLOAT16) ||
                                        (input_type == VX_TYPE_FLOAT32 && weight_type == VX_TYPE_FLOAT32 && (bias_type == VX_TYPE_INVALID || bias_type == VX_TYPE_FLOAT32) && output_type == VX_TYPE_FLOAT32));
        supportDataFormat3 = (vx_bool)(input_type == VX_TYPE_UINT8 && weight_type == VX_TYPE_UINT8 && (bias_type == VX_TYPE_INVALID || bias_type == VX_TYPE_INT32) && output_type == VX_TYPE_UINT8);
        supportDataFormat2 = (vx_bool)(input_type == VX_TYPE_UINT8 && weight_type == VX_TYPE_UINT8 && (bias_type == VX_TYPE_INVALID || bias_type == VX_TYPE_INT32) &&
                                      ((output_type == VX_TYPE_INT32) || (output_type == VX_TYPE_FLOAT32) || (output_type == VX_TYPE_FLOAT16) || (output_type == VX_TYPE_INT16)));
        enable_shader      = (supportDataFormat0 || supportDataFormat3 || supportDataFormat2);
    }

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_fully_connected_relu_layer_s), (gctPOINTER*)&fullyConnectedLayer);
    if (!fullyConnectedLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        goto exit;
    }

    gcoOS_ZeroMemory(fullyConnectedLayer, sizeof(vxnne_fully_connected_relu_layer_s));

    vxnneLayer_Initialize(&fullyConnectedLayer->base,
                          "FullyConnectedLayer",
                          node,
                          vxmOPERATION_COUNT(fullyConnectedLayer),
                          fullyConnectedLayer->operations,
                          vxnneLayer_Deinitialize);

    if (enable_shader && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
    {
        vxnne_shader_executable shaderExecutable = VX_NULL;
        vx_bool enable_cast_format = vx_false_e;
        vx_tensor input_rs = NULL;
        vx_tensor weights_rs = NULL;

        if ((inputDims % 16 == 0) && input_type == VX_TYPE_UINT8 && weight_type == VX_TYPE_UINT8 && biases
            && node->base.context->evisNoInst.supportEVIS == vx_false_e && (output_type  == VX_TYPE_UINT8))
        {
            enable_cast_format = vx_true_e;

            input_rs = vxoTensor_ReformatTensor(inputs, VX_TYPE_UINT32);
            weights_rs = vxoTensor_ReformatTensor(weights, VX_TYPE_UINT32);

            fullyConnectedLayer->base.temp_tensors[0] = input_rs;
            fullyConnectedLayer->base.temp_tensors[1] = weights_rs;

            fullyConnectedLayer->base.num_temp_tensors = 2;
        }
        else
        {
            input_rs = inputs;
            weights_rs = weights;
        }

        if(node->base.context->evisNoInst.supportEVIS)
        {
            shaderExecutable = vxnneGetFullyConnectedShaderExecutable(node->base.context, VXNNE_KERNEL_FULLYCONNECTED,
                &node->kernelAttributes.borderMode, input_rs, weights, biases, VX_NN_ACTIVATION_NONE, outputs);
        }
        else
        {
            shaderExecutable = vxnneGetGPUFullyConnectedShaderExecutable(node->base.context, VXNNE_KERNEL_FULLYCONNECTED,
                &node->kernelAttributes.borderMode, enable_cast_format, input_rs, weights_rs, biases, VX_NN_ACTIVATION_NONE, outputs);
        }

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto exit;
        }

        status = vxnneShaderOperation_Initialize(&fullyConnectedLayer->fully_connected_SHoperation,
            &fullyConnectedLayer->base,
            VXNNE_OPERATOR_FULLYCONNECTED,
            1,
            shaderExecutable);

        if (status != VX_SUCCESS) goto exit;

        vxnneLayer_SetOperation(
            &fullyConnectedLayer->base,
            &fullyConnectedLayer->fully_connected_SHoperation.base,
            0);

        vxnneOperation_AddReference(&fullyConnectedLayer->fully_connected_SHoperation.base, (vx_reference)input_rs, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&fullyConnectedLayer->fully_connected_SHoperation.base, (vx_reference)weights_rs, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&fullyConnectedLayer->fully_connected_SHoperation.base, (vx_reference)biases, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&fullyConnectedLayer->fully_connected_SHoperation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
    }
    else
    {
        status = vxnneOperation_Initialize(&fullyConnectedLayer->fully_connected_operation.base,
                                           &fullyConnectedLayer->base,
                                           VXNNE_OPERATION_TARGET_SW,
                                           VXNNE_OPERATOR_FULLYCONNECTED,
                                           vxnneExecuteSWFullyConnected,
                                           VX_NULL,
                                           1,
                                           0);

        vxnneLayer_SetOperation(
            &fullyConnectedLayer->base,
            &fullyConnectedLayer->fully_connected_operation.base,
            0);

        fullyConnectedLayer->fully_connected_operation.inputs           = inputs;
        fullyConnectedLayer->fully_connected_operation.weights          = weights;
        fullyConnectedLayer->fully_connected_operation.biases           = biases;
        fullyConnectedLayer->fully_connected_operation.outputs          = outputs;

        vxnneOperation_AddReference(&fullyConnectedLayer->fully_connected_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&fullyConnectedLayer->fully_connected_operation.base, (vx_reference)weights, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&fullyConnectedLayer->fully_connected_operation.base, (vx_reference)biases, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&fullyConnectedLayer->fully_connected_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
    }

    node->layer = &fullyConnectedLayer->base;
    return status;

exit:
    if (fullyConnectedLayer) gcoOS_Free(gcvNULL, (gctPOINTER)fullyConnectedLayer);
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNFullyConnectedLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return VX_SUCCESS;
}

