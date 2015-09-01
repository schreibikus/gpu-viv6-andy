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


#ifndef __gc_egl_h_
#define __gc_egl_h_

#include <EGL/egl.h>
#include <EGL/eglext.h>

#if defined(LINUX)
/* Undef some badly named X defines. */
#   undef Status
#   undef Always
#   undef CurrentTime
#endif

#include "gc_hal.h"
#include "gc_hal_driver.h"
#include "gc_hal_engine.h"
#include "gc_egl_common.h"

/* Swap rectangle extension on android. */
#define VEGL_MAX_NUM_SURFACE_BUFFERS            8

/* Always Disable 2xAA EGL configs by default. */
#define VEGL_ENABLE_2xAA_CONFIG                 0

/* Max SwapBuffersRegion rectangle count. */
#define VEGL_MAX_SWAP_BUFFER_REGION_RECTS       10

/* Duplicate 8888 configs to ARGB and ABGR visual. */
#if defined(ANDROID)
#   define VEGL_DUPLICATE_ABGR_EGL_CONFIGS      1
#else
#   define VEGL_DUPLICATE_ABGR_EGL_CONFIGS      0
#endif

/* Enable 4444 and 5551 EGL configs. */
#if defined(ANDROID)
#   define VEGL_ENABLE_4444_5551_EGL_CONFIGS    0
#else
#   define VEGL_ENABLE_4444_5551_EGL_CONFIGS    1
#endif


#ifdef __cplusplus
extern "C" {
#endif

#define gcdZONE_EGL_API             (gcvZONE_API_EGL | (1 << 0))
#define gcdZONE_EGL_SURFACE         (gcvZONE_API_EGL | (1 << 1))
#define gcdZONE_EGL_CONTEXT         (gcvZONE_API_EGL | (1 << 2))
#define gcdZONE_EGL_CONFIG          (gcvZONE_API_EGL | (1 << 3))
#define gcdZONE_EGL_OS              (gcvZONE_API_EGL | (1 << 4))
#define gcdZONE_EGL_IMAGE           (gcvZONE_API_EGL | (1 << 5))
#define gcdZONE_EGL_SWAP            (gcvZONE_API_EGL | (1 << 6))
#define gcdZONE_EGL_INIT            (gcvZONE_API_EGL | (1 << 7))
#define gcdZONE_EGL_SYNC            (gcvZONE_API_EGL | (1 << 8))
#define glvZONE_EGL_COMPOSE         (gcvZONE_API_EGL | (1 << 9))
#define glvZONE_EGL_RENDER_THREAD   (gcvZONE_API_EGL | (1 << 10))


typedef struct eglDisplay       *VEGLDisplay;
typedef struct eglResObj        *VEGLResObj;
typedef struct eglContext       *VEGLContext;
typedef struct eglConfig        *VEGLConfig;
typedef struct eglSurface       *VEGLSurface;
typedef struct eglImage         *VEGLImage;
typedef struct eglSync          *VEGLSync;
typedef struct eglImageRef      *VEGLImageRef;
typedef struct eglThreadData    *VEGLThreadData;
typedef struct eglWorkerInfo    *VEGLWorkerInfo;
typedef struct eglBackBuffer    *VEGLBackBuffer;

/* Wraps up native display/window/pixmap variables/operations. */

typedef struct eglDisplayInfo   *VEGLDisplayInfo;
typedef struct eglWindowInfo    *VEGLWindowInfo;
typedef struct eglPixmapInfo    *VEGLPixmapInfo;

#define VEGL_DISPLAY(p)         ((VEGLDisplay) (p))
#define VEGL_RESOBJ(p)          ((VEGLResObj)  (p))
#define VEGL_CONTEXT(p)         ((VEGLContext) (p))
#define VEGL_CONFIG(p)          ((VEGLConfig)  (p))
#define VEGL_SURFACE(p)         ((VEGLSurface) (p))
#define VEGL_IMAGE(p)           ((VEGLImage)   (p))
#define VEGL_SYNC(p)            ((VEGLSync)    (p))

#define EGL_DISPLAY_SIGNATURE   gcmCC('E','G','L','D')
#define EGL_SURFACE_SIGNATURE   gcmCC('E','G','L','S')
#define EGL_CONTEXT_SIGNATURE   gcmCC('E','G','L','C')
#define EGL_IMAGE_SIGNATURE     gcmCC('E','G','L','I')
#define EGL_SYNC_SIGNATURE      gcmCC('E','G','L','Y')

#define veglUSE_HAL_DUMP        0

#define  __EGL_INVALID_CONFIG__ 0

#define MAJOR_API_VER(x) ((x) >> 4)
#define MINOR_API_VER(x) ((x) & 0xF)


struct eglResObj
{
    /* Signature. */
    gctUINT32                   signature;

    /* Next eglResObj */
    VEGLResObj                  next;
};

typedef void * EGLResObj;

struct eglBackBuffer
{
    gctPOINTER                  context;
    gcoSURF                     surface;
    gcsPOINT                    origin;
    gctBOOL                     flip;
};

struct eglWorkerInfo
{
    gctSIGNAL                   signal;
    gctSIGNAL                   targetSignal;

    /* Owner of this surface. */
    VEGLSurface                 draw;

    struct eglBackBuffer        backBuffer;

    gctINT                      numRects;
    gctINT                      rects[VEGL_MAX_SWAP_BUFFER_REGION_RECTS * 4];

    /* Used by eglDisplay worker list. */
    VEGLWorkerInfo              prev;
    VEGLWorkerInfo              next;
};

#if defined(ANDROID)
#   define EGL_WORKER_COUNT     24
#elif defined(__QNXNTO__)
#   define EGL_WORKER_COUNT     1
#else
#   define EGL_WORKER_COUNT     4
#endif

typedef void (* VEGL_ESPrivDestructor) (gctPOINTER priv);

struct eglThreadData
{
    /* gcoDUMP object. */
    gcoDUMP                     dump;

    /* Last known error code. */
    EGLenum                     error;

    /* Current API. */
    EGLenum                     api;

    /* Hardware capabilities. */
    gceCHIPMODEL                chipModel;
    EGLint                      maxWidth;
    EGLint                      maxHeight;
    EGLint                      maxSamples;
    gctBOOL                     openVGpipe;

    /* fastMSAA and small MSAA only support 4x mode */
    gctBOOL                     fastMSAA;

    /* Current context of current rendering API
    ** Shortcut to below three sorts of current contexts.
    */
    VEGLContext                 context;
    /* Current context of OES API*/
    VEGLContext                 esContext;
    /* Current context of VG API */
    VEGLContext                 vgContext;
    /* Current context of GL API */
    VEGLContext                 glContext;

    /* Private thread data pointer */
    gctPOINTER                  esPrivate;
    VEGL_ESPrivDestructor       destroyESPrivate;

    /* GL_OES_EGL_image */
    EGL_PROC                    imageTargetTex2DFunc[3];
    EGL_PROC                    imageTargetRBStorageFunc[3];
    /* GL_EXT_multi_draw_arrays */
    EGL_PROC                    multiDrawArraysFunc[3];
    EGL_PROC                    multiDrawElementsFunc[3];
    /* GL_OES_mapbuffer */
    EGL_PROC                    getBufferPointervFunc[3];
    EGL_PROC                    mapBufferFunc[3];
    EGL_PROC                    unmapBufferFunc[3];
    /* GL_EXT_discard_framebuffer */
    EGL_PROC                    discardFramebuffer[3];
    /* GL_EXT_multisampled_render_to_texture */
    EGL_PROC                    renderbufferStorageMultisampleFunc[3];
    EGL_PROC                    framebufferTexture2DMultisampleFunc[3];
    /* GL_OES_get_program_binary */
    EGL_PROC                    getProgramBinaryFunc[3];
    EGL_PROC                    programBinaryFunc[3];
    /* GL_VIV_direct_texture */
    EGL_PROC                    texDirectFunc[3];
    EGL_PROC                    texDirectInvalidateFunc[3];
    EGL_PROC                    texDirectMapFunc[3];
    EGL_PROC                    texDirectTiledMapFunc[3];

    gctINT32                    chipCount;
    gcsHAL_LIMITS               chipLimits[gcdCHIP_COUNT];

    struct eglWorkerInfo *      worker;

#if gcdGC355_MEM_PRINT
#ifdef LINUX
    gctINT                      fbMemSize;
#endif
#endif
};

struct eglImageRef
{
    NativePixmapType            pixmap;
    VEGLPixmapInfo              pixInfo;
    gcoSURF                     surface;
#ifdef EGL_API_DRI
    gcoSURF                     tmpSurface;
#endif
    VEGLImageRef                next;
};

struct eglDisplay
{
    /* Next EGLDisplay. */
    VEGLDisplay                 next;

    /* Platform type, '0' for default (ie, by eglGetDisplay). */
    EGLenum                     platform;

    /* Native screen and native display. */
    void *                      nativeScreen;
    void *                      nativeDisplay;

    /* Handle to device context. */
    NativeDisplayType           hdc;

    /* Local info. */
    gctPOINTER                  localInfo;

    gctBOOL                     releaseDpy;

    /* Process handle. */
    gctHANDLE                   process;
    gctHANDLE                   ownerThread;

    /* Number of configurations. */
    EGLint                      configCount;

    /* Pointer to configurations. */
    VEGLConfig                  config;

    /* access lock to access this display */
    gctPOINTER                  accessMutex;

    /* resource lock */
    gctPOINTER                  resourceMutex;

    /* Allocated resources. */
    VEGLSurface                 surfaceStack;
    VEGLContext                 contextStack;
    VEGLImage                   imageStack;
    VEGLImageRef                imageRefStack;
    VEGLSync                    syncStack;

    /* Initialize flag. */
    gctBOOL                     initialized;

    /* Worker thread for copying data. */
    gctHANDLE                   workerThread;
    gctSIGNAL                   startSignal;
    gctSIGNAL                   stopSignal;
    gctPOINTER                  suspendMutex;

    /* EGL Blob Cache Functions */
    EGLGetBlobFuncANDROID       blobCacheGet;
    EGLSetBlobFuncANDROID       blobCacheSet;

    /* Sentinel for a list of active workers that are queued up. */
    struct eglWorkerInfo        workerSentinel;
};

struct eglConfig
{
    EGLint                      bufferSize;
    EGLint                      configBufferSize;
    EGLint                      alphaSize;
    EGLint                      blueSize;
    EGLint                      greenSize;
    EGLint                      redSize;
    EGLint                      depthSize;
    EGLint                      stencilSize;
    EGLenum                     configCaveat;
    EGLint                      configId;
    EGLBoolean                  defaultConfig;
    EGLBoolean                  nativeRenderable;
    EGLint                      nativeVisualType;
    EGLint                      samples;
    EGLint                      sampleBuffers;
    EGLenum                     surfaceType;
    EGLBoolean                  bindToTetxureRGB;
    EGLBoolean                  bindToTetxureRGBA;
    EGLint                      luminanceSize;
    EGLint                      alphaMaskSize;
    EGLenum                     colorBufferType;
    EGLenum                     renderableType;
    EGLenum                     conformant;
    EGLenum                     matchFormat;
    EGLint                      matchNativePixmap;
    EGLint                      width;
    EGLint                      height;
    EGLint                      level;
    EGLint                      maxSwapInterval;
    EGLenum                     transparentType;
    EGLint                      transparentRedValue;
    EGLint                      transparentGreenValue;
    EGLint                      transparentBlueValue;
    /* Special config on android to return BGRA visual. */
    EGLBoolean                  swizzleRB;
    /* EGL_ANDROID_recordable extension. */
    EGLBoolean                  recordableConfig;
};

enum _VEGL_DIRECT_RENDERING_MODE
{
    VEGL_INDIRECT_RENDERING = 0,

    /* no-resolve, with tile status, but fc filled after swap. */
    VEGL_DIRECT_RENDERING_FCFILL,

    /* special no-resolve, do not allocate tile status. */
    VEGL_DIRECT_RENDERING_NOFC,

    /* no-resolve, with tile status, no compression. */
    VEGL_DIRECT_RENDERING_FC_NOCC,

    /* full no-resolve mode, tile status, compression. */
    VEGL_DIRECT_RENDERING
};

struct eglSurface
{
    /* Should be the first member */
    struct eglResObj            resObj;

    /*
     * Set if VG pipe is present and this surface was created when the current
     * API was set to OpenVG
     */
    gctBOOL                     openVG;

    /* Render target. */
    gcoSURF                     renderTarget;
    gceSURF_FORMAT              renderTargetFormat;
    gctBOOL                     rtFormatChanged;

    /* Previous render target. */
    gcoSURF                     prevRenderTarget;

    /* Depth buffer. */
    gcoSURF                     depthBuffer;
    gceSURF_FORMAT              depthFormat;

    /* For "render to texture" To bind texture. */
    gcoSURF                     texBinder;

    /* Surface Config Data. */
    struct eglConfig            config;

    /* Reference counter. */
    gcsATOM_PTR                 reference;

    /* Surface type. */
    EGLint                      type;
    EGLenum                     buffer;
    gceSURF_COLOR_TYPE          colorType;
    EGLint                      swapBehavior;
    EGLint                      multisampleResolve;
    EGLint                      bufferAge;
    EGLBoolean                  protectedContent;

    /* VG attribute */
    EGLenum                     vgColorSpace;
    EGLenum                     vgAlphaFormat;


    /* Window attributes. */
    NativeWindowType            hwnd;
    EGLBoolean                  bound;

    struct eglBackBuffer        backBuffer;

    /*
     * Wrap up native window specific variables/operations.
     * Do not access/modify outside egl platform layer.
     */
    VEGLWindowInfo              winInfo;


    /* Pixmap attributes. */
    NativePixmapType            pixmap;
    gcoSURF                     pixmapSurface;

    /* Wrap up native pixmap specific variables/operations. */
    VEGLPixmapInfo              pixInfo;

    /* PBuffer attributes. */
    EGLBoolean                  largestPBuffer;
    EGLBoolean                  mipmapTexture;
    EGLint                      mipmapLevel;
    EGLenum                     textureFormat;
    EGLenum                     textureTarget;


    /* Lock surface attributes. */
    EGLBoolean                  locked;
    gcoSURF                     lockBuffer;
    void *                      lockBits;
    gceSURF_FORMAT              lockBufferFormat;
    gctINT                      lockBufferStride;
    EGLBoolean                  lockPreserve;

    /*
     * To temporarily preserve the lockBuffer pixels when there's no
     * renderTarget. Render target is defered allocated for performance.
     */
    gcoSURF                     lockBufferMirror;

    /* Render mode and swap model, valid for window surface for now. */
    EGLint                      renderMode;
    EGLBoolean                  newSwapModel;

    /* Image swap workers. */
    struct eglWorkerInfo        workers[EGL_WORKER_COUNT];

    gctUINT                     totalWorkerCount;
    gctUINT                     freeWorkerCount;

    VEGLWorkerInfo              availableWorkers;
    VEGLWorkerInfo              lastSubmittedWorker;
    gctPOINTER                  workerMutex;

    /* Signaled when at least one worker is available. */
    gctSIGNAL                   workerAvaiableSignal;

    /* Signaled when all workers are free, ie, no worker in workerThread. */
    gctSIGNAL                   workerDoneSignal;

    EGLint                      clipRects[VEGL_MAX_SWAP_BUFFER_REGION_RECTS * 4];


#if defined(ANDROID)
    /* EGL_ANDROID_set_swap_rectangle. */
    struct
    {
        /* Current swap rectangle. */
        EGLint                  x;
        EGLint                  y;
        EGLint                  width;
        EGLint                  height;

#if gcdSUPPORT_SWAP_RECTANGLE
        gctUINT                 count;

        /* Accumulated swap rectangles. */
        gcsRECT                 rects[VEGL_MAX_NUM_SURFACE_BUFFERS];
        gctPOINTER              buffers[VEGL_MAX_NUM_SURFACE_BUFFERS];
#   endif
    }
    swapRect;

    /* EGL_ANDROID_get_render_buffer. */
    /* Skip resolve if hwc composition is used. */
    EGLBoolean                  skipResolve;
#endif

    /* drawable handle which owned by API driver */
    EGLDrawable                 drawable;
};

struct eglImage
{
    /* Internal image struct pointer. Should be the first one */
    khrEGL_IMAGE                image;

    /* Signature. */
    gctUINT                     signature;

    /* Owner display. */
    VEGLDisplay                 display;

    /* Destroyed by eglDestroyImage.*/
    EGLBoolean                  destroyed;

    /* Reference counter. */
    gcsATOM_PTR                 reference;

    /* Protected Content */
    EGLBoolean                  protectedContent;

    /* Next eglImage. */
    VEGLImage                   next;
};

struct eglContext
{
    /* Should be the first member */
    struct eglResObj            resObj;

    /* Bounded thread. */
    VEGLThreadData              thread;

    /* Bounded API. */
    EGLenum                     api;
    EGLint                      client;

    /* Context dispatch table for API. */
    veglDISPATCH *              dispatch;

    /* Attached display. */
    VEGLDisplay                 display;

    /* Context Config Data. */
    struct eglConfig            config;

    /* Shared configuration. */
    VEGLContext                 sharedContext;

    /* Current read/draw surface */
    VEGLSurface                 read;
    VEGLSurface                 draw;

    /* Context for API. */
    void *                      context;

    /* Delete requested */
    gctBOOL                     deleteReq;

    /* EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR set when creating context */
    EGLint                      flags;

    /* EGL_CONTEXT_OPENGL_ROBUST_ACCESS_EXT value */
    gctBOOL                     robustAccess;

    /* EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_EXT value */
    gctINT                      resetNotification;

#if gcdGC355_PROFILER
    gctUINT64                   appStartTime;
    gctFILE                     apiTimeFile;
#endif
};

struct eglSync
{
    /* Should be the first member */
    struct eglResObj            resObj;

    /* Type */
    EGLenum                     type;

    /* Type */
    EGLenum                     condition;

    /* Signal */
    gctSIGNAL                   signal;

#if gcdANDROID_NATIVE_FENCE_SYNC
    /* Sync point. */
    gctSYNC_POINT               pt;

    /* Native sync fence fd. */
    EGLint                      fenceFD;
#endif
};

typedef enum _VEGL_COLOR_FORMAT
{
    VEGL_DEFAULT    = (1 << 0),
    VEGL_ALPHA      = (1 << 1),
    VEGL_444        = (1 << 2),
    VEGL_555        = (1 << 3),
    VEGL_565        = (1 << 4),
    VEGL_888        = (1 << 5),
    VEGL_YUV        = (1 << 6),

    VEGL_4444       = VEGL_ALPHA | VEGL_444,
    VEGL_5551       = VEGL_ALPHA | VEGL_555,
    VEGL_8888       = VEGL_ALPHA | VEGL_888
}
VEGL_COLOR_FORMAT;

typedef struct EGL_CONFIG_COLOR
{
    EGLint                      bufferSize;
    EGLint                      configBufferSize;

    EGLint                      redSize;
    EGLint                      greenSize;
    EGLint                      blueSize;
    EGLint                      alphaSize;

    VEGL_COLOR_FORMAT           formatFlags;
}
* VEGLConfigColor;

typedef struct EGL_CONFIG_DEPTH
{
    int depthSize;
    int stencilSize;
}
* VEGLConfigDepth;


/*******************************************************************************
** Thread API functions.
*/

VEGLThreadData
veglGetThreadData(
    void
    );

/*******************************************************************************
** Display API functions.
*/

void
veglDereferenceDisplay(
    IN VEGLThreadData Thread,
    IN VEGLDisplay Display,
    EGLBoolean Always
    );

VEGLDisplay
veglGetDisplay(
    IN EGLDisplay Dpy
    );

/*
 * Sync EGL objects to native objects.
 * This function is asyncrhonized.
 */
void
veglSyncNative(
    IN VEGLThreadData Thread,
    IN VEGLDisplay Display
    );

/*******************************************************************************
** Surface API functions.
*/

EGLBoolean
veglReferenceSurface(
    IN VEGLThreadData Thread,
    IN VEGLDisplay Display,
    IN VEGLSurface Surface
    );

void
veglDereferenceSurface(
    IN VEGLThreadData Thread,
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN EGLBoolean Always
    );

EGLBoolean
veglDestroySurface(
    IN EGLDisplay Dpy,
    IN EGLSurface Surface
    );

EGLBoolean
veglCreateRenderTarget(
    IN VEGLThreadData Thread,
    IN VEGLSurface Surface
    );

EGLint
veglResizeSurface(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN gctUINT Width,
    IN gctUINT Height
    );

void
veglUpdateSurfaceAge(
    IN VEGLDisplay Display
    );

/*******************************************************************************
** Context API functions.
*/
EGLBoolean
veglDestroyContext(
    IN EGLDisplay Dpy,
    IN EGLContext Ctx
    );

EGLBoolean
veglReleaseThread(
    IN VEGLThreadData Thread
    );

/*******************************************************************************
** eglImage API functions.
*/
void
veglDestroyImage(
    IN VEGLThreadData Thread,
    IN VEGLDisplay Display,
    IN VEGLImage Image
    );

void
veglReferenceImage(
    IN VEGLThreadData Thread,
    IN VEGLDisplay Display,
    IN VEGLImage Image
    );

void
veglDereferenceImage(
    IN VEGLThreadData Thread,
    IN VEGLDisplay Display,
    IN VEGLImage Image
    );

/*******************************************************************************
** eglSync API functions.
*/
EGLBoolean
veglDestroySync(
    IN EGLDisplay Dpy,
    IN EGLSync Sync
    );

/*******************************************************************************
** Swap worker API functions.
*/

void
veglSuspendSwapWorker(
    VEGLDisplay Display
    );

void
veglResumeSwapWorker(
    VEGLDisplay Display
    );

VEGLWorkerInfo
veglGetWorker(
    IN VEGLThreadData Thread,
    IN VEGLDisplay Display,
    IN VEGLSurface Surface
    );

VEGLWorkerInfo
veglFreeWorker(
    VEGLWorkerInfo Worker
    );

gctBOOL
veglSubmitWorker(
    IN VEGLThreadData Thread,
    IN VEGLDisplay Display,
    IN VEGLWorkerInfo Worker,
    IN gctBOOL ScheduleSignals
    );

/*******************************************************************************
** Misc functions.
*/

void
veglSetEGLerror(
    VEGLThreadData Thread,
    EGLint Error
    );

void
veglGetFormat(
    IN VEGLThreadData Thread,
    IN VEGLConfig Config,
    OUT gceSURF_FORMAT * RenderTarget,
    OUT gceSURF_FORMAT * DepthBuffer
    );

VEGLResObj
veglGetResObj(
    IN VEGLDisplay Dpy,
    IN VEGLResObj *pResHead,
    IN EGLResObj   ResObj,
    IN gctUINT     ResSig
    );

void
veglPushResObj(
    IN VEGLDisplay Dpy,
    INOUT VEGLResObj *pResHead,
    IN VEGLResObj ResObj
    );

void
veglPopResObj(
    IN VEGLDisplay Dpy,
    INOUT VEGLResObj *pResHead,
    IN VEGLResObj ResObj
    );

EGLsizeiANDROID
veglGetBlobCache(
    const void* key,
    EGLsizeiANDROID keySize,
    void* value,
    EGLsizeiANDROID valueSize
    );

void
veglSetBlobCache(
    const void* key,
    EGLsizeiANDROID keySize,
    const void* value,
    EGLsizeiANDROID valueSize
    );

#if defined(_WIN32)
#define veglTHREAD_RETURN DWORD
#else
#define veglTHREAD_RETURN void *
#endif

#ifndef WINAPI
#define WINAPI
#endif

veglTHREAD_RETURN
WINAPI
veglSwapWorker(
    void* Display
    );

gctHANDLE
veglGetModule(
    gcoOS Os,
    EGLBoolean Egl,
    VEGLContext Context,
    gctINT_PTR Index
    );


/*******************************************************************************
** Bridge functions.
*/

veglDISPATCH *
_GetDispatch(
    VEGLThreadData Thread,
    VEGLContext Context
    );

void *
_CreateApiContext(
    VEGLThreadData Thread,
    VEGLContext Context,
    VEGLConfig Config,
    void * SharedContext
    );

EGLBoolean
_DestroyApiContext(
    VEGLThreadData Thread,
    VEGLContext Context,
    void * ApiContext
    );

EGLBoolean
_FlushApiContext(
    VEGLThreadData Thread,
    VEGLContext Context,
    void * ApiContext
    );

EGLBoolean
_SetDrawable(
    VEGLThreadData Thread,
    VEGLContext Context,
    VEGLDrawable Draw,
    VEGLDrawable Read
    );

gceSTATUS
_SetBuffer(
    VEGLThreadData Thread,
    gcoSURF Draw
    );

EGLBoolean
_Flush(
    VEGLThreadData Thread
    );

EGLBoolean
_Finish(
    VEGLThreadData Thread
    );

EGLBoolean
_eglProfileCallback(
    VEGLThreadData Thread,
    IN gctUINT32 Enum,
    IN gctHANDLE Value
    );

gcoSURF
_GetClientBuffer(
    VEGLThreadData Thread,
    void * Context,
    EGLClientBuffer buffer
    );

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
    );

EGLenum
_CreateImageTexture(
    VEGLThreadData Thread,
    VEGLContext Context,
    EGLenum Target,
    gctINT Texture,
    gctINT Level,
    gctINT Depth,
    gctPOINTER Image
    );

EGLenum
_CreateImageFromRenderBuffer(
    VEGLThreadData Thread,
    VEGLContext Context,
    gctUINT Framebuffer,
    gctPOINTER Image
    );

EGLenum
_CreateImageFromVGParentImage(
    VEGLThreadData  Thread,
    VEGLContext Context,
    unsigned int    vgimage_obj,
    VEGLImage       eglimage
    );

#define VEGL_TRACE_API(func) if (veglTracerDispatchTable.func)(*veglTracerDispatchTable.func)
#define VEGL_TRACE_API_PRE(func) if (veglTracerDispatchTable.func##_pre)(*veglTracerDispatchTable.func##_pre)
#define VEGL_TRACE_API_POST(func) if (veglTracerDispatchTable.func##_post)(*veglTracerDispatchTable.func##_post)


#define VEGL_LOCK_DISPLAY(dpy)   \
    if (dpy->accessMutex) \
    {\
        gcoOS_AcquireMutex(gcvNULL, dpy->accessMutex, gcvINFINITE);\
    }\

#define VEGL_UNLOCK_DISPLAY(dpy)  \
    if (dpy->accessMutex) \
    {\
        gcoOS_ReleaseMutex(gcvNULL, dpy->accessMutex);\
    }\


#define VEGL_LOCK_DISPLAY_RESOURCE(dpy)   \
    if (dpy->resourceMutex) \
    {\
        gcoOS_AcquireMutex(gcvNULL, dpy->resourceMutex, gcvINFINITE);\
    }\

#define VEGL_UNLOCK_DISPLAY_RESOURCE(dpy)  \
    if (dpy->resourceMutex) \
    {\
        gcoOS_ReleaseMutex(gcvNULL, dpy->resourceMutex);\
    }\



/* EGL Tracer Dispatch Function Table */
typedef struct
{
    /* Note:
    ** The following 36 interfaces are used to link with vTracer EGL entry functions in libGLES_vlogger.so.
    ** So the following interfaces must match with vTracer EGL entry functions.
    */
    EGLint     (*GetError_pre)(void);
    EGLDisplay (*GetDisplay_post)(EGLNativeDisplayType display_id, EGLDisplay ret_dpy);
    EGLBoolean (*Initialize)(EGLDisplay dpy, EGLint *major, EGLint *minor);
    EGLBoolean (*Terminate)(EGLDisplay dpy);
    const char * (*QueryString_pre)(EGLDisplay dpy, EGLint name);
    EGLBoolean (*GetConfigs_pre)(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config);
    EGLBoolean (*ChooseConfig_pre)(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config);
    EGLBoolean (*GetConfigAttrib_post)(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value);
    EGLSurface (*CreateWindowSurface_post)(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list, EGLSurface ret_surface);
    EGLSurface (*CreatePbufferSurface_post)(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list, EGLSurface ret_surface);
    EGLSurface (*CreatePixmapSurface_post)(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list, EGLSurface ret_surface);
    EGLBoolean (*DestroySurface)(EGLDisplay dpy, EGLSurface surface);
    EGLBoolean (*QuerySurface_pre)(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value);
    EGLBoolean (*BindAPI)(EGLenum api);
    EGLenum    (*QueryAPI_pre)(void);
    EGLBoolean (*WaitClient)(void);
    EGLBoolean (*ReleaseThread)(void);
    EGLSurface (*CreatePbufferFromClientBuffer_post)(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list, EGLSurface ret_surface);
    EGLBoolean (*SurfaceAttrib)(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value);
    EGLBoolean (*BindTexImage)(EGLDisplay dpy, EGLSurface surface, EGLint buffer);
    EGLBoolean (*ReleaseTexImage)(EGLDisplay dpy, EGLSurface surface, EGLint buffer);
    EGLBoolean (*SwapInterval)(EGLDisplay dpy, EGLint interval);
    EGLContext (*CreateContext_post)(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list, EGLContext ret_ctx);
    EGLBoolean (*DestroyContext)(EGLDisplay dpy, EGLContext ctx);
    EGLBoolean (*MakeCurrent)(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
    EGLContext (*GetCurrentContext_post)(EGLContext ret_ctx);
    EGLSurface (*GetCurrentSurface_post)(EGLint readdraw, EGLSurface ret_furface);
    EGLDisplay (*GetCurrentDisplay_post)(EGLDisplay ret_dpy);
    EGLBoolean (*QueryContext_pre)(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value);
    EGLBoolean (*WaitGL)(void);
    EGLBoolean (*WaitNative)(EGLint engine);
    EGLBoolean (*SwapBuffers)(EGLDisplay dpy, EGLSurface surface);
    EGLBoolean (*CopyBuffers)(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target);
    __eglMustCastToProperFunctionPointerType (*GetProcAddress_pre)(const char *procname);

    /* EGL 1.5 */
    EGLSync    (*CreateSync_post)(EGLDisplay dpy, EGLenum type, const EGLAttrib *attrib_list, EGLSync ret_sync);
    EGLBoolean (*DestroySync)(EGLDisplay dpy, EGLSync sync);
    EGLint     (*ClientWaitSync)(EGLDisplay dpy, EGLSync sync, EGLint flags, EGLTime timeout);
    EGLBoolean (*GetSyncAttrib_post)(EGLDisplay dpy, EGLSync sync, EGLint attribute, EGLAttrib *value, EGLAttrib ret_value);
    EGLImage   (*CreateImage_post)(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLAttrib *attrib_list, EGLImage ret_image);
    EGLBoolean (*DestroyImage)(EGLDisplay dpy, EGLImage image);
    EGLDisplay (*GetPlatformDisplay_post)(EGLenum platform, void *native_display, const EGLAttrib *attrib_list, EGLDisplay ret_dpy);
    EGLSurface (*CreatePlatformWindowSurface_post)(EGLDisplay dpy, EGLConfig config, void *native_window, const EGLAttrib *attrib_list, EGLSurface ret_surface);
    EGLSurface (*CreatePlatformPixmapSurface_post)(EGLDisplay dpy, EGLConfig config, void *native_pixmap, const EGLAttrib *attrib_list, EGLSurface ret_surface);
    EGLBoolean (*WaitSync)(EGLDisplay dpy, EGLSync sync, EGLint flags);

    /* EGL_KHR_lock_surface. */
    EGLBoolean (* LockSurfaceKHR)(EGLDisplay dpy, EGLSurface surface, const EGLint *attrib_list);
    EGLBoolean (* UnlockSurfaceKHR)(EGLDisplay dpy, EGLSurface surface);

    /* EGL_KHR_image. */
    EGLImageKHR (*CreateImageKHR_post)(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list, EGLImageKHR ret_image);
    EGLBoolean (*DestroyImageKHR)(EGLDisplay dpy, EGLImageKHR image);

    /* EGL_KHR_fence_sync. */
    EGLSyncKHR (*CreateSyncKHR_post)(EGLDisplay dpy, EGLenum type, const EGLint *attrib_list, EGLSyncKHR ret_sync);
    EGLBoolean (*DestroySyncKHR)(EGLDisplay dpy, EGLSyncKHR sync);
    EGLint     (*ClientWaitSyncKHR)(EGLDisplay dpy, EGLSyncKHR sync, EGLint flags, EGLTimeKHR timeout);
    EGLBoolean (*GetSyncAttribKHR_post)(EGLDisplay dpy, EGLSyncKHR sync, EGLint attribute, EGLint *value, EGLint ret_value);

    /* EGL_KHR_wait_sync. */
    EGLint     (*WaitSyncKHR)(EGLDisplay dpy, EGLSyncKHR sync, EGLint flags);

    /* EGL_KHR_reusable_sync. */
    EGLBoolean (*SignalSyncKHR)(EGLDisplay dpy, EGLSyncKHR sync, EGLenum mode);

    /******  The above interfaces are used to link with external vTracer library libGLES_vlogger.so ******/

    EGLDisplay (*GetDisplay_pre)(EGLNativeDisplayType display_id);
    EGLBoolean (*GetConfigAttrib_pre)(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value);
    EGLSurface (*CreateWindowSurface_pre)(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list);
    EGLSurface (*CreatePbufferSurface_pre)(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list);
    EGLSurface (*CreatePixmapSurface_pre)(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list);
    EGLSurface (*CreatePbufferFromClientBuffer_pre)(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list);
    EGLContext (*CreateContext_pre)(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list);
    EGLContext (*GetCurrentContext_pre)();
    EGLSurface (*GetCurrentSurface_pre)(EGLint readdraw);
    EGLDisplay (*GetCurrentDisplay_pre)();
    __eglMustCastToProperFunctionPointerType (*GetProcAddress_post)(const char *procname, EGLint *func);

    EGLint     (*GetError_post)(EGLint err);
    const char * (*QueryString_post)(EGLDisplay dpy, EGLint name, const char* str);
    EGLBoolean (*GetConfigs_post)(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config);
    EGLBoolean (*ChooseConfig_post)(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config);
    EGLBoolean (*QuerySurface_post)(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value);
    EGLenum    (*QueryAPI_post)(EGLenum api);
    EGLBoolean (*QueryContext_post)(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value);

    /* EGL 1.5 */
    EGLSync    (*CreateSync_pre)(EGLDisplay dpy, EGLenum type, const EGLAttrib *attrib_list);
    EGLBoolean (*GetSyncAttrib_pre)(EGLDisplay dpy, EGLSync sync, EGLint attribute, EGLAttrib *value);
    EGLImage   (*CreateImage_pre)(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLAttrib *attrib_list);
    EGLDisplay (*GetPlatformDisplay_pre)(EGLenum platform, void *native_display, const EGLAttrib *attrib_list);
    EGLSurface (*CreatePlatformWindowSurface_pre)(EGLDisplay dpy, EGLConfig config, void *native_window, const EGLAttrib *attrib_list);
    EGLSurface (*CreatePlatformPixmapSurface_pre)(EGLDisplay dpy, EGLConfig config, void *native_pixmap, const EGLAttrib *attrib_list);

    /* EGL_KHR_image */
    EGLImageKHR (*CreateImageKHR_pre)(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list);

    /* EGL_KHR_fence_sync*/
    EGLSyncKHR (*CreateSyncKHR_pre)(EGLDisplay dpy, EGLenum type, const EGLint *attrib_list);
    EGLBoolean (*GetSyncAttribKHR_pre)(EGLDisplay dpy, EGLSyncKHR sync, EGLint attribute, EGLint *value);
}
eglTracerDispatchTableStruct;


EGLBoolean
veglInitTracerDispatchTable(
    void
    );

extern eglTracerDispatchTableStruct veglTracerDispatchTable;
extern eglTracerDispatchTableStruct veglLogFunctionTable;
extern gceTRACEMODE veglTraceMode;


#ifdef __cplusplus
}
#endif

#endif /* __gc_egl_h_ */
