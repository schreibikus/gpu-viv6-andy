##############################################################################
#
#    Copyright (C) 2005 - 2010 by Vivante Corp.
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public Lisence as published by
#    the Free Software Foundation; either version 2 of the license, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#    GNU General Public Lisence for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not write to the Free Software
#    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
##############################################################################

##############################################################################
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
##############################################################################

#
# Common inlude file for QNX build.
#

PLATFORM ?= dummy
qnx_build_dir ?= $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
test_root ?= $(qnx_build_dir)/../../../gpu_test_src_dir
driver_root ?= $(qnx_build_dir)/../../../gpu_driver_src_dir
VIVANTE_SDK_DIR ?= $(driver_root)/build_qnx/platform_binaries
VIVANTE_SDK_LIB ?= $(VIVANTE_SDK_DIR)/drivers
EXTRA_LIBVPATH += $(VIVANTE_SDK_LIB)

HAL2D_UNIT_TEST_DIR ?= $(test_root)/test/hal/common/UnitTest
VDK_TEST_DIR ?= $(test_root)/sdk/samples/vdk
ES11_TEST_DIR ?= $(test_root)/test/es11
ES20_TEST_DIR ?= $(test_root)/test/es20
VG11_TEST_DIR ?= $(test_root)/test/vg11
CL11_TEST_DIR ?= $(test_root)/test/cl11
OVX_TEST_DIR ?= $(test_root)/test/ovx

# This prevents the platform/board name from getting appended to every build target name.
# This happens automatically as the build directory structure now includes the board above the
# common.mk for each component.
EXTRA_SILENT_VARIANTS+=$(PLATFORM)

INSTALLDIR=$(firstword $(INSTALLDIR_$(OS)) usr/lib/graphics/$(PLATFORM))
# Copy all build targets to a local folder for incremental building
LOCAL_INSTALL=$(qnx_build_dir)/platform_binaries/$(CPU)$(filter le be, $(VARIANT_LIST))$(CPUVARDIR_SUFFIX)/$(PLATFORM)

# Remove *S.a files that get built along with any so build.
# POST_INSTALL=$(RM_HOST) $(INSTALL_ROOT_$(OS))/$(CPU)$(filter le be, $(VARIANT_LIST))$(CPUVARDIR_SUFFIX)/$(INSTALLDIR)/*$(NAME)*.a

# Perform local install
POST_BUILD=$(CP_HOST) $@ $(LOCAL_INSTALL)/$(@F)

# Perform clean of local install directory
POST_CLEAN=$(RM_HOST) $(LOCAL_INSTALL)/*$(NAME)*

include $(qnx_build_dir)/platform_config/$(PLATFORM)/platform_config

###############################################################
# Common CCFLAGS.
ifneq ($(ABI), 0)
	CCFLAGS += -mabi=$(ABI)
endif

# 64 bit
ifneq ($(CPU),aarch64)
AARCH64 = 1
CCFLAGS += -DAARCH64
endif

#CCFLAGS += -Werror
#CCFLAGS += -ansi
#CCFLAGS += -pedantic
#CCFLAGS += -fmudflap
#LDFLAGS += -lmudflap
CCFLAGS += -Wno-error=unused-but-set-variable -Wno-strict-aliasing

ifeq ($(EGL_API_FB), 1)
	CCFLAGS += -DEGL_API_FB
endif

ifeq ($(STATIC_LINK), 1)
	CCFLAGS += -DSTATIC_LINK
endif

ifeq ($(USE_VDK), 1)
	CCFLAGS += -DUSE_VDK=1 -DUSE_SW_FB=$(USE_SW_FB)
else
	CCFLAGS += -DUSE_VDK=0
endif

ifeq ($(USE_FB_DOUBLE_BUFFER), 1)
	CCFLAGS += -DUSE_FB_DOUBLE_BUFFER=1
else
	CCFLAGS += -DUSE_FB_DOUBLE_BUFFER=0
endif

ifeq ($(USE_NEW_LINUX_SIGNAL), 1)
	CCFLAGS += -DUSE_NEW_LINUX_SIGNAL=1
else
	CCFLAGS += -DUSE_NEW_LINUX_SIGNAL=0
endif

ifeq ($(VIVANTE_ENABLE_3D), 1)
	CCFLAGS += -DgcdENABLE_3D=1
else
	CCFLAGS += -DgcdENABLE_3D=0
endif

ifeq ($(VIVANTE_ENABLE_2D), 1)
	CCFLAGS += -DgcdENABLE_2D=1
else
	CCFLAGS += -DgcdENABLE_2D=0
endif

ifeq ($(VIVANTE_ENABLE_VG), 1)
	CCFLAGS += -DgcdENABLE_VG=1
else
	CCFLAGS += -DgcdENABLE_VG=0
endif

CCFLAGS += -DgcdDISPLAY_BACK_BUFFERS=2

CCFLAGS += -DgcdDYNAMIC_MAP_RESERVED_MEMORY=0

# GN TODO Check to see if this is required
#CXXFLAGS += -fno-short-enums

################################################################################
# Build with profiler
ifeq ($(USE_PROFILER),1)
	CCFLAGS += -DVIVANTE_PROFILER=1
else
	CCFLAGS += -DVIVANTE_PROFILER=0
endif

################################################################################
# Build with HAL tracer
ifeq ($(USE_HAL_TRACER),1)
	CCFLAGS += -DgcdDUMP=1
else
	CCFLAGS += -DgcdDUMP=0
endif

################################################################################
# Build with HAL tracer
ifeq ($(USE_CMDBUF_TRACER),1)
	CCFLAGS += -DgcdDUMP_COMMAND=1
else
	CCFLAGS += -DgcdDUMP_COMMAND=0
endif

################################################################################
# Build with GL tracer
ifeq ($(USE_GL_TRACER),1)
	CCFLAGS += -DgcdDEBUG=1 -DgcdDUMP_API=1
else
	CCFLAGS += -DgcdDUMP_API=0
endif

################################################################################
# Build with VG tracer
ifeq ($(USE_VG_TRACER),1)
	CCFLAGS += -DOVG_DBGOUT=1
endif

################################################################################
# Build for debugging
ifeq ($(ENABLE_DEBUG),1)
	CCFLAGS += -g -O0 -DgcdDEBUG=1
#else
#	CCFLAGS += -O2
#	CCFLAGS += -Wno-strict-aliasing
endif

################################################################################
# Build for profiling
ifeq ($(ENABLE_PROFILING),1)
	CCFLAGS += -p
	CXXFLAGS += -p
	LDFLAGS += -p
	CCFLAGS += -g -finstrument-functions
	LIBS += profilingS
	LDFLAGS += -Wl,-E
endif

ENABLE_BUILD_TIME_SYMBOL_CHECK ?= 0
ifeq ($(ENABLE_BUILD_TIME_SYMBOL_CHECK),1)
   SCREEN_LIBS = screen
   LDFLAGS += -Wl,--unresolved-symbols=report-all
else
   SCREEN_LIBS =
endif


