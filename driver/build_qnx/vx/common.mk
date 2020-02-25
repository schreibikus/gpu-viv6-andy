#
# Copyright 2010, QNX Software Systems Ltd.  All Rights Reserved
#
# This source code has been published by QNX Software Systems Ltd.
# (QSSL).  However, any use, reproduction, modification, distribution
# or transfer of this software, or any software which includes or is
# based upon any of this code, is only permitted under the terms of
# the QNX Open Community License version 1.0 (see licensing.qnx.com for
# details) or as otherwise expressly authorized by a written license
# agreement from QSSL.  For more information, please email licensing@qnx.com.
#

# find the driver's root directory, which is 3 levels below the current make file
driver_root:=$(abspath ../../$(dir $(lastword $(MAKEFILE_LIST))))
qnx_build_dir:=$(abspath $(dir $(lastword $(MAKEFILE_LIST)))/..)

ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION="OpenVX"
endef

NAME=libOpenVX

include $(qnx_build_dir)/common.mk

EXTRA_INCVPATH += $(driver_root)/hal/inc
EXTRA_INCVPATH += $(driver_root)/hal/user
EXTRA_INCVPATH += $(driver_root)/sdk/inc
EXTRA_INCVPATH += $(driver_root)/arch/XAQ2/cmodel/inc
EXTRA_INCVPATH += $(driver_root)/compiler/libVSC/include
EXTRA_INCVPATH += $(driver_root)/hal/os/qnx/user
ifeq ($(ORI_NNARCHPERF),1)
EXTRA_INCVPATH += $(driver_root)/driver/khronos/libOpenVX/libarchmodel/include
else
EXTRA_INCVPATH += $(driver_root)/driver/khronos/libOpenVX/libarchmodelInterface/include
endif
EXTRA_INCVPATH += $(driver_root)/driver/khronos/libOpenVX/driver/include
EXTRA_INCVPATH += $(driver_root)/driver/khronos/libOpenVX/kernels

ifeq ($(USE_VXC_BINARY),1)
EXTRA_INCVPATH += $(driver_root)/driver/khronos/libOpenVX/libkernel/libnngpu
EXTRA_INCVPATH += $(driver_root)/driver/khronos/libOpenVX/libkernel/libnnvxc
EXTRA_INCVPATH += $(driver_root)/driver/khronos/libOpenVX/libkernel/libovx12
endif
ifneq ($(ORI_NNARCHPERF),1)
EXTRA_INCVPATH += $(driver_root)/driver/khronos/libOpenVX/vipArchPerfMdl_dev/vipArchPerf
EXTRA_INCVPATH += $(driver_root)/driver/khronos/libOpenVX/vipArchPerfMdl_dev/libarchmodelSw/include
endif
# from libCL (trunk/driver/khronos/libCL/makefile.linux)
# Core
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_target.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_json.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_context.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_array.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_convolution.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_delay.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_distribution.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_error.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_graph.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_graph_optimization.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_image.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_kernel.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_log.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_lut.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_matrix.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_memory.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_meta_format.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_node.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_node_api.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_object_array.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_runtime.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_parameter.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_pyramid.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_reference.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_remap.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_scalar.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_threshold.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_program.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_profiler.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_tensor.o

# Operations
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/ops/gc_vx_op_fc.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/ops/gc_vx_op_debug.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/ops/gc_vx_op_tensor_copy.o

# Layers
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/layers/gc_vx_layer_lstm.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/layers/gc_vx_layer_activation.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/layers/gc_vx_layer_conv.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/layers/gc_vx_layer_fc.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/layers/gc_vx_layer_adapter.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/layers/gc_vx_layer_batch_norm.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/layers/gc_vx_layer_concat.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/layers/gc_vx_layer_deconv.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/layers/gc_vx_layer_depthwise_conv.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/layers/gc_vx_layer_div.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/layers/gc_vx_layer_eltwise.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/layers/gc_vx_layer_rnn.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/layers/gc_vx_layer_hash_lut.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/layers/gc_vx_layer_l2norm.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/layers/gc_vx_layer_lrn.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/layers/gc_vx_layer_lsh_project.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/layers/gc_vx_layer_lut.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/layers/gc_vx_layer_pool.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/layers/gc_vx_layer_reduce_sum.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/layers/gc_vx_layer_reorg.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/layers/gc_vx_layer_roi_pool.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/layers/gc_vx_layer_rpn.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/layers/gc_vx_layer_tensor_mean.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/layers/gc_vx_layer_tensor_reshape.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/layers/gc_vx_layer_tensor_round.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/layers/gc_vx_layer_tensor_scale.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/layers/gc_vx_layer_tensor_squeeze.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/layers/gc_vx_layer_tensor_stride_slice.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/layers/gc_vx_layer_tensor_transpose.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/layers/gc_vx_layer_tensor_pad.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/layers/gc_vx_layer_softmax.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/layers/gc_vx_layer_tensor_copy.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/layers/gc_vx_layer_tensor_reverse.o

# API
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_interface.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_nn_extension_interface.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_internal_node_api.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_layer.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_gpu_layer.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_nn_util.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_nn_encoder.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_nn_wb.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_nn_command.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/driver/src/gc_vx_binary.o

#SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/extension/gc_vxc_interface.o
# Kernels
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/kernels/gc_vxk_absdiff.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/kernels/gc_vxk_accumulate.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/kernels/gc_vxk_addsub.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/kernels/gc_vxk_bitwise.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/kernels/gc_vxk_channel.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/kernels/gc_vxk_convertcolor.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/kernels/gc_vxk_convertdepth.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/kernels/gc_vxk_convolve.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/kernels/gc_vxk_fast9.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/kernels/gc_vxk_filter.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/kernels/gc_vxk_harris.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/kernels/gc_vxk_histogram.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/kernels/gc_vxk_integralimage.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/kernels/gc_vxk_lut.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/kernels/gc_vxk_magnitude.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/kernels/gc_vxk_common.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/kernels/gc_vxk_morphology.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/kernels/gc_vxk_multiply.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/kernels/gc_vxk_nn_layers.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/kernels/gc_vxk_norm.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/kernels/gc_vxk_nonmax.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/kernels/gc_vxk_optpyrlk.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/kernels/gc_vxk_phase.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/kernels/gc_vxk_remap.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/kernels/gc_vxk_scale.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/kernels/gc_vxk_sobel3x3.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/kernels/gc_vxk_statistics.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/kernels/gc_vxk_threshold.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/kernels/gc_vxk_warp.o
#SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/utility/vx_utility.o

EXTRA_SRCVPATH += $(driver_root)/driver/khronos/libOpenVX/driver/src
EXTRA_SRCVPATH += $(driver_root)/driver/khronos/libOpenVX/driver/src/ops
EXTRA_SRCVPATH += $(driver_root)/driver/khronos/libOpenVX/driver/src/layers
EXTRA_SRCVPATH += $(driver_root)/driver/khronos/libOpenVX/kernels
#EXTRA_SRCVPATH += $(driver_root)/driver/khronos/libOpenVX/utility

EXTRA_LIBVPATH += $(LOCAL_INSTALL)

OBJECTS_FROM_SRCVPATH := $(basename $(wildcard $(foreach dir, $(EXTRA_SRCVPATH), $(addprefix $(dir)/*., s S c cc cpp))))
MISSING_OBJECTS := $(filter-out $(OBJECTS_FROM_SRCVPATH), $(basename $(SOURCE_OBJECTS)))
ifneq ($(MISSING_OBJECTS), )
$(error ***** Missing source objects:  $(MISSING_OBJECTS))
endif
CCFLAGS += -g -O0 -Wno-error=switch -Wno-error=missing-braces -Wno-unused-function -Wno-unused-variable -Wno-sign-compare -Wno-narrowing
CCFLAGS += -Wno-unused-local-typedefs

CCFLAGS += -D_LITTLE_ENDIAN_ \
           -DOPENVX_USE_LIST \
           -DOPENVX_USE_NODE_MEMORY \
           -DOPENVX_USE_TILING \
           -DOPENVX_USE_DOT \
           -DOPENVX_USE_TARGET \
           -D__linux__ \
           -fPIC

CCFLAGS += -D__GL_EXPORTS  -DHAVE_PTHREAD

EXCLUDE_OBJS += $(addsuffix .o, $(notdir $(filter-out $(basename $(SOURCE_OBJECTS)), $(OBJECTS_FROM_SRCVPATH))))
#$(warning ***** Excluded objects: $(EXCLUDE_OBJS))

include $(MKFILES_ROOT)/qmacros.mk

#For original nnarchperf
ifeq ($(ORI_NNARCHPERF),1)
STATIC_LIBS += archmodelS
else
STATIC_LIBS += archmodelInterface
endif
$(foreach lib, $(STATIC_LIBS), $(eval LIBPREF_$(lib) = -Bstatic))
$(foreach lib, $(STATIC_LIBS), $(eval LIBPOST_$(lib) = -Bdynamic))

LIBS += $(STATIC_LIBS)
ifeq ($(ORI_NNARCHPERF),1)
LIBS += CLC VSC GAL
else
LIBS += CLC VSC GAL ArchModelSw NNArchPerf
endif
#CCFLAGS += -DCL_USE_DEPRECATED_OPENCL_1_0_APIS
#CCFLAGS += -DCL_USE_DEPRECATED_OPENCL_1_1_APIS
#CCFLAGS += -DBUILD_OPENCL_12=1

include $(qnx_build_dir)/math.mk

ifeq ($(filter so dll, $(VARIANT_LIST)),)
INSTALLDIR=/dev/null
endif

include $(MKFILES_ROOT)/qtargets.mk

