/* $XFree86: xc/lib/GL/glx/XF86dri.c,v 1.1 2004/12/10 16:06:56 alanh Exp $ */
/**************************************************************************

Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
Copyright 2000 VA Linux Systems, Inc.
All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors:
 *   Kevin E. Martin <martin@valinux.com>
 *   Jens Owen <jens@tungstengraphics.com>
 *   Rickard E. (Rik) Faith <faith@valinux.com>
 *
 */

/* THIS IS NOT AN X CONSORTIUM STANDARD */

#define NEED_REPLIES
#include <X11/Xlibint.h>
#include "xf86dristr.h"
#include <X11/extensions/Xext.h>
#include "extutil.h"

static XExtensionInfo _xf86dri_info_data;
static XExtensionInfo *xf86dri_info = &_xf86dri_info_data;
static char *xf86dri_extension_name = XF86DRINAME;

#define XF86DRICheckExtension(dpy,i,val) \
  XextCheckExtension (dpy, i, xf86dri_extension_name, val)

/*****************************************************************************
 *                                                                           *
 *                           private utility routines                          *
 *                                                                           *
 *****************************************************************************/

static int close_display(Display *dpy, XExtCodes *extCodes);
static /* const */ XExtensionHooks xf86dri_extension_hooks = {
    NULL,                                /* create_gc */
    NULL,                                /* copy_gc */
    NULL,                                /* flush_gc */
    NULL,                                /* free_gc */
    NULL,                                /* create_font */
    NULL,                                /* free_font */
    close_display,                        /* close_display */
    NULL,                                /* wire_to_event */
    NULL,                                /* event_to_wire */
    NULL,                                /* error */
    NULL,                                /* error_string */
};

static XEXT_GENERATE_FIND_DISPLAY (find_display, xf86dri_info,
                                   xf86dri_extension_name,
                                   &xf86dri_extension_hooks,
                                   0, NULL)

static XEXT_GENERATE_CLOSE_DISPLAY (close_display, xf86dri_info)


/*****************************************************************************
 *                                                                           *
 *                    public XFree86-DRI Extension routines                    *
 *                                                                           *
 *****************************************************************************/

#define TRACE(msg)


Bool XF86DRIQueryExtension (dpy, event_basep, error_basep)
    Display *dpy;
    int *event_basep, *error_basep;
{
    XExtDisplayInfo *info = find_display (dpy);

    TRACE("QueryExtension...");
    if (XextHasExtension(info)) {
        *event_basep = info->codes->first_event;
        *error_basep = info->codes->first_error;
        TRACE("QueryExtension... return True");
        return True;
    } else {
        TRACE("QueryExtension... return False");
        return False;
    }
}

Bool XF86DRIQueryVersion(dpy, majorVersion, minorVersion, patchVersion)
    Display* dpy;
    int* majorVersion;
    int* minorVersion;
    int* patchVersion;
{
    XExtDisplayInfo *info = find_display (dpy);
    xXF86DRIQueryVersionReply rep;
    xXF86DRIQueryVersionReq *req;

    TRACE("QueryVersion...");
    XF86DRICheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(XF86DRIQueryVersion, req);
    req->reqType = info->codes->major_opcode;
    req->driReqType = X_XF86DRIQueryVersion;
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
        UnlockDisplay(dpy);
        SyncHandle();
        TRACE("QueryVersion... return False");
        return False;
    }
    *majorVersion = rep.majorVersion;
    *minorVersion = rep.minorVersion;
    *patchVersion = rep.patchVersion;
    UnlockDisplay(dpy);
    SyncHandle();
    TRACE("QueryVersion... return True");
    return True;
}

Bool XF86DRIQueryDirectRenderingCapable(dpy, screen, isCapable)
    Display* dpy;
    int screen;
    Bool* isCapable;
{
    XExtDisplayInfo *info = find_display (dpy);
    xXF86DRIQueryDirectRenderingCapableReply rep;
    xXF86DRIQueryDirectRenderingCapableReq *req;

    TRACE("QueryDirectRenderingCapable...");
    XF86DRICheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(XF86DRIQueryDirectRenderingCapable, req);
    req->reqType = info->codes->major_opcode;
    req->driReqType = X_XF86DRIQueryDirectRenderingCapable;
    req->screen = screen;
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
        UnlockDisplay(dpy);
        SyncHandle();
        TRACE("QueryDirectRenderingCapable... return False");
        return False;
    }
    *isCapable = rep.isCapable;
    UnlockDisplay(dpy);
    SyncHandle();
    TRACE("QueryDirectRenderingCapable... return True");
    return True;
}

Bool XF86DRIOpenConnection(dpy, screen, hSAREA, busIdString)
    Display* dpy;
    int screen;
    drm_handle_t * hSAREA;
    char **busIdString;
{
    XExtDisplayInfo *info = find_display (dpy);
    xXF86DRIOpenConnectionReply rep;
    xXF86DRIOpenConnectionReq *req;

    TRACE("OpenConnection...");
    XF86DRICheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(XF86DRIOpenConnection, req);
    req->reqType = info->codes->major_opcode;
    req->driReqType = X_XF86DRIOpenConnection;
    req->screen = screen;
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
        UnlockDisplay(dpy);
        SyncHandle();
        TRACE("OpenConnection... return False");
        return False;
    }

    *hSAREA = rep.hSAREALow;
#ifdef LONG64
    *hSAREA |= ((drm_handle_t)rep.hSAREAHigh) << 32;
#endif

    if (rep.length) {
        if (!(*busIdString = (char *)Xcalloc(rep.busIdStringLength + 1, 1))) {
            _XEatData(dpy, ((rep.busIdStringLength+3) & ~3));
            UnlockDisplay(dpy);
            SyncHandle();
            TRACE("OpenConnection... return False");
            return False;
        }
        _XReadPad(dpy, *busIdString, rep.busIdStringLength);
    } else {
        *busIdString = NULL;
    }
    UnlockDisplay(dpy);
    SyncHandle();
    TRACE("OpenConnection... return True");
    return True;
}

Bool XF86DRIAuthConnection(dpy, screen, magic)
    Display* dpy;
    int screen;
    drm_magic_t magic;
{
    XExtDisplayInfo *info = find_display (dpy);
    xXF86DRIAuthConnectionReq *req;
    xXF86DRIAuthConnectionReply rep;

    TRACE("AuthConnection...");
    XF86DRICheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(XF86DRIAuthConnection, req);
    req->reqType = info->codes->major_opcode;
    req->driReqType = X_XF86DRIAuthConnection;
    req->screen = screen;
    req->magic = magic;
    rep.authenticated = 0;
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse) || !rep.authenticated) {
        UnlockDisplay(dpy);
        SyncHandle();
        TRACE("AuthConnection... return False");
        return False;
    }
    UnlockDisplay(dpy);
    SyncHandle();
    TRACE("AuthConnection... return True");
    return True;
}

Bool XF86DRICloseConnection(dpy, screen)
    Display* dpy;
    int screen;
{
    XExtDisplayInfo *info = find_display (dpy);
    xXF86DRICloseConnectionReq *req;

    TRACE("CloseConnection...");

    XF86DRICheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(XF86DRICloseConnection, req);
    req->reqType = info->codes->major_opcode;
    req->driReqType = X_XF86DRICloseConnection;
    req->screen = screen;
    UnlockDisplay(dpy);
    SyncHandle();
    TRACE("CloseConnection... return True");
    return True;
}

Bool XF86DRIGetClientDriverName(dpy, screen, ddxDriverMajorVersion,
        ddxDriverMinorVersion, ddxDriverPatchVersion, clientDriverName)
    Display* dpy;
    int screen;
    int* ddxDriverMajorVersion;
    int* ddxDriverMinorVersion;
    int* ddxDriverPatchVersion;
    char** clientDriverName;
{
    XExtDisplayInfo *info = find_display (dpy);
    xXF86DRIGetClientDriverNameReply rep;
    xXF86DRIGetClientDriverNameReq *req;

    TRACE("GetClientDriverName...");
    XF86DRICheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(XF86DRIGetClientDriverName, req);
    req->reqType = info->codes->major_opcode;
    req->driReqType = X_XF86DRIGetClientDriverName;
    req->screen = screen;
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
        UnlockDisplay(dpy);
        SyncHandle();
        TRACE("GetClientDriverName... return False");
        return False;
    }

    *ddxDriverMajorVersion = rep.ddxDriverMajorVersion;
    *ddxDriverMinorVersion = rep.ddxDriverMinorVersion;
    *ddxDriverPatchVersion = rep.ddxDriverPatchVersion;

    if (rep.length) {
        if (!(*clientDriverName = (char *)Xcalloc(rep.clientDriverNameLength + 1, 1))) {
            _XEatData(dpy, ((rep.clientDriverNameLength+3) & ~3));
            UnlockDisplay(dpy);
            SyncHandle();
            TRACE("GetClientDriverName... return False");
            return False;
        }
        _XReadPad(dpy, *clientDriverName, rep.clientDriverNameLength);
    } else {
        *clientDriverName = NULL;
    }
    UnlockDisplay(dpy);
    SyncHandle();
    TRACE("GetClientDriverName... return True");
    return True;
}

Bool XF86DRICreateContextWithConfig(dpy, screen, configID, context, hHWContext)
    Display* dpy;
    int screen;
    int configID;
    XID* context;
    drm_context_t * hHWContext;
{
    XExtDisplayInfo *info = find_display (dpy);
    xXF86DRICreateContextReply rep;
    xXF86DRICreateContextReq *req;

    TRACE("CreateContext...");
    XF86DRICheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(XF86DRICreateContext, req);
    req->reqType = info->codes->major_opcode;
    req->driReqType = X_XF86DRICreateContext;
    req->visual = configID;
    req->screen = screen;
    *context = XAllocID(dpy);
    req->context = *context;
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
        UnlockDisplay(dpy);
        SyncHandle();
        TRACE("CreateContext... return False");
        return False;
    }
    *hHWContext = rep.hHWContext;
    UnlockDisplay(dpy);
    SyncHandle();
    TRACE("CreateContext... return True");
    return True;
}

Bool XF86DRICreateContext(dpy, screen, visual, context, hHWContext)
    Display* dpy;
    int screen;
    Visual* visual;
    XID* context;
    drm_context_t * hHWContext;
{
    return XF86DRICreateContextWithConfig( dpy, screen, visual->visualid,
                                           context, hHWContext );
}

Bool XF86DRIDestroyContext( Display * dpy, int screen,
    __DRIid context )
{
    XExtDisplayInfo *info = find_display (dpy);
    xXF86DRIDestroyContextReq *req;

    TRACE("DestroyContext...");
    XF86DRICheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(XF86DRIDestroyContext, req);
    req->reqType = info->codes->major_opcode;
    req->driReqType = X_XF86DRIDestroyContext;
    req->screen = screen;
    req->context = context;
    UnlockDisplay(dpy);
    SyncHandle();
    TRACE("DestroyContext... return True");
    return True;
}

Bool XF86DRICreateDrawable( Display * dpy, int screen,
    __DRIid drawable, drm_drawable_t * hHWDrawable )
{
    XExtDisplayInfo *info = find_display (dpy);
    xXF86DRICreateDrawableReply rep;
    xXF86DRICreateDrawableReq *req;

    TRACE("CreateDrawable...");
    XF86DRICheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(XF86DRICreateDrawable, req);
    req->reqType = info->codes->major_opcode;
    req->driReqType = X_XF86DRICreateDrawable;
    req->screen = screen;
    req->drawable = drawable;
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
        UnlockDisplay(dpy);
        SyncHandle();
        TRACE("CreateDrawable... return False");
        return False;
    }
    *hHWDrawable = rep.hHWDrawable;
    UnlockDisplay(dpy);
    SyncHandle();
    TRACE("CreateDrawable... return True");
    return True;
}

Bool XF86DRIDestroyDrawable( Display * dpy, int screen,
    __DRIid drawable )
{
    XExtDisplayInfo *info = find_display (dpy);
    xXF86DRIDestroyDrawableReq *req;

    TRACE("DestroyDrawable...");
    XF86DRICheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(XF86DRIDestroyDrawable, req);
    req->reqType = info->codes->major_opcode;
    req->driReqType = X_XF86DRIDestroyDrawable;
    req->screen = screen;
    req->drawable = drawable;
    UnlockDisplay(dpy);
    SyncHandle();
    TRACE("DestroyDrawable... return True");
    return True;
}

Bool XF86DRIGetDrawableInfo(Display* dpy, int screen, Drawable drawable,
    unsigned int* index, unsigned int* stamp,
    int* X, int* Y, int* W, int* H,
    int* numClipRects, drm_clip_rect_t ** pClipRects,
    int* backX, int* backY,
    int* numBackClipRects, drm_clip_rect_t ** pBackClipRects )
{
    XExtDisplayInfo *info = find_display (dpy);
    xXF86DRIGetDrawableInfoReply rep;
    xXF86DRIGetDrawableInfoReq *req;
    int total_rects;

    TRACE("GetDrawableInfo...");
    XF86DRICheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(XF86DRIGetDrawableInfo, req);
    req->reqType = info->codes->major_opcode;
    req->driReqType = X_XF86DRIGetDrawableInfo;
    req->screen = screen;
    req->drawable = drawable;

    if (!_XReply(dpy, (xReply *)&rep, 1, xFalse))
    {
        UnlockDisplay(dpy);
        SyncHandle();
        TRACE("GetDrawableInfo... return False");
        return False;
    }
    *index = rep.drawableTableIndex;
    *stamp = rep.drawableTableStamp;
    *X = (int)rep.drawableX;
    *Y = (int)rep.drawableY;
    *W = (int)rep.drawableWidth;
    *H = (int)rep.drawableHeight;
    *numClipRects = rep.numClipRects;
    total_rects = *numClipRects;

    *backX = rep.backX;
    *backY = rep.backY;
    *numBackClipRects = rep.numBackClipRects;
    total_rects += *numBackClipRects;


    if (*numClipRects) {
       int len = sizeof(drm_clip_rect_t) * (*numClipRects);

       *pClipRects = (drm_clip_rect_t *)Xcalloc(len, 1);
       if (*pClipRects)
          _XRead(dpy, (char*)*pClipRects, len);
    } else {
        *pClipRects = NULL;
    }

    if (*numBackClipRects) {
       int len = sizeof(drm_clip_rect_t) * (*numBackClipRects);

       *pBackClipRects = (drm_clip_rect_t *)Xcalloc(len, 1);
       if (*pBackClipRects)
          _XRead(dpy, (char*)*pBackClipRects, len);
    } else {
        *pBackClipRects = NULL;
    }

    UnlockDisplay(dpy);
    SyncHandle();
    TRACE("GetDrawableInfo... return True");
    return True;
}

Bool XF86DRIGetDeviceInfo(dpy, screen, hFrameBuffer,
        fbOrigin, fbSize, fbStride, devPrivateSize, pDevPrivate)
    Display* dpy;
    int screen;
    drm_handle_t * hFrameBuffer;
    int* fbOrigin;
    int* fbSize;
    int* fbStride;
    int* devPrivateSize;
    GLvoid** pDevPrivate;
{
    XExtDisplayInfo *info = find_display (dpy);
    xXF86DRIGetDeviceInfoReply rep;
    xXF86DRIGetDeviceInfoReq *req;

    TRACE("GetDeviceInfo...");
    XF86DRICheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(XF86DRIGetDeviceInfo, req);
    req->reqType = info->codes->major_opcode;
    req->driReqType = X_XF86DRIGetDeviceInfo;
    req->screen = screen;
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
        UnlockDisplay(dpy);
        SyncHandle();
        TRACE("GetDeviceInfo... return False");
        return False;
    }

    *hFrameBuffer = rep.hFrameBufferLow;
#ifdef LONG64
    *hFrameBuffer |= ((drm_handle_t)rep.hFrameBufferHigh) << 32;
#endif

    *fbOrigin = rep.framebufferOrigin;
    *fbSize = rep.framebufferSize;
    *fbStride = rep.framebufferStride;
    *devPrivateSize = rep.devPrivateSize;

    if (rep.length) {
        if (!(*pDevPrivate = (GLvoid *)Xcalloc(rep.devPrivateSize, 1))) {
            _XEatData(dpy, ((rep.devPrivateSize+3) & ~3));
            UnlockDisplay(dpy);
            SyncHandle();
            TRACE("GetDeviceInfo... return False");
            return False;
        }
        _XRead(dpy, (char*)*pDevPrivate, rep.devPrivateSize);
    } else {
        *pDevPrivate = NULL;
    }

    UnlockDisplay(dpy);
    SyncHandle();
    TRACE("GetDeviceInfo... return True");
    return True;
}

Bool XF86DRIOpenFullScreen(dpy, screen, drawable)
    Display* dpy;
    int screen;
    Drawable drawable;
{
    /* This function and the underlying X protocol are deprecated.
     */
    (GLvoid) dpy;
    (GLvoid) screen;
    (GLvoid) drawable;
    return False;
}

Bool XF86DRICloseFullScreen(dpy, screen, drawable)
    Display* dpy;
    int screen;
    Drawable drawable;
{
    /* This function and the underlying X protocol are deprecated.
     */
    (GLvoid) dpy;
    (GLvoid) screen;
    (GLvoid) drawable;
    return True;
}
