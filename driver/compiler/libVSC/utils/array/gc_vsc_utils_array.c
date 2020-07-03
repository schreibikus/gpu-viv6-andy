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


#include "gc_vsc.h"

#define RESIZE_FACTOR              1.5

VSC_SIMPLE_RESIZABLE_ARRAY* vscSRARR_Create(VSC_MM* pMM, gctUINT initAllocEleCount, gctUINT elementSize, PFN_VSC_ARRAY_ELE_CMP pfnEleCmp)
{
    VSC_SIMPLE_RESIZABLE_ARRAY*   pArray = gcvNULL;

    pArray = (VSC_SIMPLE_RESIZABLE_ARRAY*)vscMM_Alloc(pMM, sizeof(VSC_SIMPLE_RESIZABLE_ARRAY));
    if (pArray == gcvNULL)
    {
        ERR_REPORT(VSC_ERR_OUT_OF_MEMORY, "Fail in vscSRARR_Create");
        return pArray;
    }
    if(vscSRARR_Initialize(pArray, pMM, initAllocEleCount, elementSize, pfnEleCmp) != VSC_ERR_NONE)
        return gcvNULL;

    return pArray;
}

VSC_ErrCode vscSRARR_Initialize(VSC_SIMPLE_RESIZABLE_ARRAY* pArray, VSC_MM* pMM, gctUINT initAllocEleCount, gctUINT elementSize, PFN_VSC_ARRAY_ELE_CMP pfnEleCmp)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    pArray->pMM = pMM;
    pArray->pfnEleCmp = pfnEleCmp;
    pArray->elementSize = elementSize;
    pArray->elementCount = 0;
    pArray->allocatedCount = initAllocEleCount;
    pArray->pElement = vscMM_Alloc(pMM, elementSize*initAllocEleCount);
    if(pArray->pElement == gcvNULL)
    {
        ERR_REPORT(VSC_ERR_OUT_OF_MEMORY, "Fail in vscSRARR_Initialize");
        errCode = VSC_ERR_OUT_OF_MEMORY;
    }
    return errCode;
}

void vscSRARR_Finalize(VSC_SIMPLE_RESIZABLE_ARRAY* pArray)
{
    pArray->allocatedCount = 0;
    pArray->elementCount = 0;
    pArray->elementSize = 0;
    vscMM_Free(pArray->pMM, pArray->pElement);
    pArray->pElement = gcvNULL;
}

void vscSRARR_Clear(VSC_SIMPLE_RESIZABLE_ARRAY* pArray)
{
    pArray->elementCount = 0;
}

void vscSRARR_Destroy(VSC_SIMPLE_RESIZABLE_ARRAY* pArray)
{
    if (pArray)
    {
        vscSRARR_Finalize(pArray);

        /* Free array itself */
        vscMM_Free(pArray->pMM, pArray);
        pArray = gcvNULL;
    }
}

static void _CheckElementSpace(VSC_SIMPLE_RESIZABLE_ARRAY* pArray)
{
    gctUINT newAllocCount;

    if (pArray->elementCount == pArray->allocatedCount)
    {
        newAllocCount= (gctUINT)((gctFLOAT)pArray->allocatedCount * RESIZE_FACTOR);

        if (newAllocCount <=  pArray->allocatedCount)
        {
            newAllocCount = pArray->allocatedCount + 1;
        }

        pArray->allocatedCount = newAllocCount;
        pArray->pElement = vscMM_Realloc(pArray->pMM, pArray->pElement, pArray->elementSize*pArray->allocatedCount);
    }
}

VSC_ErrCode vscSRARR_SetElementCount(VSC_SIMPLE_RESIZABLE_ARRAY* pArray, gctUINT newEleCount)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    if (pArray->allocatedCount < newEleCount)
    {
        pArray->allocatedCount = newEleCount;
        pArray->pElement = vscMM_Realloc(pArray->pMM, pArray->pElement, pArray->elementSize*pArray->allocatedCount);
        if (pArray->pElement == gcvNULL)
        {
            errCode = VSC_ERR_OUT_OF_MEMORY;
            ON_ERROR(errCode, "Failed to allocate memory for Array->pElement.");
        }
    }

    pArray->elementCount = newEleCount;

OnError:
    return errCode;
}

gctUINT vscSRARR_GetElementCount(VSC_SIMPLE_RESIZABLE_ARRAY* pArray)
{
    return pArray->elementCount;
}

gctUINT vscSRARR_AddElement(VSC_SIMPLE_RESIZABLE_ARRAY* pArray, void* pNewEle)
{
    if (pArray->elementSize == 0)
    {
        WARNING_REPORT(VSC_ERR_INVALID_DATA, "The element size of an array is 0, something is wrong!!!");
    }

    _CheckElementSpace(pArray);
    memcpy((gctUINT8*)pArray->pElement + pArray->elementSize*pArray->elementCount,
           pNewEle, pArray->elementSize);
    return pArray->elementCount ++;
}

gctUINT vscSRARR_AddElementToSpecifiedIndex(VSC_SIMPLE_RESIZABLE_ARRAY* pArray, void* pNewEle, gctINT index)
{
    gctUINT i, j;

    if (index < 0)
    {
        WARNING_REPORT(VSC_ERR_INVALID_DATA, "The index is negative, something is wrong!!!");
        index = (gctINT)pArray->elementCount;
    }

    if (index > (gctINT)pArray->elementCount)
    {
        WARNING_REPORT(VSC_ERR_INVALID_DATA, "The index is larger than the element count, something is wrong!!!");
        index = (gctINT)pArray->elementCount;
    }

    /* When the index is the element count, just append a new element. */
    if (index == (gctINT)pArray->elementCount)
    {
        return vscSRARR_AddElement(pArray, pNewEle);
    }

    if (pArray->elementSize == 0)
    {
        WARNING_REPORT(VSC_ERR_INVALID_DATA, "The element size of an array is 0, something is wrong!!!");
    }

    _CheckElementSpace(pArray);

    for (i = pArray->elementCount - 1; i >= (gctUINT)index; i--)
    {
        for (j = 0; j < pArray->elementSize; j++)
        {
            *((gctUINT8*)pArray->pElement + pArray->elementSize * (i + 1) + j) =
                *((gctUINT8*)pArray->pElement + pArray->elementSize * i + j);
        }
    }

    memcpy((gctUINT8*)pArray->pElement + pArray->elementSize * index, pNewEle, pArray->elementSize);

    return pArray->elementCount ++;
}

void* vscSRARR_GetNextEmpty(VSC_SIMPLE_RESIZABLE_ARRAY* pArray, gctUINT *pIndex)
{
    _CheckElementSpace(pArray);
    *pIndex = pArray->elementCount ++;
    return (gctUINT8*)pArray->pElement + pArray->elementSize*(*pIndex);
}

void* vscSRARR_GetElement(VSC_SIMPLE_RESIZABLE_ARRAY* pArray, gctUINT index)
{
    if (index >= pArray->elementCount)
    {
        return gcvNULL;
    }

    return (gctUINT8*)pArray->pElement + pArray->elementSize*index;
}

gctUINT vscSRARR_GetElementIndexByContent(VSC_SIMPLE_RESIZABLE_ARRAY* pArray, void* pEle)
{
    gctUINT idx;

    if (pArray->pfnEleCmp == gcvNULL)
    {
        gcmASSERT(gcvFALSE);
        return VSC_INVALID_ARRAY_INDEX;
    }

    for (idx = 0; idx < pArray->elementCount; idx ++)
    {
        if (pArray->pfnEleCmp(pEle, (gctUINT8*)pArray->pElement + pArray->elementSize*idx))
        {
            return idx;
        }
    }

    return VSC_INVALID_ARRAY_INDEX;
}

void vscSRARR_RemoveElementByIndex(VSC_SIMPLE_RESIZABLE_ARRAY* pArray, gctUINT index)
{
    gctUINT8*  pDst;
    gctUINT8*  pSrc;
    gctUINT    byteCount;

    if (index >= pArray->elementCount)
    {
        return;
    }

    pDst = (gctUINT8*)pArray->pElement + pArray->elementSize*index;
    pSrc = (gctUINT8*)pArray->pElement + pArray->elementSize*(index+1);
    byteCount = pArray->elementSize*(pArray->elementCount - index - 1);

    if (byteCount)
    {
        if (byteCount == pArray->elementSize)
        {
            /* Non-overlap case */
            memcpy(pDst, pSrc, byteCount);
        }
        else
        {
            /* Overlap case */
            memmove(pDst, pSrc, byteCount);
        }
    }

    pArray->elementCount --;
}

gctBOOL vscSRARR_RemoveElementByContent(VSC_SIMPLE_RESIZABLE_ARRAY* pArray, void* pEleToRemove)
{
    gctUINT idx = vscSRARR_GetElementIndexByContent(pArray, pEleToRemove);

    if (idx != VSC_INVALID_ARRAY_INDEX)
    {
        vscSRARR_RemoveElementByIndex(pArray, idx);
        return gcvTRUE;
    }

    return gcvFALSE;
}

