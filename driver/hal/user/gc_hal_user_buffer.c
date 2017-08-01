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


/**
**  @file
**  gcoBUFFER object for user HAL layers.
**
*/

#include "gc_hal_user_precomp.h"
#include "gc_hal_user_buffer.h"

#if (gcdENABLE_3D || gcdENABLE_2D)

#define _GC_OBJ_ZONE            gcvZONE_BUFFER

#define gcmMAX_TEMPCMD_BUFFER_SIZE  0x20000
/******************************************************************************\
********************************** Structures **********************************
\******************************************************************************/

struct _gcoBUFFER
{
    /* Object. */
    gcsOBJECT                   object;

    /* Pointer to the required objects. */
    gcoHAL                      hal;

    /* Pointer to the hardware objects. */
    gcoHARDWARE                 hardware;

    /* Flag to indicate thread default or not */
    gctBOOL                     threadDefault;

    /* Requested command buffer size. */
    gctSIZE_T                   bytes;

    /* Number of command buffers. */
    gctSIZE_T                   count;
    gctSIZE_T                   maxCount;

    /* List of command buffers. */
    gcoCMDBUF                   commandBufferList;
    gcoCMDBUF                   commandBufferTail;

    /* Reserved bytes. */
    struct _gcsCOMMAND_INFO
    {
        gctUINT32   alignment;
        gctUINT32   reservedHead;
        gctUINT32   reservedTail;
        gctUINT32   reservedUser;
        gceCMDBUF_SOURCE source;
        gceENGINE   engine;
    }                           info;
    gctUINT32                   totalReserved;

    struct _gcsPATCH_LIST *     patchList;

    /* Cache of free patch lists. */
    struct _gcsPATCH_LIST *     freePatchList;

    struct _gcsTEMPCMDBUF       tempCMDBUF;

#if gcdENABLE_3D
    gctBOOL                     tfbPaused;
    gctUINT32                   tfbResumeBytes;
    gctBOOL                     queryPaused[gcvQUERY_MAX_NUM];
    gctUINT32                   queryResumeBytes[gcvQUERY_MAX_NUM];
    gctBOOL                     probePaused;
    gctUINT32                   probeResumeBytes;
#endif

    gctBOOL                     inRerserved;

    /* Commit stamp of each commit. */
    gctUINT64                   commitStamp;

    gctUINT32                   mirrorCount;

    gcsFENCE_LIST_PTR           fenceList;
};

/******************************************************************************\
******************************* Private Functions ******************************
\******************************************************************************/

static gceSTATUS
_FreeCommandBuffer(
    IN gcoHARDWARE Hardware,
    IN gcsCOMMAND_INFO_PTR Info,
    IN gcoCMDBUF CommandBuffer
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsHAL_INTERFACE iface;

    gcmHEADER_ARG("CommandBuffer=0x%x", CommandBuffer);

    if (gcmUINT64_TO_PTR(CommandBuffer) != gcvNULL)
    {
        if (gcmUINT64_TO_PTR(CommandBuffer->logical) != gcvNULL)
        {
            /* Has the command buffer ever been committed? */
            if (CommandBuffer->commitCount == 0)
            {
                switch (Info->source)
                {
                case gcvCMDBUF_VIRTUAL:
                    iface.command = gcvHAL_FREE_VIRTUAL_COMMAND_BUFFER;
                    iface.u.FreeVirtualCommandBuffer.bytes    = CommandBuffer->bytes;
                    iface.u.FreeVirtualCommandBuffer.physical = CommandBuffer->physical;
                    iface.u.FreeVirtualCommandBuffer.logical  = CommandBuffer->logical;
                    gcmONERROR(gcoHAL_Call(gcvNULL, &iface));
                    break;
                case gcvCMDBUF_CONTIGUOUS:
                    gcmONERROR(gcoOS_FreeContiguous(
                        gcvNULL,
                        gcmINT2PTR(CommandBuffer->physical),
                        gcmUINT64_TO_PTR(CommandBuffer->logical),
                        (gctSIZE_T) CommandBuffer->bytes
                        ));
                    break;
                case gcvCMDBUF_RESERVED:
                    gcmONERROR(gcoHAL_UnlockVideoMemory(CommandBuffer->physical, gcvSURF_BITMAP, Info->engine));
                    gcmONERROR(gcoHAL_ReleaseVideoMemory(CommandBuffer->physical));
                    break;
                default:
                    break;
                }
            }
            else
            {
                switch (Info->source)
                {
                case gcvCMDBUF_VIRTUAL:
                    iface.command = gcvHAL_FREE_VIRTUAL_COMMAND_BUFFER;
                    iface.engine = Info->engine;
                    iface.u.FreeVirtualCommandBuffer.bytes    = CommandBuffer->bytes;
                    iface.u.FreeVirtualCommandBuffer.physical = CommandBuffer->physical;
                    iface.u.FreeVirtualCommandBuffer.logical  = CommandBuffer->logical;

                    /* Send event. */
                    gcmONERROR(gcoHARDWARE_CallEvent(Hardware, &iface));

                    break;
                case gcvCMDBUF_CONTIGUOUS:
                    iface.command = gcvHAL_FREE_CONTIGUOUS_MEMORY;
                    iface.engine = Info->engine;
                    iface.u.FreeContiguousMemory.bytes    = CommandBuffer->bytes;
                    iface.u.FreeContiguousMemory.physical = CommandBuffer->physical;
                    iface.u.FreeContiguousMemory.logical  = CommandBuffer->logical;

                    /* Send event. */
                    gcmONERROR(gcoHARDWARE_CallEvent(Hardware, &iface));

                    break;
                case gcvCMDBUF_RESERVED:
                    gcmONERROR(gcoHAL_UnlockVideoMemory(CommandBuffer->physical, gcvSURF_BITMAP, Info->engine));
                    gcmONERROR(gcoHAL_ReleaseVideoMemory(CommandBuffer->physical));
                    break;
                default:
                    break;
                }
            }

            /* Reset the buffer pointer. */
            CommandBuffer->logical = 0;
        }

#if gcdSECURE_USER
        if (gcmUINT64_TO_PTR(CommandBuffer->hintArray) != gcvNULL)
        {
            gcmONERROR(gcmOS_SAFE_FREE_SHARED_MEMORY(gcvNULL, gcmUINT64_TO_PTR(CommandBuffer->hintArray)));
            CommandBuffer->hintArray =
            CommandBuffer->hintArrayTail = 0;
        }
#endif
    }

    /* Success. */
    gcmFOOTER();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

static gceSTATUS
_ConstructMirrorCommandBuffer(
    IN gcoHARDWARE Hardware,
    IN gcoBUFFER Buffer,
    IN gcoCMDBUF CommandBuffer
    )
{
    gceSTATUS status;
    gctUINT32 i;

    gcmHEADER();

    /* Get mirror count from gcoBUFFER. */
    CommandBuffer->mirrorCount = Buffer->mirrorCount;

    if (CommandBuffer->mirrorCount)
    {
        /* Allocate mirror of command buffer. */
        gcmONERROR(gcoOS_Allocate(
            gcvNULL,
            gcmSIZEOF(gcoCMDBUF *) * CommandBuffer->mirrorCount,
            (gctPOINTER *)&CommandBuffer->mirrors
            ));

        for (i = 0; i < CommandBuffer->mirrorCount; i++)
        {
            gcmONERROR(gcoCMDBUF_Construct(
                gcvNULL,
                Buffer->hardware,
                CommandBuffer->bytes,
                (gcsCOMMAND_INFO_PTR) &Buffer->info,
                &CommandBuffer->mirrors[i]));

            if (CommandBuffer->bytes != CommandBuffer->mirrors[i]->bytes)
            {
                gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
            }
        }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* gcoCMDBUF_Construct() will handle error and release the whole gcoBUFFER. */
    gcmFOOTER();
    return status;
}

static gceSTATUS
_GetCommandBuffer(
    IN gcoBUFFER Buffer
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcoCMDBUF commandBuffer = gcvNULL;
    gcePIPE_SELECT entryPipe;

    gcmHEADER_ARG("Buffer=0x%x", Buffer);

    /* Determine the next command buffer. */
    if (Buffer->commandBufferTail == gcvNULL)
    {
        /* Select 3D pipe for the first buffer. */
        entryPipe = gcvPIPE_3D;

        /* Get the head of the list. */
        Buffer->commandBufferTail =
        commandBuffer = Buffer->commandBufferList;
    }
    else
    {
        /* Get current entry pipe. */
        entryPipe = Buffer->commandBufferTail->entryPipe;

        /* Get the next command buffer. */
        Buffer->commandBufferTail =
        commandBuffer = Buffer->commandBufferTail->next;
    }

    if (commandBuffer != gcvNULL)
    {
        /* Test if command buffer is available. */
        status = gcoOS_WaitSignal(gcvNULL, commandBuffer->signal, 0);
    }

    /* Not available? */
    if ((status == gcvSTATUS_TIMEOUT) || (commandBuffer == gcvNULL))
    {
        /* Construct new command buffer. */
        if ((Buffer->maxCount == 0) || (Buffer->count < Buffer->maxCount))
        {
            gcoCMDBUF temp = gcvNULL;
            gcmONERROR(gcoCMDBUF_Construct(gcvNULL, Buffer->hardware,
                                           Buffer->bytes,
                                           (gcsCOMMAND_INFO_PTR) &Buffer->info,
                                           &temp));

            /* Insert into the list. */
            if (commandBuffer == gcvNULL)
            {
                Buffer->commandBufferList =
                temp->next = temp->prev = temp;
            }
            else
            {
                temp->prev = commandBuffer->prev;
                temp->next = commandBuffer;
                commandBuffer->prev->next = temp;
                commandBuffer->prev = temp;
            }

            Buffer->commandBufferTail = temp;

            Buffer->count += 1;
            gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_BUFFER,
                          "Using %lu command buffers.",
                          Buffer->count);

            commandBuffer = temp;

            gcmONERROR(_ConstructMirrorCommandBuffer(
                Buffer->hardware,
                Buffer,
                commandBuffer
                ));
        }

        /* Wait for buffer to become available. */
        gcmONERROR(gcoOS_WaitSignal(gcvNULL, commandBuffer->signal, gcvINFINITE));
    }
    else
    {
        gcmONERROR(status);
    }

    /* Set the entry pipe. */
    commandBuffer->entryPipe = entryPipe;

    /* Reset command buffer. */
    commandBuffer->startOffset = 0;
    commandBuffer->offset      = Buffer->info.reservedHead;
    commandBuffer->free        = commandBuffer->bytes - Buffer->totalReserved;

    /* Succees. */
    gcmFOOTER_ARG("commandBuffer=0x%x", commandBuffer);
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

static gceSTATUS
_DuplicateCommandBuffer(
    gcoCMDBUF Dest,
    gcoCMDBUF Src
    )
{
    gctUINT commandBufferSize;

    commandBufferSize = Src->offset + Src->reservedTail - Src->startOffset;
    Dest->startOffset = Src->startOffset;
    Dest->offset    = Src->offset;

    gcoOS_MemCopy(
        gcmUINT64_TO_PTR(Dest->logical + Dest->startOffset),
        gcmUINT64_TO_PTR(Src->logical + Src->startOffset),
        commandBufferSize
        );

    return gcvSTATUS_OK;
}

static gceSTATUS
_DestroyMirrorCommandBuffer(
    IN gcoBUFFER Buffer,
    IN gcoCMDBUF CommandBuffer
    )
{
    gctUINT32 mirrorCount = CommandBuffer->mirrorCount;
    gctUINT32 i;

    for (i = 0; i < mirrorCount; i++)
    {
        if (CommandBuffer->mirrors[i])
        {
            gcmVERIFY_OK(gcoCMDBUF_Destroy(
                Buffer->hardware, &Buffer->info, CommandBuffer->mirrors[i]));
        }
    }

    if (CommandBuffer->mirrors)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, CommandBuffer->mirrors));
    }

    return gcvSTATUS_OK;
}

/******************************************************************************\
***************************** gcoCMDBUF Object Code *****************************
\******************************************************************************/

/*******************************************************************************
**
**  gcoCMDBUF_Construct
**
**  Construct a new gcoCMDBUF object.
**
**  INPUT:
**
**      gcoOS Os
**          Pointer to a gcoOS object.
**
**      gcoHARDWARE Hardware
**          Pointer to a gcoHARDWARE object.
**
**      gctSIZE_T Bytes
**          Number of bytes for the buffer.
**
**      gcsCOMMAND_BUFFER_PTR Info
**          Alignment and head/tail information.
**
**  OUTPUT:
**
**      gcoCMDBUF * CommandBuffer
**          Pointer to a variable that will hold the the gcoCMDBUF object
**          pointer.
*/
gceSTATUS
gcoCMDBUF_Construct(
    IN gcoOS Os,
    IN gcoHARDWARE Hardware,
    IN gctSIZE_T Bytes,
    IN gcsCOMMAND_INFO_PTR Info,
    OUT gcoCMDBUF * CommandBuffer
    )
{
    gceSTATUS status;
    gcoCMDBUF commandBuffer = gcvNULL;
    gctSIZE_T objectSize    = 0;
    gctPOINTER pointer      = gcvNULL;
    gctPHYS_ADDR physical   = gcvNULL;
    gctSIZE_T tmpSize       = 0;
    gctUINT32 node          = 0;

    gcmHEADER_ARG("Bytes=%lu Info=0x%x", Bytes, Info);

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(Bytes > 0);
    gcmDEBUG_VERIFY_ARGUMENT(CommandBuffer != gcvNULL);

    /* Set the size of the object. */
    objectSize = gcmSIZEOF(struct _gcoCMDBUF);

    /* Allocate the gcoCMDBUF object. */
    /* Currently in most OS we are able to access the user-side data from
       the kernel by simple memory mapping, therefore here we allocate the
       object from the cached user memory. */
    gcmONERROR(gcoOS_AllocateSharedMemory(gcvNULL, objectSize, &pointer));
    commandBuffer = pointer;

    /* Reset the command buffer object. */
    gcoOS_ZeroMemory(commandBuffer, objectSize);

    /* Initialize the gcoCMDBUF object. */
    commandBuffer->object.type = gcvOBJ_COMMANDBUFFER;

    /* Create the signal. */
    gcmONERROR(gcoOS_CreateSignal(gcvNULL, gcvFALSE, &commandBuffer->signal));

    gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_SIGNAL,
                  "%s(%d): buffer signal created 0x%08X",
                  __FUNCTION__, __LINE__, commandBuffer->signal);

    /* Mark the buffer as available. */
    gcmONERROR(gcoOS_Signal(gcvNULL, commandBuffer->signal, gcvTRUE));

    /* Try to allocate the command buffer space. */
    while (gcvTRUE)
    {
        gcsHAL_INTERFACE iface;

        iface.ignoreTLS = gcvFALSE;

        /* Set the desired size. */
        gcmSAFECASTSIZET(commandBuffer->bytes, Bytes);

        /* Allocate the buffer for the command. */
        switch (Info->source)
        {
        case gcvCMDBUF_VIRTUAL:
            iface.command = gcvHAL_ALLOCATE_VIRTUAL_COMMAND_BUFFER;
            iface.u.AllocateVirtualCommandBuffer.bytes = commandBuffer->bytes;

            /* Call kernel service. */
            gcmONERROR(gcoOS_DeviceControl(gcvNULL, IOCTL_GCHAL_INTERFACE,
                                           &iface, gcmSIZEOF(iface),
                                           &iface, gcmSIZEOF(iface)));
            status = iface.status;
            if (status == gcvSTATUS_OUT_OF_MEMORY)
                goto retry;
            gcmONERROR(status);

            commandBuffer->bytes = (gctUINT32)iface.u.AllocateVirtualCommandBuffer.bytes;
            commandBuffer->physical = iface.u.AllocateVirtualCommandBuffer.physical;
            commandBuffer->logical = iface.u.AllocateVirtualCommandBuffer.logical;
            break;

        case gcvCMDBUF_CONTIGUOUS:
            tmpSize = (gctSIZE_T) commandBuffer->bytes;
            status = gcoHAL_AllocateContiguous(gcvNULL,
                                              &tmpSize,
                                              &physical,
                                              &pointer);
            if (status == gcvSTATUS_OUT_OF_MEMORY)
                goto retry;
            gcmONERROR(status);

            gcmSAFECASTSIZET(commandBuffer->bytes, tmpSize);
            commandBuffer->physical = gcmPTR2INT32(physical);
            commandBuffer->logical = gcmPTR_TO_UINT64(pointer);
            break;

        case gcvCMDBUF_RESERVED:
            tmpSize = (gctSIZE_T) commandBuffer->bytes;
            status = gcoHAL_AllocateVideoMemory(
                        Info->alignment,
                        gcvSURF_BITMAP,
                        0,
                        gcvPOOL_SYSTEM,
                        &tmpSize,
                        &node);
            if (status == gcvSTATUS_OUT_OF_MEMORY)
                goto retry;
            gcmONERROR(status);

            gcmONERROR(gcoHAL_LockVideoMemory(
                node, gcvFALSE, Info->engine, (gctUINT32 *)&physical, &pointer));
            gcmSAFECASTSIZET(commandBuffer->bytes, tmpSize);
            commandBuffer->physical = node;
            commandBuffer->logical = gcmPTR_TO_UINT64(pointer);

            break;

        default:
            break;
        }

        /* Initialize command buffer. */
        commandBuffer->free = commandBuffer->bytes;

#if gcdSECURE_USER
        /* Determine the size of the state array. */
        commandBuffer->hintArraySize = Bytes;

        /* Allocate the state array. */
        status = gcoOS_AllocateSharedMemory(gcvNULL,
                                (gctSIZE_T) commandBuffer->hintArraySize,
                                &pointer);
        if (status == gcvSTATUS_OUT_OF_MEMORY)
            goto retry;
        gcmONERROR(status);
        commandBuffer->hintArray = gcmPTR_TO_UINT64(pointer);

        /* Initialize the state array tail pointer. */
        commandBuffer->hintArrayTail = commandBuffer->hintArray;
#endif

        /* The command buffer is successfully allocated. */
        break;

retry:
        if (Bytes <= (4 << 10))
            goto OnError;

        /* Free the command buffer objects. */
        gcmONERROR(_FreeCommandBuffer(Hardware, Info, commandBuffer));

        /* Try lower size. */
        Bytes >>= 1;
    }

    commandBuffer->reservedTail = Info->reservedTail;

    /* Return pointer to the gcoCMDBUF object. */
    *CommandBuffer = commandBuffer;

    /* Success. */
    gcmFOOTER_ARG("*CommandBuffer=0x%x", *CommandBuffer);
    return gcvSTATUS_OK;

OnError:
    /* Roll back. */
    gcmVERIFY_OK(gcoCMDBUF_Destroy(Hardware, Info, commandBuffer));

    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoCMDBUF_Destroy
**
**  Destroy a gcoCMDBUF object.
**
**  INPUT:
**
**      gcoCMDBUF CommandBuffer
**          Pointer to an gcoCMDBUF object.
**
**  OUTPUT:
**
**      None.
*/
gceSTATUS
gcoCMDBUF_Destroy(
    IN gcoHARDWARE Hardware,
    IN gcsCOMMAND_INFO_PTR Info,
    IN gcoCMDBUF CommandBuffer
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("CommandBuffer=0x%x", CommandBuffer);

    /* Verify the object. */
    gcmVERIFY_OBJECT(CommandBuffer, gcvOBJ_COMMANDBUFFER);

    /* Destroy command buffer allocations. */
    gcmONERROR(_FreeCommandBuffer(Hardware, Info, CommandBuffer));

    /* Destroy signal. */
    if (CommandBuffer->signal != gcvNULL)
    {
        gcmONERROR(gcoOS_DestroySignal(gcvNULL, CommandBuffer->signal));
        CommandBuffer->signal = gcvNULL;
    }

    /* Free the gcoCMDBUF object. */
    gcmONERROR(gcmOS_SAFE_FREE_SHARED_MEMORY(gcvNULL, CommandBuffer));

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/******************************************************************************\
******************************* gcoBUFFER API Code ******************************
\******************************************************************************/

/*******************************************************************************
**
**  gcoBUFFER_Construct
**
**  Construct a new gcoBUFFER object.
**
**  INPUT:
**
**      gcoHAL Hal
**          Pointer to a gcoHAL object.
**
**      gcoHARDWARE Hardware
**          Pointer to a gcoHARDWARE object.
**
**      gctSIZE_T MaxSize
**          Maximum size of buffer.
**
**      gctBOOL ThreadDefault
**          If gcvTRUE, the buffer is for thread default's hardware object
**
**  OUTPUT:
**
**      gcoBUFFER * Buffer
**          Pointer to a variable that will hold the the gcoBUFFER object
**          pointer.
*/
gceSTATUS
gcoBUFFER_Construct(
    IN gcoHAL Hal,
    IN gcoHARDWARE Hardware,
    IN gceENGINE Engine,
    IN gctSIZE_T MaxSize,
    IN gctBOOL ThreadDefault,
    OUT gcoBUFFER * Buffer
    )
{
    gceSTATUS status;
    gcoBUFFER buffer = gcvNULL;
    gctPOINTER pointer = gcvNULL;
    gcoCMDBUF commandBuffer;
    gctUINT32 mGpuModeSwitchBytes = 0;
    gctUINT i = 0;

    gcmHEADER_ARG("Hal=0x%x Hardware=0x%x MaxSize=%lu",
                  Hal, Hardware, MaxSize);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hal, gcvOBJ_HAL);
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmDEBUG_VERIFY_ARGUMENT(Buffer != gcvNULL);


    /***************************************************************************
    ** Allocate and reset the gcoBUFFER object.
    */

    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(struct _gcoBUFFER), &pointer));

    gcoOS_ZeroMemory(pointer, gcmSIZEOF(struct _gcoBUFFER));

    buffer = (gcoBUFFER) pointer;

    /* Initialize the gcoBUFFER object. */
    buffer->object.type = gcvOBJ_BUFFER;
    buffer->hal         = Hal;
    buffer->threadDefault = ThreadDefault;

    /* Zero the command buffers. */
    buffer->commandBufferList = gcvNULL;
    buffer->commandBufferTail = gcvNULL;

    /* Set the requested size. */
    buffer->bytes = MaxSize;

    /* Set the default maximum number of command buffers. */
    buffer->maxCount = gcdMAX_CMD_BUFFERS;

    /* Reset free patch list. */
    buffer->freePatchList = gcvNULL;
    buffer->patchList     = gcvNULL;

    buffer->hardware = Hardware;
    buffer->info.engine   = Engine;

    /**************************************************************************
    ** Allocate temp command buffer.
    */
    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmMAX_TEMPCMD_BUFFER_SIZE, &buffer->tempCMDBUF.buffer));
    buffer->tempCMDBUF.currentByteSize = 0;
    buffer->tempCMDBUF.inUse = gcvFALSE;

    /***************************************************************************
    ** Query alignment.
    */
    gcmONERROR(gcoHARDWARE_QueryCommandBuffer(
        buffer->hardware,
        buffer->info.engine,
        &buffer->info.alignment,
        &buffer->info.reservedHead,
        &buffer->info.reservedTail,
        &buffer->info.reservedUser,
        &buffer->info.source,
        &mGpuModeSwitchBytes
        ));

#if gcdENABLE_3D
    {
        gctUINT32 gpuCount = 0;
        gcoHARDWARE_Query3DCoreCount(buffer->hardware, &gpuCount);
        if (gpuCount > 1)
        {
            buffer->queryResumeBytes[gcvQUERY_OCCLUSION] = gpuCount * (2 * gcmSIZEOF(gctUINT32) + gcdRESUME_OQ_LENGTH)
                                                         + 2 * gcmSIZEOF(gctUINT32);
            buffer->probeResumeBytes = gpuCount * (2 * gcmSIZEOF(gctUINT32) + gcdRESUME_PROBE_LENGH)
                                     + 2 * gcmSIZEOF(gctUINT32);
        }
        else
        {
            buffer->queryResumeBytes[gcvQUERY_OCCLUSION] = gcdRESUME_OQ_LENGTH;
            buffer->probeResumeBytes = gcdRESUME_PROBE_LENGH;
        }
    }

    buffer->tfbResumeBytes                            = mGpuModeSwitchBytes + gcdRESUME_XFB_LENGH;
    buffer->queryResumeBytes[gcvQUERY_XFB_WRITTEN]    = mGpuModeSwitchBytes + gcdRESUME_XFBWRITTEN_QUERY_LENGTH;
    buffer->queryResumeBytes[gcvQUERY_PRIM_GENERATED] = mGpuModeSwitchBytes + gcdRESUME_PRIMGEN_QUERY_LENGTH;
#endif

    buffer->totalReserved
        = buffer->info.reservedHead
        + buffer->info.reservedTail
        + buffer->info.reservedUser
        + buffer->info.alignment;

    if (!gcoHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_CHIPENABLE_LINK))
    {
        /* NOTE: This function only valid before gcoHARDWARE_Construct() exits. */
        gcoHARDWARE_Query3DCoreCount(Hardware, &buffer->mirrorCount);

        buffer->mirrorCount -= 1;
    }

    /***************************************************************************
    ** Initialize the command buffers.
    */

#if defined(ANDROID)
    /* No buffer when initialization. */
    buffer->count = gcdCMD_BUFFERS;
#else
    /* Construct a command buffer. */
    for (i = 0; i < gcdCMD_BUFFERS; ++i)
    {
        gcmONERROR(gcoCMDBUF_Construct(gcvNULL, buffer->hardware,
                                       MaxSize,
                                       (gcsCOMMAND_INFO_PTR) &buffer->info,
                                       &commandBuffer));

        if (buffer->commandBufferList == gcvNULL)
        {
            buffer->commandBufferList = commandBuffer;
            commandBuffer->prev =
            commandBuffer->next = commandBuffer;
        }
        else
        {
            /* Add to the tail. */
            commandBuffer->prev = buffer->commandBufferList->prev;
            commandBuffer->next = buffer->commandBufferList;
            buffer->commandBufferList->prev->next = commandBuffer;
            buffer->commandBufferList->prev = commandBuffer;
        }

        gcmONERROR(_ConstructMirrorCommandBuffer(Hardware, buffer, commandBuffer));
    }

    /* Number of buffers initialized. */
    buffer->count = gcdCMD_BUFFERS;

    /* Get the current command buffer. */
    gcmONERROR(_GetCommandBuffer(buffer));
#endif

    /* Return pointer to the gcoBUFFER object. */
    *Buffer = buffer;

    /* Success. */
    gcmFOOTER_ARG("*Buffer=0x%x", *Buffer);
    return gcvSTATUS_OK;

OnError:
    /* Roll back. */
    gcmVERIFY_OK(gcoBUFFER_Destroy(buffer));

    /* Return the status. */
    gcmFOOTER();
    return status;
}

static gcsPATCH_LIST *
_GetPatchList(
    IN gcoBUFFER Buffer
    )
{
    gcsPATCH_LIST * patchList = gcvNULL;

    gcmHEADER_ARG("Buffer=0x%x", Buffer);

    if (Buffer->freePatchList)
    {
        patchList = Buffer->freePatchList;
        Buffer->freePatchList = patchList->next;
        patchList->count = 0;
        patchList->next  = gcvNULL;
    }
    else
    {
        if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsPATCH_LIST), (gctPOINTER *) &patchList)))
        {
            return gcvNULL;
        }

        gcoOS_ZeroMemory(patchList, gcmSIZEOF(gcsPATCH_LIST));
    }

    /* Succees. */
    gcmFOOTER_NO();
    return patchList;
}


static gctUINT32
_GetResumeCommandLength(
    IN gcoBUFFER Buffer
    )
{
    gctUINT sizeInBytes = 0;

#if gcdENABLE_3D

    gctINT32 type;

    for (type = gcvQUERY_OCCLUSION; type < gcvQUERY_MAX_NUM; type++)
    {
        if (Buffer->queryPaused[type])
        {
            sizeInBytes += Buffer->queryResumeBytes[type];
        }
    }

    if (Buffer->tfbPaused)
    {
        sizeInBytes += Buffer->tfbResumeBytes;
    }

    if (Buffer->probePaused)
    {
        sizeInBytes += Buffer->probeResumeBytes;
    }
#endif

    return sizeInBytes;
}

gceSTATUS
gcoBUFFER_InsertToPatchList(
    IN gcoBUFFER Buffer,
    IN gctUINT32 Handle,
    IN gctUINT32 Flag
    )
{
    gcsPATCH_LIST * patchList = gcvNULL;
    gcsPATCH_LIST * patchHead = gcvNULL;

    gcmHEADER_ARG("Buffer=0x%x, Handle=0x%x, Flag=0x%x", Buffer, Handle, Flag);

    if (Handle == 0)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

    patchHead = Buffer->patchList;

    /* Always insert patch list from head, so, only check the first patch list */
    if (patchHead && patchHead->count < gcdPATCH_LIST_SIZE)
    {
        patchList = patchHead;
    }

    if (patchList == gcvNULL ||
        patchList->count == gcdPATCH_LIST_SIZE)
    {
        /* Get new patch list */
        patchHead = _GetPatchList(Buffer);
        patchHead->next = patchList;
        Buffer->patchList = patchHead;
        patchList = patchHead;
    }

    gcmASSERT(patchList);

    patchList->patch[patchList->count].handle = Handle;
    patchList->patch[patchList->count].flag = Flag;
    patchList->count++;

    /* Succees. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_FreePatchList(
    IN gcoBUFFER Buffer,
    IN struct _gcsPATCH_LIST *PatchList
    )
{
    gcmHEADER_ARG("Buffer=0x%x, PatchList=0x%x", Buffer, PatchList);

    /* Add to the cache. */
    PatchList->next = Buffer->freePatchList;
    Buffer->freePatchList = PatchList;
    PatchList->count = 0;

    /* Succees. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_RecyclePatchList(
    IN gcoBUFFER Buffer,
    IN gcoCMDBUF CommandBuffer
    )
{
    struct _gcsPATCH_LIST *patchList;

    gcmHEADER_ARG("Buffer=0x%x, CommandBuffer=0x%x", Buffer, CommandBuffer);

    while (CommandBuffer->patchHead != gcmPTR_TO_UINT64(gcvNULL))
    {
        /* Extract the head of the list. */
        patchList = gcmUINT64_TO_PTR(CommandBuffer->patchHead);
        CommandBuffer->patchHead = gcmPTR_TO_UINT64(patchList->next);

        /* Free the patch list item. */
        _FreePatchList(Buffer, patchList);
    }

    /* Succees. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

#if gcdENABLE_3D
static gceSTATUS
_FreeFenceList(
    gcsFENCE_LIST_PTR fenceList
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("fenceList=0x%x", fenceList);

    if (fenceList)
    {
        if (fenceList->pendingList)
        {
            gcoOS_Free(gcvNULL, fenceList->pendingList);
            fenceList->pendingList = gcvNULL;
        }

        if (fenceList->onIssueList)
        {
            gcoOS_Free(gcvNULL, fenceList->onIssueList);
            fenceList->onIssueList = gcvNULL;
        }

        gcoOS_Free(gcvNULL, fenceList);
    }

    gcmFOOTER();
    return status;
}

static gceSTATUS
_AllocFenceList(
    IN gcsFENCE_LIST_PTR srcList,
    IN gcsFENCE_LIST_PTR * outList
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctPOINTER pointer;
    gcsFENCE_LIST_PTR fenceList = gcvNULL;
    gctUINT pendingCount;
    gctUINT onIssueCount;
    gctUINT size;

    gcmHEADER_ARG("srcList=0x%x, outList=0x%x", srcList, outList);

    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsFENCE_LIST), &pointer));
    fenceList = (gcsFENCE_LIST_PTR)pointer;
    gcoOS_ZeroMemory(fenceList, gcmSIZEOF(gcsFENCE_LIST));

    pendingCount = FENCE_NODE_LIST_INIT_COUNT;
    onIssueCount = FENCE_NODE_LIST_INIT_COUNT;

    if (srcList)
    {
        pendingCount += srcList->pendingCount;
        onIssueCount += srcList->onIssueCount;
    }

    size = gcmSIZEOF(gcsFENCE_APPEND_NODE) * pendingCount;
    gcmONERROR(gcoOS_Allocate(gcvNULL, size, &pointer));
    fenceList->pendingList = (gcsFENCE_APPEND_NODE_PTR)pointer;
    fenceList->pendingAllocCount = pendingCount;

    if (srcList && srcList->pendingCount > 0)
    {
        gcoOS_MemCopy(fenceList->pendingList,
            srcList->pendingList,
            srcList->pendingCount * gcmSIZEOF(gcsFENCE_APPEND_NODE));
        fenceList->pendingCount += srcList->pendingCount;
    }

    size = gcmSIZEOF(gcsFENCE_APPEND_NODE) * onIssueCount;
    gcmONERROR(gcoOS_Allocate(gcvNULL, size, &pointer));
    fenceList->onIssueList = ((gcsFENCE_APPEND_NODE_PTR)pointer);
    fenceList->onIssueAllocCount = onIssueCount;

    if (srcList && srcList->onIssueCount > 0)
    {
        gcoOS_MemCopy(fenceList->onIssueList,
            srcList->onIssueList,
            srcList->onIssueCount * gcmSIZEOF(gcsFENCE_APPEND_NODE));
        fenceList->onIssueCount += srcList->onIssueCount;
    }

    *outList = fenceList;

    gcmFOOTER();
    return status;

OnError:
    _FreeFenceList(fenceList);
    *outList = gcvNULL;

    gcmFOOTER();
    return status;
}

gceSTATUS
gcoBUFFER_AppendFence(
    IN gcoBUFFER Buffer,
    IN gcsSURF_NODE_PTR Node,
    IN gceFENCE_TYPE Type
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsFENCE_LIST_PTR fenceList;

    gcmHEADER_ARG("Buffer=0x%x", Buffer);

    fenceList = Buffer->fenceList;

    /* empty or full */
    if (!fenceList ||
        (fenceList->pendingAllocCount == fenceList->pendingCount))
    {
        gcmONERROR(_AllocFenceList(fenceList, &fenceList));

        if (Buffer->fenceList)
        {
            _FreeFenceList(Buffer->fenceList);
        }

        Buffer->fenceList = fenceList;
    }

    fenceList->pendingList[fenceList->pendingCount].node = Node;
    fenceList->pendingList[fenceList->pendingCount].type = Type;
    fenceList->pendingCount++;

OnError:
    gcmFOOTER();
    return status;
}

/*after command that use resource in pending list, we need move it to onIssue list  */
gceSTATUS
gcoBUFFER_OnIssueFence(
    IN gcoBUFFER Buffer
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT count;
    gctPOINTER pointer;
    gcsFENCE_LIST_PTR fenceList = gcvNULL;

    gcmHEADER_ARG("Buffer=0x%x", Buffer);

    fenceList = Buffer->fenceList;

    if (Buffer->tempCMDBUF.inUse)
    {
        gcmPRINT("Warning, should not OnIssue Fence in temp command buffer");
    }

    if (fenceList && fenceList->pendingCount > 0)
    {
        if (fenceList->onIssueAllocCount - fenceList->onIssueCount < fenceList->pendingCount)
        {
            gcsFENCE_APPEND_NODE_PTR ptr;

            count = fenceList->onIssueCount + fenceList->pendingCount + FENCE_NODE_LIST_INIT_COUNT;

            gcmONERROR(gcoOS_Allocate(gcvNULL, count * gcmSIZEOF(gcsFENCE_APPEND_NODE), &pointer));

            ptr = (gcsFENCE_APPEND_NODE_PTR)pointer;
            fenceList->onIssueAllocCount = count;

            gcoOS_MemCopy(ptr, fenceList->onIssueList, fenceList->onIssueCount * gcmSIZEOF(gcsFENCE_APPEND_NODE));
            gcoOS_Free(gcvNULL, fenceList->onIssueList);

            fenceList->onIssueList = ptr;
        }

        gcoOS_MemCopy(&fenceList->onIssueList[fenceList->onIssueCount],
            fenceList->pendingList,
            fenceList->pendingCount * gcmSIZEOF(gcsFENCE_APPEND_NODE));

        fenceList->onIssueCount += fenceList->pendingCount;
        fenceList->pendingCount = 0;
    }

OnError:
    gcmFOOTER();
    return status;
}

/* Before send fence, we need get fence for all onIssue fence */
gceSTATUS
gcoBUFFER_GetFence(
    IN gcoBUFFER Buffer
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT i;
    gcsSURF_NODE_PTR node;
    gcsFENCE_LIST_PTR fenceList;

    gcmHEADER_ARG("Buffer=0x%x", Buffer);

    fenceList = Buffer->fenceList;

    if(fenceList)
    {
        for (i = 0; i < fenceList->onIssueCount; i++)
        {
            node = fenceList->onIssueList[i].node;
            gcoHARDWARE_GetFence(gcvNULL, &node->fenceCtx, Buffer->info.engine, fenceList->onIssueList[i].type);
        }

        fenceList->onIssueCount = 0;
    }

    gcmFOOTER();
    return status;
}
#endif

/*******************************************************************************
**
**  gcoBUFFER_Destroy
**
**  Destroy an gcoBUFFER object.
**
**  INPUT:
**
**      gcoBUFFER Buffer
**          Pointer to an gcoBUFFER object to delete.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoBUFFER_Destroy(
    IN gcoBUFFER Buffer
    )
{
    gceSTATUS status;
    gcoCMDBUF commandBuffer;
    struct _gcsPATCH_LIST *patchList;

    gcmHEADER_ARG("Buffer=0x%x", Buffer);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Buffer, gcvOBJ_BUFFER);

    /* Destroy all command buffers. */
    while (Buffer->commandBufferList != gcvNULL)
    {
        /* Get the head of the list. */
        commandBuffer = Buffer->commandBufferList;

        /* Remove the head buffer from the list. */
        if (commandBuffer->next == commandBuffer)
        {
            Buffer->commandBufferList = gcvNULL;
        }
        else
        {
            commandBuffer->prev->next =
            Buffer->commandBufferList = commandBuffer->next;
            commandBuffer->next->prev = commandBuffer->prev;
        }

        /* Free all patch list items. */
        gcmONERROR(_RecyclePatchList(Buffer, commandBuffer));

        gcmVERIFY_OK(_DestroyMirrorCommandBuffer(Buffer, commandBuffer));

        /* Destroy command buffer. */
        gcmONERROR(gcoCMDBUF_Destroy(Buffer->hardware, &Buffer->info, commandBuffer));
    }
#if gcdENABLE_3D
    gcmONERROR(_FreeFenceList(Buffer->fenceList));
    Buffer->fenceList = gcvNULL;
#endif

    /* Destroy the free patch list. */
    while (Buffer->freePatchList != NULL)
    {
        patchList = Buffer->freePatchList;
        Buffer->freePatchList = patchList->next;
        gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, patchList));
    }

    while (Buffer->patchList != gcvNULL)
    {
        patchList = Buffer->patchList;
        Buffer->patchList = patchList->next;
        gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, patchList));
    }

    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, Buffer->tempCMDBUF.buffer));

    /* Free the object memory. */
    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, Buffer));

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoBUFFER_Write
**
**  Copy a number of bytes into the buffer.
**
**  INPUT:
**
**      gcoBUFFER Buffer
**          Pointer to an gcoBUFFER object.
**
**      gctCONST_POINTER Data
**          Pointer to a buffer that contains the data to be copied.
**
**      IN gctSIZE_T Bytes
**          Number of bytes to copy.
**
**      IN gctBOOL Aligned
**          gcvTRUE if the data needs to be aligned to 64-bit.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoBUFFER_Write(
    IN gcoBUFFER Buffer,
    IN gctCONST_POINTER Data,
    IN gctSIZE_T Bytes,
    IN gctBOOL Aligned
    )
{
    gceSTATUS status;
    gcoCMDBUF reserve;

    gcmHEADER_ARG("Buffer=0x%x Data=0x%x Bytes=%lu Aligned=%d",
                  Buffer, Data, Bytes, Aligned);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Buffer, gcvOBJ_BUFFER);
    gcmDEBUG_VERIFY_ARGUMENT(Data != gcvNULL);
    gcmDEBUG_VERIFY_ARGUMENT(Bytes > 0);

    /* Reserve data in the buffer. */
    gcmONERROR(gcoBUFFER_Reserve(Buffer, Bytes, Aligned, gcvCOMMAND_3D, &reserve));

    /* Write data into the buffer. */
    gcoOS_MemCopy(gcmUINT64_TO_PTR(reserve->lastReserve), Data, Bytes);

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoBUFFER_Reserve
**
**  Reserve a number of bytes in the buffer.
**
**  INPUT:
**
**      gcoBUFFER Buffer
**          Pointer to an gcoBUFFER object.
**
**      gctSIZE_T Bytes
**          Number of bytes to reserve.
**
**      gctBOOL Aligned
**          gcvTRUE if the data needs to be aligned to 64-bit.
**
**  OUTPUT:
**
**      gctUINT32_PTR ** AddressHints
**          Pointer to a variable that receives the current position in the
**          state hint array.  gcvNULL is allowed.
**
**      gctPOINTER * Memory
**          Pointer to a variable that will hold the address of location in the
**          buffer that has been reserved.
*/
gceSTATUS
gcoBUFFER_Reserve(
    IN gcoBUFFER Buffer,
    IN gctSIZE_T Bytes,
    IN gctBOOL Aligned,
    IN gctUINT32 Usage,
    OUT gcoCMDBUF * Reserve
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcoCMDBUF commandBuffer;
    gctUINT32 alignBytes, finalBytes = 0, originalBytes, resumeBytes, reserveBytes, alignedBytes;
    gctUINT32 offset;
    gcsHAL_INTERFACE iface;
    gctBOOL notInSamePage = gcvFALSE;

#if gcdENABLE_3D
    gctINT32 queryType;
#endif

    gcmHEADER_ARG("Buffer=0x%x Bytes=%lu Aligned=%d Reserve=0x%x",
                  Buffer, Bytes, Aligned, Reserve);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Buffer, gcvOBJ_BUFFER);
    gcmDEBUG_VERIFY_ARGUMENT(Reserve != gcvNULL);

    gcmSAFECASTSIZET(originalBytes, Bytes);

    /* Recursive call must be detected and avoided */
    gcmASSERT(Buffer->inRerserved == gcvFALSE);
    Buffer->inRerserved = gcvTRUE;

    resumeBytes = _GetResumeCommandLength(Buffer);

    reserveBytes = originalBytes + resumeBytes;

    /* Get the current command buffer. */
    commandBuffer = Buffer->commandBufferTail;

    if (commandBuffer == gcvNULL)
    {
        /* Grab a new command buffer. */
        gcmONERROR(_GetCommandBuffer(Buffer));

        /* Get the new buffer. */
        commandBuffer = Buffer->commandBufferTail;
    }

    if (Buffer->threadDefault)
    {
        gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcvZONE_BUFFER,
            "Thread Default command buffer is accumulating commands");
    }


    if (Buffer->tempCMDBUF.inUse)
    {
        gcmFATAL("!!!temp command buffer is in using, should not send command into real command buffer");
    }

    /* Compute the number of aligned bytes. */
    alignBytes = Aligned
               ? ( gcmALIGN(commandBuffer->offset, Buffer->info.alignment)
                 - commandBuffer->offset
                 )
               : 0;

    /* Compute the number of required bytes. */
    alignedBytes = reserveBytes + alignBytes;

    if (gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_PIPE_2D)
     && gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_PIPE_3D)
     && !gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_MMU)
     && (Usage == gcvCOMMAND_2D))
    {
        offset = commandBuffer->offset + alignBytes;

        if (((offset + reserveBytes - 1) & ~0xFFF) != (offset & ~0xFFF))
        {
            notInSamePage = gcvTRUE;
        }
    }

    if (alignedBytes > commandBuffer->free || notInSamePage)
    {
        /* Sent event to signal when command buffer completes. */
        iface.command            = gcvHAL_SIGNAL;
        iface.engine             = Buffer->info.engine;
        iface.u.Signal.signal    = gcmPTR_TO_UINT64(commandBuffer->signal);
        iface.u.Signal.auxSignal = 0;
        iface.u.Signal.process   = gcmPTR_TO_UINT64(gcoOS_GetCurrentProcessID());
        iface.u.Signal.fromWhere = gcvKERNEL_COMMAND;

        /* Send event. */
        gcmONERROR(gcoHARDWARE_CallEvent(Buffer->hardware, &iface));

        /* Commit current command buffer. */
        gcmONERROR(gcoHARDWARE_Commit(Buffer->hardware));

        /* Grab a new command buffer. */
        gcmONERROR(_GetCommandBuffer(Buffer));

        /* Get new buffer. */
        commandBuffer = Buffer->commandBufferTail;

        if (resumeBytes == 0)
        {
            resumeBytes = _GetResumeCommandLength(Buffer);
            reserveBytes = originalBytes + resumeBytes;
        }

        if (reserveBytes > commandBuffer->free)
        {
            /* This just won't fit! */
            gcmFATAL("FATAL: Command of %lu original bytes + %lu resume bytes is too big!", originalBytes, resumeBytes);
            gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
        }
        /* Calculate total bytes again. */
        alignBytes = 0;
        finalBytes = reserveBytes;
    }
    else
    {
        finalBytes = alignedBytes;
    }

    gcmASSERT(commandBuffer != gcvNULL);
    gcmASSERT(finalBytes <= commandBuffer->free);

    /* Determine the data offset. */
    offset = commandBuffer->offset + alignBytes;

    /* Update the last reserved location. */
    commandBuffer->lastReserve = gcmPTR_TO_UINT64((gctUINT8_PTR) gcmUINT64_TO_PTR(commandBuffer->logical) + offset);
    commandBuffer->lastOffset  = offset;

    /* Adjust command buffer size. */
    commandBuffer->offset += finalBytes;
    commandBuffer->free   -= finalBytes;

#if gcdENABLE_3D
    for (queryType = gcvQUERY_OCCLUSION; queryType < gcvQUERY_MAX_NUM; queryType++)
    {
        if (Buffer->queryPaused[queryType])
        {
            gctUINT64 resumeCommand, resumeCommandSaved;
            resumeCommand = resumeCommandSaved = commandBuffer->lastReserve;
            gcoHARDWARE_SetQuery(Buffer->hardware,
                                 ~0U,
                                 queryType,
                                 gcvQUERYCMD_RESUME,
                                 (gctPOINTER *)&resumeCommand);
            gcmASSERT((resumeCommand - resumeCommandSaved) == Buffer->queryResumeBytes[queryType]);
            commandBuffer->lastReserve = resumeCommand;
            commandBuffer->lastOffset += (gctUINT32)(resumeCommand - resumeCommandSaved);
            Buffer->queryPaused[queryType] = gcvFALSE;
        }
    }

    if (Buffer->tfbPaused)
    {
        gctUINT64 resumeCommand, resumeCommandSaved;
        resumeCommand = resumeCommandSaved = commandBuffer->lastReserve;
        gcoHARDWARE_SetXfbCmd(Buffer->hardware, gcvXFBCMD_RESUME_INCOMMIT, (gctPOINTER *)&resumeCommand);
        gcmASSERT((resumeCommand - resumeCommandSaved) == Buffer->tfbResumeBytes);
        commandBuffer->lastReserve = resumeCommand;
        commandBuffer->lastOffset += (gctUINT32)(resumeCommand - resumeCommandSaved);
        Buffer->tfbPaused = gcvFALSE;
    }

    if (Buffer->probePaused)
    {
        gctUINT64 resumeCommand, resumeCommandSaved;
        resumeCommand = resumeCommandSaved = commandBuffer->lastReserve;
        gcoHARDWARE_SetProbeCmd(Buffer->hardware, gcvPROBECMD_RESUME, ~0U, (gctPOINTER *)&resumeCommand);
        gcmASSERT((resumeCommand - resumeCommandSaved) == Buffer->probeResumeBytes);
        commandBuffer->lastReserve = resumeCommand;
        commandBuffer->lastOffset += (gctUINT32)(resumeCommand - resumeCommandSaved);
        Buffer->probePaused = gcvFALSE;
    }
#endif

    if (Usage & gcvCOMMAND_3D)
    {
        commandBuffer->using3D = gcvTRUE;
    }

    if (Usage & gcvCOMMAND_2D)
    {
        commandBuffer->using2D = gcvTRUE;
    }

    /* Set the result. */
    * Reserve = commandBuffer;

    Buffer->inRerserved = gcvFALSE;

    /* Success. */
    gcmFOOTER();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoBUFFER_Commit
**
**  Commit the command buffer to the hardware.
**
**  INPUT:
**
**      gcoBUFFER Buffer
**          Pointer to a gcoBUFFER object.
**
**      gcePIPE_SELECT CurrentPipe
**          Current graphics pipe.
**
**      gcsSTATE_DELTA_PTR StateDelta
**          Pointer to the state delta.
**
**      gcoQUEUE Queue
**          Pointer to a gcoQUEUE object.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoBUFFER_Commit(
    IN gcoBUFFER Buffer,
    IN gcePIPE_SELECT CurrentPipe,
    IN gcsSTATE_DELTA_PTR StateDelta,
    IN gcsSTATE_DELTA_PTR *StateDeltas,
    IN gctUINT32 Context,
    IN gctUINT32_PTR Contexts,
    IN gcoQUEUE Queue,
    OUT gctPOINTER *DumpLogical,
    OUT gctUINT32 *DumpBytes
    )
{
    gceSTATUS status;
    gcoCMDBUF commandBuffer;
    gctBOOL emptyBuffer;

    gcmHEADER_ARG("Buffer=0x%x Queue=0x%x",
                  Buffer, Queue);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Buffer, gcvOBJ_BUFFER);
    gcmVERIFY_OBJECT(Queue, gcvOBJ_QUEUE);

    if (Buffer->threadDefault)
    {
        gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcvZONE_BUFFER, "Thread Default command buffer is comitting commands");
    }

    /* Grab the head command buffer. */
    commandBuffer = Buffer->commandBufferTail;

    if (commandBuffer == gcvNULL)
    {
        /* No current command buffer, of course empty buffer. */
        emptyBuffer = gcvTRUE;
    }
    else
    {
        /* Advance commit count. */
        commandBuffer->commitCount++;

        /* Determine whether the buffer is empty. */
        emptyBuffer = (commandBuffer->offset - commandBuffer->startOffset <= Buffer->info.reservedHead);
    }

    /* Send for execution. */
    if (emptyBuffer)
    {
        gcmONERROR(gcoQUEUE_Commit(Queue, gcvFALSE));
    }
    else
    {
        gcsHAL_INTERFACE iface;
        gctUINT32 newOffset;
        gctSIZE_T spaceLeft;
        gctUINT32 coreCount = 1;
        gcoCMDBUF *commandBufferMirrors = commandBuffer->mirrors;

#if gcdENABLE_3D
        gcoCMDBUF tailCommandBuffer = Buffer->commandBufferTail;
        gctUINT alignedBytes = gcmALIGN(tailCommandBuffer->offset, Buffer->info.alignment) - tailCommandBuffer->offset;

        tailCommandBuffer->offset += alignedBytes;

        if (tailCommandBuffer->using3D)
        {
            gctINT32 type;

            if (Buffer->info.engine == gcvENGINE_RENDER)
            {
                /* need pauseQuery per commit and resumeQuery when next draw flushQuery */
                for (type = gcvQUERY_OCCLUSION; type < gcvQUERY_MAX_NUM; type++)
                {
                    gctUINT64 pauseQueryCommand, pauseQueryCommandsaved;
                    pauseQueryCommandsaved =
                    pauseQueryCommand      = gcmPTR_TO_UINT64((gctUINT8_PTR) gcmUINT64_TO_PTR(tailCommandBuffer->logical)
                                                    + tailCommandBuffer->offset);

                    gcoHARDWARE_SetQuery(gcvNULL, ~0U, (gceQueryType)type, gcvQUERYCMD_PAUSE, (gctPOINTER*)&pauseQueryCommand);
                    if ((pauseQueryCommand - pauseQueryCommandsaved) > 0)
                    {
                        tailCommandBuffer->offset += (gctUINT32)(pauseQueryCommand - pauseQueryCommandsaved);
                        Buffer->queryPaused[type] = gcvTRUE;
                    }
                }

                /* need pauseXFB per commit and resuemeXFB when next draw flushXFB */
                if (gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_HW_TFB))
                {
                    gctUINT64 pauseXfbCommand, pauseXfbCommandsaved;

                    pauseXfbCommandsaved =
                    pauseXfbCommand      = gcmPTR_TO_UINT64((gctUINT8_PTR) gcmUINT64_TO_PTR(tailCommandBuffer->logical)
                                                + tailCommandBuffer->offset);

                    gcoHARDWARE_SetXfbCmd(gcvNULL, gcvXFBCMD_PAUSE_INCOMMIT, (gctPOINTER*)&pauseXfbCommand);
                    if (pauseXfbCommand - pauseXfbCommandsaved > 0)
                    {
                        tailCommandBuffer->offset += (gctUINT32)(pauseXfbCommand - pauseXfbCommandsaved);
                        Buffer->tfbPaused = gcvTRUE;
                    }
                }

                if (gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_PROBE))
                {
                    gctUINT64 pauseProbeCommand, pauseProbeCommandsaved;

                    pauseProbeCommandsaved =
                    pauseProbeCommand      = gcmPTR_TO_UINT64((gctUINT8_PTR)gcmUINT64_TO_PTR(tailCommandBuffer->logical)
                                                    + tailCommandBuffer->offset);

                    gcoHARDWARE_SetProbeCmd(gcvNULL, gcvPROBECMD_PAUSE, ~0U, (gctPOINTER*)&pauseProbeCommand);
                    if (pauseProbeCommand - pauseProbeCommandsaved > 0)
                    {
                        tailCommandBuffer->offset += (gctUINT32)(pauseProbeCommand - pauseProbeCommandsaved);
                        Buffer->probePaused = gcvTRUE;
                    }
                }

                gcoHARDWARE_Query3DCoreCount(gcvNULL, &coreCount);

                alignedBytes = gcmALIGN(tailCommandBuffer->offset, Buffer->info.alignment) - tailCommandBuffer->offset;
                if (coreCount > 1)
                {
                    gctUINT32 bytes;
                    gctUINT32_PTR syncCommandLogical;

                    syncCommandLogical = (gctUINT32_PTR)((gctUINT8_PTR)gcmUINT64_TO_PTR(tailCommandBuffer->logical) +
                                                         tailCommandBuffer->offset + alignedBytes);

                    gcoHARDWARE_MultiGPUCacheFlush(gcvNULL, &syncCommandLogical);

                    gcoHARDWARE_MultiGPUSync(gcvNULL, &syncCommandLogical);
                    gcoHARDWARE_QueryMultiGPUSyncLength(gcvNULL, &bytes);
                    tailCommandBuffer->offset += bytes + alignedBytes;
                    gcoHARDWARE_QueryMultiGPUCacheFlushLength(gcvNULL, &bytes);
                    tailCommandBuffer->offset += bytes;
                }
                else
                {
                    gctUINT8_PTR flushCacheCommand, flushCacheCommandsaved;

                    flushCacheCommandsaved =
                    flushCacheCommand = (gctUINT8_PTR)gcmUINT64_TO_PTR(tailCommandBuffer->logical)
                                      + tailCommandBuffer->offset + alignedBytes;

                    /* need flush ccache zcache and shl1_cache per commit */
                    gcoHARDWARE_FlushCache(gcvNULL, (gctPOINTER *)&flushCacheCommand);
                    tailCommandBuffer->offset += (gctUINT32)(flushCacheCommand - flushCacheCommandsaved) + alignedBytes;
                }
            }

#if gcdSYNC
            /*Only send hw fence when commit, the fence type is hw fence when support gcvFEAUTRE_FENCE_32BIT*/
            if ((gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_FENCE_32BIT) || gcoHARDWARE_IsFeatureAvailable(gcvNULL, gcvFEATURE_FENCE_64BIT)) &&
                Buffer->inRerserved == gcvFALSE)
            {
                gctUINT8_PTR fenceCommand, fenceCommandSaved;

                alignedBytes = gcmALIGN(tailCommandBuffer->offset, Buffer->info.alignment) - tailCommandBuffer->offset;

                fenceCommandSaved =
                fenceCommand = (gctUINT8_PTR)gcmUINT64_TO_PTR(tailCommandBuffer->logical)
                                                      + tailCommandBuffer->offset + alignedBytes;

                if (Buffer->info.engine == gcvENGINE_BLT)
                {
                    /*no need flush cache and do multi gpu sync here*/
                    gcoHARDWARE_SendFence(gcvNULL, gcvFALSE, gcvENGINE_BLT,(gctPOINTER*)&fenceCommand);
                }
                else
                {
                    /*no need flush cache and do multi gpu sync here*/
                    gcoHARDWARE_SendFence(gcvNULL, gcvFALSE, gcvENGINE_RENDER,(gctPOINTER*)&fenceCommand);
                }
                tailCommandBuffer->offset += (gctUINT32)(fenceCommand - fenceCommandSaved) + alignedBytes;
            }
#endif
        }

#endif
        /* Make sure the tail got aligned properly. */
        commandBuffer->offset = gcmALIGN(commandBuffer->offset, Buffer->info.alignment);

#if (gcdDUMP || (gcdUSE_VX && gcdENABLE_3D))
        *DumpLogical = (gctUINT8_PTR) gcmUINT64_TO_PTR(commandBuffer->logical)
                     + commandBuffer->startOffset
                     + Buffer->info.reservedHead;

        *DumpBytes = commandBuffer->offset
                   - commandBuffer->startOffset
                   - Buffer->info.reservedHead;
#endif

        /* The current pipe becomes the exit pipe for the current command buffer. */
        commandBuffer->exitPipe = CurrentPipe;

        if (gcoHAL_GetOption(gcvNULL, gcvOPTION_KERNEL_FENCE))
        {
#if gcdENABLE_3D
            gcmONERROR(gcoHARDWARE_BuildPatchList(Buffer->hardware, Buffer, commandBuffer, Buffer->info.engine));
#endif

            commandBuffer->patchHead = gcmPTR_TO_UINT64(Buffer->patchList);
            Buffer->patchList = gcvNULL;
        }

        iface.u.Commit.count = coreCount;
        iface.u.Commit.delta = gcmPTR_TO_UINT64(iface.u.Commit.deltas);
        iface.u.Commit.context = gcmPTR_TO_UINT64(iface.u.Commit.contexts);
        iface.u.Commit.commandBuffer = gcmPTR_TO_UINT64(iface.u.Commit.commandBuffers);

        /* Send command and context buffer to hardware. */
#if gcdENABLE_3D
        iface.u.Commit.contexts[0] = (commandBuffer->using2D && !commandBuffer->using3D)
                               ? 0 : Context;
#else
        iface.u.Commit.contexts[0] = 0;
#endif

        iface.ignoreTLS = gcvFALSE;
        iface.command = gcvHAL_COMMIT;
        iface.engine = Buffer->info.engine;
        iface.u.Commit.commandBuffers[0] = gcmPTR_TO_UINT64(commandBuffer);
        iface.u.Commit.deltas[0] = gcmPTR_TO_UINT64(StateDelta);
        iface.u.Commit.queue = gcmPTR_TO_UINT64(Queue->head);

        iface.u.Commit.shared = (coreCount > 1) ? gcvTRUE : gcvFALSE;
        iface.u.Commit.index = 0;

        if (Buffer->info.engine == gcvENGINE_RENDER)
        {
            gctUINT32 i;
            gctUINT32 originalCoreIndex;

            gcmONERROR(gcoHAL_GetCurrentCoreIndex(gcvNULL, &originalCoreIndex));

            for (i = 1; i < coreCount; i++)
            {
                gctUINT32 coreIndex;

                /* Convert core index in this hardware to global core index. */
                gcmONERROR(gcoHARDWARE_QueryCoreIndex(gcvNULL, i, &coreIndex));

                if (StateDeltas)
                {
                    /* Get State Deltas of the GPU. */
                    iface.u.Commit.deltas[i] = gcmPTR_TO_UINT64(StateDeltas[coreIndex]);
                }
                else
                {
                    iface.u.Commit.deltas[i] = iface.u.Commit.deltas[0];
                }

                if (Contexts)
                {
                    /* Get Context of the GPU. */
                    iface.u.Commit.contexts[i] = Contexts[coreIndex];
                }
                else
                {
                    iface.u.Commit.contexts[i] = iface.u.Commit.contexts[0];
                }

                if (commandBufferMirrors)
                {
                    /* It is not needed to index commandBufferMirrors with real coreIndex,
                    *  because content in mirrors always keeps same as command buffer, no
                    *  matter what its index is. */
                    _DuplicateCommandBuffer(commandBufferMirrors[i - 1], commandBuffer);
                    iface.u.Commit.commandBuffers[i] = gcmPTR_TO_UINT64(commandBufferMirrors[i - 1]);
                    /* Advance commit count. */
                    commandBufferMirrors[i - 1]->commitCount++;
                }
                else
                {
                    iface.u.Commit.commandBuffers[i] = 0;
                }

                /* Set it to TLS to find correct command queue. */
                gcmONERROR(gcoHAL_SetCoreIndex(gcvNULL, coreIndex));
            }

            /* Restore core index in TLS. */
            gcmONERROR(gcoHAL_SetCoreIndex(gcvNULL, originalCoreIndex));
        }

        /* Call kernel service. */
        gcmONERROR(gcoOS_DeviceControl(gcvNULL,
                                       IOCTL_GCHAL_INTERFACE,
                                       &iface, gcmSIZEOF(iface),
                                       &iface, gcmSIZEOF(iface)));
        gcmONERROR(iface.status);

        /* Update commit stamp. */
        Buffer->commitStamp = iface.u.Commit.commitStamp;

        /* Advance the offset for next commit. */
        newOffset = commandBuffer->offset + Buffer->info.reservedTail;

        /* Compute the size of the space left in the buffer. */
        spaceLeft = commandBuffer->bytes - newOffset;

        if (spaceLeft > Buffer->totalReserved)
        {
            /* Adjust buffer offset and size. */
            commandBuffer->startOffset = newOffset;
            commandBuffer->offset      = commandBuffer->startOffset + Buffer->info.reservedHead;
            commandBuffer->free        = commandBuffer->bytes - commandBuffer->offset
                                       - Buffer->info.alignment - Buffer->info.reservedTail - Buffer->info.reservedUser;
        }
        else
        {
            /* Buffer is full. */
            commandBuffer->startOffset = commandBuffer->bytes;
            commandBuffer->offset      = commandBuffer->bytes;
            commandBuffer->free        = 0;
        }

        /* The exit pipe becomes the entry pipe for the next command buffer. */
        commandBuffer->entryPipe = commandBuffer->exitPipe;

#if gcdSECURE_USER
        /* Reset the state array tail. */
        commandBuffer->hintArrayTail = commandBuffer->hintArray;
#endif

        /* Reset usage flags. */
        commandBuffer->using2D         = gcvFALSE;
        commandBuffer->using3D         = gcvFALSE;

        /* Kernel already set fenceID to all node of patchList */
        gcmONERROR(_RecyclePatchList(Buffer, commandBuffer));
    }

    /* Empty buffer must be the tail. */
    gcmONERROR(gcoQUEUE_Free(Queue));

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoBUFFER_IsEmpty
**
**  Check if no command acculated in the buffer.
**
**  INPUT:
**
**      gcoBUFFER Buffer
**          Pointer to a gcoBUFFER object
**
**  OUTPUT:
**
**      None.
*/
gceSTATUS
gcoBUFFER_IsEmpty(
    gcoBUFFER Buffer
    )
{
    gceSTATUS status;
    gcoCMDBUF commandBuffer;
    gcmHEADER_ARG("Buffer=0x%x", Buffer);

    commandBuffer = Buffer->commandBufferTail;

    if (commandBuffer == gcvNULL)
    {
        status = gcvSTATUS_TRUE;
    }
    else if (commandBuffer->offset - commandBuffer->startOffset <= Buffer->info.reservedHead)
    {
        status = gcvSTATUS_TRUE;
    }
    else
    {
        status = gcvSTATUS_FALSE;
    }

    gcmFOOTER_ARG("status=%d", status);
    return status;
}

/*******************************************************************************
**
**  gcoBUFFER_StartTEMPCMDBUF
**
**  Star to use temp command buffer
**
**  INPUT:
**
**      gcoBUFFER Buffer
**          Pointer to a gcoBUFFER object
**
**
**
**  OUTPUT:
**
**      gcsTEMPCMDBUF *tempCMDBUF
**          Pointer to a variable that will hold the gcsTEMPCMDBUF
*/

gceSTATUS
gcoBUFFER_StartTEMPCMDBUF(
    IN gcoBUFFER Buffer,
    OUT gcsTEMPCMDBUF *tempCMDBUF
    )
{
    gcmHEADER_ARG("Buffer=0x%x tempCMDBUF=0x%x", Buffer, tempCMDBUF);

    gcmVERIFY_OBJECT(Buffer, gcvOBJ_BUFFER);
    gcmVERIFY_ARGUMENT(tempCMDBUF != gcvNULL);

    *tempCMDBUF = &Buffer->tempCMDBUF;

    gcmASSERT((*tempCMDBUF)->currentByteSize == 0);

    gcmASSERT((*tempCMDBUF)->inUse == gcvFALSE);

    (*tempCMDBUF)->inUse = gcvTRUE;

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoBUFFER_EndTEMPCMDBUF
**
**  End using temp command buffer and send to real command buffer.
**
**  INPUT:
**      gcoBUFFER Buffer
**          Pointer to a gcoBUFFER object
**
**      gctBOOL Drop
**          The tempCmdbuffer has incomplete command to drop off.
**
**  OUTPUT:
**
**      Nothing
*/

gceSTATUS
gcoBUFFER_EndTEMPCMDBUF(
    IN gcoBUFFER Buffer,
    IN gctBOOL   Drop
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsTEMPCMDBUF tempCMDBUF;
    gcmHEADER_ARG("Buffer=0x%x Drop=%d", Buffer, Drop);

    gcmVERIFY_OBJECT(Buffer, gcvOBJ_BUFFER);

    tempCMDBUF = &Buffer->tempCMDBUF;

    if (tempCMDBUF->currentByteSize >  gcmMAX_TEMPCMD_BUFFER_SIZE)
    {
        gcmPRINT(" Temp command buffer is overflowed!");
    }

    tempCMDBUF->inUse = gcvFALSE;

    if (tempCMDBUF->currentByteSize && !Drop)
    {
        status = gcoBUFFER_Write(Buffer,
                                 tempCMDBUF->buffer,
                                 tempCMDBUF->currentByteSize,
                                 gcvTRUE);
    }

    tempCMDBUF->currentByteSize = 0;

    gcmFOOTER_NO();
    return status;
}
#endif  /* gcdENABLE_3D */
