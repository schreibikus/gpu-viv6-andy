#!/bin/bash

export BUILD_OPTION_EGL_API_FB=1
export BUILD_OPTION_EGL_API_DFB=0
export BUILD_OPTION_EGL_API_DRI=0
export BUILD_OPTION_EGL_API_WL=1
export BUILD_OPTION_EGL_API_GBM=1
export BUILD_OPTION_EGL_API_NULLWS=0
./driver_build_sample.sh $*
unset BUILD_OPTION_EGL_API_NULLWS
unset BUILD_OPTION_EGL_API_BGM
unset BUILD_OPTION_EGL_API_WL
unset BUILD_OPTION_EGL_API_DRI
unset BUILD_OPTION_EGL_API_DFB
unset BUILD_OPTION_EGL_API_FB
