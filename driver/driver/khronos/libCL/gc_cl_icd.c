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


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#ifndef CL_USE_DEPRECATED_OPENCL_1_1_APIS
#define CL_USE_DEPRECATED_OPENCL_1_1_APIS
#endif
#ifndef CL_USE_DEPRECATED_OPENCL_1_0_APIS
#define CL_USE_DEPRECATED_OPENCL_1_0_APIS
#endif

#if BUILD_OPENCL_ICD
#ifndef WIN32
/* Need to rename all CL API functions to prevent ICD loader functions calling */
/* themselves via the dispatch table. Include this before cl headers. */
#include "gc_cl_icd_rename_api.h"
#endif
#endif

#include "CL/cl.h"
#include "CL/cl_gl.h"
#include "CL/cl_gl_ext.h"
#include "gc_cl_icd_structs.h"

CLIicdDispatchTable * clgDispatchTable = NULL;
/*
 * Prototypes for deprecated functions no longer present in cl.h
 */
extern CL_API_ENTRY cl_int CL_API_CALL
clSetCommandQueueProperty(cl_command_queue              /* command_queue */,
                          cl_command_queue_properties   /* properties */,
                          cl_bool                       /* enable */,
                          cl_command_queue_properties * /* old_properties */);

#define ICD_DISPATCH_TABLE_ENTRY(fn) \
    assert(clgDispatchTable->entryCount < 256); \
    clgDispatchTable->entries[clgDispatchTable->entryCount++] = (void*)(fn)

cl_int cliIcdDispatchTableCreate(CLIicdDispatchTable **outDispatchTable)
{
    cl_int result = CL_SUCCESS;

    if(clgDispatchTable == NULL)
    {
        /* allocate the public handle */
        clgDispatchTable = (CLIicdDispatchTable *) malloc(sizeof(*clgDispatchTable));
        if (!clgDispatchTable) {
            result = CL_OUT_OF_HOST_MEMORY;
            goto Error;
        }
        memset(clgDispatchTable, 0, sizeof(*clgDispatchTable));

        /* OpenCL 1.0 */
        ICD_DISPATCH_TABLE_ENTRY ( clGetPlatformIDs              );
        ICD_DISPATCH_TABLE_ENTRY ( clGetPlatformInfo             );
        ICD_DISPATCH_TABLE_ENTRY ( clGetDeviceIDs                );
        ICD_DISPATCH_TABLE_ENTRY ( clGetDeviceInfo               );
        ICD_DISPATCH_TABLE_ENTRY ( clCreateContext               );
        ICD_DISPATCH_TABLE_ENTRY ( clCreateContextFromType       );
        ICD_DISPATCH_TABLE_ENTRY ( clRetainContext               );
        ICD_DISPATCH_TABLE_ENTRY ( clReleaseContext              );
        ICD_DISPATCH_TABLE_ENTRY ( clGetContextInfo              );
        ICD_DISPATCH_TABLE_ENTRY ( clCreateCommandQueue          );
        ICD_DISPATCH_TABLE_ENTRY ( clRetainCommandQueue          );
        ICD_DISPATCH_TABLE_ENTRY ( clReleaseCommandQueue         );
        ICD_DISPATCH_TABLE_ENTRY ( clGetCommandQueueInfo         );
        ICD_DISPATCH_TABLE_ENTRY ( clSetCommandQueueProperty     );
        ICD_DISPATCH_TABLE_ENTRY ( clCreateBuffer                );
        ICD_DISPATCH_TABLE_ENTRY ( clCreateImage2D               );
        ICD_DISPATCH_TABLE_ENTRY ( clCreateImage3D               );
        ICD_DISPATCH_TABLE_ENTRY ( clRetainMemObject             );
        ICD_DISPATCH_TABLE_ENTRY ( clReleaseMemObject            );
        ICD_DISPATCH_TABLE_ENTRY ( clGetSupportedImageFormats    );
        ICD_DISPATCH_TABLE_ENTRY ( clGetMemObjectInfo            );
        ICD_DISPATCH_TABLE_ENTRY ( clGetImageInfo                );
        ICD_DISPATCH_TABLE_ENTRY ( clCreateSampler               );
        ICD_DISPATCH_TABLE_ENTRY ( clRetainSampler               );
        ICD_DISPATCH_TABLE_ENTRY ( clReleaseSampler              );
        ICD_DISPATCH_TABLE_ENTRY ( clGetSamplerInfo              );
        ICD_DISPATCH_TABLE_ENTRY ( clCreateProgramWithSource     );
        ICD_DISPATCH_TABLE_ENTRY ( clCreateProgramWithBinary     );
        ICD_DISPATCH_TABLE_ENTRY ( clRetainProgram               );
        ICD_DISPATCH_TABLE_ENTRY ( clReleaseProgram              );
        ICD_DISPATCH_TABLE_ENTRY ( clBuildProgram                );
        ICD_DISPATCH_TABLE_ENTRY ( clUnloadCompiler              );
        ICD_DISPATCH_TABLE_ENTRY ( clGetProgramInfo              );
        ICD_DISPATCH_TABLE_ENTRY ( clGetProgramBuildInfo         );
        ICD_DISPATCH_TABLE_ENTRY ( clCreateKernel                );
        ICD_DISPATCH_TABLE_ENTRY ( clCreateKernelsInProgram      );
        ICD_DISPATCH_TABLE_ENTRY ( clRetainKernel                );
        ICD_DISPATCH_TABLE_ENTRY ( clReleaseKernel               );
        ICD_DISPATCH_TABLE_ENTRY ( clSetKernelArg                );
        ICD_DISPATCH_TABLE_ENTRY ( clGetKernelInfo               );
        ICD_DISPATCH_TABLE_ENTRY ( clGetKernelWorkGroupInfo      );
        ICD_DISPATCH_TABLE_ENTRY ( clWaitForEvents               );
        ICD_DISPATCH_TABLE_ENTRY ( clGetEventInfo                );
        ICD_DISPATCH_TABLE_ENTRY ( clRetainEvent                 );
        ICD_DISPATCH_TABLE_ENTRY ( clReleaseEvent                );
        ICD_DISPATCH_TABLE_ENTRY ( clGetEventProfilingInfo       );
        ICD_DISPATCH_TABLE_ENTRY ( clFlush                       );
        ICD_DISPATCH_TABLE_ENTRY ( clFinish                      );
        ICD_DISPATCH_TABLE_ENTRY ( clEnqueueReadBuffer           );
        ICD_DISPATCH_TABLE_ENTRY ( clEnqueueWriteBuffer          );
        ICD_DISPATCH_TABLE_ENTRY ( clEnqueueCopyBuffer           );
        ICD_DISPATCH_TABLE_ENTRY ( clEnqueueReadImage            );
        ICD_DISPATCH_TABLE_ENTRY ( clEnqueueWriteImage           );
        ICD_DISPATCH_TABLE_ENTRY ( clEnqueueCopyImage            );
        ICD_DISPATCH_TABLE_ENTRY ( clEnqueueCopyImageToBuffer    );
        ICD_DISPATCH_TABLE_ENTRY ( clEnqueueCopyBufferToImage    );
        ICD_DISPATCH_TABLE_ENTRY ( clEnqueueMapBuffer            );
        ICD_DISPATCH_TABLE_ENTRY ( clEnqueueMapImage             );
        ICD_DISPATCH_TABLE_ENTRY ( clEnqueueUnmapMemObject       );
        ICD_DISPATCH_TABLE_ENTRY ( clEnqueueNDRangeKernel        );
        ICD_DISPATCH_TABLE_ENTRY ( clEnqueueTask                 );
        ICD_DISPATCH_TABLE_ENTRY ( clEnqueueNativeKernel         );
        ICD_DISPATCH_TABLE_ENTRY ( clEnqueueMarker               );
        ICD_DISPATCH_TABLE_ENTRY ( clEnqueueWaitForEvents        );
        ICD_DISPATCH_TABLE_ENTRY ( clEnqueueBarrier              );
        ICD_DISPATCH_TABLE_ENTRY ( clGetExtensionFunctionAddress );
#if gcdENABLE_CL_GL
        /* cl_khr_gl_sharing */
        ICD_DISPATCH_TABLE_ENTRY ( clCreateFromGLBuffer          );
        ICD_DISPATCH_TABLE_ENTRY ( clCreateFromGLTexture2D       );
        ICD_DISPATCH_TABLE_ENTRY ( clCreateFromGLTexture3D       );
        ICD_DISPATCH_TABLE_ENTRY ( clCreateFromGLRenderbuffer    );
        ICD_DISPATCH_TABLE_ENTRY ( clGetGLObjectInfo             );
        ICD_DISPATCH_TABLE_ENTRY ( clGetGLTextureInfo            );
        ICD_DISPATCH_TABLE_ENTRY ( clEnqueueAcquireGLObjects     );
        ICD_DISPATCH_TABLE_ENTRY ( clEnqueueReleaseGLObjects     );
        ICD_DISPATCH_TABLE_ENTRY ( clGetGLContextInfoKHR         );
#else
        ICD_DISPATCH_TABLE_ENTRY( NULL );
        ICD_DISPATCH_TABLE_ENTRY( NULL );
        ICD_DISPATCH_TABLE_ENTRY( NULL );
        ICD_DISPATCH_TABLE_ENTRY( NULL );
        ICD_DISPATCH_TABLE_ENTRY( NULL );
        ICD_DISPATCH_TABLE_ENTRY( NULL );
        ICD_DISPATCH_TABLE_ENTRY( NULL );
        ICD_DISPATCH_TABLE_ENTRY( NULL );
        ICD_DISPATCH_TABLE_ENTRY( NULL );
#endif

        /* cl_khr_d3d10_sharing (windows-only) */
        ICD_DISPATCH_TABLE_ENTRY( NULL );
        ICD_DISPATCH_TABLE_ENTRY( NULL );
        ICD_DISPATCH_TABLE_ENTRY( NULL );
        ICD_DISPATCH_TABLE_ENTRY( NULL );
        ICD_DISPATCH_TABLE_ENTRY( NULL );
        ICD_DISPATCH_TABLE_ENTRY( NULL );

        /* OpenCL 1.1 */
        ICD_DISPATCH_TABLE_ENTRY ( clSetEventCallback);
        ICD_DISPATCH_TABLE_ENTRY ( clCreateSubBuffer);
        ICD_DISPATCH_TABLE_ENTRY ( clSetMemObjectDestructorCallback);
        ICD_DISPATCH_TABLE_ENTRY ( clCreateUserEvent);
        ICD_DISPATCH_TABLE_ENTRY ( clSetUserEventStatus);
        ICD_DISPATCH_TABLE_ENTRY ( clEnqueueReadBufferRect);
        ICD_DISPATCH_TABLE_ENTRY ( clEnqueueWriteBufferRect);
        ICD_DISPATCH_TABLE_ENTRY ( clEnqueueCopyBufferRect);

        ICD_DISPATCH_TABLE_ENTRY ( /*clCreateSubDevicesEXT*/NULL);
        ICD_DISPATCH_TABLE_ENTRY ( /*clRetainDeviceEXT*/ NULL);
        ICD_DISPATCH_TABLE_ENTRY ( /*clReleaseDevice*/NULL);

#if gcdENABLE_CL_GL
        ICD_DISPATCH_TABLE_ENTRY ( clCreateEventFromGLsyncKHR);
#else
        ICD_DISPATCH_TABLE_ENTRY( NULL );
#endif

        ICD_DISPATCH_TABLE_ENTRY ( clCreateSubDevices);
        ICD_DISPATCH_TABLE_ENTRY ( clRetainDevice);
        ICD_DISPATCH_TABLE_ENTRY ( clReleaseDevice);
        ICD_DISPATCH_TABLE_ENTRY ( clCreateImage);
        ICD_DISPATCH_TABLE_ENTRY ( clCreateProgramWithBuiltInKernels);
        ICD_DISPATCH_TABLE_ENTRY ( clCompileProgram);
        ICD_DISPATCH_TABLE_ENTRY ( clLinkProgram);
        ICD_DISPATCH_TABLE_ENTRY ( clUnloadPlatformCompiler);
        ICD_DISPATCH_TABLE_ENTRY ( clGetKernelArgInfo);
        ICD_DISPATCH_TABLE_ENTRY ( clEnqueueFillBuffer);
        ICD_DISPATCH_TABLE_ENTRY ( clEnqueueFillImage);
        ICD_DISPATCH_TABLE_ENTRY ( clEnqueueMigrateMemObjects);
        ICD_DISPATCH_TABLE_ENTRY ( clEnqueueMarkerWithWaitList);
        ICD_DISPATCH_TABLE_ENTRY ( clEnqueueBarrierWithWaitList);
        ICD_DISPATCH_TABLE_ENTRY ( clGetExtensionFunctionAddressForPlatform);
#if gcdENABLE_CL_GL
        ICD_DISPATCH_TABLE_ENTRY ( clCreateFromGLTexture);
#else
        ICD_DISPATCH_TABLE_ENTRY( NULL );
#endif
    }

    /* return success */
    *outDispatchTable = clgDispatchTable;
    return CL_SUCCESS;

Error:
    return result;
}

void
cliIcdDispatchTableDestroy(CLIicdDispatchTable *dispatchTable)
{
    free(dispatchTable);
}
