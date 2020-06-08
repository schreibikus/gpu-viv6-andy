/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include <gc_hal.h>
#include <gc_feature_database.h>

#include "gc_hal_kernel_hardware_func_flop_reset.h"

/*
 * Flop reset.
 *
 * The flops can be reset with PPU, NN and TP programs.
 * PPU:
 *   Requirements:
 *   1. DP inst with all bins enabled.
 *   2. Load inst which has at least two shader group,
 *      and every thread should load from different 64-byte address.
 *   3. Stroe inst which has at least 6 threads, whose addresses are
 *      from different 64-byte address and flush.
 *   Case:
 *   * InImage: 64x6 = {1}, unsigned int8
 *   * OutImage: 64x6, unsigned int8
 *   * OutImage = InImage + InImage
 * NN:
 *   Requirements:
 *   1. A XYDP6 case.
 *   2. NN cmd that uses only 1 core and make othere core's kernel size
 *      to be 0.
 *   Case:
 *   * Input: 3x2x1 = {1}
 *   * Kernel: 2x2x1 = {1}
 *   * Output: 2x1x1
 * TP:
 *   Requirements:
 *   1. Run TP fc on all TP cores.
 *   Case:
 *   * Input: 1x1x2 = {1}
 *   * Kernel: 1x1x2x64 = {1}
 *   * Output: 1x64
 */

/*
 * PPU.
 */
#define PPU_IMAGE_XSIZE 64
#define PPU_IMAGE_YSIZE 6
#define PPU_IMAGE_DATA 0x01010101
#define MAX_PPU_INSTRUCTION_COUNT 16
#define MAX_PPU_COMMAND_NUM 128

#define GCREG_SH_INSTRUCTION_TYPE_INVALID (~0U)

typedef enum _gceFLOP_RESET_PPU_DATA {
    gcvFLOP_RESET_PPU_INSTRUCTION = 0,
    gcvFLOP_RESET_PPU_INPUT       = 1,
    gcvFLOP_RESET_PPU_OUTPUT      = 2,
    gcvFLOP_RESET_PPU_DATA_NUM
}
gceFLOP_RESET_PPU_DATA;

/*
 * NN convolution.
 */
#define MAX_NN_COMMAND_NUM 64

#define NN_KERNEL_XSIZE 2
#define NN_KERNEL_YSIZE 2
#define NN_KERNEL_ZSIZE 1

#define NN_INPUT_XSIZE 3
#define NN_INPUT_YSIZE 2
#define NN_INPUT_ZSIZE 1

#define NN_OUTPUT_XSIZE 2
#define NN_OUTPUT_YSIZE 1
#define NN_OUTPUT_ZSIZE 1

typedef enum _gceVIP_ARCH_TYPE {
    gcvVIP_ARCH_TYPE_V6,
    gcvVIP_ARCH_TYPE_V7,
    gcvVIP_ARCH_TYPE_V8
}
gceVIP_ARCH_TYPE;

typedef enum _gceFLOP_RESET_NN_DATA {
    gcvFLOP_RESET_NN_INSTRUCTION = 0,
    gcvFLOP_RESET_NN_INPUT       = 1,
    gcvFLOP_RESET_NN_OUTPUT      = 2,
    gcvFLOP_RESET_NN_KERNEL      = 3,
    gcvFLOP_RESET_NN_DATA_NUM
}
gceFLOP_RESET_NN_DATA;

static gceSTATUS
_AllocateVideoMemory(
    IN gckKERNEL Kernel,
    IN gceVIDMEM_TYPE Type,
    IN gctUINT32 AllocFlag,
    IN OUT gcePOOL *Pool,
    IN OUT gctSIZE_T *Bytes,
    OUT gckVIDMEM_NODE *Node,
    OUT gctPOINTER *Logical,
    OUT gctUINT32 *Address
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gckVIDMEM_NODE bufferNode = gcvNULL;
    gctPOINTER bufferLogical = gcvNULL;
    gctUINT32 bufferAddress = 0;
    gctSIZE_T bufferBytes = 0;
    gcePOOL pool = gcvPOOL_DEFAULT;

    if (!Bytes || *Bytes == 0)
    {
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (Pool)
    {
        pool = *Pool;
    }

    bufferBytes = *Bytes;

    gcmkONERROR(gckKERNEL_AllocateVideoMemory(
        Kernel,
        64,
        Type,
        AllocFlag,
        &bufferBytes,
        &pool,
        &bufferNode
        ));

    gcmkONERROR(gckVIDMEM_NODE_LockCPU(
        Kernel,
        bufferNode,
        gcvFALSE,
        gcvFALSE,
        &bufferLogical
        ));

    gcmkONERROR(gckVIDMEM_NODE_Lock(
        Kernel,
        bufferNode,
        &bufferAddress
        ));

    gcmkONERROR(gckOS_ZeroMemory(bufferLogical, bufferBytes));

    *Bytes = bufferBytes;

    if (Pool)
    {
        *Pool = pool;
    }

    if (Node)
    {
        *Node = bufferNode;
    }

    if (Logical)
    {
        *Logical = bufferLogical;
    }

    if (Address)
    {
        *Address = bufferAddress;
    }

    return gcvSTATUS_OK;

OnError:
    if (bufferNode)
    {
        if (bufferAddress)
        {
            gcmkVERIFY_OK(gckVIDMEM_NODE_Unlock(
                Kernel,
                bufferNode,
                0,
                gcvNULL
                ));
        }

        if (bufferLogical)
        {
            gcmkVERIFY_OK(gckVIDMEM_NODE_UnlockCPU(
                Kernel,
                bufferNode,
                0,
                gcvFALSE,
                gcvFALSE
                ));
        }

        gcmkVERIFY_OK(gckVIDMEM_NODE_Dereference(
            Kernel,
            bufferNode
            ));
    }

    return status;
}

static gceSTATUS
_FreeVideoMemory(
    IN gckKERNEL Kernel,
    IN gckVIDMEM_NODE Node
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    if (!Node)
    {
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    gcmkVERIFY_OK(gckVIDMEM_NODE_Unlock(
        Kernel,
        Node,
        0,
        gcvNULL
        ));

    gcmkVERIFY_OK(gckVIDMEM_NODE_UnlockCPU(
        Kernel,
        Node,
        0,
        gcvFALSE,
        gcvFALSE
        ));

    gcmkVERIFY_OK(gckVIDMEM_NODE_Dereference(
        Kernel,
        Node
        ));

    return gcvSTATUS_OK;

OnError:
    return status;
}

static gceSTATUS
_BitValue(
    IN gctUINT8_PTR *Base,
    IN gctUINT32 Value,
    IN gctUINT32_PTR Offset,
    IN gctUINT Length
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gctUINT32_PTR msb = (gctUINT32_PTR)(*Base) + 1, lsb = (gctUINT32_PTR)(*Base);

    gcmkASSERT(*Offset <= 32 && Length <= 32);

    if ((*Offset) < 32)
    {
        gctUINT32 end = (*Offset) + Length, data = *lsb;

        if (end < 32)
        {
            /************************************************************************
             *       offset    32           64                                      *
             *     _________________________                                        *
             *    |_____|////|_|____________|                                       *
             *              end                                                     *
             ************************************************************************/
            data  = (*lsb & ((1 << *Offset) - 1));
            data |= (*lsb & ~((1 << end) - 1));
            data |= (Value << *Offset);

            *lsb = data;
            *Offset = end;
        }
        else if (end < 64)
        {
            /************************************************************************
             *       offset    32           64                                      *
             *     _________________________                                        *
             *    |_____|//////|//|_________|                                       *
             *                   end                                                *
             ************************************************************************/
            gctUINT32 length_m = end - 32;
            gctUINT32 data_l = (*lsb & ((1 << *Offset) - 1));
            gctUINT32 data_m = (*msb & ~((1 << length_m) - 1));

            data_l |= (Value << *Offset);
            data_m |= (Value >> (32 - *Offset));

            *lsb = data_l;

            if (end > 32)
                *msb = data_m;

            *Offset = length_m;

            *Base = (gctUINT8_PTR)msb;
        }

    }

    return status;
}

static gceSTATUS
_GetVIPCoreInfo(
    IN gckHARDWARE Hardware,
    OUT gceVIP_ARCH_TYPE *ArchType,
    OUT gctUINT8 *DataType,
    OUT gctUINT32 *CoreCount,
    OUT gctUINT32 *Zdp,
    OUT gctUINT32 *KernelBurstSize
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcsFEATURE_DATABASE *database = (gcsFEATURE_DATABASE *)(Hardware->featureDatabase);
    gceVIP_ARCH_TYPE archType;
    gctUINT8 dataType = 0x0;
    gctUINT32 coreCount = 0;
    gctUINT32 zdp = 1;
    gctUINT32 kernelBurstSize;

    gcmkASSERT(database);

    /* Make compiler happy. */
    gcQueryFeatureDB(0, 0, 0, 0, 0);

    /* Choose one supported format. */
    if (database->NNCoreCount_INT8 > 0)
    {
        dataType = 0x0;
        coreCount = database->NNCoreCount_INT8;
    }
    else if (database->NNCoreCount_INT16 > 0)
    {
        dataType = 0x4;
        coreCount = database->NNCoreCount_INT16;
    }
    else if (database->NNCoreCount_FLOAT16 > 0)
    {
        dataType = 0x1;
        coreCount = database->NNCoreCount_FLOAT16;
    }
    else if (database->NNCoreCount_BFLOAT > 0)
    {
        dataType = 0x7;
        coreCount = database->NNCoreCount_BFLOAT;
    }
    else
    {
        gcmkONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    if (database->NN_XYDP0)
    {
        archType = gcvVIP_ARCH_TYPE_V8;
    }
    else if (database->VIP_V7)
    {
        archType = gcvVIP_ARCH_TYPE_V7;
    }
    else
    {
        archType = gcvVIP_ARCH_TYPE_V6;
    }

    zdp = database->NN_ZDP3 ? 3 : 1;

    kernelBurstSize = database->DDR_KERNEL_BURST_SIZE;

    if (ArchType)
    {
        *ArchType = archType;
    }

    if (DataType)
    {
        *DataType = dataType;
    }

    if (CoreCount)
    {
        *CoreCount = coreCount;
    }

    if (Zdp)
    {
        *Zdp = zdp;
    }

    if (KernelBurstSize)
    {
        *KernelBurstSize = kernelBurstSize;
    }

    return gcvSTATUS_OK;

OnError:
    return status;
}

static gceSTATUS
_GetMapIndex(
    gctUINT8 DataType,
    gctUINT32_PTR Index
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    if (!Index)
    {
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    switch (DataType)
    {
    case 0x1:
        *Index = 1;
        break;

    case 0x7:
        *Index = 2;
        break;

    default:
        *Index = 0;
        break;
    }

    return gcvSTATUS_OK;

OnError:
    return status;
}

static gceSTATUS
_GetNNDataSize(
   IN gctUINT8 DataType,
   OUT gctUINT32_PTR DataSize
   )
{
    gceSTATUS status = gcvSTATUS_OK;

    if (!DataSize)
    {
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    switch (DataType)
    {
    case 0x2:
    case 0x0:
        *DataSize = 1;
        break;

    case 0x4:
    case 0x1:
    case 0x7:
        *DataSize = 2;
        break;

    default:
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
        break;
    }

    return gcvSTATUS_OK;

OnError:
    return status;
}

/*
 * PPU.
 */
static gceSTATUS
_ProgramPPUInput(
    IN gckHARDWARE Hardware,
    IN gctUINT32 InImageXSize,
    IN gctUINT32 InImageYSize,
    IN gctUINT32 AllocFlag,
    IN OUT gcePOOL *Pool,
    OUT gcsFUNCTION_EXECUTION_DATA *Data
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gctUINT32 itemBytes = 1; /* U8 format. */
    gckVIDMEM_NODE bufferNode = gcvNULL;
    gctPOINTER bufferLogical = gcvNULL;
    gctUINT32 bufferAddress = 0;
    gctSIZE_T bufferBytes, bytes;
    gctUINT32 *buffer = gcvNULL;
    gctUINT32 i;

    bufferBytes = bytes = InImageXSize * InImageYSize * itemBytes;

    gcmkONERROR(_AllocateVideoMemory(
        Hardware->kernel,
        gcvVIDMEM_TYPE_BITMAP,
        AllocFlag,
        Pool,
        &bufferBytes,
        &bufferNode,
        &bufferLogical,
        &bufferAddress
        ));

    buffer = (gctUINT32_PTR)bufferLogical;

    /* Fill the data. */
    for (i = 0; i < bytes / 4; i++)
    {
        buffer[i] = PPU_IMAGE_DATA;
    }

    gcmkONERROR(gckVIDMEM_NODE_CleanCache(
        Hardware->kernel,
        bufferNode,
        0,
        bufferLogical,
        bytes
        ));

#if gcdDUMP_IN_KERNEL
    gcmkDUMP(Hardware->os, "#[flop reset: ppu input]");
    gcmkDUMP_BUFFER(
        Hardware->os,
        gcvDUMP_BUFFER_KERNEL_COMMAND,
        bufferLogical,
        bufferAddress,
        bytes
        );
#endif

    Data->bufVidMem = bufferNode;
    Data->bufVidMemBytes = bufferBytes;
    Data->address = bufferAddress;
    Data->logical = bufferLogical;
    Data->bytes = bytes;

    return gcvSTATUS_OK;

OnError:
    if (bufferNode)
    {
        gcmkVERIFY_OK(_FreeVideoMemory(
            Hardware->kernel,
            bufferNode
            ));
    }

    return status;
}

static gceSTATUS
_ProgramPPUOutput(
    IN gckHARDWARE Hardware,
    IN gctUINT32 Width,
    IN gctUINT32 Height,
    IN gctUINT32 AllocFlag,
    IN OUT gcePOOL *Pool,
    OUT gcsFUNCTION_EXECUTION_DATA *Data
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gctUINT32 itemBytes = 1;
    gckVIDMEM_NODE bufferNode = gcvNULL;
    gctPOINTER bufferLogical = gcvNULL;
    gctUINT32 bufferAddress = 0;
    gctSIZE_T bufferBytes, bytes;

    bufferBytes = bytes = Width * Height * itemBytes;

    gcmkONERROR(_AllocateVideoMemory(
        Hardware->kernel,
        gcvVIDMEM_TYPE_BITMAP,
        AllocFlag,
        Pool,
        &bufferBytes,
        &bufferNode,
        &bufferLogical,
        &bufferAddress
        ));

    gcmkONERROR(gckVIDMEM_NODE_CleanCache(
        Hardware->kernel,
        bufferNode,
        0,
        bufferLogical,
        bufferBytes
        ));

    Data->bufVidMem = bufferNode;
    Data->bufVidMemBytes = bufferBytes;
    Data->address = bufferAddress;
    Data->logical = bufferLogical;
    Data->bytes = bytes;

    return gcvSTATUS_OK;

OnError:
    if (bufferNode)
    {
        gcmkVERIFY_OK(_FreeVideoMemory(
            Hardware->kernel,
            bufferNode
            ));
    }

    return status;
}

static gctUINT32
_SETBITS(
    IN gctUINT32 Data,
    IN gctUINT32 Start,
    IN gctUINT32 End,
    IN gctUINT32 Value
    )
{
    gctUINT32 data = Data;
    gctUINT32 mask;

    if (End >= Start)
    {
        mask =  ((~0ULL >> (63 - End + Start)) << Start);
        data &= ~mask;
        data |= ((Value) << Start) & mask;
        return data;
    }
    else
    {
        mask =  ((~0ULL >> (63 - Start + End)) << End);
        data &= ~mask;
        data |= ((Value) << End) & mask;
        return data;
    }
}

static gctUINT32
_SETBIT(
    IN gctUINT32 Data,
    IN gctUINT32 Position,
    IN gctUINT32 Value
    )
{
    gctUINT32 data;

    data = _SETBITS(Data, Position, Position, Value);

    return data;
}

static gctUINT32
_GETBITS(
    IN gctUINT32 Data,
    IN gctUINT32 Start,
    IN gctUINT32 End
    )
{
    gctUINT32 data = Data;
    gctUINT32 mask;

    if (End >= Start)
    {
        mask = (~0ULL >> (63 - (End - Start)));
        return (data >> Start) & mask;
    }
    else
    {
        mask = (~0ULL >> (63 - (Start - End)));
        return (data >> End) & mask;;
    }
}

static gctUINT32
_GETBIT(
    IN gctUINT32 Data,
    IN gctUINT32 Position
    )
{
    gctUINT32 data;

    data = _GETBITS(Data, Position, Position);

    return data;
}

static gceSTATUS
gckPPU_SetImmediate(
    IN gctUINT32 Where,
    IN gctUINT32 Value,
    IN gctUINT32 Type,
    IN OUT gctUINT32_PTR Inst
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    if (!Inst)
    {
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    switch (Where)
    {
    case 0:
        Inst[1] = ((((gctUINT32) (Inst[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:12) - (0 ?
 20:12) + 1))))))) << (0 ?
 20:12))) | (((gctUINT32) ((gctUINT32) (_GETBITS(Value, 8, 0)) & ((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:12) - (0 ? 20:12) + 1))))))) << (0 ? 20:12)));
        Inst[1] = ((((gctUINT32) (Inst[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:22) - (0 ?
 29:22) + 1))))))) << (0 ?
 29:22))) | (((gctUINT32) ((gctUINT32) (_GETBITS(Value, 16, 9)) & ((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ? 29:22)));
        Inst[1] = ((((gctUINT32) (Inst[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:30) - (0 ?
 30:30) + 1))))))) << (0 ?
 30:30))) | (((gctUINT32) ((gctUINT32) (_GETBIT(Value, 17)) & ((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ? 30:30)));
        Inst[1] = ((((gctUINT32) (Inst[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:31) - (0 ?
 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (_GETBIT(Value, 18)) & ((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31)));
        Inst[2] = ((((gctUINT32) (Inst[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (_GETBIT(Value, 19) | (Type << 1)) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)));
        Inst[2] = ((((gctUINT32) (Inst[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0x7) & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ? 5:3)));
        break;

    case 1:
        Inst[2] = ((((gctUINT32) (Inst[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:7) - (0 ?
 15:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:7) - (0 ?
 15:7) + 1))))))) << (0 ?
 15:7))) | (((gctUINT32) ((gctUINT32) (_GETBITS(Value, 8, 0)) & ((gctUINT32) ((((1 ?
 15:7) - (0 ?
 15:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:7) - (0 ? 15:7) + 1))))))) << (0 ? 15:7)));
        Inst[2] = ((((gctUINT32) (Inst[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:17) - (0 ?
 24:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:17) - (0 ?
 24:17) + 1))))))) << (0 ?
 24:17))) | (((gctUINT32) ((gctUINT32) (_GETBITS(Value, 16, 9)) & ((gctUINT32) ((((1 ?
 24:17) - (0 ?
 24:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:17) - (0 ? 24:17) + 1))))))) << (0 ? 24:17)));
        Inst[2] = ((((gctUINT32) (Inst[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:25) - (0 ?
 25:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:25) - (0 ?
 25:25) + 1))))))) << (0 ?
 25:25))) | (((gctUINT32) ((gctUINT32) (_GETBIT(Value, 17)) & ((gctUINT32) ((((1 ?
 25:25) - (0 ?
 25:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:25) - (0 ? 25:25) + 1))))))) << (0 ? 25:25)));
        Inst[2] = ((((gctUINT32) (Inst[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (_GETBIT(Value, 18)) & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ? 26:26)));
        Inst[2] = ((((gctUINT32) (Inst[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:27) - (0 ?
 29:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:27) - (0 ?
 29:27) + 1))))))) << (0 ?
 29:27))) | (((gctUINT32) ((gctUINT32) (_GETBIT(Value, 19) | (Type << 1)) & ((gctUINT32) ((((1 ?
 29:27) - (0 ?
 29:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:27) - (0 ? 29:27) + 1))))))) << (0 ? 29:27)));
        Inst[3] = ((((gctUINT32) (Inst[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0x7) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)));
        break;

    case 2:
        Inst[3] = ((((gctUINT32) (Inst[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:4) - (0 ?
 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (_GETBITS(Value, 8, 0)) & ((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ? 12:4)));
        Inst[3] = ((((gctUINT32) (Inst[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:14) - (0 ?
 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (_GETBITS(Value, 16, 9)) & ((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ? 21:14)));
        Inst[3] = ((((gctUINT32) (Inst[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:22) - (0 ?
 22:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:22) - (0 ?
 22:22) + 1))))))) << (0 ?
 22:22))) | (((gctUINT32) ((gctUINT32) (_GETBIT(Value, 17)) & ((gctUINT32) ((((1 ?
 22:22) - (0 ?
 22:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ? 22:22)));
        Inst[3] = ((((gctUINT32) (Inst[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:23) - (0 ?
 23:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:23) - (0 ?
 23:23) + 1))))))) << (0 ?
 23:23))) | (((gctUINT32) ((gctUINT32) (_GETBIT(Value, 18)) & ((gctUINT32) ((((1 ?
 23:23) - (0 ?
 23:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ? 23:23)));
        Inst[3] = ((((gctUINT32) (Inst[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:25) - (0 ?
 27:25) + 1))))))) << (0 ?
 27:25))) | (((gctUINT32) ((gctUINT32) (_GETBIT(Value, 19) | (Type << 1)) & ((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ? 27:25)));
        Inst[3] = ((((gctUINT32) (Inst[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (0x7) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));
        break;
    }

    return gcvSTATUS_OK;

OnError:
    return status;
}

static gceSTATUS
gckPPU_SetInstructionType(
    IN gctUINT32 Type,
    OUT gctUINT32_PTR Inst
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    if (!Inst)
    {
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    Inst[1] = ((((gctUINT32) (Inst[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:21) - (0 ?
 21:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:21) - (0 ?
 21:21) + 1))))))) << (0 ?
 21:21))) | (((gctUINT32) ((gctUINT32) (_GETBIT(Type, 0)) & ((gctUINT32) ((((1 ?
 21:21) - (0 ?
 21:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ? 21:21)));
    Inst[2] = ((((gctUINT32) (Inst[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:30) - (0 ?
 31:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:30) - (0 ?
 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) ((gctUINT32) (_GETBITS(Type, 2, 1)) & ((gctUINT32) ((((1 ?
 31:30) - (0 ?
 31:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30)));

    return gcvSTATUS_OK;

OnError:
    return status;
}

static gceSTATUS
gckPPU_IsEndOfBB(
    IN gckHARDWARE Hardware,
    IN gctUINT32 OpCode,
    OUT gctUINT32_PTR Inst
)
{
    gceSTATUS status = gcvSTATUS_OK;

    gcsFEATURE_DATABASE *database = (gcsFEATURE_DATABASE *)Hardware->featureDatabase;
    gctUINT32 bits = 0;

    if (!Inst)
    {
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (!database->SH_END_OF_BB)
    {
        return gcvSTATUS_OK;
    }

    switch (OpCode)
    {
    case 0x09:
    case 0x56:
    case 0x0A:
    case 0x0B:
    case 0x0F:
    case 0x31:
    case 0x10:
        bits = _SETBITS(Inst[1], 3, 3, 1);
        Inst[1] = ((((gctUINT32) (Inst[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:3) - (0 ?
 10:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:3) - (0 ?
 10:3) + 1))))))) << (0 ?
 10:3))) | (((gctUINT32) ((gctUINT32) (bits) & ((gctUINT32) ((((1 ?
 10:3) - (0 ?
 10:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:3) - (0 ? 10:3) + 1))))))) << (0 ? 10:3)));
        break;

    case 0x65:
    case 0x66:
    case 0x67:
    case 0x68:
    case 0x69:
    case 0x6A:
    case 0x6B:
    case 0x6C:
    case 0x46:
        bits = _SETBITS(Inst[0], 2, 2, 1);
        Inst[0] = ((((gctUINT32) (Inst[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ?
 10:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:6) - (0 ?
 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) ((gctUINT32) (bits) & ((gctUINT32) ((((1 ?
 10:6) - (0 ?
 10:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));
        break;

    case 0x32:
    case 0x39:
    case 0x33:
    case 0x3A:
    case 0x79:
    case 0x34:
    case 0x7A:
    case 0x35:
        bits = _SETBITS(Inst[0], 2, 2, 1);
        Inst[0] = ((((gctUINT32) (Inst[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ?
 10:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:6) - (0 ?
 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) ((gctUINT32) (bits) & ((gctUINT32) ((((1 ?
 10:6) - (0 ?
 10:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));
        break;

    default:
        if (OpCode != 0x16 &&
            OpCode != 0x24 &&
            OpCode != 0x14 &&
            OpCode != 0x15 &&
            OpCode != 0x17)
        {
            bits = _SETBITS(Inst[0], 2, 2, 1);
            Inst[0] = ((((gctUINT32) (Inst[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ?
 10:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:6) - (0 ?
 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) ((gctUINT32) (bits) & ((gctUINT32) ((((1 ?
 10:6) - (0 ?
 10:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));
        }
        break;
    }

    return gcvSTATUS_OK;

OnError:
    return status;
}

static gceSTATUS
gckPPU_AddOpCode(
    IN gckHARDWARE Hardware,
    IN gctUINT32 OpCode,
    IN gctUINT32 Extended,
    IN gctUINT32 Type,
    IN OUT gctUINT32_PTR Inst
    )
{
    gceSTATUS status = gcvSTATUS_OK;


    Inst[0] = ((((gctUINT32) (Inst[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (_GETBITS(OpCode, 5, 0)) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)));
    Inst[2] = ((((gctUINT32) (Inst[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:16) - (0 ?
 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) ((gctUINT32) (_GETBIT(OpCode, 6)) & ((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16)));

    if (!Inst)
    {
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    switch (OpCode)
    {
    case 0x7F:
        gcmkONERROR(gckPPU_SetImmediate(2, Extended, 0x2, Inst));
        break;

    case 0x45:
        Inst[0] = ((((gctUINT32) (Inst[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:13) - (0 ?
 15:13) + 1))))))) << (0 ?
 15:13))) | (((gctUINT32) ((gctUINT32) (_GETBITS(Extended, 2, 0)) & ((gctUINT32) ((((1 ?
 15:13) - (0 ?
 15:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:13) - (0 ? 15:13) + 1))))))) << (0 ? 15:13)));
        Inst[0] = _SETBIT(Inst[0], 31, _GETBIT(Extended, 3));
        Inst[1] = _SETBITS(Inst[1], 1, 0, _GETBITS(Extended, 5, 4));
        break;

    case 0x31:
    case 0x09:
    case 0x0F:
        Inst[0] = ((((gctUINT32) (Inst[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:6) - (0 ?
 10:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:6) - (0 ?
 10:6) + 1))))))) << (0 ?
 10:6))) | (((gctUINT32) ((gctUINT32) (_GETBITS(Extended, 4, 0)) & ((gctUINT32) ((((1 ?
 10:6) - (0 ?
 10:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:6) - (0 ? 10:6) + 1))))))) << (0 ? 10:6)));
        break;

    default:
        break;
    }

    if (Type != GCREG_SH_INSTRUCTION_TYPE_INVALID)
    {
        gcmkONERROR(gckPPU_SetInstructionType(Type, Inst));
    }

    gcmkONERROR(gckPPU_IsEndOfBB(Hardware, OpCode, Inst));

    return gcvSTATUS_OK;

OnError:
    return status;
}

static gceSTATUS
gckPPU_SetDestination(
    IN gctUINT32 Address,
    IN gctUINT32 WriteEnable,
    IN gctUINT32 Saturate,
    IN OUT gctUINT32_PTR Inst
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    if (!Inst)
    {
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    Inst[0] = ((((gctUINT32) (Inst[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)));
    Inst[0] = ((((gctUINT32) (Inst[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:16) - (0 ?
 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (Address) & ((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ? 22:16)));
    Inst[0] = ((((gctUINT32) (Inst[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:23) - (0 ?
 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (WriteEnable) & ((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ? 26:23)));
    Inst[0] = ((((gctUINT32) (Inst[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) ((gctUINT32) (Saturate) & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)));

    return gcvSTATUS_OK;

OnError:
    return status;
}

#define gcdVX_ENABLE ((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3))
#define gcdVX_ENABLE4(X, Y, Z, W) ((1 << (X)) | (1 << (Y)) | (1 << (Z)) | (1 << (W)))
#define gcdVX_ENABLE1(X) (1 << (X))
#define gcdVX_ENABLE2(X, Y) ((1 << (X)) | (1 << (Y)))
#define gcdVX_ENABLE3(X, Y, Z) ((1 << (X)) | (1 << (Y)) | (1 << (Z)))
#define gcdVX_SWIZZLE (0 | (1 << 2) | (2 << 4) | (3 << 6))
#define gcdVX_SWIZZLE1(X) ((X) | ((X) << 2) | ((X) << 4) | ((X) << 6))
#define gcdVX_SWIZZLE2(X, Y) ((X) | ((Y) << 2) | ((Y) << 4) | ((Y) << 6))
#define gcdVX_SWIZZLE4(X, Y, Z, W) ((X) | ((Y) << 2) | ((Z) << 4) | ((W) << 6))

static gctUINT32
gckPPU_GetPixel(
    IN gctUINT32 Format
    )
{
    gctUINT32 pixel = 0;

    switch(Format)
    {
    case 0x7:
        pixel = 15;
        break;

    case 0x3:
    case 0x6:
        pixel = 7;
        break;

    default:
        pixel = 15;
        break;
    }

    return pixel;
}

gceSTATUS
gckPPU_SetEVIS(
    IN gctUINT32 Start,
    IN gctUINT32 End,
    IN gctUINT32 Evis,
    IN OUT gctUINT32_PTR Inst
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    if (!Inst)
    {
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    Inst[0] = ((((gctUINT32) (Inst[0])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:23) - (0 ?
 26:23) + 1))))))) << (0 ?
 26:23))) | (((gctUINT32) ((gctUINT32) (Start) & ((gctUINT32) ((((1 ?
 26:23) - (0 ?
 26:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:23) - (0 ? 26:23) + 1))))))) << (0 ? 26:23)));
    Inst[0] = _SETBITS(Inst[0], 30, 27, End);
    Inst[1] = _SETBITS(Inst[1], 10, 2, Evis);

    return gcvSTATUS_OK;

OnError:
    return status;
}

static gceSTATUS
gckPPU_SetSource(
    IN gctUINT32 Where,
    IN gctUINT32 Address,
    IN gctUINT32 Swizzle,
    IN gctUINT32 Type,
    IN gctBOOL Negate,
    IN gctBOOL Absolute,
    IN gctUINT32 Relative,
    IN OUT gctUINT32_PTR Inst
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    if (!Inst)
    {
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    switch (Where)
    {
    case 0:
        Inst[1] = ((((gctUINT32) (Inst[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)));
        Inst[1] = ((((gctUINT32) (Inst[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:12) - (0 ?
 20:12) + 1))))))) << (0 ?
 20:12))) | (((gctUINT32) ((gctUINT32) (Address) & ((gctUINT32) ((((1 ?
 20:12) - (0 ?
 20:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:12) - (0 ? 20:12) + 1))))))) << (0 ? 20:12)));
        Inst[1] = ((((gctUINT32) (Inst[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:22) - (0 ?
 29:22) + 1))))))) << (0 ?
 29:22))) | (((gctUINT32) ((gctUINT32) (Swizzle) & ((gctUINT32) ((((1 ?
 29:22) - (0 ?
 29:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:22) - (0 ? 29:22) + 1))))))) << (0 ? 29:22)));
        Inst[1] = ((((gctUINT32) (Inst[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:30) - (0 ?
 30:30) + 1))))))) << (0 ?
 30:30))) | (((gctUINT32) ((gctUINT32) (Negate) & ((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ? 30:30)));
        Inst[1] = ((((gctUINT32) (Inst[1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:31) - (0 ?
 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (Absolute) & ((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31)));
        Inst[2] = ((((gctUINT32) (Inst[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (Relative) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)));
        Inst[2] = ((((gctUINT32) (Inst[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (Type) & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ? 5:3)));
        break;

    case 1:
        Inst[2] = ((((gctUINT32) (Inst[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)));
        Inst[2] = ((((gctUINT32) (Inst[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:7) - (0 ?
 15:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:7) - (0 ?
 15:7) + 1))))))) << (0 ?
 15:7))) | (((gctUINT32) ((gctUINT32) (Address) & ((gctUINT32) ((((1 ?
 15:7) - (0 ?
 15:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:7) - (0 ? 15:7) + 1))))))) << (0 ? 15:7)));
        Inst[2] = ((((gctUINT32) (Inst[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:17) - (0 ?
 24:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:17) - (0 ?
 24:17) + 1))))))) << (0 ?
 24:17))) | (((gctUINT32) ((gctUINT32) (Swizzle) & ((gctUINT32) ((((1 ?
 24:17) - (0 ?
 24:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:17) - (0 ? 24:17) + 1))))))) << (0 ? 24:17)));
        Inst[2] = ((((gctUINT32) (Inst[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:25) - (0 ?
 25:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:25) - (0 ?
 25:25) + 1))))))) << (0 ?
 25:25))) | (((gctUINT32) ((gctUINT32) (Negate) & ((gctUINT32) ((((1 ?
 25:25) - (0 ?
 25:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:25) - (0 ? 25:25) + 1))))))) << (0 ? 25:25)));
        Inst[2] = ((((gctUINT32) (Inst[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (Absolute) & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ? 26:26)));
        Inst[2] = ((((gctUINT32) (Inst[2])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:27) - (0 ?
 29:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:27) - (0 ?
 29:27) + 1))))))) << (0 ?
 29:27))) | (((gctUINT32) ((gctUINT32) (Relative) & ((gctUINT32) ((((1 ?
 29:27) - (0 ?
 29:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:27) - (0 ? 29:27) + 1))))))) << (0 ? 29:27)));
        Inst[3] = ((((gctUINT32) (Inst[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (Type) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)));
        break;

    case 2:
        Inst[3] = ((((gctUINT32) (Inst[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)));
        Inst[3] = ((((gctUINT32) (Inst[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:4) - (0 ?
 12:4) + 1))))))) << (0 ?
 12:4))) | (((gctUINT32) ((gctUINT32) (Address) & ((gctUINT32) ((((1 ?
 12:4) - (0 ?
 12:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:4) - (0 ? 12:4) + 1))))))) << (0 ? 12:4)));
        Inst[3] = ((((gctUINT32) (Inst[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:14) - (0 ?
 21:14) + 1))))))) << (0 ?
 21:14))) | (((gctUINT32) ((gctUINT32) (Swizzle) & ((gctUINT32) ((((1 ?
 21:14) - (0 ?
 21:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:14) - (0 ? 21:14) + 1))))))) << (0 ? 21:14)));
        Inst[3] = ((((gctUINT32) (Inst[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:22) - (0 ?
 22:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:22) - (0 ?
 22:22) + 1))))))) << (0 ?
 22:22))) | (((gctUINT32) ((gctUINT32) (Negate) & ((gctUINT32) ((((1 ?
 22:22) - (0 ?
 22:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ? 22:22)));
        Inst[3] = ((((gctUINT32) (Inst[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:23) - (0 ?
 23:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:23) - (0 ?
 23:23) + 1))))))) << (0 ?
 23:23))) | (((gctUINT32) ((gctUINT32) (Absolute) & ((gctUINT32) ((((1 ?
 23:23) - (0 ?
 23:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ? 23:23)));
        Inst[3] = ((((gctUINT32) (Inst[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:25) - (0 ?
 27:25) + 1))))))) << (0 ?
 27:25))) | (((gctUINT32) ((gctUINT32) (Relative) & ((gctUINT32) ((((1 ?
 27:25) - (0 ?
 27:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:25) - (0 ? 27:25) + 1))))))) << (0 ? 27:25)));
        Inst[3] = ((((gctUINT32) (Inst[3])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (Type) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));
        break;

    default:
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
        break;
    }

    return gcvSTATUS_OK;

OnError:
    return status;
}

static const gctUINT32 NEGATE_FLAG   = 1 << 0;
static const gctUINT32 ABSOLUTE_FLAG = 1 << 1;

static gceSTATUS
gckPPU_SetUniform(
    IN gctUINT32 Where,
    IN gctUINT32 Address,
    IN gctUINT32 Swizzle,
    IN gctUINT32 Modifiers,
    OUT gctUINT32_PTR Inst
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gctBOOL negate = (Modifiers & NEGATE_FLAG) ? gcvTRUE : gcvFALSE;
    gctBOOL absolute = (Modifiers & ABSOLUTE_FLAG) ? gcvTRUE : gcvFALSE;

    if (!Inst)
    {
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    gcmkONERROR(gckPPU_SetSource(
        Where,
        Address,
        Swizzle,
        0x2,
        negate,
        absolute,
        0,
        Inst
        ));

    return gcvSTATUS_OK;

OnError:
    return status;
}

gceSTATUS
gckPPU_SetTempReg(
    IN gctUINT32 Where,
    IN gctUINT32 Address,
    IN gctUINT32 Swizzle,
    IN gctUINT32 Modifiers,
    OUT gctUINT32_PTR Inst
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gctBOOL negate = (Modifiers & NEGATE_FLAG) ? gcvTRUE : gcvFALSE;
    gctBOOL absolute = (Modifiers & ABSOLUTE_FLAG) ? gcvTRUE : gcvFALSE;

    if (!Inst)
    {
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    gcmkONERROR(gckPPU_SetSource(
        Where,
        Address,
        Swizzle,
        0x0,
        negate,
        absolute,
        0,
        Inst
        ));

    return gcvSTATUS_OK;

OnError:
    return status;
}


static gceSTATUS
_ProgramPPUInstruction(
    IN gckHARDWARE Hardware,
    IN gctUINT32 DataType,
    IN gctUINT32 AllocFlag,
    IN OUT gcePOOL *Pool,
    OUT gctUINT32 *InstCount,
    OUT gctUINT32 *RegCount,
    OUT gcsFUNCTION_EXECUTION_DATA *Data
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gckVIDMEM_NODE bufferNode = gcvNULL;
    gctPOINTER bufferLogical = gcvNULL;
    gctUINT32 bufferAddress = 0;
    gctSIZE_T bufferBytes, bytes;

    gctUINT32 instCount = 0;
    gctUINT32_PTR inst = gcvNULL;

    gctUINT32 inImage1DataType = DataType;
    gctUINT32 inImage2DataType = DataType;
    gctUINT32 outImageDataType = DataType;

    if (!Data || !InstCount || !RegCount)
    {
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    bufferBytes = bytes = gcmSIZEOF(gctUINT32) * MAX_PPU_INSTRUCTION_COUNT;

    gcmkONERROR(_AllocateVideoMemory(
        Hardware->kernel,
        gcvVIDMEM_TYPE_COMMAND,
        AllocFlag,
        Pool,
        &bufferBytes,
        &bufferNode,
        &bufferLogical,
        &bufferAddress
        ));

    inst = (gctUINT32_PTR)bufferLogical;

    /* img_load.u8 r1, c0, r0.xy */
    gcmkONERROR(gckPPU_AddOpCode(Hardware, 0x79, 0, inImage1DataType, &inst[instCount]));
    gcmkONERROR(gckPPU_SetDestination(1, gcdVX_ENABLE, gcvFALSE, &inst[instCount]));
    gcmkONERROR(gckPPU_SetEVIS(0, gckPPU_GetPixel(inImage1DataType), 1, &inst[instCount]));
    gcmkONERROR(gckPPU_SetUniform(0, 0, gcdVX_SWIZZLE, 0, &inst[instCount]));
    gcmkONERROR(gckPPU_SetTempReg(1, 0, gcdVX_SWIZZLE2(0, 1), 0, &inst[instCount]));
    instCount += 4;

    /*img_load.u8 r2, c0, r0.xy */
    gcmkONERROR(gckPPU_AddOpCode(Hardware, 0x79, 0, inImage2DataType, &inst[instCount]));
    gcmkONERROR(gckPPU_SetDestination(2, gcdVX_ENABLE, gcvFALSE, &inst[instCount]));
    gcmkONERROR(gckPPU_SetEVIS(0, gckPPU_GetPixel(inImage2DataType), 1, &inst[instCount]));
    gcmkONERROR(gckPPU_SetUniform(0, 0, gcdVX_SWIZZLE, 0, &inst[instCount]));
    gcmkONERROR(gckPPU_SetTempReg(1, 0, gcdVX_SWIZZLE2(0, 1), 0, &inst[instCount]));
    instCount += 4;

    /* dp2x8 r1, r1, r2, c3_512 */
    gcmkONERROR(gckPPU_AddOpCode(Hardware, 0x45, 0x0B, outImageDataType, &inst[instCount]));
    gcmkONERROR(gckPPU_SetDestination(1, gcdVX_ENABLE, gcvFALSE, &inst[instCount]));
    gcmkONERROR(gckPPU_SetEVIS(0, 7, (inImage1DataType | (inImage2DataType << 3)), &inst[instCount]));
    gcmkONERROR(gckPPU_SetTempReg(0, 1, gcdVX_SWIZZLE, 0, &inst[instCount]));
    gcmkONERROR(gckPPU_SetTempReg(1, 2, gcdVX_SWIZZLE, 0, &inst[instCount]));
    gcmkONERROR(gckPPU_SetSource (2, 2, gcdVX_SWIZZLE, 0x4, gcvFALSE, gcvFALSE, 0, &inst[instCount]));
    instCount += 4;

    /* img_store.u8 r1, c2, r0.xy, r1 */
    gcmkONERROR(gckPPU_AddOpCode(Hardware, 0x7A, 0, outImageDataType, &inst[instCount]));
    gcmkONERROR(gckPPU_SetEVIS(0, gckPPU_GetPixel(outImageDataType), 1, &inst[instCount]));
    gcmkONERROR(gckPPU_SetUniform(0, 1, gcdVX_SWIZZLE, 0, &inst[instCount]));
    gcmkONERROR(gckPPU_SetTempReg(1, 0, gcdVX_SWIZZLE2(0, 1), 0, &inst[instCount]));
    gcmkONERROR(gckPPU_SetTempReg(2, 1, gcdVX_SWIZZLE, 0, &inst[instCount]));
    instCount += 4;

    bytes = gcmSIZEOF(gctUINT32) * instCount;

    gcmkONERROR(gckVIDMEM_NODE_CleanCache(
        Hardware->kernel,
        bufferNode,
        0,
        bufferLogical,
        bytes
        ));

#if gcdDUMP_IN_KERNEL
    gcmkDUMP(Hardware->os, "#[flop reset: ppu instruction]");
    gcmkDUMP_BUFFER(
        Hardware->os,
        gcvDUMP_BUFFER_KERNEL_COMMAND,
        bufferLogical,
        bufferAddress,
        bytes
        );
#endif

    *InstCount = instCount;
    *RegCount = 0x3;

    Data->bufVidMem = bufferNode;
    Data->bufVidMemBytes = bufferBytes;
    Data->address = bufferAddress;
    Data->logical = bufferLogical;
    Data->bytes = bytes;

    return gcvSTATUS_OK;

OnError:
    if (bufferNode)
    {
        gcmkVERIFY_OK(_FreeVideoMemory(
            Hardware->kernel,
            bufferNode
            ));
    }

    return status;
}

static gceSTATUS
_ProgramPPUCommand(
    IN gckHARDWARE Hardware,
    IN gctUINT32 Stride,
    IN gctUINT32 Width,
    IN gctUINT32 Height,
    IN gctUINT32 WorkDim,
    IN gctUINT32 ValueOrder,
    IN gctUINT32 GroupSizeX,
    IN gctUINT32 GroupSizeY,
    IN gctUINT32 GroupSizeZ,
    IN gctUINT32 GlobalScaleX,
    IN gctUINT32 GlobalScaleY,
    IN gctUINT32 GlobalScaleZ,
    IN gctUINT32 GlobalOffsetX,
    IN gctUINT32 GlobalOffsetY,
    IN gctUINT32 GlobalOffsetZ,
    IN gctUINT32 ThreadAllocation,
    IN gctUINT32 InImageAddress,
    IN gctUINT32 OutImageAddress,
    IN gctUINT32 InstAddress,
    IN gctUINT32 InstCount,
    IN gctUINT32 RegCount,
    IN gctUINT32 AllocFlag,
    IN OUT gcePOOL *Pool,
    OUT gcsFUNCTION_COMMAND_PTR Command
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gckVIDMEM_NODE bufferNode = gcvNULL;
    gctPOINTER bufferLogical = gcvNULL;
    gctUINT32 bufferAddress = 0;
    gctSIZE_T bufferBytes = 0;
    gctUINT32 bytes = 0;
    gctUINT8_PTR endLogical;
    gctUINT32 endAddress;
    gctUINT32 endBytes = 0;
    gctUINT32_PTR commands = gcvNULL;
    gctUINT32 index = 0;
    gctUINT32 groupCountX = (Width +  GlobalScaleX - 1) / GlobalScaleX;
    gctUINT32 groupCountY = (Height +  GlobalScaleY - 1) / GlobalScaleY;
    gctUINT32 groupCountZ = 0;

    if (!Command)
    {
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    bufferBytes = gcmSIZEOF(gctUINT32) * MAX_PPU_COMMAND_NUM;

    gcmkONERROR(_AllocateVideoMemory(
        Hardware->kernel,
        gcvVIDMEM_TYPE_COMMAND,
        AllocFlag,
        Pool,
        &bufferBytes,
        &bufferNode,
        &bufferLogical,
        &bufferAddress
        ));

    commands = (gctUINT32_PTR)bufferLogical;

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E13) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)));

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E02) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x07 & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)));

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));
    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x07 & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)));

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0xD800) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (4) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = InImageAddress;
    commands[index++] = Stride;
    commands[index++] = Height << 16 | Width;
    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) ((gctUINT32) (0x3) & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) ((gctUINT32) (0x7) & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:10) - (0 ?
 11:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:10) - (0 ?
 11:10) + 1))))))) << (0 ?
 11:10))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 11:10) - (0 ?
 11:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:10) - (0 ? 11:10) + 1))))))) << (0 ? 11:10)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));

    commands[index++] = 0xFFFFFFFF;

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0xD800 + 0x04) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (4) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = OutImageAddress;
    commands[index++] = Stride;
    commands[index++] = Height << 16 | Width;
    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) ((gctUINT32) (0x3) & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) ((gctUINT32) (0x7) & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:10) - (0 ?
 11:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:10) - (0 ?
 11:10) + 1))))))) << (0 ?
 11:10))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 11:10) - (0 ?
 11:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:10) - (0 ? 11:10) + 1))))))) << (0 ? 11:10)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));

    commands[index++] = 0xFFFFFFFF;

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0xD800 + 0x08) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (16) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = 0x55555555;
    commands[index++] = 0x00000000; /* TCfg. */
    commands[index++] = 0x01234567;
    commands[index++] = 0x89abcdef;
    commands[index++] = 0x55555555;
    commands[index++] = 0x01234567;
    commands[index++] = 0x89abcdef; /* BinSelect. */
    commands[index++] = 0x00000000; /* AccumType, ConstantType, and PostShift. */
    commands[index++] = 0x00000000;
    commands[index++] = 0x00000000;
    commands[index++] = 0x00000000;
    commands[index++] = 0x00000000;
    commands[index++] = 0x00000000;
    commands[index++] = 0x00000000;
    commands[index++] = 0x00000000;
    commands[index++] = 0x00000000; /* Constant. */

    commands[index++] = 0xFFFFFFFF;

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0240) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (0x2) & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:4) - (0 ?
 6:4) + 1))))))) << (0 ?
 6:4))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ? 6:4)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) ((gctUINT32) (0x2) & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)));

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x022C) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:2) - (0 ?
 2:2) + 1))))))) << (0 ?
 2:2))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ? 2:2)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)));

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0420) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)));

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0403) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = RegCount;

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0416) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = 0x00000000;

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0409) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = 0x00000000;

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x021F) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = 0x00000000;

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0424) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = InstCount / 4;

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x040A) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = InstAddress;

    if (gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_HALTI5))
    {
        commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x5580) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
        commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)));
    }
    else
    {
        commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0218) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
        commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)));
    }

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x021A) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0425) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = InstCount / 4 - 1;

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0402) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)));

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0228) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = 0x00000000;

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x02AA) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = 0x00000000;

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E07) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = 0x00000000;

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x040C) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = 0x00000000;

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0201) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)));

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E22) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = 0x00000000;

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0412) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = 0x00000000;

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0240) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (0x2) & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:4) - (0 ?
 6:4) + 1))))))) << (0 ?
 6:4))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ? 6:4)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) ((gctUINT32) (0x3) & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)));

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0249) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = 0x00000000;

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0247) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = ThreadAllocation;

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x024B) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = GlobalOffsetX;

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x024D) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = GlobalOffsetY;

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x024F) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = GlobalOffsetZ;

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0256) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = GlobalScaleX;

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0257) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = GlobalScaleY;

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0258) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = GlobalScaleZ;

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0250) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (6) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = groupCountX - 1;
    commands[index++] = groupCountY - 1;
    commands[index++] = groupCountZ - 1;
    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:0) - (0 ?
 9:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:0) - (0 ?
 9:0) + 1))))))) << (0 ?
 9:0))) | (((gctUINT32) ((gctUINT32) (GroupSizeX - 1) & ((gctUINT32) ((((1 ?
 9:0) - (0 ?
 9:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:0) - (0 ? 9:0) + 1))))))) << (0 ? 9:0)));
    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:0) - (0 ?
 9:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:0) - (0 ?
 9:0) + 1))))))) << (0 ?
 9:0))) | (((gctUINT32) ((gctUINT32) (GroupSizeY - 1) & ((gctUINT32) ((((1 ?
 9:0) - (0 ?
 9:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:0) - (0 ? 9:0) + 1))))))) << (0 ? 9:0)));
    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:0) - (0 ?
 9:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:0) - (0 ?
 9:0) + 1))))))) << (0 ?
 9:0))) | (((gctUINT32) ((gctUINT32) (GroupSizeZ - 1) & ((gctUINT32) ((((1 ?
 9:0) - (0 ?
 9:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:0) - (0 ? 9:0) + 1))))))) << (0 ? 9:0)));
    commands[index++] = 0x00000000;

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0248) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = 0xBADABEEB;

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E03) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:5) - (0 ?
 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:10) - (0 ?
 10:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:10) - (0 ?
 10:10) + 1))))))) << (0 ?
 10:10))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 10:10) - (0 ?
 10:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ? 10:10)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)));

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E02) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x07 & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)));

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x07 & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)));

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E03) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:5) - (0 ?
 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:10) - (0 ?
 10:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:10) - (0 ?
 10:10) + 1))))))) << (0 ?
 10:10))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 10:10) - (0 ?
 10:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ? 10:10)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)));

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E03) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:5) - (0 ?
 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:10) - (0 ?
 10:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:10) - (0 ?
 10:10) + 1))))))) << (0 ?
 10:10))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 10:10) - (0 ?
 10:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ? 10:10)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)));

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0594) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E03) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:5) - (0 ?
 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:10) - (0 ?
 10:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:10) - (0 ?
 10:10) + 1))))))) << (0 ?
 10:10))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 10:10) - (0 ?
 10:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ? 10:10)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)));

    bytes = gcmSIZEOF(gctUINT32) * index;

    endLogical = (gctUINT8_PTR)bufferLogical + bytes;
    endAddress = bufferAddress + bytes;

    if (Hardware->wlFE)
    {
        gcmkONERROR(gckWLFE_End(Hardware, gcvNULL, ~0U, &endBytes));
        gcmkONERROR(gckWLFE_End(Hardware, endLogical, endAddress, &endBytes));
    }

    bytes += endBytes;

    gcmkASSERT(bytes <= bufferBytes);

    gcmkONERROR(gckVIDMEM_NODE_CleanCache(
        Hardware->kernel,
        bufferNode,
        0,
        bufferLogical,
        bytes
        ));

    Command->funcVidMem = bufferNode;
    Command->funcVidMemBytes = bufferBytes;
    Command->logical = bufferLogical;
    Command->address = bufferAddress;
    Command->bytes = bytes;
    Command->endAddress = endAddress;
    Command->endLogical = endLogical;

    return gcvSTATUS_OK;

OnError:
    if (bufferNode)
    {
        gcmkVERIFY_OK(_FreeVideoMemory(
            Hardware->kernel,
            bufferNode
            ));
    }

    return status;
}

/*
**  gckHARDWARE_ResetFlopWithPPU
**
**  Generate the command to do PPU program as follows.
**      InImage: 64x6 = {1}, unsigned int8
**      OutImage: 64x6, unsigned int8
**      OutImage = InImage + InImage
**
**  INPUT:
**
**      gckHARDWARE Hardware
**
**      gctUINT32 AllocFlag
**
**      gcePOOL *Pool
**
**  OUTPUT:
**
**      gcePOOL *Pool
**
**      gcsFUNCTION_COMMAND *Command
*/
gceSTATUS
gckHARDWARE_ResetFlopWithPPU(
    IN gckHARDWARE Hardware,
    IN gctUINT32 AllocFlag,
    IN OUT gcePOOL *Pool,
    OUT gcsFUNCTION_COMMAND *Command
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gctUINT32 dataType = 0x7;
    gcsFEATURE_DATABASE *database = gcvNULL;
    gctUINT32 numShaderCores;
    gctUINT32 stride, width, height;
    gctUINT32 workDim;
    gctUINT32 valueOrder;
    gctUINT32 groupSizeX, groupSizeY, groupSizeZ;
    gctUINT32 globalScaleX, globalScaleY, globalScaleZ;
    gctUINT32 globalOffsetX, globalOffsetY, globalOffsetZ;
    gctUINT32 threadAllocation;
    gctUINT32 inImageAddress = 0, outImageAddress = 0, instAddress = 0;
    gctUINT32 instCount = 0, regCount = 0;
    gctUINT32 dataCount;
    gctPOINTER pointer = gcvNULL;
    gcsFUNCTION_EXECUTION_DATA *data = gcvNULL;
    gctUINT32 i;

    /* Exectution data. */
    dataCount = gcvFLOP_RESET_PPU_DATA_NUM;
    gcmkASSERT(dataCount > 0);

    gcmkONERROR(gckOS_Allocate(
        Hardware->os,
        gcmSIZEOF(gcsFUNCTION_EXECUTION_DATA) * dataCount,
        &pointer
        ));
    gckOS_ZeroMemory(pointer, gcmSIZEOF(gcsFUNCTION_EXECUTION_DATA) * dataCount);
    data = (gcsFUNCTION_EXECUTION_DATA_PTR)pointer;

    database = (gcsFEATURE_DATABASE *)Hardware->featureDatabase;

    numShaderCores = database->NumShaderCores;

    stride = PPU_IMAGE_XSIZE * 1;
    width = PPU_IMAGE_XSIZE;
    height = PPU_IMAGE_YSIZE;

    gcmkONERROR(_ProgramPPUInput(
        Hardware,
        width,
        height,
        AllocFlag,
        Pool,
        &data[gcvFLOP_RESET_PPU_INPUT]
        ));

    gcmkONERROR(_ProgramPPUOutput(
        Hardware,
        width,
        height,
        AllocFlag,
        Pool,
        &data[gcvFLOP_RESET_PPU_OUTPUT]
        ));

    gcmkONERROR(_ProgramPPUInstruction(
        Hardware,
        dataType,
        AllocFlag,
        Pool,
        &instCount,
        &regCount,
        &data[gcvFLOP_RESET_PPU_INSTRUCTION]
        ));

    workDim = 0x2;
    valueOrder = 0x2;
    groupSizeX = 1;
    groupSizeY = 1;
    groupSizeZ = 0;
    globalScaleX = 4;
    globalScaleY = 1;
    globalScaleZ = 0;
    globalOffsetX = 0;
    globalOffsetY = 0;
    globalOffsetZ = 0;
    threadAllocation = (groupSizeX * groupSizeY + numShaderCores * 4 - 1) / (numShaderCores * 4);
    inImageAddress = data[gcvFLOP_RESET_PPU_INPUT].address;
    outImageAddress = data[gcvFLOP_RESET_PPU_OUTPUT].address;
    instAddress = data[gcvFLOP_RESET_PPU_INSTRUCTION].address;

    gcmkONERROR(_ProgramPPUCommand(
        Hardware,
        stride,
        width,
        height,
        workDim,
        valueOrder,
        groupSizeX,
        groupSizeY,
        groupSizeZ,
        globalScaleX,
        globalScaleY,
        globalScaleZ,
        globalOffsetX,
        globalOffsetY,
        globalOffsetZ,
        threadAllocation,
        inImageAddress,
        outImageAddress,
        instAddress,
        instCount,
        regCount,
        AllocFlag,
        Pool,
        Command
        ));

    Command->data = data;
    Command->dataCount = dataCount;

    return gcvSTATUS_OK;

OnError:
    if (Command->funcVidMem)
    {
        gcmkVERIFY_OK(_FreeVideoMemory(
            Hardware->kernel,
            Command->funcVidMem
            ));
        Command->funcVidMem = gcvNULL;
    }

    if (data)
    {
        for (i = 0; i < dataCount; i++)
        {
            if (data[i].bufVidMem)
            {
                gcmkVERIFY_OK(_FreeVideoMemory(
                    Hardware->kernel,
                    data[i].bufVidMem
                    ));
            }
        }

        gcmkVERIFY_OK(gckOS_Free(Hardware->os, data));
    }

    return status;
}

/*
 * NN
 */
static gceSTATUS
_ProgramNNKernel(
    IN gckHARDWARE Hardware,
    IN gceVIP_ARCH_TYPE ArchType,
    IN gctUINT32 CoreCount,
    IN gctUINT32 Zdp,
    IN gctUINT8 DataType,
    IN gctUINT32 KernelXSize,
    IN gctUINT32 KernelYSize,
    IN gctUINT32 KernelZSize,
    IN gctUINT32 AllocFlag,
    IN OUT gcePOOL *Pool,
    OUT gcsFUNCTION_EXECUTION_DATA *Data
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gctUINT32 filterBytes = 0;
    gctUINT8_PTR kernels = gcvNULL;
    gctUINT32 offset = 0;
    gctUINT8_PTR kernelStreamSizePtr = gcvNULL;
    gctUINT32 filterTotalCount = 1;
    gctUINT32 itemBytes = 1;
    gctUINT32 biasBytes = 4;
    gckVIDMEM_NODE bufferNode = gcvNULL;
    gctPOINTER bufferLogical = gcvNULL;
    gctUINT32 bufferAddress = 0;
    gctSIZE_T bufferBytes, bytes;

    gcmkONERROR(_GetNNDataSize(DataType, &itemBytes));

    filterBytes = KernelXSize * KernelYSize * KernelZSize * itemBytes;

    /* Kernel buffer. */
    if (gcvVIP_ARCH_TYPE_V8 == ArchType)
    {
        /* Head (align to 64) + body (align to 64) + tail (align to 64). */
        gcmkASSERT(Zdp == 1 || Zdp == 3);
        bufferBytes = 64 + gcmALIGN_NP2((gcmALIGN_NP2(filterBytes, Zdp * 3) * (Zdp * 3) * filterTotalCount) + (1 * (16 / itemBytes)), 64) + 64;
    }
    else
    {
        /* Head (align to 64) + body (align to 64). */
        bufferBytes = 64 + gcmALIGN_NP2(((filterBytes + biasBytes +  3) * filterTotalCount + 3), 64);
    }

    gcmkONERROR(_AllocateVideoMemory(
        Hardware->kernel,
        gcvVIDMEM_TYPE_BITMAP,
        AllocFlag,
        Pool,
        &bufferBytes,
        &bufferNode,
        &bufferLogical,
        &bufferAddress
        ));

    kernels = (gctUINT8_PTR)bufferLogical;

    /* V8 huffman encoder. */
    if (gcvVIP_ARCH_TYPE_V8 == ArchType)
    {
        gctUINT32 i = 0;
        gctUINT8 rlt[][18] = {
            {0}, /* uint8 */
            {1, 1, 0, 1}, /* fp16 */
            {7, 1}, /* int8 */
            {0}, /* uint16 */
            {3, 1, 0, 1}, /* int16 */
            {0}, /* uint4 */
            {0}, /* int4 */
            {1, 1, 0, 1}    /* bf16 */
        };
        gctUINT8 map[][9] = {
            {1, 8, 7, 0, 4, 5, 6, 2, 3},
            {1, 5, 0, 7, 8, 2, 6, 3, 4},
            {1, 0, 7, 8, 4, 5, 6, 2, 3},
        };
        gctBOOL bit16 = DataType == 0x4 ||
                        DataType == 0x1  ||
                        DataType == 0x7;
        gctBOOL fp16 = DataType == 0x1;
        gctUINT32 index = 0;

        if (Hardware->identity.customerID == 0x9f)
        {
            rlt[0][0] = 3;
            rlt[0][1] = 1;
            rlt[0][3] = 1;
        }

        gcmkONERROR(_GetMapIndex(DataType, &index));

        gcmkONERROR(_BitValue(&kernels, 0, &offset, 1));        /* precode */
        gcmkONERROR(_BitValue(&kernels, bit16, &offset, 1));    /* bit16 */
        gcmkONERROR(_BitValue(&kernels, fp16, &offset, 1));     /* fp16 */
        gcmkONERROR(_BitValue(&kernels, 0, &offset, 1));        /* reserved */
        gcmkONERROR(_BitValue(&kernels, 1, &offset, 4));        /* version, 1 */
        gcmkONERROR(_BitValue(&kernels, 4, &offset, 8));        /* zero run length size */

        for (i = 0; i < 18; i++)
        {
            /* Zero run length x 18. */
            gcmkONERROR(_BitValue(&kernels, rlt[DataType][i], &offset, 8));
        }

        for (i = 0; i < 4; i++)
        {
            /* Map x 4. */
            gcmkONERROR(_BitValue(&kernels, (map[index][2 * i + 1] << 4) + map[index][2 * i], &offset, 8));
        }

        /* Avg bias */
        gcmkONERROR(_BitValue(&kernels, 0, &offset, 16));

        /* Reserved, must zero. */
        gcmkONERROR(_BitValue(&kernels, 0, &offset, 16));

        kernelStreamSizePtr = kernels;

        for (i = 0; i < CoreCount; i ++)
        {
            /* Stream size. */
            gcmkONERROR(_BitValue(&kernels, 0, &offset, 32));
        }

        kernels = (gctUINT8_PTR)bufferLogical + gcmALIGN_NP2((gctUINT32)((gctUINT8_PTR)kernels - (gctUINT8_PTR)bufferLogical), 64);

        switch (DataType)
        {
        case 0x4:
            /* Huffman data: 00000018 00924600 */
            gcmkONERROR(_BitValue(&kernels, 0x04058000, &offset, 32));
            /* Huffman data. */
            gcmkONERROR(_BitValue(&kernels, 0x640101fc, &offset, 32));
            /* Huffman data. */
            gcmkONERROR(_BitValue(&kernels, 0x00001200, &offset, 32));

            /* Only on core, stream size. */
            gcmkONERROR(_BitValue(&kernelStreamSizePtr, 0x0000006d, &offset, 32));

            break;

        case 0x0:
        case 0x2:
            /* Huffman data. */
            gcmkONERROR(_BitValue(&kernels, 0xec000038, &offset, 32));

            /* Only on core, stream size. */
            gcmkONERROR(_BitValue(&kernelStreamSizePtr, 0x35, &offset, 32));

            break;

        case 0x1:
            /* Huffman data: 0009db68 000006c0 000001f0 00000900 00024000. */
            gcmkONERROR(_BitValue(&kernels, 0x0009db68, &offset, 32));
            /* Huffman data. */
            gcmkONERROR(_BitValue(&kernels, 0x000006c0, &offset, 32));
            /* Huffman data. */
            gcmkONERROR(_BitValue(&kernels, 0x000001f0, &offset, 32));
            /* Huffman data. */
            gcmkONERROR(_BitValue(&kernels, 0x00000900, &offset, 32));
            /* Huffman data. */
            gcmkONERROR(_BitValue(&kernels, 0x00024000, &offset, 32));

            /* Only on core, stream size. */
            gcmkONERROR(_BitValue(&kernelStreamSizePtr, 0x000000a3, &offset, 32));

            break;

        case 0x7:
            /* Huffman data: 0007fff8 7f00fdfc c0397f00 0900001f 40000000 00000002. */
            gcmkONERROR(_BitValue(&kernels, 0x0007fff8, &offset, 32));
            /* Huffman data. */
            gcmkONERROR(_BitValue(&kernels, 0x7f00fdfc, &offset, 32));
            /* Huffman data. */
            gcmkONERROR(_BitValue(&kernels, 0xc0397f00, &offset, 32));
            /* Huffman data. */
            gcmkONERROR(_BitValue(&kernels, 0x0900001f, &offset, 32));
            /* Huffman data. */
            gcmkONERROR(_BitValue(&kernels, 0x40000000, &offset, 32));
            /* Huffman data. */
            gcmkONERROR(_BitValue(&kernels, 0x00000002, &offset, 32));

            /* Only on core, stream size. */
            gcmkONERROR(_BitValue(&kernelStreamSizePtr, 0x000000b2, &offset, 32));

            break;

        default:
            gcmkFATAL("Huffman encode not support this format! Please check!");
            break;
        }
    }
    else
    {
        gctBOOL zeroAll = gcvFALSE;
        gctUINT8 zrl = 0;
        gctUINT16 vzNum = 1;
        gctUINT32 bias = 0;
        gctUINT32 totalSize = gcmALIGN_NP2((filterTotalCount * (filterBytes + biasBytes +  3) + 3), 64);

        gckOS_ZeroMemory(kernels, totalSize + 64);

        *((gctUINT32_PTR)kernels) = totalSize;
        kernels += totalSize;
        if (zeroAll)
        {
            /*
             * Zrl & coreFilterCount, both compressed weight and bias are zero,
             * the size (1 * 1 * 2 * 2 + 4 ) < 64, aligned to 64.
             */
            *((gctUINT32_PTR)kernels) = (vzNum << (8 * itemBytes));
        }
        else
        {
            gctINT16 value = (DataType == 0x1) ? 0x3c00 /*1.0f*/ : 1;
            gctUINT32 i = 0;

            _BitValue(&kernels, zrl, &offset, 8);
            _BitValue(&kernels, vzNum, &offset, 16);
            _BitValue(&kernels, value, &offset, 8 * itemBytes);
            _BitValue(&kernels, bias, &offset, 32);

            if (DataType == 0x3 ||
                DataType == 0x4)
            {
                _BitValue(&kernels, 0, &offset, 16);
            }

            for (i = 1; i < filterBytes / itemBytes; i++)
            {
                _BitValue(&kernels, value, &offset, 8 * itemBytes);
            }
        }
    }

    bytes = kernels + (offset + 7) / 8 - (gctUINT8_PTR)bufferLogical;

    gcmkONERROR(gckVIDMEM_NODE_CleanCache(
        Hardware->kernel,
        bufferNode,
        0,
        bufferLogical,
        bytes
        ));

#if gcdDUMP_IN_KERNEL
    gcmkDUMP(Hardware->os, "#[flop reset: nn kernel]");
    gcmkDUMP_BUFFER(
        Hardware->os,
        gcvDUMP_BUFFER_KERNEL_COMMAND,
        bufferLogical,
        bufferAddress,
        bytes
        );
#endif

    Data->bufVidMem = bufferNode;
    Data->bufVidMemBytes = bufferBytes;
    Data->address = bufferAddress;
    Data->logical = bufferLogical;
    Data->bytes = bytes;

    return gcvSTATUS_OK;

OnError:
    if (bufferNode)
    {
        gcmkVERIFY_OK(_FreeVideoMemory(
            Hardware->kernel,
            bufferNode
            ));
    }

    return status;
}

static gceSTATUS
_ProgramNNInput(
    IN gckHARDWARE Hardware,
    IN gceVIP_ARCH_TYPE ArchType,
    IN gctUINT8 DataType,
    IN gctUINT32 InImageXSize,
    IN gctUINT32 InImageYSize,
    IN gctUINT32 InImageZSize,
    IN gctUINT32 AllocFlag,
    IN OUT gcePOOL *Pool,
    OUT gcsFUNCTION_EXECUTION_DATA_PTR Data
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gctUINT32 inputSize = InImageXSize * InImageYSize * InImageZSize;
    gctUINT32 itemBytes = 0;
    gckVIDMEM_NODE bufferNode = gcvNULL;
    gctPOINTER bufferLogical = gcvNULL;
    gctUINT32 bufferAddress = 0;
    gctSIZE_T bufferBytes, bytes;
    gctUINT8_PTR buffer = gcvNULL;

    gctUINT32 i = 0;
    gctUINT32 offset = 0;
    gctUINT32 value[] = {
        1, /* uint8 */
        0x3c00, /* fp16 */
        1, /* int8 */
        1, /* uint16 */
        1, /* int16 */
        1, /* uint4 */
        1, /* int4 */
        0x3f80  /* bf16 */
    };

    gcmkONERROR(_GetNNDataSize(DataType, &itemBytes));

    bufferBytes = inputSize * itemBytes;

    gcmkONERROR(_AllocateVideoMemory(
        Hardware->kernel,
        gcvVIDMEM_TYPE_BITMAP,
        AllocFlag,
        Pool,
        &bufferBytes,
        &bufferNode,
        &bufferLogical,
        &bufferAddress
        ));

    if (gcvVIP_ARCH_TYPE_V8 == ArchType)
    {
        value[0x4] = 0x81;
    }

    buffer = (gctUINT8_PTR)bufferLogical;

    for (i = 0; i < inputSize; i++)
    {
        _BitValue(&buffer, value[DataType], &offset, itemBytes * 8);
    }

    bytes = buffer + (offset + 7) / 8 - (gctUINT8_PTR)bufferLogical;

    gcmkONERROR(gckVIDMEM_NODE_CleanCache(
        Hardware->kernel,
        bufferNode,
        0,
        bufferLogical,
        bytes
        ));

#if gcdDUMP_IN_KERNEL
    gcmkDUMP(Hardware->os, "#[flop reset: nn input]");
    gcmkDUMP_BUFFER(
        Hardware->os,
        gcvDUMP_BUFFER_KERNEL_COMMAND,
        bufferLogical,
        bufferAddress,
        bytes
        );
#endif

    Data->bufVidMem = bufferNode;
    Data->bufVidMemBytes = bufferBytes;
    Data->address = bufferAddress;
    Data->logical = bufferLogical;
    Data->bytes = bytes;

    return gcvSTATUS_OK;

OnError:
    gcmkVERIFY_OK(_FreeVideoMemory(
        Hardware->kernel,
        bufferNode
        ));

    return status;
}

static gceSTATUS
_ProgramNNOutput(
    IN gckHARDWARE Hardware,
    IN gctUINT8 DataType,
    IN gctUINT32 OutputXSize,
    IN gctUINT32 OutputYSize,
    IN gctUINT32 OutputZSize,
    IN gctUINT32 AllocFlag,
    IN OUT gcePOOL *Pool,
    OUT gcsFUNCTION_EXECUTION_DATA *Data
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gctUINT32 itemBytes = 0;
    gckVIDMEM_NODE bufferNode = gcvNULL;
    gctPOINTER bufferLogical = gcvNULL;
    gctUINT32 bufferAddress = 0;
    gctSIZE_T bufferBytes, bytes;

    if (!Data)
    {
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    gcmkONERROR(_GetNNDataSize(DataType, &itemBytes));

    bufferBytes = bytes = OutputXSize * OutputYSize * OutputZSize * itemBytes;

    gcmkONERROR(_AllocateVideoMemory(
        Hardware->kernel,
        gcvVIDMEM_TYPE_BITMAP,
        AllocFlag,
        Pool,
        &bufferBytes,
        &bufferNode,
        &bufferLogical,
        &bufferAddress
        ));

    gcmkONERROR(gckVIDMEM_NODE_CleanCache(
        Hardware->kernel,
        bufferNode,
        0,
        bufferLogical,
        bytes
        ));

    Data->bufVidMem = bufferNode;
    Data->bufVidMemBytes = bufferBytes;
    Data->address = bufferAddress;
    Data->logical = bufferLogical;
    Data->bytes = bytes;

    return gcvSTATUS_OK;

OnError:
    gcmkVERIFY_OK(_FreeVideoMemory(
        Hardware->kernel,
        bufferNode
        ));

    return status;
}

static gceSTATUS
_ProgramNNInstruction(
    IN gckHARDWARE Hardware,
    IN gceVIP_ARCH_TYPE ArchType,
    IN gctUINT8 DataType,
    IN gctUINT32 InImageXSize,
    IN gctUINT32 InImageYSize,
    IN gctUINT32 OutImageXSize,
    IN gctUINT32 OutImageYSize,
    IN gctUINT32 OutImageZSize,
    IN gctUINT32 KernelXSize,
    IN gctUINT32 KernelYSize,
    IN gctUINT32 KernelZSize,
    IN gctUINT32 InImageAddress,
    IN gctUINT32 OutImageAddress,
    IN gctUINT32 KernelAddress,
    IN gctUINT32 AllocFlag,
    IN gcePOOL *Pool,
    OUT gcsFUNCTION_EXECUTION_DATA_PTR Data
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gckOS os = Hardware->os;

    gctUINT32 itemBytes;
    gckVIDMEM_NODE bufferNode = gcvNULL;
    gctPOINTER bufferLogical = gcvNULL;
    gctUINT32 bufferAddress = 0;
    gctSIZE_T bufferBytes, bytes;
    gctUINT32 *command = gcvNULL;

    gctUINT8 kernelDataType;
    gctUINT8 inImageDataType;
    gctUINT8 outImageDataType;

    gctUINT32 kernelsPerCore = 1;

    gctUINT32 nnLayerFlush = 1;
    gctUINT32 noZOffset = 0;
    gctUINT32 imageEndAddress = 2048;
    gctUINT32 postShift = 0;
    gctUINT32 postShiftBit56 = 0;
    gctUINT8 coefZP = 0;
    gctUINT8 outputZP = 0;

    bufferBytes = bytes = gcmSIZEOF(gctUINT32) * ((ArchType == gcvVIP_ARCH_TYPE_V6) ? 16 : 32);

    /* Allocate buffer. */
    gcmkONERROR(_AllocateVideoMemory(
        Hardware->kernel,
        gcvVIDMEM_TYPE_COMMAND,
        AllocFlag,
        Pool,
        &bufferBytes,
        &bufferNode,
        &bufferLogical,
        &bufferAddress
        ));

    command = (gctUINT32_PTR)bufferLogical;

    gcmkONERROR(_GetNNDataSize(DataType, &itemBytes));

    kernelDataType   =
    inImageDataType  =
    outImageDataType = DataType;

    switch (ArchType)
    {
    case gcvVIP_ARCH_TYPE_V8:
        noZOffset = 1;
        outputZP = 0;
        postShift = (gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_NN_FLOAT_POST_MULT)) ? 0x1f : 0;
        postShiftBit56 = (gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_NN_FLOAT_POST_MULT)) ? 3 : 0;
        break;

    case gcvVIP_ARCH_TYPE_V7:
    case gcvVIP_ARCH_TYPE_V6:
        postShift = (DataType == 0x2) ? 15 : 0;
        break;

    default:
        gcmkONERROR(gcvSTATUS_NOT_SUPPORTED);
        break;
    }

    /* gcregNNInstWord0 */
    gcmkWRITE_MEMORY(
        command,
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) ((gctUINT32) (noZOffset) & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:2) - (0 ?
 5:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:2) - (0 ?
 5:2) + 1))))))) << (0 ?
 5:2))) | (((gctUINT32) ((gctUINT32) (KernelXSize) & ((gctUINT32) ((((1 ?
 5:2) - (0 ?
 5:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:2) - (0 ? 5:2) + 1))))))) << (0 ? 5:2)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:6) - (0 ?
 19:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:6) - (0 ?
 19:6) + 1))))))) << (0 ?
 19:6))) | (((gctUINT32) ((gctUINT32) ((KernelZSize & 0x3FFF)) & ((gctUINT32) ((((1 ?
 19:6) - (0 ?
 19:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:6) - (0 ? 19:6) + 1))))))) << (0 ? 19:6)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:20) - (0 ?
 26:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:20) - (0 ?
 26:20) + 1))))))) << (0 ?
 26:20))) | (((gctUINT32) ((gctUINT32) (kernelsPerCore) & ((gctUINT32) ((((1 ?
 26:20) - (0 ?
 26:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:20) - (0 ? 26:20) + 1))))))) << (0 ? 26:20)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:27) - (0 ?
 28:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:27) - (0 ?
 28:27) + 1))))))) << (0 ?
 28:27))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 28:27) - (0 ?
 28:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 28:27) - (0 ? 28:27) + 1))))))) << (0 ? 28:27)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:29) - (0 ?
 29:29) + 1))))))) << (0 ?
 29:29))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ? 29:29)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:30) - (0 ?
 30:30) + 1))))))) << (0 ?
 30:30))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ? 30:30)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:31) - (0 ?
 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (nnLayerFlush) & ((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31)))
        );

    /* gcregNNInstWord1 */
    gcmkWRITE_MEMORY(
        command,
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:6) - (0 ?
 18:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:6) - (0 ?
 18:6) + 1))))))) << (0 ?
 18:6))) | (((gctUINT32) ((gctUINT32) (InImageXSize) & ((gctUINT32) ((((1 ?
 18:6) - (0 ?
 18:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:6) - (0 ? 18:6) + 1))))))) << (0 ? 18:6)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:19) - (0 ?
 31:19) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:19) - (0 ?
 31:19) + 1))))))) << (0 ?
 31:19))) | (((gctUINT32) ((gctUINT32) (InImageYSize) & ((gctUINT32) ((((1 ?
 31:19) - (0 ?
 31:19) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:19) - (0 ? 31:19) + 1))))))) << (0 ? 31:19)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) ((gctUINT32) (kernelDataType >> 1) & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (inImageDataType >> 1) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:5) - (0 ?
 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) ((gctUINT32) (outImageDataType >> 1) & ((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (kernelDataType & 0x1) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:2) - (0 ?
 2:2) + 1))))))) << (0 ?
 2:2))) | (((gctUINT32) ((gctUINT32) (inImageDataType & 0x1) & ((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ? 2:2)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) ((gctUINT32) (outImageDataType & 0x1) & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)))
        );

    /* gcregNNInstWord2 */
    gcmkWRITE_MEMORY(
        command,
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:24) - (0 ?
 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ? 24:24)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:25) - (0 ?
 25:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:25) - (0 ?
 25:25) + 1))))))) << (0 ?
 25:25))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 25:25) - (0 ?
 25:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:25) - (0 ? 25:25) + 1))))))) << (0 ? 25:25)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ? 5:3)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ? 26:26)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:7) - (0 ?
 7:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:7) - (0 ?
 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 7:7) - (0 ?
 7:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ? 7:7)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:8) - (0 ?
 23:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:8) - (0 ?
 23:8) + 1))))))) << (0 ?
 23:8))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 23:8) - (0 ?
 23:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:8) - (0 ? 23:8) + 1))))))) << (0 ? 23:8)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) ((gctUINT32) (postShift) & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
        );

    /* gcregNNInstWord3 */
    gcmkWRITE_MEMORY(
        command,
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:6) - (0 ?
 18:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:6) - (0 ?
 18:6) + 1))))))) << (0 ?
 18:6))) | (((gctUINT32) ((gctUINT32) (OutImageXSize) & ((gctUINT32) ((((1 ?
 18:6) - (0 ?
 18:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:6) - (0 ? 18:6) + 1))))))) << (0 ? 18:6)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:19) - (0 ?
 31:19) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:19) - (0 ?
 31:19) + 1))))))) << (0 ?
 31:19))) | (((gctUINT32) ((gctUINT32) (OutImageYSize) & ((gctUINT32) ((((1 ?
 31:19) - (0 ?
 31:19) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:19) - (0 ? 31:19) + 1))))))) << (0 ? 31:19)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:2) - (0 ?
 2:2) + 1))))))) << (0 ?
 2:2))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ? 2:2)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)))
        );

    /* gcregNNInstWord4 */
    gcmkWRITE_MEMORY(
        command,
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:0) - (0 ?
 13:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:0) - (0 ?
 13:0) + 1))))))) << (0 ?
 13:0))) | (((gctUINT32) ((gctUINT32) (OutImageZSize) & ((gctUINT32) ((((1 ?
 13:0) - (0 ?
 13:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:0) - (0 ? 13:0) + 1))))))) << (0 ? 13:0)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:18) - (0 ?
 24:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:18) - (0 ?
 24:18) + 1))))))) << (0 ?
 24:18))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 24:18) - (0 ?
 24:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:18) - (0 ? 24:18) + 1))))))) << (0 ? 24:18)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:25) - (0 ?
 31:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:25) - (0 ?
 31:25) + 1))))))) << (0 ?
 31:25))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 31:25) - (0 ?
 31:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:25) - (0 ? 31:25) + 1))))))) << (0 ? 31:25)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:16) - (0 ?
 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) ((gctUINT32) ((0 >> 3) & 0x1) & ((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:17) - (0 ?
 17:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 17:17) - (0 ?
 17:17) + 1))))))) << (0 ?
 17:17))) | (((gctUINT32) ((gctUINT32) ((0 >> 3) & 0x1) & ((gctUINT32) ((((1 ?
 17:17) - (0 ?
 17:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 17:17) - (0 ? 17:17) + 1))))))) << (0 ? 17:17)))
        );

    /* gcregNNInstWord5 */
    gcmkWRITE_MEMORY(
        command,
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:26) - (0 ?
 31:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:26) - (0 ?
 31:26) + 1))))))) << (0 ?
 31:26))) | (((gctUINT32) ((gctUINT32) (((KernelZSize >> 14) & 0x3F)) & ((gctUINT32) ((((1 ?
 31:26) - (0 ?
 31:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:26) - (0 ? 31:26) + 1))))))) << (0 ? 31:26)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:0) - (0 ?
 25:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:0) - (0 ?
 25:0) + 1))))))) << (0 ?
 25:0))) | (((gctUINT32) ((gctUINT32) ((KernelAddress >> 6)) & ((gctUINT32) ((((1 ?
 25:0) - (0 ?
 25:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:0) - (0 ? 25:0) + 1))))))) << (0 ? 25:0)))
        );

    /* gcregNNInstWord6 */
    gcmkWRITE_MEMORY(command, InImageAddress);

    /* gcregNNInstWord7 */
    gcmkWRITE_MEMORY(command, OutImageAddress);

    /* gcregNNInstWord8 */
    gcmkWRITE_MEMORY(
        command,
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:12) - (0 ?
 15:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:12) - (0 ?
 15:12) + 1))))))) << (0 ?
 15:12))) | (((gctUINT32) ((gctUINT32) (KernelYSize) & ((gctUINT32) ((((1 ?
 15:12) - (0 ?
 15:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ? 15:12)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (OutImageYSize) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)))
        );

    /* gcregNNInstWord9 */
    gcmkWRITE_MEMORY(command, 0);

    /* gcregNNInstWord10 */
    gcmkWRITE_MEMORY(command, 0);

    /* gcregNNInstWord11 */
    gcmkWRITE_MEMORY(command, 0);

    /* gcregNNInstWord12 */
    gcmkWRITE_MEMORY(command, 0);

    /* gcregNNInstWord13 */
    gcmkWRITE_MEMORY(command, 0);

    /* gcregNNInstWord14 */
    gcmkWRITE_MEMORY(command, imageEndAddress);

    /* gcregNNInstWord15 */
    gcmkWRITE_MEMORY(
        command,
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:2) - (0 ?
 17:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 17:2) - (0 ?
 17:2) + 1))))))) << (0 ?
 17:2))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 17:2) - (0 ?
 17:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 17:2) - (0 ? 17:2) + 1))))))) << (0 ? 17:2)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:19) - (0 ?
 19:19) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:19) - (0 ?
 19:19) + 1))))))) << (0 ?
 19:19))) | (((gctUINT32) ((gctUINT32) (kernelDataType >> 2) & ((gctUINT32) ((((1 ?
 19:19) - (0 ?
 19:19) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:19) - (0 ? 19:19) + 1))))))) << (0 ? 19:19)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:20) - (0 ?
 20:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:20) - (0 ?
 20:20) + 1))))))) << (0 ?
 20:20))) | (((gctUINT32) ((gctUINT32) (inImageDataType >> 2) & ((gctUINT32) ((((1 ?
 20:20) - (0 ?
 20:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:20) - (0 ? 20:20) + 1))))))) << (0 ? 20:20)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:21) - (0 ?
 21:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:21) - (0 ?
 21:21) + 1))))))) << (0 ?
 21:21))) | (((gctUINT32) ((gctUINT32) (outImageDataType >> 2) & ((gctUINT32) ((((1 ?
 21:21) - (0 ?
 21:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ? 21:21)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:22) - (0 ?
 27:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:22) - (0 ?
 27:22) + 1))))))) << (0 ?
 27:22))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 27:22) - (0 ?
 27:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:22) - (0 ? 27:22) + 1))))))) << (0 ? 27:22)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:28) - (0 ?
 29:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:28) - (0 ?
 29:28) + 1))))))) << (0 ?
 29:28))) | (((gctUINT32) ((gctUINT32) (postShiftBit56) & ((gctUINT32) ((((1 ?
 29:28) - (0 ?
 29:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:28) - (0 ? 29:28) + 1))))))) << (0 ? 29:28)))
        );

    /* V7 or V8 */
    if (ArchType == gcvVIP_ARCH_TYPE_V7 ||
        ArchType == gcvVIP_ARCH_TYPE_V8)
    {
        /* gcregNNInstWord16 */
        gcmkWRITE_MEMORY(
            command,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (InImageXSize * itemBytes) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (InImageYSize) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)))
            );

        /* gcregNNInstWord17 */
        gcmkWRITE_MEMORY(
            command,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (OutImageXSize * itemBytes) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:24) - (0 ?
 31:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:24) - (0 ?
 31:24) + 1))))))) << (0 ?
 31:24))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 31:24) - (0 ?
 31:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:24) - (0 ? 31:24) + 1))))))) << (0 ? 31:24)))
            );

        /* gcregNNInstWord18 */
        gcmkWRITE_MEMORY(
            command,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:0) - (0 ?
 25:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:0) - (0 ?
 25:0) + 1))))))) << (0 ?
 25:0))) | (((gctUINT32) ((gctUINT32) (0 >> 6) & ((gctUINT32) ((((1 ?
 25:0) - (0 ?
 25:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:0) - (0 ? 25:0) + 1))))))) << (0 ? 25:0)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ? 26:26)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:29) - (0 ?
 29:29) + 1))))))) << (0 ?
 29:29))) | (((gctUINT32) ((gctUINT32) ((0 >> 4) & 0x1) & ((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ? 29:29)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:30) - (0 ?
 30:30) + 1))))))) << (0 ?
 30:30))) | (((gctUINT32) ((gctUINT32) ((0 >> 4) & 0x1) & ((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ? 30:30)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:28) - (0 ?
 28:28) + 1))))))) << (0 ?
 28:28))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 28:28) - (0 ? 28:28) + 1))))))) << (0 ? 28:28)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:31) - (0 ?
 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (kernelDataType >> 3) & ((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31)))
            );

        /* 25:0 */
        gcmkWRITE_MEMORY(
            command,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:0) - (0 ?
 25:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:0) - (0 ?
 25:0) + 1))))))) << (0 ?
 25:0))) | (((gctUINT32) ((gctUINT32) (0xFFFFFFFF >> 6) & ((gctUINT32) ((((1 ?
 25:0) - (0 ?
 25:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:0) - (0 ? 25:0) + 1))))))) << (0 ? 25:0)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:30) - (0 ?
 30:30) + 1))))))) << (0 ?
 30:30))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ? 30:30)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:31) - (0 ?
 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (inImageDataType >> 3) & ((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31)))
            );

        /* GCREG_NN_INST_WORD20 */
        gcmkWRITE_MEMORY(
            command,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:0) - (0 ?
 25:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:0) - (0 ?
 25:0) + 1))))))) << (0 ?
 25:0))) | (((gctUINT32) ((gctUINT32) (0 >> 6) & ((gctUINT32) ((((1 ?
 25:0) - (0 ?
 25:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:0) - (0 ? 25:0) + 1))))))) << (0 ? 25:0)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (outImageDataType >> 3) & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ? 26:26)))
            );

        /* 25:0 */
        gcmkWRITE_MEMORY(
            command,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:0) - (0 ?
 25:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:0) - (0 ?
 25:0) + 1))))))) << (0 ?
 25:0))) | (((gctUINT32) ((gctUINT32) (0xFFFFFFFF >> 6) & ((gctUINT32) ((((1 ?
 25:0) - (0 ?
 25:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:0) - (0 ? 25:0) + 1))))))) << (0 ? 25:0)))
            );

        /*GCREG_NN_INST_WORD22*/
        gcmkWRITE_MEMORY(
            command,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:0) - (0 ?
 7:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:0) - (0 ?
 7:0) + 1))))))) << (0 ?
 7:0))) | (((gctUINT32) ((gctUINT32) (coefZP) & ((gctUINT32) ((((1 ?
 7:0) - (0 ?
 7:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ? 7:0)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:8) - (0 ?
 15:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:8) - (0 ?
 15:8) + 1))))))) << (0 ?
 15:8))) | (((gctUINT32) ((gctUINT32) (outputZP) & ((gctUINT32) ((((1 ?
 15:8) - (0 ?
 15:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ? 15:8)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:16) - (0 ?
 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:17) - (0 ?
 17:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 17:17) - (0 ?
 17:17) + 1))))))) << (0 ?
 17:17))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 17:17) - (0 ?
 17:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 17:17) - (0 ? 17:17) + 1))))))) << (0 ? 17:17)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:18) - (0 ?
 25:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:18) - (0 ?
 25:18) + 1))))))) << (0 ?
 25:18))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 25:18) - (0 ?
 25:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:18) - (0 ? 25:18) + 1))))))) << (0 ? 25:18)))
            );

        /*GCREG_NN_INST_WORD23*/
        gcmkWRITE_MEMORY(command, 0);

        /*GCREG_NN_INST_WORD24*/
        gcmkWRITE_MEMORY(
            command,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:0) - (0 ?
 3:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:0) - (0 ?
 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 3:0) - (0 ?
 3:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ? 3:0)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:4) - (0 ?
 31:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:4) - (0 ?
 31:4) + 1))))))) << (0 ?
 31:4))) | (((gctUINT32) ((gctUINT32) (0 >> 4) & ((gctUINT32) ((((1 ?
 31:4) - (0 ?
 31:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:4) - (0 ? 31:4) + 1))))))) << (0 ? 31:4)))
            );

        /*GCREG_NN_INST_WORD25*/
        gcmkWRITE_MEMORY(
            command,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:0) - (0 ?
 3:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:0) - (0 ?
 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 3:0) - (0 ?
 3:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ? 3:0)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:4) - (0 ?
 31:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:4) - (0 ?
 31:4) + 1))))))) << (0 ?
 31:4))) | (((gctUINT32) ((gctUINT32) (0 >> 4) & ((gctUINT32) ((((1 ?
 31:4) - (0 ?
 31:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:4) - (0 ? 31:4) + 1))))))) << (0 ? 31:4)))
            );
    }

    gcmkONERROR(gckVIDMEM_NODE_CleanCache(
        Hardware->kernel,
        bufferNode,
        0,
        bufferLogical,
        bytes
        ));

#if gcdDUMP_IN_KERNEL
    gcmkDUMP(Hardware->os, "#[flop reset: nn instruction]");
    gcmkDUMP_BUFFER(
        Hardware->os,
        gcvDUMP_BUFFER_KERNEL_COMMAND,
        bufferLogical,
        bufferAddress,
        bytes
        );
#endif

    Data->bufVidMem = bufferNode;
    Data->bufVidMemBytes = bufferBytes;
    Data->address = bufferAddress;
    Data->logical = bufferLogical;
    Data->bytes = bytes;

    return gcvSTATUS_OK;

OnError:
    if (bufferNode)
    {
        gcmkVERIFY_OK(_FreeVideoMemory(
            Hardware->kernel,
            bufferNode
            ));
    }

    return status;
}

static gceSTATUS
_ProgramNNCommand(
    IN gckHARDWARE Hardware,
    IN gceVIP_ARCH_TYPE ArchType,
    IN gctUINT32 KernelBurstSize,
    IN gctUINT32 InstAddress,
    IN gctUINT32 AllocFlag,
    IN gcePOOL *Pool,
    OUT gcsFUNCTION_COMMAND_PTR Command
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gckVIDMEM_NODE bufferNode = gcvNULL;
    gctPOINTER bufferLogical = gcvNULL;
    gctUINT32 bufferAddress = 0;
    gctSIZE_T bufferBytes;
    gctUINT32 bytes;
    gctUINT8_PTR endLogical;
    gctUINT32 endAddress;
    gctUINT32 endBytes = 0;
    gcsFEATURE_DATABASE *database = (gcsFEATURE_DATABASE *)(Hardware->featureDatabase);
    gctUINT32 index = 0;
    gctINT32 disableZDPN = 1, disableSWTiling = 1;
    gctBOOL enableNNStride = gcvFALSE;
    gctUINT32 smallBatch;
    gctUINT32 ddrBurstSize;
    gctUINT32 *commands = gcvNULL;

    bufferBytes = gcmSIZEOF(gctUINT32) * MAX_NN_COMMAND_NUM;

    gcmkONERROR(_AllocateVideoMemory(
        Hardware->kernel,
        gcvVIDMEM_TYPE_COMMAND,
        AllocFlag,
        Pool,
        &bufferBytes,
        &bufferNode,
        &bufferLogical,
        &bufferAddress
        ));

    commands = (gctUINT32 *)bufferLogical;

    disableZDPN = (database->NN_ZDP3 || database->NN_ZDP6) ? 0 : 1;

    enableNNStride = database->NN_STRIDE_SUPPORT;
    disableSWTiling = enableNNStride ? 0 : 1;

    if (Hardware->identity.chipModel == 0x8000 &&
        Hardware->identity.chipRevision == 0x7120 &&
        (Hardware->identity.customerID == 0x80 ||
         Hardware->identity.customerID == 0x92))
    {
        smallBatch = 0x0;
    }
    else
    {
        smallBatch = (database->NN_SMALLBATCH_PHASE1 && database->NN_COMMAND_KERNEL_REQUEST_CONFICT_FIX)
                     ? 0x0 : 0x1;
    }

    switch(KernelBurstSize)
    {
    case 256:
        ddrBurstSize = 0x2;
        break;

    case 64:
    default:
        ddrBurstSize = 0x0;
        break;
    }

    commands = (gctUINT32_PTR)bufferLogical;

    if (gcvVIP_ARCH_TYPE_V6 == ArchType)
    {
        commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x006B) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
        commands[index++] = 0;

        commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E03) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
        commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)));
    }

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E4E) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = Hardware->options.sRAMGPUVirtAddrs[0];
    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E4F) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = 0x00000000;

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E50) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = 0x00000000;

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E03) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)));

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E4C) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:2) - (0 ?
 2:2) + 1))))))) << (0 ?
 2:2))) | (((gctUINT32) ((gctUINT32) (disableZDPN) & ((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ? 2:2)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (disableSWTiling) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) ((gctUINT32) (smallBatch) & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:5) - (0 ?
 6:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:5) - (0 ?
 6:5) + 1))))))) << (0 ?
 6:5))) | (((gctUINT32) ((gctUINT32) (ddrBurstSize) & ((gctUINT32) ((((1 ?
 6:5) - (0 ?
 6:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:5) - (0 ? 6:5) + 1))))))) << (0 ? 6:5)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)));

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E54) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)));

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0428) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:6) - (0 ?
 31:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:6) - (0 ?
 31:6) + 1))))))) << (0 ?
 31:6))) | (((gctUINT32) ((gctUINT32) ((InstAddress >> 6)) & ((gctUINT32) ((((1 ?
 31:6) - (0 ?
 31:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:6) - (0 ? 31:6) + 1))))))) << (0 ? 31:6)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)));

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0429) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = 0;

    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E03) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
    commands[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:5) - (0 ?
 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5)));

    bytes = gcmSIZEOF(gctUINT32) * index;

    endLogical = (gctUINT8_PTR)bufferLogical + bytes;
    endAddress = bufferAddress + bytes;

    if (Hardware->wlFE)
    {
        gcmkONERROR(gckWLFE_End(Hardware, gcvNULL, ~0U, &endBytes));
        gcmkONERROR(gckWLFE_End(Hardware, endLogical, endAddress, &endBytes));
    }

    bytes += endBytes;

    gcmkASSERT(bytes <= bufferBytes);

    gcmkONERROR(gckVIDMEM_NODE_CleanCache(
        Hardware->kernel,
        bufferNode,
        0,
        bufferLogical,
        bytes
        ));

    Command->funcVidMem = bufferNode;
    Command->funcVidMemBytes = bufferBytes;
    Command->logical = bufferLogical;
    Command->address = bufferAddress;
    Command->bytes = bytes;
    Command->endAddress = endAddress;
    Command->endLogical = endLogical;

    return gcvSTATUS_OK;

OnError:
    if (bufferNode)
    {
        gcmkONERROR(_FreeVideoMemory(
            Hardware->kernel,
            bufferNode
            ));
    }

    return status;
}

gceSTATUS
gckHARDWARE_ResetFlopWithNN(
    IN gckHARDWARE Hardware,
    IN gctUINT32 AllocFlag,
    IN OUT gcePOOL *Pool,
    OUT gcsFUNCTION_COMMAND *Command
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gctUINT32 kernelXSize = NN_KERNEL_XSIZE;
    gctUINT32 kernelYSize = NN_KERNEL_YSIZE;
    gctUINT32 kernelZSize = NN_KERNEL_ZSIZE;

    gctUINT32 inImageXSize = NN_INPUT_XSIZE;
    gctUINT32 inImageYSize = NN_INPUT_YSIZE;
    gctUINT32 inImageZSize = NN_INPUT_ZSIZE;

    gctUINT32 outImageXSize = NN_OUTPUT_XSIZE;
    gctUINT32 outImageYSize = NN_OUTPUT_YSIZE;
    gctUINT32 outImageZSize = NN_OUTPUT_ZSIZE;

    gctUINT32 i;
    gctPOINTER pointer = gcvNULL;

    gceVIP_ARCH_TYPE archType;
    gctUINT8 dataType;
    gctUINT32 coreCount = 0;
    gctUINT32 itemBytes = 0;
    gctUINT32 zdp = 1;
    gctUINT32 kernelBurstSize;
    gcsFUNCTION_EXECUTION_DATA_PTR data = gcvNULL;
    gctUINT32 dataCount = 0;

    if (!Command)
    {
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    gcmkONERROR(_GetVIPCoreInfo(
        Hardware,
        &archType,
        &dataType,
        &coreCount,
        &zdp,
        &kernelBurstSize
        ));

    gcmkONERROR(_GetNNDataSize(dataType, &itemBytes));

    /* Exectution data. */
    dataCount = gcvFLOP_RESET_NN_DATA_NUM;
    gcmkASSERT(dataCount > 0);

    gcmkONERROR(gckOS_Allocate(
        Hardware->os,
        gcmSIZEOF(gcsFUNCTION_EXECUTION_DATA) * dataCount,
        &pointer
        ));
    gckOS_ZeroMemory(pointer, gcmSIZEOF(gcsFUNCTION_EXECUTION_DATA) * dataCount);
    data = (gcsFUNCTION_EXECUTION_DATA *)pointer;

    /* Kernel. */
    gcmkONERROR(_ProgramNNKernel(
        Hardware,
        archType,
        coreCount,
        zdp,
        dataType,
        kernelXSize,
        kernelYSize,
        kernelZSize,
        AllocFlag,
        Pool,
        &data[gcvFLOP_RESET_NN_KERNEL]
        ));

    /* Input. */
    gcmkONERROR(_ProgramNNInput(
        Hardware,
        archType,
        dataType,
        inImageXSize,
        inImageYSize,
        inImageZSize,
        AllocFlag,
        Pool,
        &data[gcvFLOP_RESET_NN_INPUT]
        ));

    /* Output. */
    gcmkONERROR(_ProgramNNOutput(
        Hardware,
        dataType,
        outImageXSize,
        outImageYSize,
        outImageZSize,
        AllocFlag,
        Pool,
        &data[gcvFLOP_RESET_NN_OUTPUT]
        ));

    /* Commands. */
    gcmkONERROR(_ProgramNNInstruction(
        Hardware,
        archType,
        dataType,
        inImageXSize,
        inImageYSize,
        outImageXSize,
        outImageYSize,
        outImageZSize,
        kernelXSize,
        kernelYSize,
        kernelZSize,
        data[gcvFLOP_RESET_NN_INPUT].address,
        data[gcvFLOP_RESET_NN_OUTPUT].address,
        data[gcvFLOP_RESET_NN_KERNEL].address,
        AllocFlag,
        Pool,
        &data[gcvFLOP_RESET_NN_INSTRUCTION]
        ));

    gcmkONERROR(_ProgramNNCommand(
        Hardware,
        archType,
        kernelBurstSize,
        data[gcvFLOP_RESET_NN_INSTRUCTION].address,
        AllocFlag,
        Pool,
        Command
        ));

    Command->data = data;
    Command->dataCount = dataCount;

    return gcvSTATUS_OK;

OnError:
    if (Command->funcVidMem)
    {
        gcmkVERIFY_OK(_FreeVideoMemory(
            Hardware->kernel,
            Command->funcVidMem
            ));
        Command->funcVidMem = gcvNULL;
    }

    if (data)
    {
        for (i = 0; i < dataCount; i++)
        {
            if (data[i].bufVidMem)
            {
                gcmkVERIFY_OK(_FreeVideoMemory(
                    Hardware->kernel,
                    data[i].bufVidMem
                    ));
            }
        }

        gcmkVERIFY_OK(gckOS_Free(Hardware->os, data));
    }

    return status;
}

/*
 * TP.
 */
gceSTATUS
gckHARDWARE_ResetFlopWithTP(
    IN gckHARDWARE Hardware,
    IN gctUINT32 AllocFlag,
    IN gcePOOL *Pool,
    OUT gcsFUNCTION_COMMAND_PTR Command
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmkONERROR(gcvSTATUS_OK);

    return gcvSTATUS_OK;

OnError:
    return status;
}

