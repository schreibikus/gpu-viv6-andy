#!/bin/bash

export BUILD_OPTION_EGL_API_FB=0
export BUILD_OPTION_EGL_API_DFB=1
export BUILD_OPTION_EGL_API_DRI=0
./test_build_sample.sh $*
unset BUILD_OPTION_EGL_API_DRI
unset BUILD_OPTION_EGL_API_DFB
unset BUILD_OPTION_EGL_API_FB
