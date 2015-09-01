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


#ifndef __gc_cl_event_h_
#define __gc_cl_event_h_

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************\
************************* Event Object Definition *************************
\******************************************************************************/

typedef struct _cl_event_callback   clsEventCallback;
struct _cl_event_callback
{
    void                    (CL_CALLBACK * pfnNotify)(cl_event, cl_int, void *);
    void                    *userData;
    clsEvent_PTR            event;
    clsEventCallback_PTR    next;
    gctBOOL                 added;
};

typedef struct _cl_event_profile_info
{
    cl_ulong                queued;
    cl_ulong                submit;
    cl_ulong                start;
    cl_ulong                end;
}
clsEventProfileInfo;

typedef struct _cl_event
{
    clsIcdDispatch_PTR      dispatch;
    cleOBJECT_TYPE          objectType;
    gctUINT                 id;
    gcsATOM_PTR             referenceCount;

    clsContext_PTR          context;
    clsCommandQueue_PTR     queue;

    gctINT                  executionStatus;
    gctBOOL                 executionStatusSet;
    gctBOOL                 userEvent;
    cl_command_type         commandType;
    clsEventCallback_PTR    callback;
    gctPOINTER              callbackMutex;
    clsEventProfileInfo     profileInfo;

    clsEvent_PTR            next;
    clsEvent_PTR            previous;

    gctSIGNAL               finishSignal;
    gctSIGNAL               runSignal;
    gctSIGNAL               completeSignal;
}
clsEvent;

/*****************************************************************************\
|*                         Supporting functions                              *|
\*****************************************************************************/

gctINT
clfAllocateEvent(
    clsContext_PTR          Context,
    clsEvent_PTR *          Event
    );

gctINT
clfSetEventExecutionStatus(
    cl_event        Event,
    gctINT          Status
    );

gctINT
clfGetEventExecutionStatus(
    cl_event        Event
    );

gctINT
clfFinishEvent(
    cl_event        Event,
    gctINT          Status
    );

gctINT
clfCheckPendingEvents(
    clsCommand_PTR  Command
    );

gctINT
clfWaitForEvent(
    cl_event        Event
    );

gctINT
clfProcessEventList(
    clsContext_PTR  Context
    );

gctINT
clfSubmitEventForFinish(
    clsCommand_PTR  Command
    );

gctINT
clfSetEventForFinish(
    clsCommand_PTR  Command
    );

gctINT
clfSubmitEventForRunning(
    clsCommand_PTR  Command
    );

gctTHREAD_RETURN CL_API_CALL
clfEventListWorker(
    gctPOINTER Data
    );

gctTHREAD_RETURN CL_API_CALL
clfEventCallbackWorker(
    gctPOINTER Data
    );

#ifdef __cplusplus
}
#endif

#endif  /* __gc_cl_event_h_ */
