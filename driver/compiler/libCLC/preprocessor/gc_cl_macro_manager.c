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


#include "gc_cl_preprocessor_int.h"


/******************************************************************************\
MACRO SYMBOL Dump
\******************************************************************************/
gceSTATUS
ppoMACRO_SYMBOL_Dump(
    ppoPREPROCESSOR     PP,
    ppoMACRO_SYMBOL MS)
{

    gceSTATUS status;

    gcmONERROR(
        cloCOMPILER_Dump(
            PP->compiler,
            clvDUMP_PREPROCESSOR,
            "<Macro name=\"%s\" argc=\"%d\" />",
            MS->name,
            MS->argc
            )
        );


    gcmONERROR(
        ppoBASE_Dump(PP, (ppoBASE)MS)
        );

    gcmONERROR(
        cloCOMPILER_Dump(
            PP->compiler,
            clvDUMP_PREPROCESSOR,
            "<Argv>"
            )
        );

    if(MS->argv)
    {
        gcmONERROR(
            ppoINPUT_STREAM_Dump(
                PP,
                (ppoINPUT_STREAM)MS->argv)
            );
    }

    gcmONERROR(
        cloCOMPILER_Dump(
            PP->compiler,
            clvDUMP_PREPROCESSOR,
            "</Argv>"
            )
        );

    gcmONERROR(
        cloCOMPILER_Dump(
            PP->compiler,
            clvDUMP_PREPROCESSOR,
            "<ReplacementList>"
            )
        );

    if( MS->replacementList )
    {
        gcmONERROR(
            ppoINPUT_STREAM_Dump(PP, (ppoINPUT_STREAM)MS->replacementList)
            );
    }

    gcmONERROR(
        cloCOMPILER_Dump(
            PP->compiler,
            clvDUMP_PREPROCESSOR,
            "</ReplacementList>"
            )
        );

    gcmONERROR(
        cloCOMPILER_Dump(
            PP->compiler,
            clvDUMP_PREPROCESSOR,
            "</Macro>"
            )
        );

    if(MS->base.node.prev == gcvNULL)
    {
        return gcvSTATUS_OK;
    }
    else
    {
        return ppoINPUT_STREAM_Dump(
                    PP, (ppoINPUT_STREAM)MS->base.node.prev
                    );
    }

OnError:
    return status;
}


gceSTATUS
ppoMACRO_MANAGER_Dump(
    ppoPREPROCESSOR             PP,
    ppoMACRO_MANAGER    MACM)
{
    cloCOMPILER_Dump(
        PP->compiler,
        clvDUMP_PREPROCESSOR,
        "<MacroManager>");

    ppoBASE_Dump(PP, (ppoBASE)MACM);


    ppoMACRO_SYMBOL_Dump(PP, MACM->ir);

    cloCOMPILER_Dump(
        PP->compiler,
        clvDUMP_PREPROCESSOR,
        "</MacroManager>");

    return gcvSTATUS_OK;
}

gceSTATUS
ppoMACRO_SYMBOL_Destroy(ppoPREPROCESSOR PP, ppoMACRO_SYMBOL MS)
{
    ppoBASE tmp,prev;
    gceSTATUS status;

    tmp = (ppoBASE)MS->argv;

    while(tmp)
    {
        prev = (ppoBASE)tmp->node.prev;

        gcmONERROR(ppoTOKEN_Destroy(PP, (ppoTOKEN_STREAM)tmp));

        tmp = prev;
    }

    tmp = (ppoBASE)MS->replacementList;

    while(tmp)
    {
        prev = (ppoBASE)tmp->node.prev;

        gcmONERROR(ppoTOKEN_Destroy(PP, (ppoTOKEN_STREAM)tmp));

        tmp = prev;
    }

    status = cloCOMPILER_Free(PP->compiler, MS);
    return status;

OnError:
    return status;
}

gceSTATUS
ppoMACRO_MANAGER_Destroy(ppoPREPROCESSOR PP, ppoMACRO_MANAGER MACM)
{
    ppoBASE tmp,prev;

    tmp = (ppoBASE)MACM->ir;

    while(tmp)
    {
        prev = (ppoBASE)tmp->node.prev;

        ppoMACRO_SYMBOL_Destroy(PP, (ppoMACRO_SYMBOL)tmp);

        tmp = prev;
    }

    return cloCOMPILER_Free(PP->compiler, MACM);
}

/******************************************************************************\
MACRO SYMBOL IsEqual
\******************************************************************************/
gceSTATUS
ppoMACRO_SYMBOL_IsEqual(
    ppoPREPROCESSOR     PP,ppoMACRO_SYMBOL M1, ppoMACRO_SYMBOL M2, gctBOOL* Result)
{
    /*
    *Result =   gcvTRUE;
    *Result &=  M1->argc == M2->argc;
    *Result &=  M1->argv
    */
    return gcvSTATUS_OK;
}

/******************************************************************************\
MACRO SYMBOL Creat
\******************************************************************************/
gceSTATUS
ppoMACRO_SYMBOL_Construct(
    ppoPREPROCESSOR         PP,
    IN  gctCONST_STRING     File,
    IN  gctINT              Line,
    IN  gctCONST_STRING     MoreInfo,
    IN  gctSTRING           Name,
    IN  gctINT              Argc,
    IN  ppoTOKEN            Argv,
    IN  ppoTOKEN            Rplst,
    OUT ppoMACRO_SYMBOL*    Created
    )
{
    ppoMACRO_SYMBOL rt = gcvNULL;
    gceSTATUS status = gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
    gctPOINTER pointer = gcvNULL;

    gcmASSERT(PP && Line && MoreInfo && Name);

    gcmONERROR(
        cloCOMPILER_Allocate(
            PP->compiler,
            sizeof(struct _ppoMACRO_SYMBOL),
            &pointer
            )
        );

    gcoOS_ZeroMemory(pointer, (gctSIZE_T)gcmSIZEOF(struct _ppoMACRO_SYMBOL));

    rt = (ppoMACRO_SYMBOL)pointer;

    gcmONERROR(ppoBASE_Init(
        PP,
        (ppoBASE)rt,
        __FILE__,
        __LINE__,
        MoreInfo,
        ppvOBJ_MACRO_SYMBOL));

    rt->name            = Name;
    rt->argc            = Argc;
    rt->argv            = Argv;
    rt->replacementList = Rplst;
    rt->undefined       = gcvFALSE;
    rt->hasPara         = gcvFALSE;

    *Created = rt;

    return gcvSTATUS_OK;

OnError:
    if (rt != gcvNULL)
    {
        cloCOMPILER_Free(PP->compiler, rt);
    }
    return status;
}
/********************************   MACRO MANAGER Creat     ********************************************/
gceSTATUS
ppoMACRO_MANAGER_Construct(
    ppoPREPROCESSOR     PP,
    char *              File,
    gctINT              Line,
    char *              MoreInfo,
    ppoMACRO_MANAGER*   Created)
{
    ppoMACRO_MANAGER    rt = gcvNULL;
    gceSTATUS           status = gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
    gcmASSERT( PP && File && Line && MoreInfo && Created);

    gcmONERROR(
        cloCOMPILER_Allocate(
            PP->compiler,
            sizeof(struct _ppoMACRO_MANAGER),
            (void*)&rt
            )
        );

    gcoOS_MemFill(rt, 0, sizeof(struct _ppoMACRO_MANAGER));

    /*00 base*/
    gcmONERROR(ppoBASE_Init(
        PP,
        (void*)rt,
        File,
        Line,
        MoreInfo,
        ppvOBJ_MACRO_MANAGER));

    /*01 hMACMIR*/
    rt->ir = gcvNULL;

    *Created = rt;

    return gcvSTATUS_OK;

OnError:
    if (rt != gcvNULL)
    {
        gcmVERIFY_OK(cloCOMPILER_Free(PP->compiler, rt));
        rt = gcvNULL;
    }
    return status;
}

gceSTATUS
ppoMACRO_MANAGER_GetMacroSymbol (
    ppoPREPROCESSOR     PP,
    ppoMACRO_MANAGER    Macm,
    gctSTRING           Name,
    ppoMACRO_SYMBOL*    Found)
{
    ppoMACRO_SYMBOL rt = gcvNULL;

    gcmASSERT(Macm && Name && Found);

    rt = Macm->ir;

    while(rt)
    {
        if(rt->name == Name)
        {
            break;
        }
        rt = (ppoMACRO_SYMBOL)rt->base.node.prev;
    }

    *Found = rt;

    return gcvSTATUS_OK;
}


gceSTATUS
ppoMACRO_MANAGER_AddMacroSymbol(
                                ppoPREPROCESSOR     PP,
                                ppoMACRO_MANAGER    Macm,
                                ppoMACRO_SYMBOL     Ms)
{
    gcmASSERT(Macm && Ms);

    /*just relpace the old macro.*/
    if(Macm->ir == gcvNULL)
    {
        Macm->ir          = Ms;
        Ms->base.node.prev = gcvNULL;
        Ms->base.node.next = gcvNULL;
    }
    else
    {
        Ms->base.node.prev          = (void*)Macm->ir;
        Ms->base.node.next          = gcvNULL;
        Macm->ir->base.node.next    = (void*)Ms;
        Macm->ir                    = Ms;
    }
    return gcvSTATUS_OK;
}

gceSTATUS
ppoMACRO_MANAGER_DestroyMacroSymbol(
                                ppoPREPROCESSOR     PP,
                                ppoMACRO_MANAGER    Macm,
                                ppoMACRO_SYMBOL     Ms)
{
    ppoMACRO_SYMBOL ms;
    gcmASSERT(Macm && Ms);

    ppoMACRO_MANAGER_GetMacroSymbol(
        PP,
        Macm,
        Ms->name,
        &ms);

    if (ms != gcvNULL)
    {
        if (ms->base.node.prev != gcvNULL)
        {
            ms->base.node.prev->next = ms->base.node.next;
        }

        if (ms->base.node.next != gcvNULL)
        {
            ms->base.node.next->prev = ms->base.node.prev;
        }

        if (ms->name == Macm->ir->name)
        {
            Macm->ir = (ppoMACRO_SYMBOL)(ms->base.node.prev);
        }
        ppoMACRO_SYMBOL_Destroy(PP, Ms);
    }
    else
    {
        return gcvSTATUS_NOT_FOUND;
    }

    return gcvSTATUS_OK;
}
