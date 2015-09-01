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


#include "gc_egl_precomp.h"

#if defined(ANDROID)
#define TAG "VIVANTE"
#define LIB_PATH "/system/lib/egl/"
#endif


#define _GC_OBJ_ZONE    gcdZONE_EGL_CONTEXT

#if !gcdSTATIC_LINK
typedef enum _veglAPIINDEX
{
    vegl_EGL,
    vegl_OPENGL_ES11_CL,
    vegl_OPENGL_ES11,
    vegl_OPENGL_ES20,
    vegl_OPENGL_ES30,
    vegl_OPENVG,
}
veglAPIINDEX;

#if defined(ANDROID)
static const char * _dlls[][2] =
{
    /* EGL */                       {LIB_PATH "libEGL_" TAG ".so",       gcvNULL},
    /* OpenGL ES 1.1 Common Lite */ {LIB_PATH "libGLESv1_CM_" TAG ".so", gcvNULL},
    /* OpenGL ES 1.1 Common */      {LIB_PATH "libGLESv1_CM_" TAG ".so", gcvNULL},
    /* OpenGL ES 2.0 */             {LIB_PATH "libGLESv2_" TAG ".so",    gcvNULL},
    /* OpenGL ES 3.0 */             {LIB_PATH "libGLESv2_" TAG ".so",    gcvNULL},
    /* OpenVG 1.0 */                {"libOpenVG",                        gcvNULL},
};
#elif defined(__QNXNTO__)
static const char * _dlls[][2] =
{
    /* EGL */                       {"libEGL_viv",   gcvNULL},
    /* OpenGL ES 1.1 Common Lite */ {"glesv1-dlls",  gcvNULL},
    /* OpenGL ES 1.1 Common */      {"glesv1-dlls",  gcvNULL},
    /* OpenGL ES 2.0 */             {"glesv2-dlls",  gcvNULL},
    /* OpenGL ES 3.0 */             {"glesv2-dlls",  gcvNULL},
    /* OpenVG 1.0 */                {"vg-dlls",      gcvNULL},
};
#elif defined(__APPLE__)
static const char * _dlls[][2] =
{
    /* EGL */                       {"libEGL.dylib",       gcvNULL},
    /* OpenGL ES 1.1 Common Lite */ {"libGLESv1_CL.dylib", gcvNULL},
    /* OpenGL ES 1.1 Common */      {"libGLESv1_CM.dylib", gcvNULL},
    /* OpenGL ES 2.0 */             {"libGLESv2.dylib",    gcvNULL},
    /* OpenGL ES 3.0 */             {"libGLESv3.dylib",    gcvNULL},
    /* OpenVG 1.0 */                {"libOpenVG.dylib",    gcvNULL},
};
#else
static const char * _dlls[][2] =
{
    /* EGL */                       {"libEGL",         gcvNULL},
    /* OpenGL ES 1.1 Common Lite */ {"libGLESv1_CL",   gcvNULL},
    /* OpenGL ES 1.1 Common */      {"libGLESv1_CM",   gcvNULL},
    /* OpenGL ES 2.0 */             {"libGLESv2",      gcvNULL},
    /* OpenGL ES 3.0 */             {"libGLESv2",      gcvNULL},
    /* OpenVG 1.0 */                {"libOpenVG",      gcvNULL},
};
#endif

static const char * _dispatchNames[] =
{
    /* EGL                       */ "",
    /* OpenGL ES 1.1 Common Lite */ "GLES_CL_DISPATCH_TABLE",
    /* OpenGL ES 1.1 Common      */ "GLES_CM_DISPATCH_TABLE",
    /* OpenGL ES 2.0             */ "GLESv2_DISPATCH_TABLE",
    /* OpenGL ES 3.0             */ "GLESv2_DISPATCH_TABLE",
    /* OpenVG                    */ "OpenVG_DISPATCH_TABLE",
};

#if gcdGC355_PROFILER
extern gctUINT64 AppstartTimeusec;
gctFILE ApiTimeFile;
#endif

static int
_GetAPIIndex(
    EGLBoolean Egl,
    VEGLContext Context
    )
{
    int index = -1;

    do
    {
        VEGLThreadData thread;
        VEGLContext context;
        EGLenum api;
        EGLint client;

        if (Egl)
        {
            index = vegl_EGL;
            break;
        }

        /* Get thread data. */
        thread = veglGetThreadData();
        if (thread == gcvNULL)
        {
            gcmTRACE(
                gcvLEVEL_ERROR,
                "%s(%d): veglGetThreadData failed.",
                __FUNCTION__, __LINE__
                );

            break;
        }

        /* Get current context. */
        context = (Context != gcvNULL) ? Context
                : (thread->api == EGL_NONE)
                ? gcvNULL
                : thread->context;

        /* Is there a current context? */
        if (context == gcvNULL)
        {
            /* No current context, use thread data. */
            api    = thread->api;
            client = 1;
        }
        else
        {
            /* Have current context set. */
            api    = context->api;
            client = context->client;
        }

        /* Dispatch base on the API. */
        switch (api)
        {
        case EGL_OPENGL_ES_API:
            index = vegl_OPENGL_ES11 + MAJOR_API_VER(client) - 1;
            break;

        case EGL_OPENVG_API:
            index = vegl_OPENVG;
            break;
        }
    }
    while (EGL_FALSE);

    /* Return result. */
    return index;
}

gctHANDLE
veglGetModule(
    IN gcoOS Os,
    IN EGLBoolean Egl,
    IN VEGLContext Context,
    IN gctINT_PTR Index
    )
{
    gctHANDLE library = gcvNULL;
    gctINT index;
    gctUINT libIndex = 0;

    /* Get API index. */
    index = _GetAPIIndex(Egl, Context);

    if (index != -1)
    {
        if (Index != gcvNULL)
        {
            /* Try the next lib, if being repetitively called. */
            if(*Index == index)
            {
                libIndex = 1;
            }
        }

        /* Query the handle. */
        if (_dlls[index][libIndex] != gcvNULL)
        {
            gcoOS_LoadLibrary(Os, _dlls[index][libIndex], &library);

            if ((library == gcvNULL) && (index == vegl_OPENGL_ES11))
            {
                --index;

                /* Query the CL handle if CM not available. */
                gcoOS_LoadLibrary(Os, _dlls[index][libIndex], &library);
            }

#ifdef __APPLE__
            if ((library == gcvNULL) && (index == vegl_OPENGL_ES20))
            {
                ++index;

                /* Query the ES 3 handle if ES 2 not available. */
                gcoOS_LoadLibrary(Os, _dlls[index][libIndex], &library);
            }
#endif
        }
    }

    if (Index != gcvNULL)
    {
        *Index = index;
    }

    /* Return result. */
    return library;
}
#endif /* gcdSTATIC_LINK */

veglDISPATCH *
_GetDispatch(
    VEGLThreadData Thread,
    VEGLContext Context
    )
{
    struct eglContext context;

    if (Thread == gcvNULL)
    {
        return gcvNULL;
    }

    if (Context == gcvNULL)
    {
        Context = Thread->context;

        if (Context == gcvNULL)
        {
            /* Use default context. */
            context.thread        = Thread;
            context.api           = Thread->api;
            context.client        = 1;
            context.display       = EGL_NO_DISPLAY;
            context.sharedContext = EGL_NO_CONTEXT;
            context.draw          = EGL_NO_SURFACE;
            context.read          = EGL_NO_SURFACE;
            context.dispatch      = gcvNULL;

            Context = &context;
        }
    }

    if (Context->dispatch == gcvNULL)
    {
#if gcdSTATIC_LINK
#if gcdENABLE_3D
        extern veglDISPATCH GLES_CM_DISPATCH_TABLE;
        extern veglDISPATCH GLESv2_DISPATCH_TABLE;

        veglDISPATCH *esDispatchTables[] = {
            &GLES_CM_DISPATCH_TABLE,
            &GLESv2_DISPATCH_TABLE,
            &GLESv2_DISPATCH_TABLE,
        };
#endif
#ifndef VIVANTE_NO_VG
        extern veglDISPATCH OpenVG_DISPATCH_TABLE;
#endif /* VIVANTE_NO_VG */

        switch (Context->api)
        {
        case EGL_OPENGL_ES_API:
#if gcdENABLE_3D
            /*Added for openVG core static link*/
            Context->dispatch = esDispatchTables[MAJOR_API_VER(Context->client) - 1];
#else
            Context->dispatch = gcvNULL;
#endif
            break;

        case EGL_OPENVG_API:
#ifndef VIVANTE_NO_VG
            Context->dispatch = &OpenVG_DISPATCH_TABLE;
#else
            gcmTRACE_ZONE(gcvLEVEL_WARNING, gcdZONE_EGL_CONTEXT,
                          "%s(%d): %s",
                          __FUNCTION__, __LINE__,
                          "VG driver is not available.");

            return gcvNULL;
#endif
            break;

        default:
            Context->dispatch = gcvNULL;
            break;
        }
#else /*gcdSTATIC_LINK*/
        int index = -1, i;
        gctHANDLE library;
        gctPOINTER pointer = gcvNULL;

        for (i = 0; i < 2; i++)
        {
            gceSTATUS status;

            /* Get module handle and API index. */
            library = veglGetModule(gcvNULL, EGL_FALSE, Context, &index);
            if (library == gcvNULL)
            {
                return gcvNULL;
            }
            /* Query the dispatch table name. */
            status =  gcoOS_GetProcAddress(gcvNULL,
                                           library,
                                           _dispatchNames[index],
                                           &pointer);

            if (gcmIS_SUCCESS(status))
            {
                Context->dispatch = pointer;
                break;
            }

            if (status != gcvSTATUS_NOT_FOUND)
            {
                return gcvNULL;
            }
        }
#endif
    }

    /* Return dispatch table. */
    return Context->dispatch;
}


/*
** Imported function for client API driver use
*/
static void *
_GetCurrentAPIContext(
    EGLenum Api
    );

static void
_SetCurrentAPIContext(
    EGLenum Api,
    void* context
    );

static void
_SyncNative(
    void
    );

static void
_ReferenceImage(
    khrEGL_IMAGE * Image
    )
{
    VEGLImage image;
    VEGLThreadData thread;

    /* Get thread data. */
    thread = veglGetThreadData();

    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        /* Fatal error. */
        return;
    }

    /* Cast EGL image. */
    image = (VEGLImage) Image;

    /* Call reference func. */
    veglReferenceImage(thread, image->display, image);
}

static void
_DereferenceImage(
    khrEGL_IMAGE * Image
    )
{
    VEGLImage image;
    VEGLThreadData thread;

    /* Get thread data. */
    thread = veglGetThreadData();

    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        /* Fatal error. */
        return;
    }

    /* Cast EGL image. */
    image = (VEGLImage) Image;

    /* Call dereference func. */
    veglDereferenceImage(thread, image->display, image);
}

static gctPOINTER
_Malloc(
    void *ctx,
    gctSIZE_T size
    )
{
    gctPOINTER data = gcvNULL;
    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, size, &data)))
    {
        gcmFATAL("%s(%d): gcoOS_Allocate failed", __FUNCTION__, __LINE__);
    }
    return data;
}

static gctPOINTER
_Calloc(
    void *ctx,
    gctSIZE_T numElements,
    gctSIZE_T elementSize
    )
{
    gctPOINTER data = gcvNULL;
    gctSIZE_T  size = numElements * elementSize;
    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, size, &data)))
    {
        gcmFATAL("%s(%d): gcoOS_Allocate failed", __FUNCTION__, __LINE__);
        return data;
    }
    gcoOS_ZeroMemory(data, size);
    return data;
}

static gctPOINTER
_Realloc(
    void *ctx,
    gctPOINTER oldPtr,
    gctSIZE_T newSize
    )
{
    gctSIZE_T  oldSize = 0;
    gctPOINTER newPtr  = gcvNULL;

    gcoOS_GetMemorySize(gcvNULL, oldPtr, &oldSize);

    if (newSize <= oldSize)
    {
        if (0 == newSize)
        {
            gcoOS_Free(gcvNULL, oldPtr);
            return gcvNULL;
        }
        return oldPtr;
    }

    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, newSize, &newPtr)))
    {
        return gcvNULL;
    }

    if (oldPtr)
    {
        gcoOS_MemCopy(newPtr, oldPtr, oldSize);
        gcoOS_Free(gcvNULL, oldPtr);
    }

    return newPtr;
}

static void
_Free(
    void *ctx,
    gctPOINTER ptr
    )
{
    gcoOS_Free(gcvNULL, ptr);
}

/* TODO: For windows only */
static void
_CreateUserMutex(
    VEGLLock *mp
    )
{
    mp->usageCount++;
    if (mp->usageCount == 1)
    {
        gcoOS_CreateMutex(gcvNULL, &mp->lock);
    }
}

static void
_DestroyUserMutex(
    VEGLLock *mp
    )
{
    mp->usageCount--;
    if (mp->usageCount == 0)
    {
        gcoOS_DeleteMutex(gcvNULL, mp->lock);
    }
}

static void
_LockUserMutex(
    VEGLLock *mp
    )
{
    gcoOS_AcquireMutex(gcvNULL, mp->lock, gcvINFINITE);
}

static void
_UnlockUserMutex(
    VEGLLock *mp
    )
{
    gcoOS_ReleaseMutex(gcvNULL, mp->lock);
}

void *
_CreateApiContext(
    VEGLThreadData Thread,
    VEGLContext    Context,
    VEGLConfig     Config,
    void         * SharedContext
    )
{
    VEGLimports imports =
    {
        _GetCurrentAPIContext,   /* getCurContext */
        _SetCurrentAPIContext,   /* setCurContext */
        _SyncNative,             /* syncNative */

        _ReferenceImage,         /* referenceImage */
        _DereferenceImage,       /* dereferenceImage */

        _Malloc,                 /* malloc */
        _Calloc,                 /* calloc */
        _Realloc,                /* realloc */
        _Free,                   /* free */

        _CreateUserMutex,        /* createMutex */
        _DestroyUserMutex,       /* destroyMutex */
        _LockUserMutex,          /* lockMutex */
        _UnlockUserMutex,        /* unlockMutex */

        gcvNULL,                 /* config */
        gcvFALSE,                /* robustAccess */
        0,                       /* resetNotification */
        gcvFALSE,                /* debuggable */
    };

    veglDISPATCH * dispatch = _GetDispatch(Thread, Context);

    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_EGL_CONTEXT,
                  "%s(%d): 0x%x,0x%x,0x%x,0x%x",
                  __FUNCTION__, __LINE__,
                  Thread, Context, SharedContext, Config);

    if ((dispatch == gcvNULL) || (dispatch->createContext == gcvNULL))
    {
        return gcvNULL;
    }

    gcmASSERT(Config);

    imports.config = Config;
    imports.robustAccess = Context->robustAccess;
    imports.resetNotification = Context->resetNotification;
    imports.debuggable = (Context->flags & EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR) ? gcvTRUE : gcvFALSE;

#if gcdGC355_PROFILER
    return (*dispatch->createContext)(Thread, Context->appStartTime, Context->apiTimeFile, MAJOR_API_VER(Context->client), &imports, SharedContext);
#else
    return (*dispatch->createContext)(Thread, Context->client, &imports, SharedContext);
#endif
}


EGLBoolean
_DestroyApiContext(
    VEGLThreadData Thread,
    VEGLContext Context,
    void * ApiContext
    )
{
    veglDISPATCH * dispatch = _GetDispatch(Thread, Context);

    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_EGL_CONTEXT,
                  "%s(%d): 0x%x,0x%x",
                  __FUNCTION__, __LINE__,
                  Thread, ApiContext);

    if ((dispatch == gcvNULL)
    ||  (dispatch->destroyContext == gcvNULL)
    )
    {
        return EGL_FALSE;
    }

    return (*dispatch->destroyContext)(Thread, ApiContext);
}

EGLBoolean
_FlushApiContext(
    VEGLThreadData Thread,
    VEGLContext Context,
    void * ApiContext
    )
{
    veglDISPATCH * dispatch = _GetDispatch(Thread, Context);

    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_EGL_CONTEXT,
                  "%s(%d): 0x%x,0x%x",
                  __FUNCTION__, __LINE__,
                  Thread, ApiContext);

    if (ApiContext == gcvNULL)
    {
        return EGL_TRUE;
    }

    if ((dispatch == gcvNULL)
    ||  (dispatch->flushContext == gcvNULL)
    )
    {
        return EGL_FALSE;
    }

    return (*dispatch->flushContext)(Context);
}

static EGLBoolean
_ApiMakeCurrent(
    VEGLThreadData Thread,
    VEGLContext    Context,
    VEGLDrawable   Draw,
    VEGLDrawable   Read
    )
{
    veglDISPATCH * dispatch = _GetDispatch(Thread, Context);
    void * ApiContext = Context->context;

    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_EGL_CONTEXT,
                  "%s(%d): 0x%x,0x%x,0x%x,0x%x",
                  __FUNCTION__, __LINE__,
                  Thread, ApiContext, Draw, Read);

    if ((dispatch == gcvNULL) || (dispatch->makeCurrent == gcvNULL))
    {
        return (ApiContext == gcvNULL) ? EGL_TRUE : EGL_FALSE;
    }

    return (*dispatch->makeCurrent)(Thread, ApiContext, Draw, Read);
}

static EGLBoolean
_ApiLoseCurrent(
    VEGLThreadData Thread,
    VEGLContext    Context
    )
{
    veglDISPATCH * dispatch = _GetDispatch(Thread, Context);
    void * ApiContext = Context->context;

    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_EGL_CONTEXT,
                  "%s(%d): 0x%x,0x%x", __FUNCTION__, __LINE__,
                  Thread, ApiContext);

    if ((dispatch == gcvNULL) || (dispatch->loseCurrent == gcvNULL))
    {
        return (ApiContext == gcvNULL) ? EGL_TRUE : EGL_FALSE;
    }

    return (*dispatch->loseCurrent)(Thread, ApiContext);
}

EGLBoolean
_SetDrawable(
    VEGLThreadData Thread,
    VEGLContext    Context,
    VEGLDrawable   Draw,
    VEGLDrawable   Read
    )
{
    veglDISPATCH * dispatch = _GetDispatch(Thread, Context);
    void * ApiContext = Context ? Context->context : gcvNULL;

    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_EGL_CONTEXT,
                  "%s(%d): 0x%x,0x%x,0x%x,0x%x",
                  __FUNCTION__, __LINE__,
                  Thread, ApiContext, Draw, Read);

    if ((dispatch == gcvNULL) || (dispatch->setDrawable == gcvNULL))
    {
        return (ApiContext == gcvNULL) ? EGL_TRUE : EGL_FALSE;
    }

    return (*dispatch->setDrawable)(Thread, ApiContext, Draw, Read);
}

gceSTATUS
_SetBuffer(
    VEGLThreadData Thread,
    gcoSURF Draw
    )
{
    veglDISPATCH * dispatch = _GetDispatch(Thread, gcvNULL);

    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_EGL_CONTEXT,
                  "%s(%d): 0x%x,0x%x",
                  __FUNCTION__, __LINE__,
                  Thread, Draw);

    if ((dispatch == gcvNULL)
    ||  (dispatch->setBuffer == gcvNULL)
    )
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    return (*dispatch->setBuffer)(Draw);
}

EGLBoolean
_Flush(
    VEGLThreadData Thread
    )
{
    veglDISPATCH * dispatch = _GetDispatch(Thread, gcvNULL);

    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_EGL_CONTEXT,
                  "%s(%d): 0x%x",
                  __FUNCTION__, __LINE__,
                  Thread);

    if ((dispatch == gcvNULL)
    ||  (dispatch->flush == gcvNULL)
    )
    {
        return EGL_FALSE;
    }

    if (Thread->context != gcvNULL)
    {
        (*dispatch->flush)();
    }

    return EGL_TRUE;
}

EGLBoolean
_Finish(
    VEGLThreadData Thread
    )
{
    veglDISPATCH * dispatch = _GetDispatch(Thread, gcvNULL);

    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_EGL_CONTEXT,
                  "%s(%d): 0x%x",
                  __FUNCTION__, __LINE__,
                  Thread);

    if ((dispatch == gcvNULL)
    ||  (dispatch->finish == gcvNULL)
    )
    {
        return EGL_FALSE;
    }

    if (Thread->context != gcvNULL)
    {
        (*dispatch->finish)();
    }

    return EGL_TRUE;
}

gcoSURF
_GetClientBuffer(
    VEGLThreadData Thread,
    void * Context,
    EGLClientBuffer Buffer
    )
{
    veglDISPATCH * dispatch = _GetDispatch(Thread, gcvNULL);

    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_EGL_CONTEXT,
                  "%s(%d): 0x%x,0x%x,0x%x",
                  __FUNCTION__, __LINE__,
                  Thread, Context, Buffer);

    if ((dispatch == gcvNULL)
    ||  (dispatch->getClientBuffer == gcvNULL)
    )
    {
        return gcvNULL;
    }

    return (*dispatch->getClientBuffer)(Context, Buffer);
}

EGLBoolean
_eglProfileCallback(
    VEGLThreadData Thread,
    gctUINT32 Enum,
    gctHANDLE Value
    )
{
#if VIVANTE_PROFILER
    veglDISPATCH * dispatch = _GetDispatch(Thread, gcvNULL);

    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_EGL_CONTEXT,
                  "%s(%d): 0x%x,%u,%lu",
                  __FUNCTION__, __LINE__,
                  Thread, Enum, Value);

    if ((dispatch == gcvNULL)
    ||  (dispatch->profiler == gcvNULL)
    )
    {
        return EGL_FALSE;
    }

    return (*dispatch->profiler)(gcvNULL, Enum, Value);
#else

    return EGL_TRUE;
#endif
}


EGLBoolean
veglBindAPI(
    EGLenum api
    )
{
    VEGLThreadData thread;

    gcmHEADER_ARG("api=0x%04x", api);

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        /* Fatal error. */
        gcmFOOTER_ARG("%d", EGL_FALSE);
        return EGL_FALSE;
    }

    switch (api)
    {
    case EGL_OPENGL_ES_API:
        /* OpenGL ES API. */
        if (thread->api != api)
        {
            thread->api = api;
            thread->context = thread->esContext;
        }
        gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D));
        break;

    case EGL_OPENGL_API:
        veglSetEGLerror(thread,  EGL_BAD_PARAMETER);;
        gcmFOOTER_ARG("%d", EGL_FALSE);
        return EGL_FALSE;

    case EGL_OPENVG_API:
        /* OpenVG API. */
        if (thread->api != api)
        {
            thread->context = thread->vgContext;
            thread->api = api;
        }

#if gcdENABLE_VG
        if (thread->openVGpipe)
        {
            gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_VG));
        }
        else
        {
            gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D));
        }
#endif
        break;

    default:
        /* Bad parameter. */
        veglSetEGLerror(thread,  EGL_BAD_PARAMETER);;
        gcmFOOTER_ARG("%d", EGL_FALSE);
        return EGL_FALSE;
    }

    /* Success. */
    veglSetEGLerror(thread,  EGL_SUCCESS);
    gcmFOOTER_ARG("%d", EGL_TRUE);
    return EGL_TRUE;
}

EGLAPI EGLBoolean EGLAPIENTRY
eglBindAPI(
    EGLenum api
    )
{
    EGLenum ret;

    VEGL_TRACE_API(BindAPI)(api);

    gcmDUMP_API("${EGL eglBindAPI 0x%08X}", api);

    ret = veglBindAPI(api);

    return ret;
}

EGLenum
veglQueryAPI(
    void
    )
{
    VEGLThreadData thread;

    gcmHEADER();

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        gcmFOOTER_ARG("%04x", EGL_NONE);
        return EGL_NONE;
    }

    /* Return current API. */
    gcmFOOTER_ARG("%04x", thread->api);
    return thread->api;
}

EGLAPI EGLenum EGLAPIENTRY
eglQueryAPI(
    void
    )
{
    EGLenum api;

    VEGL_TRACE_API_PRE(QueryAPI)();
    api = veglQueryAPI();
    VEGL_TRACE_API_POST(QueryAPI)(api);

    gcmDUMP_API("${EGL eglQueryAPI := 0x%08X}", api);

    return api;
}

EGLAPI EGLContext EGLAPIENTRY
eglCreateContext(
    EGLDisplay Dpy,
    EGLConfig config,
    EGLContext SharedContext,
    const EGLint *attrib_list
    )
{
    VEGLThreadData thread;
    VEGLDisplay dpy;
    VEGLContext sharedContext;
    gceSTATUS status;
    VEGLContext context;
    EGLint major = 1;
    EGLint minor = 0;
    gctPOINTER pointer = gcvNULL;
    VEGLConfig  eglConfig;
    EGLint flags = 0;
    gctBOOL robustAccess = EGL_FALSE;
    gctINT resetNotification = EGL_NO_RESET_NOTIFICATION_EXT;
    gctBOOL robustAttribSet = gcvFALSE;

    gcmHEADER_ARG("Dpy=0x%x config=0x%x SharedContext=0x%x attrib_list=0x%x",
                  Dpy, config, SharedContext, attrib_list);

    VEGL_TRACE_API_PRE(CreateContext)(Dpy, config, SharedContext, attrib_list);

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        gcmFOOTER_ARG("0x%x", EGL_NO_CONTEXT);

        return EGL_NO_CONTEXT;
    }

    /* Test for valid bounded API. */
    if (thread->api == EGL_NONE)
    {
        /* Bad match. */
        veglSetEGLerror(thread, EGL_BAD_MATCH);

        gcmFOOTER_ARG("0x%x", EGL_NO_CONTEXT);
        return EGL_NO_CONTEXT;
    }

    /* Create shortcuts to objects. */
    sharedContext = VEGL_CONTEXT(SharedContext);

    /* Test for valid EGLDisplay structure. */
    dpy = veglGetDisplay(Dpy);
    if (dpy == gcvNULL)
    {
        /* Bad display. */
        veglSetEGLerror(thread,  EGL_BAD_DISPLAY);

        gcmFOOTER_ARG("0x%x", EGL_NO_CONTEXT);
        return EGL_NO_CONTEXT;
    }

    /* Test if EGLDisplay structure has been initialized. */
    if (!dpy->initialized)
    {
        /* Not initialized. */
        veglSetEGLerror(thread,  EGL_NOT_INITIALIZED);;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Test for valid config. */
    if (((EGLint)(intptr_t)config <= __EGL_INVALID_CONFIG__)
    ||  ((EGLint)(intptr_t)config > dpy->configCount)
    )
    {
        veglSetEGLerror(thread,  EGL_BAD_CONFIG);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    eglConfig = VEGL_CONFIG(&dpy->config[(EGLint)(intptr_t)config - 1]);

    /* Get attribute. */
    if (attrib_list != gcvNULL)
    {
        EGLint i = 0;

        for (i = 0; attrib_list[i] != EGL_NONE; i += 2)
        {
            switch (attrib_list[i])
            {
            case 0x4098:
                /* This check is added here for bug 7297. The app sends an unknown attribute 0x4098.
                 * This is most probably an extension they are using and we need to let it go.
                 */
                break;

            case EGL_CONTEXT_CLIENT_VERSION:
                switch (thread->api)
                {
                case EGL_OPENGL_API:
                case EGL_OPENGL_ES_API:
                    major = attrib_list[i + 1];
                    break;
                default:
                    veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
                    gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                }
                break;

            case EGL_CONTEXT_MINOR_VERSION_KHR:
                switch (thread->api)
                {
                case EGL_OPENGL_API:
                case EGL_OPENGL_ES_API:
                    minor = attrib_list[i + 1];
                    break;
                default:
                    veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
                    gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                }
                break;

            case EGL_CONTEXT_FLAGS_KHR:
                {
                    flags = attrib_list[i + 1];

                    if (flags & ~(EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR |
                                  EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE_BIT_KHR |
                                  EGL_CONTEXT_OPENGL_ROBUST_ACCESS_BIT_KHR))
                    {
                        veglSetEGLerror(thread,  EGL_BAD_PARAMETER);
                        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                    }

                    if (flags & EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR)
                    {
                        switch (thread->api)
                        {
                        case EGL_OPENGL_API:
                        case EGL_OPENGL_ES_API:
                            break;
                        default:
                            veglSetEGLerror(thread,  EGL_BAD_ATTRIBUTE);
                            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                        }
                    }

                    if (flags & (EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE_BIT_KHR |
                                 EGL_CONTEXT_OPENGL_ROBUST_ACCESS_BIT_KHR))
                    {
                        if (thread->api != EGL_OPENGL_API)
                        {
                            veglSetEGLerror(thread,  EGL_BAD_ATTRIBUTE);
                            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                        }
                    }
                }
                break;

            case EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR:
            case EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_KHR:
                if (thread->api != EGL_OPENGL_API)
                {
                    veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
                    gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                }
                break;

            case EGL_CONTEXT_OPENGL_ROBUST_ACCESS_EXT:
                if (thread->api != EGL_OPENGL_ES_API)
                {
                    veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
                    gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                }
                robustAccess = (gctBOOL)attrib_list[i + 1];
                if (robustAccess != EGL_FALSE && robustAccess != EGL_TRUE)
                {
                    veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
                    gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                }
                robustAttribSet = robustAttribSet || (robustAccess == EGL_TRUE);
                break;

            case EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_EXT:
                if (thread->api != EGL_OPENGL_ES_API)
                {
                    veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
                    gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                }
                resetNotification = (gctINT)attrib_list[i + 1];
                if (resetNotification != EGL_NO_RESET_NOTIFICATION_EXT &&
                    resetNotification != EGL_LOSE_CONTEXT_ON_RESET_EXT)
                {
                    veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
                    gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                }
                robustAttribSet = gcvTRUE;
                break;

            default:
                veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }
        }
    }

    if (thread->api == EGL_OPENGL_ES_API)
    {
        EGLBoolean valid = EGL_FALSE;
        EGLBoolean match = EGL_FALSE;
        gceCHIPMODEL chipModel;
        gctUINT32 chipRevision;
        EGLenum renderableType = eglConfig->renderableType;

        switch (major)
        {
        case 1:
            match = (minor == 0 || minor == 1) ? EGL_TRUE : EGL_FALSE;
            valid = (renderableType & EGL_OPENGL_ES_BIT ) ? EGL_TRUE : EGL_FALSE;
            valid = valid && (!robustAttribSet);
            break;

        case 2:
            match = (minor == 0) ? EGL_TRUE : EGL_FALSE;
            valid = (renderableType & EGL_OPENGL_ES2_BIT) ? EGL_TRUE : EGL_FALSE;
            break;

        case 3:
            gcmONERROR(gcoHAL_QueryChipIdentity(gcvNULL, &chipModel, &chipRevision, gcvNULL, gcvNULL));

            /* Halti2 HW support ES30 and ES31 */
            if (gcoHAL_IsFeatureAvailable(NULL, gcvFEATURE_HALTI2) ||
                (chipModel == gcv900 && chipRevision == 0x5250))
            {
                match = (minor >= 0 && minor <= 1) ? EGL_TRUE : EGL_FALSE;
            }
            /* Halti0 HW support ES30 only */
            else if (gcoHAL_IsFeatureAvailable(NULL, gcvFEATURE_HALTI0))
            {
                match = (minor == 0) ? EGL_TRUE : EGL_FALSE;
            }
            else
            {
                match = EGL_FALSE;
            }
            valid = (renderableType & EGL_OPENGL_ES3_BIT_KHR) ? EGL_TRUE : EGL_FALSE;
            break;
        }

        if (!match)
        {
            veglSetEGLerror(thread, EGL_BAD_MATCH);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        if (!valid)
        {
            veglSetEGLerror(thread,  EGL_BAD_CONFIG);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }
    else
    {
        if ((thread->api == EGL_OPENVG_API)
        &&  !(eglConfig->renderableType & EGL_OPENVG_BIT)
        )
        {
            veglSetEGLerror(thread,  EGL_BAD_CONFIG);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }


    /* Test if shared context is compatible. */
    if (sharedContext != gcvNULL)
    {
        /* Test for a valid context. */
        VEGLContext p_ctx = (VEGLContext)veglGetResObj(dpy,
                                                       (VEGLResObj*)&dpy->contextStack,
                                                       (EGLResObj)SharedContext ,
                                                       EGL_CONTEXT_SIGNATURE);
        if (p_ctx == gcvNULL)
        {
            veglSetEGLerror(thread,  EGL_BAD_CONTEXT);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        if (sharedContext->api != thread->api)
        {
            /* Bad match. */
            veglSetEGLerror(thread, EGL_BAD_MATCH);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        if (sharedContext->resetNotification != resetNotification)
        {
            /* Bad match. */
            veglSetEGLerror(thread, EGL_BAD_MATCH);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }

    /* Flush any existing context. */
    if (thread->context != EGL_NO_CONTEXT)
    {
        gcmVERIFY(_FlushApiContext(thread,
                                   thread->context,
                                   thread->context->context));
    }

    /* Create new context. */
    status = gcoOS_Allocate(gcvNULL,
                            gcmSIZEOF(struct eglContext),
                            &pointer);

    if (gcmIS_ERROR(status))
    {
        /* Error. */
        veglSetEGLerror(thread, EGL_BAD_ALLOC);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    context = pointer;

    /* Zero the context memory. */
    gcoOS_ZeroMemory(context, gcmSIZEOF(struct eglContext));

    /* Initialize context. */
    context->thread        = gcvNULL;
    context->api           = thread->api;
    context->client        = (major << 4) | minor;
    context->display       = dpy;
    context->sharedContext = sharedContext;
    context->draw          = EGL_NO_SURFACE;
    context->read          = EGL_NO_SURFACE;
    context->dispatch      = gcvNULL;
    context->resObj.signature = EGL_CONTEXT_SIGNATURE;
    context->flags         = flags;
    context->robustAccess  = robustAccess;
    context->resetNotification = resetNotification;

#if gcdGC355_PROFILER
    context->appStartTime = AppstartTimeusec;

    {
        gctCHAR fileName[256] = {'\0'};

        gctUINT offset = 0;
        gctHANDLE pid = gcoOS_GetCurrentProcessID();
        gctHANDLE tid = gcoOS_GetCurrentThreadID();

        gcoOS_PrintStrSafe(
            fileName,
            gcmSIZEOF(fileName),
            &offset,
#if defined(ANDROID)
            "/data/data/APITimes_pid%d_tid%d.log",
#   else
            "APITimes_pid%d_tid%d.log",
#   endif
            (gctUINTPTR_T)pid,
            (gctUINTPTR_T)tid
            );

        gcoOS_Open(
            gcvNULL,
            fileName,
            gcvFILE_CREATE,
            &context->apiTimeFile
            );

        ApiTimeFile = context->apiTimeFile;
    }
#endif

    /* Copy config information */
    gcoOS_MemCopy(&context->config, eglConfig, sizeof(context->config));

    /* Push context into stack. */
    veglPushResObj(dpy, (VEGLResObj *)&dpy->contextStack, (VEGLResObj)context);

    /* Create context for API. */
    context->context = _CreateApiContext(
        thread, context,
        eglConfig,
        (sharedContext != gcvNULL) ? sharedContext->context : gcvNULL);

    if (context->context == gcvNULL)
    {
        /* Roll back. */
        veglPopResObj(dpy, (VEGLResObj *)&dpy->contextStack, (VEGLResObj)context);

        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, context));

        veglSetEGLerror(thread,  EGL_BAD_CONFIG);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

#ifdef EGL_API_DRI
    gcoOS_CreateContext(dpy->localInfo, context);
#endif

    /* Useful for debugging */
    gcmTRACE(
        gcvLEVEL_INFO,"a,b,g,r=%d,%d,%d,%d, d,s=%d,%d, id=%d, AA=%d, t=0x%08X",
            eglConfig->alphaSize,
            eglConfig->blueSize,
            eglConfig->greenSize,
            eglConfig->redSize,
            eglConfig->depthSize,
            eglConfig->stencilSize,
            eglConfig->configId,
            eglConfig->samples,
            eglConfig->surfaceType
        );

    /* Success. */
    veglSetEGLerror(thread,  EGL_SUCCESS);
    VEGL_TRACE_API_POST(CreateContext)(Dpy, config, SharedContext, attrib_list, context);
    gcmDUMP_API("${EGL eglCreateContext 0x%08X 0x%08X 0x%08X (0x%08X) := "
                "0x%08X",
                Dpy, config, SharedContext, attrib_list, context);
    gcmDUMP_API_ARRAY_TOKEN(attrib_list, EGL_NONE);
    gcmDUMP_API("$}");

    gcmFOOTER_ARG("0x%x", context);
    return context;

OnError:

    gcmFOOTER_ARG("0x%x", EGL_NO_CONTEXT);
    return EGL_NO_CONTEXT;

}

static EGLBoolean
_DestroyContext(
    IN VEGLThreadData Thread,
    VEGLDisplay Display,
    VEGLContext Context
    )
{
    VEGLContext current;

    gcmHEADER_ARG("Thread=0x%x Display=0x%x Context=0x%x",
                  Thread, Display, Context);

    switch (Context->api)
    {
    case EGL_OPENGL_API:
        current = Thread->glContext;
        break;
    case EGL_OPENGL_ES_API:
        current = Thread->esContext;
        break;
    case EGL_OPENVG_API:
        current = Thread->vgContext;
        break;
    default:
        current = gcvNULL;
        gcmTRACE(gcvLEVEL_ERROR,
                 "%s(%d): _DestroyContext bad current rendering API.",
                 __FUNCTION__, __LINE__);
        break;
    }

    if (Context->thread && (Context->thread != Thread))
    {
        Context->deleteReq = gcvTRUE;

        veglSetEGLerror(Thread,  EGL_SUCCESS);

        gcmTRACE(gcvLEVEL_INFO,
                 "%s(%d): _DestroyContext:context is still in current of other thread",
                 __FUNCTION__, __LINE__);

        gcmFOOTER_ARG("%d", EGL_TRUE);
        return EGL_TRUE;
    }

    if (current == Context)
    {
        gcmTRACE(gcvLEVEL_INFO,
                 "%s(%d): _DestroyContext: context is still in current of current thread.",
                 __FUNCTION__, __LINE__);

        /* Flush current context of current client API */
        gcmVERIFY(_FlushApiContext(Thread,
                                 current,
                                 current->context));
        /* Remove context. */
        gcmVERIFY(_ApiLoseCurrent(Thread, current));
    }

    /* Destroy client API context. */
    if (Context->context != gcvNULL)
    {
        _DestroyApiContext(Thread, Context, Context->context);
        Context->context = gcvNULL;
    }

    /* Deference any surfaces. */
    if (Context->draw != EGL_NO_SURFACE)
    {
        VEGLSurface surface = Context->draw;

        veglDereferenceSurface(Thread, Display, surface, EGL_FALSE);

        /* If the surface has been destoried then free the struct .*/
        if (surface->reference == gcvNULL)
        {
            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, surface));
        }
    }

    if (Context->read != EGL_NO_SURFACE)
    {
        VEGLSurface surface = Context->read;

        veglDereferenceSurface(Thread, Display, surface, EGL_FALSE);

        /* If the surface has been destoried then free the struct .*/
        if (surface->reference == gcvNULL)
        {
            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, surface));
        }
    }

    if (Thread->context == Context)
    {
        /* Current context has been destroyed. */
        Thread->context  = gcvNULL;
    }

    if (Thread->esContext == Context)
    {
        /* Current context has been destroyed. */
        Thread->esContext  = gcvNULL;
    }

    if (Thread->vgContext == Context)
    {
        /* Current context has been destroyed. */
        Thread->vgContext  = gcvNULL;
    }

    if (Thread->glContext == Context)
    {
        /* Current context has been destroyed. */
        Thread->glContext  = gcvNULL;
    }

    /* Pop EGLContext from the stack. */
    veglPopResObj(Display, (VEGLResObj *)&Display->contextStack, (VEGLResObj)Context);

    /* Reset the context. */
    Context->thread = gcvNULL;
    Context->api    = EGL_NONE;

#ifdef EGL_API_DRI
    gcoOS_DestroyContext(Display->localInfo, Context);
#endif

    /* Execute events accumulated in the buffer if any. */
    gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvFALSE));

    /* Free the eglContext structure. */
    gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, Context));

    gcmFOOTER_ARG("%d", EGL_TRUE);
    return EGL_TRUE;
}

EGLBoolean
veglDestroyContext(
    EGLDisplay Dpy,
    EGLContext Ctx
    )
{
    VEGLThreadData thread;
    VEGLDisplay dpy;
    VEGLContext ctx;
    gceSTATUS status;

    gcmHEADER_ARG("Dpy=0x%x Ctx=0x%x", Dpy, Ctx);

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        gcmFOOTER_ARG("%d (line %d)", EGL_FALSE, __LINE__);
        return EGL_FALSE;
    }

    /* Test for valid bounded API. */
    if (thread->api == EGL_NONE)
    {
        /* Bad match. */
        veglSetEGLerror(thread,  EGL_NOT_INITIALIZED);;
        gcmFOOTER_ARG("%d (line %d)", EGL_FALSE, __LINE__);
        return EGL_FALSE;
    }

    /* Test for valid EGLDisplay structure. */
    dpy = veglGetDisplay(Dpy);
    if (dpy == gcvNULL)
    {
        /* Bad display. */
        veglSetEGLerror(thread,  EGL_BAD_DISPLAY);
        gcmFOOTER_ARG("%d (line %d)", EGL_FALSE, __LINE__);
        return EGL_FALSE;
    }

    /* Test if EGLDisplay structure has been initialized. */
    if (!dpy->initialized)
    {
        /* Not initialized. */
        veglSetEGLerror(thread,  EGL_NOT_INITIALIZED);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Find the context object on the stack. */
    ctx = (VEGLContext)veglGetResObj(dpy,
                                     (VEGLResObj*)&dpy->contextStack,
                                     (EGLResObj)Ctx ,
                                     EGL_CONTEXT_SIGNATURE);

    /* Test if ctx is valid. */
    if (ctx == gcvNULL)
    {
        veglSetEGLerror(thread,  EGL_BAD_CONTEXT);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

#if gcdGC355_PROFILER
    if (ctx->apiTimeFile != gcvNULL)
    {
        gcoOS_Flush(gcvNULL,ctx->apiTimeFile);
        gcoOS_Close(gcvNULL, ctx->apiTimeFile);
        ctx->apiTimeFile = gcvNULL;
    }
#endif

    /* Do actual destroy. */
    _DestroyContext(thread, dpy, ctx);

    /* Success. */
    veglSetEGLerror(thread,  EGL_SUCCESS);
    gcmFOOTER_ARG("%d", EGL_TRUE);
    return EGL_TRUE;

OnError:
    gcmFOOTER_ARG("%d", EGL_FALSE);
    return EGL_FALSE;
}

EGLAPI EGLBoolean EGLAPIENTRY
eglDestroyContext(
    EGLDisplay Dpy,
    EGLContext Ctx
    )
{
    EGLBoolean ret;
    VEGL_TRACE_API(DestroyContext)(Dpy, Ctx);

    gcmDUMP_API("${EGL eglDestroyContext 0x%08X 0x%08X}", Dpy, Ctx);

    ret = veglDestroyContext(Dpy, Ctx);

    return ret;
}

EGLBoolean
veglMakeCurrent(
    EGLDisplay Dpy,
    EGLSurface Draw,
    EGLSurface Read,
    EGLContext Ctx
    )
{
    EGLBoolean result;
    VEGLThreadData thread;
    VEGLDisplay dpy;
    VEGLSurface draw;
    VEGLSurface read;
    VEGLContext ctx, current;
    EGLint width = 0, height = 0;
    gceSTATUS status;

    gcmHEADER_ARG("Dpy=0x%x Draw=0x%x Read=0x%x Ctx=0x%x",
                  Dpy, Draw, Read, Ctx);

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        gcmFOOTER_ARG("%d", EGL_FALSE);
        return EGL_FALSE;
    }

    /* Test for valid EGLDisplay structure. */
    dpy = veglGetDisplay(Dpy);

    if (dpy == gcvNULL)
    {
        /* Bad display. */
        veglSetEGLerror(thread,  EGL_BAD_DISPLAY);

        gcmFOOTER_ARG("%d", EGL_FALSE);
        return EGL_FALSE;
    }

    /* Test for valid EGLContext structure */
    ctx = (VEGLContext)veglGetResObj(dpy,
                                     (VEGLResObj*)&dpy->contextStack,
                                     (EGLResObj)Ctx ,
                                     EGL_CONTEXT_SIGNATURE);

    if (Ctx != EGL_NO_CONTEXT && ctx == gcvNULL)
    {
        veglSetEGLerror(thread,  EGL_BAD_CONTEXT);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Test for valid EGLSurface structure */
    draw = (VEGLSurface)veglGetResObj(dpy,
                                      (VEGLResObj*)&dpy->surfaceStack,
                                      (EGLResObj)Draw,
                                      EGL_SURFACE_SIGNATURE);

    if (Draw != EGL_NO_SURFACE && draw == gcvNULL)
    {
        veglSetEGLerror(thread, EGL_BAD_SURFACE);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    read = (VEGLSurface)veglGetResObj(dpy,
                                      (VEGLResObj*)&dpy->surfaceStack,
                                      (EGLResObj)Read,
                                      EGL_SURFACE_SIGNATURE);

    if (Read != EGL_NO_SURFACE && read == gcvNULL)
    {
        veglSetEGLerror(thread, EGL_BAD_SURFACE);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Test for content protection */
    if (draw && (draw->protectedContent == EGL_FALSE) &&
        read && (read->protectedContent == EGL_TRUE))

    {
        veglSetEGLerror(thread, EGL_BAD_ACCESS);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (draw && (draw->type & EGL_WINDOW_BIT))
    {
        /* Validate native window. */
        result = veglGetWindowSize(dpy, draw, &width, &height);

        if (!result)
        {
            /* Window is gone. */
            veglSetEGLerror(thread, EGL_BAD_NATIVE_WINDOW);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }

    else if (draw && (draw->type & EGL_PIXMAP_BIT))
    {
        /* Validate native pixmap. */
        result = veglGetPixmapSize(dpy, draw->pixmap,
                                   draw->pixInfo,
                                   &width, &height);

        if (!result)
        {
            /* Pixmap is gone. */
            veglSetEGLerror(thread, EGL_BAD_NATIVE_PIXMAP);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }

    /* Test for release of context. */
    if (!ctx && !draw && !read)
    {
        if (thread->context != EGL_NO_CONTEXT)
        {
#if VIVANTE_PROFILER
            if (thread->context->draw != gcvNULL && thread->context->draw != draw && _eglProfileCallback(thread, 0, 0))
            {
#if defined(ANDROID)
                veglSuspendSwapWorker(dpy);
                gcmVERIFY_OK(gcoHAL_Commit(gcvNULL,gcvTRUE));
#else
                if (dpy->workerThread == gcvNULL)
                {
                    gcmVERIFY_OK(gcoHAL_Commit(gcvNULL,gcvTRUE));
                }
#endif
                if (thread->api == EGL_OPENGL_ES_API)
                {
                    if (MAJOR_API_VER(thread->context->client) == 1)
                    {
                        /* GL_PROFILER_FRAME_END */
                        _eglProfileCallback(thread, 10, 0);
                    }
                    else if (MAJOR_API_VER(thread->context->client) == 2 || MAJOR_API_VER(thread->context->client) == 3)
                    {
                        /* GL3_PROFILER_FRAME_END */
                        _eglProfileCallback(thread, 10, 0);
                    }
                }
                else if (thread->api == EGL_OPENVG_API)
                {
                    _eglProfileCallback(thread, 10, 0);
                }
#if defined(ANDROID)
                veglResumeSwapWorker(dpy);
#endif
            }
#endif
            current = thread->context;
            /* Flush current context. */
            gcmVERIFY(
                _FlushApiContext(thread,
                                 current,
                                 current->context));


            /* Remove context. */
            gcmVERIFY(_ApiLoseCurrent(thread, current));

            if (current->draw != gcvNULL)
            {
                VEGLSurface sur = current->draw;

                /* Make sure all workers have been processed. */
                gcmVERIFY_OK(gcoOS_WaitSignal(gcvNULL,
                                              sur->workerDoneSignal,
                                              gcvINFINITE));

                if (sur->type & EGL_WINDOW_BIT)
                {
                    if (sur->newSwapModel)
                    {
                        /* A back buffer is acquired if new swap model. */
                        veglCancelWindowBackBuffer(dpy, sur, &sur->backBuffer);

                        /* Clear back buffer. */
                        sur->backBuffer.context = gcvNULL;
                        sur->backBuffer.surface = gcvNULL;
                    }

                    /* Unbind this window. */
                    veglUnbindWindow(dpy, sur);
                    sur->bound = EGL_FALSE;
                }

                if (sur->renderMode > 0)
                {
                    if (sur->prevRenderTarget && sur->renderTarget &&
                        !gcoSURF_QueryFlags(sur->renderTarget,
                                            gcvSURF_FLAG_CONTENT_UPDATED) &&
                         gcoSURF_QueryFlags(sur->renderTarget,
                                            gcvSURF_FLAG_CONTENT_PRESERVED))
                    {
                        /* Copy previous to current. */
                        if (sur->openVG)
                        {
                            gcoSURF_Copy(sur->renderTarget,
                                         sur->prevRenderTarget);
                        }
                        else
                        {
#if gcdENABLE_3D
                            gcsSURF_VIEW prevRtView = {sur->prevRenderTarget, 0, 1};
                            gcsSURF_VIEW rtView = {sur->renderTarget, 0, 1};
                            gcoSURF_ResolveRect_v2(&prevRtView, &rtView, gcvNULL);
#endif
                        }
                    }

                    if (sur->renderTarget != gcvNULL)
                    {
                        /* Dereference external window back buffer. */
                        gcoSURF_Destroy(sur->renderTarget);
                        sur->renderTarget = gcvNULL;
                    }

                    if (sur->prevRenderTarget != gcvNULL)
                    {
                        /* Dereference external window back buffer. */
                        gcoSURF_Destroy(sur->prevRenderTarget);
                        sur->prevRenderTarget = gcvNULL;
                    }

                    /* Sync drawable with renderTarget. */
                    sur->drawable.rtHandle = gcvNULL;
                    sur->drawable.prevRtHandle = gcvNULL;
                }

                /* Dereference the current draw surface. */
                veglDereferenceSurface(thread, dpy, sur, EGL_FALSE);

                /* If the surface has been destoried then free the struct .*/
                if (sur->reference == gcvNULL)
                {
                    gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, sur));
                }

                current->draw = gcvNULL;
            }

            if (current->read != gcvNULL)
            {
                VEGLSurface sur = current->read;

                /* Dereference the current read surface. */
                veglDereferenceSurface(thread, dpy, sur, EGL_FALSE);

                /* If the surface has been destoried then free the struct .*/
                if (sur->reference == gcvNULL)
                {
                    gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, sur));
                }

                current->read = gcvNULL;
            }

            /* Unbind thread from context. */
            current->thread = gcvNULL;

            /* Set context to EGL_NO_CONTEXT. */
            thread->context = EGL_NO_CONTEXT;

            switch(thread->api)
            {
            case EGL_OPENGL_ES_API:
                thread->esContext = EGL_NO_CONTEXT;
                break;
            case EGL_OPENVG_API:
                thread->vgContext = EGL_NO_CONTEXT;
                break;
            case EGL_OPENGL_API:
                thread->glContext = EGL_NO_CONTEXT;
                break;
            }

            /*
             * If this context was requested to be deleted,
             * now it can be deleted.
             */
            if (current->deleteReq)
            {
                gcmTRACE(gcvLEVEL_INFO,
                         "%s(%d): veglMakeCurren delete defer freed context.",
                         __FUNCTION__, __LINE__);

                _DestroyContext(thread, dpy, current);
            }
        }

        /* Uninstall the owner thread */
        dpy->ownerThread = gcvNULL;

        /* Success. */
        veglSetEGLerror(thread,  EGL_SUCCESS);
        gcmFOOTER_ARG("%d", EGL_TRUE);
        return EGL_TRUE;
    }

    /* Test for EGL_KHR_surfaceless_context */
    if (ctx && !draw && !read)
    {
        switch (ctx->api)
        {
        case EGL_OPENGL_ES_API:
            break;

        default:
            /* Do not support surfaceless context for other API. */
            veglSetEGLerror(thread, EGL_BAD_MATCH);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }

    /* OpenGL ES didn't support make current w/o default framebuffer */
    else if (!(ctx && draw && read))
    {
        veglSetEGLerror(thread, EGL_BAD_MATCH);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Test if surface is locked. */
    if ((draw && draw->locked) || (read && read->locked))
    {
        veglSetEGLerror(thread,  EGL_BAD_ACCESS);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Check if context is current to other thread. */
    if ((ctx->thread != gcvNULL) && (ctx->thread != thread))
    {
        /* Bad access. */
        veglSetEGLerror(thread, EGL_BAD_ACCESS);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (draw != gcvNULL)
    {
        VEGLContext context = (VEGLContext) dpy->contextStack;

        while (context != gcvNULL)
        {
            if (context != ctx)
            {
                if ((context->draw == draw) || (context->read == draw) ||
                    (context->draw == read) || (context->read == read))
                {
                    if ((context->thread != gcvNULL) &&
                        (context->thread != thread))
                    {
                        /* Surface is current to other thread. */
                        veglSetEGLerror(thread, EGL_BAD_ACCESS);
                        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                    }
                }
            }

            context = (VEGLContext) context->resObj.next;
        }
    }

    /*
     * Step 1: unlink old draw and read for new context.
     * link with new read and draw
     */
    do
    {
        if (ctx->draw != draw)
        {
#if gcdENABLE_3D
            if (ctx->draw != gcvNULL &&
                ctx->draw->texBinder != gcvNULL &&
                ctx->draw->renderTarget != gcvNULL)
            {
                gcsSURF_VIEW rtView = {ctx->draw->renderTarget, 0, 1};
                gcsSURF_VIEW texView = {ctx->draw->texBinder, 0, 1};
                /*
                 * Resolve surface to texture when surface becomes not current.
                 * During resolve, we do not need flip.
                 */

                /* Sync the render target content to texture. */
                if (gcmIS_ERROR(gcoSURF_ResolveRect_v2(&rtView, &texView, gcvNULL)))
                {
                    /* Resolve failed, set error enum. */
                    veglSetEGLerror(thread, EGL_BAD_SURFACE);
                }

                /* Reset texture orientation. */
                gcmVERIFY_OK(gcoSURF_SetOrientation(
                    ctx->draw->texBinder,
                    gcvORIENTATION_TOP_BOTTOM
                    ));

                if (thread->error != EGL_SUCCESS)
                {
                    gcmONERROR(gcvSTATUS_INVALID_REQUEST);
                }
            }
#endif

            if (ctx->draw != gcvNULL)
            {
                VEGLSurface sur = ctx->draw;

                if (sur->type & EGL_WINDOW_BIT)
                {
                    if (sur->newSwapModel)
                    {
                        /* A back buffer is acquired if new swap model. */
                        veglCancelWindowBackBuffer(dpy, sur, &sur->backBuffer);

                        /* Clear back buffer. */
                        sur->backBuffer.context = gcvNULL;
                        sur->backBuffer.surface = gcvNULL;
                    }

                    /* Unbind this window. */
                    veglUnbindWindow(dpy, sur);
                    sur->bound = EGL_FALSE;
                }

                if (sur->renderMode > 0)
                {
                    if (sur->prevRenderTarget && sur->renderTarget &&
                        !gcoSURF_QueryFlags(sur->renderTarget,
                                            gcvSURF_FLAG_CONTENT_UPDATED) &&
                         gcoSURF_QueryFlags(sur->renderTarget,
                                            gcvSURF_FLAG_CONTENT_PRESERVED))
                    {
                        /* Copy previous to current. */
                        if (sur->openVG)
                        {
                            gcoSURF_Copy(sur->renderTarget,
                                         sur->prevRenderTarget);
                        }
                        else
                        {
#if gcdENABLE_3D
                            gcsSURF_VIEW prevRtView = {sur->prevRenderTarget, 0, 1};
                            gcsSURF_VIEW rtView = {sur->renderTarget, 0, 1};
                            gcoSURF_ResolveRect_v2(&prevRtView, &rtView, gcvNULL);
#endif
                        }
                    }

                    if (sur->renderTarget != gcvNULL)
                    {
                        /* Dereference external window back buffer. */
                        gcoSURF_Destroy(sur->renderTarget);
                        sur->renderTarget = gcvNULL;
                    }

                    if (sur->prevRenderTarget != gcvNULL)
                    {
                        /* Dereference external previous window back buffer. */
                        gcoSURF_Destroy(sur->prevRenderTarget);
                        sur->prevRenderTarget = gcvNULL;
                    }

                    /* Sync drawable with renderTarget. */
                    sur->drawable.rtHandle = gcvNULL;
                    sur->drawable.prevRtHandle = gcvNULL;
                }

                /* Dereference the current draw surface. */
                veglDereferenceSurface(thread, dpy, sur, EGL_FALSE);

                /* If the surface has been destoried then free the struct .*/
                if (sur->reference == gcvNULL)
                {
                    gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, sur));
                }

                ctx->draw = EGL_NO_SURFACE;
            }

            /* Reference the new draw surface. */
            if (draw)
            {
                if (!veglReferenceSurface(thread, dpy, draw))
                {
                    /* Error. */
                    result = EGL_FALSE;
                    break;
                }
            }

            /* Set the new draw surface. */
            ctx->draw = draw;
        }

        if (ctx->read != read)
        {
            if (ctx->read != gcvNULL)
            {
                VEGLSurface sur = ctx->read;

                /* Dereference the current read surface. */
                veglDereferenceSurface(thread, dpy, sur, EGL_FALSE);

                /* If the surface has been destoried then free the struct .*/
                if (sur->reference == gcvNULL)
                {
                    gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, sur));
                }

                ctx->read = EGL_NO_SURFACE;
            }

            /* Reference the new read surface. */
            if (read)
            {
                if (!veglReferenceSurface(thread, dpy, read))
                {
                    /* Error. */
                    result = EGL_FALSE;
                    break;
                }
            }

            /* Set the new read surface. */
            ctx->read = read;
        }

        /* Success. */
        result = EGL_TRUE;
    }
    while (gcvFALSE);

    if (!result)
    {
        veglSetEGLerror(thread,  EGL_BAD_PARAMETER);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    switch(ctx->api)
    {
    case EGL_OPENGL_API:
        current = thread->glContext;
        break;
    case EGL_OPENGL_ES_API:
        current = thread->esContext;
        break;
    case EGL_OPENVG_API:
        current = thread->vgContext;
        break;
    default:
        current = gcvNULL;
        gcmTRACE(gcvLEVEL_ERROR, "%s(%d): veglMakeCurrent bad current rendering API.",__FUNCTION__, __LINE__);
        break;
    }

    /* Step 2: flush and unlink current context from thread. */
    if (current)
    {
        /* Flush current context of current client API */
        gcmVERIFY(_FlushApiContext(thread,
                                 current,
                                 current->context));
        /* Remove context. */
        gcmVERIFY(_ApiLoseCurrent(thread, current));

        current->thread = gcvNULL;

        if (current->deleteReq && current != ctx)
        {
            gcmTRACE(gcvLEVEL_INFO,
                     "%s(%d): veglMakeCurrent delete defer freed context.",
                     __FUNCTION__, __LINE__);

            _DestroyContext(thread, dpy, current);
        }
    }

    /* Step 3: process draw and read. */
    if (draw && (draw->type & EGL_WINDOW_BIT))
    {
        /* Bind native window for rendering. */
        result = veglBindWindow(dpy, draw, &draw->renderMode);

        if (!result)
        {
            veglSetEGLerror(thread, EGL_BAD_NATIVE_WINDOW);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        draw->bound = EGL_TRUE;

        /* Must use new swap model for direct rendering. */
        draw->newSwapModel = (draw->renderMode > 0);

#if defined(ANDROID)
        /* Force new swap model on android platform. */
        draw->newSwapModel = gcvTRUE;
#endif

        if (draw->newSwapModel)
        {
            /*
             * Get window buffer if not acquired.
             * If context is not switched, window is already connected.
             * Or if hwc obtains buffers when no current surface, the window
             * buffer should be also acquired already.
             */
            if (draw->backBuffer.surface == gcvNULL)
            {
                result = veglGetWindowBackBuffer(dpy, draw, &draw->backBuffer);

                if (!result)
                {
                    /* FIXME: Unbind window here?? */
                    veglSetEGLerror(thread, EGL_BAD_NATIVE_WINDOW);
                    gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                }
            }
        }

        if (draw->renderMode > 0)
        {
            /* Switch render target to external surface. */
            if (draw->renderTarget != gcvNULL)
            {
                /* Destroy internal render target. */
                gcmVERIFY_OK(gcoSURF_Destroy(draw->renderTarget));
            }

            /* Reference external surface. */
            draw->renderTarget = draw->backBuffer.surface;

            if (draw->renderTarget != gcvNULL)
            {
                /* Inc reference count. */
                gcmVERIFY_OK(gcoSURF_ReferenceSurface(draw->renderTarget));
            }

            if (draw->prevRenderTarget)
            {
                /* Destroy previous external render target. */
                gcmVERIFY_OK(gcoSURF_Destroy(draw->prevRenderTarget));
                draw->prevRenderTarget = gcvNULL;
            }

            /* Sync drawable with renderTarget. */
            draw->drawable.rtHandle = draw->renderTarget;
            draw->drawable.prevRtHandle = gcvNULL;
        }
        else if (draw->renderTarget == gcvNULL)
        {
            /* Defer allocate render target Create render target. */
            result = veglCreateRenderTarget(thread, draw);

            if (!result)
            {
                veglSetEGLerror(thread, EGL_BAD_ALLOC);
                gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
            }
        }

        /* The renderTarget is not determined. Check lockBuffer. */
        if (draw->lockBufferMirror != gcvNULL)
        {
#if gcdENABLE_3D
            gcsSURF_VIEW mirrorView = {draw->lockBufferMirror, 0, 1};
            gcsSURF_VIEW rtView = {draw->renderTarget, 0, 1};

            /* Resolve to renderTarget. */
            status = gcoSURF_ResolveRect_v2(&mirrorView, &rtView, gcvNULL);

            if (gcmIS_ERROR(status))
            {
                veglSetEGLerror(thread,  EGL_BAD_ACCESS);
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }
#endif

            /* Destroy the lockBuffer mirror. */
            gcmVERIFY_OK(gcoSURF_Destroy(draw->lockBufferMirror));
            draw->lockBufferMirror = gcvNULL;

            gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvFALSE));
        }

        /* Detect native window resize. */
        result = veglGetWindowSize(dpy, draw, &width, &height);

        if (result == EGL_FALSE)
        {
            veglSetEGLerror(thread, EGL_BAD_NATIVE_WINDOW);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        if ((width  != draw->config.width) ||
            (height != draw->config.height))
        {
            /* Native window resized. */
            if (EGL_SUCCESS != veglResizeSurface(dpy, draw,
                                                 (gctUINT) width,
                                                 (gctUINT) height))
            {
                veglSetEGLerror(thread, EGL_BAD_SURFACE);
                result = EGL_FALSE;
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }
        }
    }

    /* Step 4: flush new context with new read and draw. */
    if (draw && read)
    {
        /* Normal make current. */
        if (!_ApiMakeCurrent(thread, ctx, &draw->drawable, &read->drawable))
        {
            veglSetEGLerror(thread, EGL_BAD_SURFACE);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }
    else
    {
        /* Make current for EGL_KHR_surfaceless_context. */
        if (!_ApiMakeCurrent(thread, ctx, gcvNULL, gcvNULL))
        {
            veglSetEGLerror(thread, EGL_BAD_SURFACE);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }

    /* step 5. link new context. */
    switch(ctx->api)
    {
    case EGL_OPENGL_ES_API:
        thread->esContext = ctx;
        break;
    case EGL_OPENVG_API:
        thread->vgContext = ctx;
        break;
    case EGL_OPENGL_API:
        thread->glContext = ctx;
        break;
    }

    /* if ctx which is being made current is current rendering API,
    ** update shortcut of current rendering API's current context.
    */
    if (ctx->api == thread->api)
    {
        thread->context = ctx;
    }

    /* Set the new current thread. */
    ctx->thread = thread;

#ifdef EGL_API_DRI
    if (draw && (draw->type & EGL_WINDOW_BIT))
    {
        struct eglBackBuffer backBuffer;

        /* Borrow it from window back buffer. */
        veglGetWindowBackBuffer(dpy, draw, &backBuffer);

        gcoOS_MakeCurrent(
            dpy->localInfo,
            (HALNativeWindowType) draw->hwnd,
            (HALNativeWindowType) read->hwnd,
            ctx,
            backBuffer.surface
            );

        /* Cancel the back buffer, actually does nothing for DRI. */
        veglCancelWindowBackBuffer(dpy, draw, &backBuffer);
    }
#endif

    if (thread->dump != gcvNULL)
    {
        /* Start a new frame. */
        gcmVERIFY_OK(gcoDUMP_FrameBegin(thread->dump));
    }

    /* Install the owner thread */
    dpy->ownerThread = gcoOS_GetCurrentThreadID();

    /* Success. */
    veglSetEGLerror(thread,  EGL_SUCCESS);

    gcmFOOTER_ARG("%d", EGL_TRUE);

    return EGL_TRUE;

OnError:
    gcmFOOTER_ARG("%d", EGL_FALSE);

    return EGL_FALSE;
}

EGLAPI EGLBoolean EGLAPIENTRY
eglMakeCurrent(
    EGLDisplay Dpy,
    EGLSurface Draw,
    EGLSurface Read,
    EGLContext Ctx
    )
{
    EGLBoolean ret;
    VEGL_TRACE_API(MakeCurrent)(Dpy, Draw, Read, Ctx);

    gcmDUMP_API("${EGL eglMakeCurrent 0x%08X 0x%08X 0x%08X 0x%08X}",
                Dpy, Draw, Read, Ctx);

    ret = veglMakeCurrent(Dpy, Draw, Read, Ctx);
    return ret;
}

EGLBoolean
veglReleaseThread(
    VEGLThreadData Thread
    )
{
    gcmHEADER_ARG("Thread=%p", Thread);

    if (Thread->esContext != gcvNULL)
    {
        veglBindAPI(EGL_OPENGL_ES_API);

        /* Unbind the context. */
        veglMakeCurrent(Thread->esContext->display,
                       EGL_NO_SURFACE,
                       EGL_NO_SURFACE,
                       EGL_NO_CONTEXT);

        Thread->esContext = gcvNULL;

    }

    if (Thread->vgContext != gcvNULL)
    {
        veglBindAPI(EGL_OPENVG_API);

        /* Unbind the context. */
        veglMakeCurrent(Thread->vgContext->display,
                       EGL_NO_SURFACE,
                       EGL_NO_SURFACE,
                       EGL_NO_CONTEXT);

        Thread->vgContext = gcvNULL;

    }

    if (Thread->glContext != gcvNULL)
    {
        veglBindAPI(EGL_OPENGL_API);

        /* Unbind the context. */
        veglMakeCurrent(Thread->glContext->display,
                       EGL_NO_SURFACE,
                       EGL_NO_SURFACE,
                       EGL_NO_CONTEXT);

        Thread->glContext = gcvNULL;

    }

    Thread->context = gcvNULL;

    /* Set API to EGL_OPENGL_ES_API */
    veglBindAPI(EGL_OPENGL_ES_API);

    /* Success. */
    gcmFOOTER_ARG("%d", EGL_TRUE);
    return EGL_TRUE;
}

EGLAPI EGLBoolean EGLAPIENTRY
eglReleaseThread(
    void
    )
{
    EGLBoolean ret;
    VEGLThreadData thread;

    VEGL_TRACE_API(ReleaseThread)();

    gcmDUMP_API("${EGL eglReleaseThread}");

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );
        return EGL_FALSE;
    }

    ret = veglReleaseThread(thread);

    /* Destroy thread data. */
    gcoOS_FreeThreadData();

    return ret;
}

void *
_GetCurrentAPIContext(
    EGLenum Api
    )
{
    VEGLThreadData thread;
    void * context;
    VEGLContext veglContext;

    gcmHEADER();

    thread = veglGetThreadData();

    if (thread == gcvNULL)
    {
        context = gcvNULL;
    }
    else
    {
        switch (Api)
        {
        case EGL_OPENGL_ES_API:
            veglContext = thread->esContext;
            break;
        case EGL_OPENVG_API:
            veglContext = thread->vgContext;
            break;
        case EGL_OPENGL_API:
            veglContext = thread->glContext;
            break;
        default:
            veglContext = gcvNULL;
            gcmTRACE(gcvLEVEL_ERROR,
                     "%s(%d): bad rendering API parameters",
                     __FUNCTION__, __LINE__);
            break;
        }

        if (veglContext)
        {
            context = veglContext->context;
        }
        else
        {
            context = gcvNULL;
        }
    }

    gcmFOOTER_ARG("0x%x", context);
    return context;
}

void
_SetCurrentAPIContext(
    EGLenum Api,
    void* context
    )
{
    VEGLThreadData thread;
    VEGLContext veglContext;

    gcmHEADER();

    thread = veglGetThreadData();

    if (thread)
    {
        switch (Api)
        {
        case EGL_OPENGL_ES_API:
            veglContext = thread->esContext;
            break;
        case EGL_OPENVG_API:
            veglContext = thread->vgContext;
            break;
        case EGL_OPENGL_API:
            veglContext = thread->glContext;
            break;
        default:
            veglContext = gcvNULL;
            gcmTRACE(gcvLEVEL_ERROR,
                     "%s(%d): bad rendering API parameters",
                     __FUNCTION__, __LINE__);
            break;
        }

        if (veglContext)
        {
            veglContext->context = context;
        }
    }

    gcmFOOTER_ARG("0x%x", context);
}

static void
_SyncPixmap(
    VEGLThreadData Thread,
    VEGLDisplay Dpy
    )
{
#if gcdENABLE_3D
    VEGLSurface surface = gcvNULL;

    do
    {
        gcsSURF_VIEW rtView = {gcvNULL, 0, 1};
        gcsSURF_VIEW pixmapView = {gcvNULL, 0, 1};

        if ((Thread->context == gcvNULL)
        ||  (Thread->context->draw == gcvNULL)
        ||  (Thread->context->draw->renderTarget == gcvNULL)
        ||  !(Thread->context->draw->type & EGL_PIXMAP_BIT)
        )
        {
            /* No pixmap. */
            return;
        }

        /* Get current surface. */
        surface = Thread->context->draw;
        rtView.surf = surface->renderTarget;
        pixmapView.surf = surface->pixmapSurface;

        /* Resolve from RT to pixmap (temp or wrapper). */
        if (gcmIS_ERROR(gcoSURF_ResolveRect_v2(&rtView, &pixmapView, gcvNULL)))
        {
            /* Resolve failed? */
            break;
        }

        /*
         * Do a stalled commit here as the
         * next operation could be a CPU operation.
         */
        gcoHAL_Commit(gcvNULL, gcvTRUE);

        /* Wait native pixmap. */
        veglSyncToPixmap(surface->pixmap, surface->pixInfo);
    }
    while (gcvFALSE);
#endif  /* gcdENABLE_3D */
}

static void
_SyncImage(
    VEGLThreadData Thread,
    VEGLDisplay Dpy
    )
{
#if gcdENABLE_3D
    VEGLImage       image;
    khrEGL_IMAGE_PTR   khrImage;

    gcmHEADER_ARG("Dpy=0x%08x",Dpy);

    if (!Dpy)
    {
        gcmFOOTER_NO();
        return;
    }

    VEGL_LOCK_DISPLAY_RESOURCE(Dpy);

    image = Dpy->imageStack;

    while (image)
    {
        if (image->signature != EGL_IMAGE_SIGNATURE)
        {
            image = image->next;
            continue;
        }

        khrImage = (khrEGL_IMAGE_PTR)&(image->image);

        /* Lock the image mutex. */
        gcoOS_AcquireMutex(gcvNULL, khrImage->mutex, gcvINFINITE);

        if ((khrImage->surface != gcvNULL) &&
            (khrImage->srcSurface != gcvNULL) &&
            (khrImage->surface != khrImage->srcSurface))
        {
            gcsSURF_VIEW srcView = {khrImage->srcSurface, 0, 1};
            gcsSURF_VIEW surfView = {khrImage->surface, 0, 1};

            gceORIENTATION srcOrient, dstOrient;

            gcoSURF_QueryOrientation(khrImage->srcSurface,&srcOrient);
            gcoSURF_QueryOrientation(khrImage->surface,&dstOrient);
            gcoSURF_SetOrientation(khrImage->surface,srcOrient);

            gcoSURF_ResolveRect_v2(&srcView, &surfView, gcvNULL);

            gcoSURF_SetOrientation(khrImage->surface,dstOrient);

            gcoSURF_Destroy(khrImage->srcSurface);
            khrImage->srcSurface = gcvNULL;
        }

        image = image->next;

        /* Release the image mutex. */
        gcoOS_ReleaseMutex(gcvNULL, khrImage->mutex);
    }
    VEGL_UNLOCK_DISPLAY_RESOURCE(Dpy);

    gcmFOOTER_NO();
#endif
}

/*
 * Sync EGL objects to native objects, includes,
 * 1. native pixmap of PixmapSurface
 * 2. native object of EGLImage source sibling.
 *
 * Notice:
 * This function is asyncrhonized.
 */
void
veglSyncNative(
    IN VEGLThreadData Thread,
    VEGLDisplay Dpy
    )
{
    if ((Thread == gcvNULL) ||
        (Thread->context == gcvNULL) ||
        (Dpy == gcvNULL))
    {
        /* No context. */
        return;
    }

    /* Sync pixmap-surface to native pixmap. */
    _SyncPixmap(Thread, Dpy);

    /* Sync EGLImage source to native objects. */
    _SyncImage(Thread, Dpy);
}

/*
** TODO: need refine about looking up current display
*/
/* Callback function exported for client. */
void
_SyncNative(
    void
    )
{
    VEGLThreadData thread       = gcvNULL;
    VEGLDisplay dpy             = gcvNULL;

    thread = veglGetThreadData();

    if ((thread == gcvNULL) ||
        (thread->context == gcvNULL))
    {
        /* No context. */
        return;
    }

    /* Get current display. */
    /* dpy = (VEGLDisplay) gcoOS_GetPLSValue(gcePLS_VALUE_EGL_DISPLAY_INFO); */
    dpy = (VEGLDisplay) thread->context->display;

    /* Call actual function. */
    veglSyncNative(thread, dpy);
}

EGLBoolean veglWaitClient(
    void
    )
{
    VEGLThreadData thread;
    EGLBoolean result = EGL_TRUE;

    gcmHEADER();

    thread = veglGetThreadData();

    if (thread == gcvNULL)
    {
        result = EGL_FALSE;
    }
    else
    {
        result = _Flush(thread);

        if (result)
        {
            _SyncNative();
        }

        gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvTRUE));
    }

    gcmFOOTER_ARG("%d", result);
    return result;
}

EGLAPI EGLBoolean EGLAPIENTRY
eglWaitClient(
    void
    )
{
    EGLBoolean ret;

    VEGL_TRACE_API(WaitClient)();

    gcmDUMP_API("${EGL eglWaitClient}");

    ret = veglWaitClient();

    return ret;
}

EGLAPI EGLBoolean EGLAPIENTRY
eglWaitGL(
    void
    )
{
    EGLenum api;
    EGLBoolean result;

    gcmHEADER();

    gcmDUMP_API("${EGL eglWaitGL}");
    VEGL_TRACE_API(WaitGL)();

    /* Backwards compatibility. */
    api = veglQueryAPI();
    veglBindAPI(EGL_OPENGL_ES_API);

    result = veglWaitClient();

    veglBindAPI(api);

    gcmFOOTER_ARG("%d", result);
    return result;
}

EGLenum
_BindTexImage(
    VEGLThreadData Thread,
    gcoSURF Surface,
    EGLenum Format,
    EGLBoolean Mipmap,
    EGLint Level,
    EGLint Width,
    EGLint Height,
    gcoSURF *BindTo
    )
{
    veglDISPATCH * dispatch = _GetDispatch(Thread, gcvNULL);

    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_EGL_CONTEXT,
                  "%s(%d): Thread=0x%08x,Surface=0x%08x,Format=0x%04x "
                  "Mipmap=%d,Level=0x%04x Width=%d Height=%d",
                  __FUNCTION__, __LINE__,
                  Thread, Surface, Format, Mipmap, Level, Width, Height);

    if ((dispatch == gcvNULL)
    ||  (dispatch->bindTexImage == gcvNULL)
    )
    {
        return EGL_BAD_ACCESS;
    }

    return (*dispatch->bindTexImage)((void*)Surface, Format, Mipmap, Level, Width, Height, (void**)BindTo);
}

EGLenum
_CreateImageTexture(
    VEGLThreadData Thread,
    VEGLContext Context,
    EGLenum Target,
    gctINT Texture,
    gctINT Level,
    gctINT Depth,
    gctPOINTER Image
    )
{
    veglDISPATCH * dispatch = _GetDispatch(Thread, Context);
    void* ApiContext = Context->context;

    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_EGL_CONTEXT,
                  "%s(%d): Thread=0x%08x,Target=0x%04x,Texture=0x%08x",
                  __FUNCTION__, __LINE__, Thread, Target, Texture);
    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_EGL_CONTEXT,
                  "Level=%d,Depth=%d,Image=0x%08x", Level, Depth, Image);

    if ((dispatch == gcvNULL)
    ||  (dispatch->createImageTexture == gcvNULL)
    )
    {
        return EGL_BAD_ACCESS;
    }

    return (*dispatch->createImageTexture)(ApiContext,
                                           Target,
                                           Texture,
                                           Level,
                                           Depth,
                                           Image);
}

EGLenum
_CreateImageFromRenderBuffer(
    VEGLThreadData Thread,
    VEGLContext Context,
    gctUINT Renderbuffer,
    gctPOINTER Image
    )
{
    veglDISPATCH * dispatch = _GetDispatch(Thread, Context);
    void* ApiContext = Context->context;

    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_EGL_CONTEXT,
                  "%s(%d): Thread=0x%08x, RenderBuffer=%u,Image=0x%08x",
                  __FUNCTION__, __LINE__,
                  Thread, Renderbuffer, Image);

    if ((dispatch == gcvNULL)
    ||  (dispatch->createImageRenderbuffer == gcvNULL)
    )
    {
        return EGL_BAD_ACCESS;
    }

    return (*dispatch->createImageRenderbuffer)(ApiContext, Renderbuffer, Image);
}

EGLenum
_CreateImageFromVGParentImage(
    VEGLThreadData  Thread,
    VEGLContext Context,
    unsigned int Parent,
    VEGLImage Image
    )
{
    veglDISPATCH * dispatch = _GetDispatch(Thread, Context);
    EGLenum status;
    gctPOINTER images = gcvNULL;
    int count = 0;

    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_EGL_CONTEXT,
                  "%s(%d): Thread=0x%08x,Parent=%u,Image=0x%08x",
                  __FUNCTION__, __LINE__,
                  Thread, Parent, Image);

    if ((dispatch == gcvNULL)
    ||  (dispatch->createImageParentImage == gcvNULL)
    )
    {
        return EGL_BAD_ACCESS;
    }

    status = (*dispatch->createImageParentImage)(Parent,
                                                 &images,
                                                 &count);

    if (count == 0)
    {
        return status;
    }

    /* Fill the VEGLImage list. */
    /* TOOD: Only copied the fist image from returned list?? */
    if ((Image != gcvNULL) && (images != gcvNULL))
    {
        khrEGL_IMAGE * khrImage = (khrEGL_IMAGE *) images;
        Image->image.magic      = khrImage->magic;
        Image->image.type       = khrImage->type;
        Image->image.surface    = khrImage->surface;
        Image->image.srcSurface = gcvNULL;
        Image->image.u.vgimage  = khrImage->u.vgimage;
    }

    if (images != gcvNULL)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, images));
    }

    return EGL_SUCCESS;
}
