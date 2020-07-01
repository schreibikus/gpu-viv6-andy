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


#include "gc_glsl_preprocessor_int.h"

gceSTATUS ppoPREPROCESSOR_SetDebug(
    IN sloCOMPILER      Compiler,
    IN gctUINT          On
    )
{
    gcmHEADER_ARG("Compiler=0x%x On=%u",
                  Compiler, On);

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS ppoPREPROCESSOR_SetOptimize(
    IN sloCOMPILER      Compiler,
    IN gctUINT          On
    )
{
    gcmHEADER_ARG("Compiler=0x%x On=%u",
                  Compiler, On);

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS ppoPREPROCESSOR_SetVersion(
    IN sloCOMPILER      Compiler,
    IN gctUINT          Version
    )
{
    gcmHEADER_ARG("Compiler=0x%x Version=%u",
                  Compiler, Version);

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}


/*******************************************************************************
**
**  ppoPREPROCESSOR_PushOntoCurrentInputStreamOfPP
**
*/
gceSTATUS
ppoPREPROCESSOR_PushOntoCurrentInputStreamOfPP(
    ppoPREPROCESSOR PP,
    ppoINPUT_STREAM IS
    )
{
    gcmHEADER_ARG("PP=0x%x IS=0x%x",
                  PP, IS);

    gcmASSERT(PP && IS);

    if (PP->inputStream)
    {
        IS->base.node.prev = (void*)PP->inputStream;

        PP->inputStream->base.node.next = (void*)IS;

        PP->inputStream = IS;

        IS->base.node.next = gcvNULL;
    }
    else
    {
        PP->inputStream = IS;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
ppoWriteBufferToFile(IN ppoPREPROCESSOR PP)
{
    gceSTATUS status = gcvSTATUS_OK;

    if (PP->ppLogFile == gcvNULL)
    {
#define _FILENAMEMAX 1024
        gctCHAR fullFileName[_FILENAMEMAX+1];
        gctCHAR fileName[64];
        gctUINT offset = 0;
        gctUINT64 time;

        gcmVERIFY_OK(vscGetTemporaryDir(fullFileName));
#if _WIN32
        gcmVERIFY_OK(gcoOS_StrCatSafe(fullFileName, _FILENAMEMAX, "\\"));
#else
        gcmVERIFY_OK(gcoOS_StrCatSafe(fullFileName, _FILENAMEMAX, "/"));
#endif
        gcoOS_GetTime(&time);
        gcmVERIFY_OK(gcoOS_PrintStrSafe(fileName, gcmSIZEOF(fileName), &offset, "viv_gl_%lld.log", (gctUINT64)time));
        gcmVERIFY_OK(gcoOS_StrCatSafe(fullFileName, _FILENAMEMAX, fileName));
        gcmVERIFY_OK(gcoOS_Open(gcvNULL, fullFileName, gcvFILE_APPENDTEXT, &PP->ppLogFile));
        if (PP->ppLogFile == gcvNULL)
        {
            gcoOS_ZeroMemory(PP->logBuffer, 1024);
            PP->logCurrentSize = 0;
            return gcvSTATUS_INVALID_DATA;
        }
    }

    gcoOS_Write(gcvNULL, PP->ppLogFile, gcoOS_StrLen(PP->logBuffer, gcvNULL), PP->logBuffer);
    gcoOS_ZeroMemory(PP->logBuffer, 1024);
    PP->logCurrentSize = 0;

    return status;
}

/*******************************************************************************
**
**  ppoPREPROCESSOR_AddToOutputStreamOfPP
**
*/
gceSTATUS
ppoPREPROCESSOR_AddToOutputStreamOfPP(
    ppoPREPROCESSOR PP,
    ppoTOKEN        Token
    )
{
    gceSTATUS status;
    ppoTOKEN ntoken = gcvNULL;
    gctSIZE_T len = 0;

    gcmHEADER_ARG("PP=0x%x Token=0x%x",
                  PP, Token);

    gcmASSERT(PP && Token);

    status = ppoTOKEN_Colon(PP,
                            Token,
                            __FILE__,
                            __LINE__,
                            "Dump for adding this token to the output of cpp.",
                            &ntoken);

    ppmCheckOK();

    ntoken->srcFileString = PP->currentSourceFileStringNumber;
    ntoken->srcFileLine = PP->currentSourceFileLineNumber;

    if ((PP->outputTokenStreamEnd == PP->outputTokenStreamHead) &&
        (PP->outputTokenStreamEnd == gcvNULL))
    {
        /*empty list*/
        PP->outputTokenStreamEnd = PP->outputTokenStreamHead = ntoken;
        ntoken->inputStream.base.node.prev = gcvNULL;
        ntoken->inputStream.base.node.next = gcvNULL;
    }
    else
    {
        /*not empty list, link the ntoken to the prev of the end one.*/
        ntoken->inputStream.base.node.next = (void*)PP->outputTokenStreamEnd;
        ntoken->inputStream.base.node.prev = gcvNULL;
        PP->outputTokenStreamEnd->inputStream.base.node.prev = (void*)ntoken;
        PP->outputTokenStreamEnd = ntoken;
    }

    if (gcmOPT_DUMP_PPEDSTR2FILE())
    {
        gcoOS_StrLen(ntoken->poolString, &len);
        if (((PP->logCurrentSize + len) > (_cldBUFFER_MAX - 5)))
        {
            ppoWriteBufferToFile(PP);
        }

        if (PP->ppLineNumber && PP->ppLineNumber < PP->currentSourceFileLineNumber)
        {
            gcoOS_StrCatSafe(PP->logBuffer, _cldBUFFER_MAX, "\n");
            if (ntoken->hasLeadingWS)
            {
                gcmVERIFY_OK(gcoOS_StrCatSafe(PP->logBuffer, _cldBUFFER_MAX, " "));
                PP->logCurrentSize = PP->logCurrentSize + 2;
            }

            gcmVERIFY_OK(gcoOS_StrCatSafe(PP->logBuffer,
                                          _cldBUFFER_MAX,
                                          ntoken->poolString));
            PP->logCurrentSize = PP->logCurrentSize + (gctUINT)len + 1;

            if (ntoken->hasTrailingControl)
            {
                gcmVERIFY_OK(gcoOS_StrCatSafe(PP->logBuffer, _cldBUFFER_MAX, " "));
                PP->logCurrentSize = PP->logCurrentSize + 2;
            }
        }
        else
        {
            if (ntoken->hasLeadingWS)
            {
                gcmVERIFY_OK(gcoOS_StrCatSafe(PP->logBuffer, _cldBUFFER_MAX, " "));
                PP->logCurrentSize = PP->logCurrentSize + 2;
            }

            status = gcoOS_StrCatSafe(PP->logBuffer,
                                      _cldBUFFER_MAX,
                                      ntoken->poolString);
            PP->logCurrentSize = PP->logCurrentSize + (gctUINT)len + 1;

            if (ntoken->hasTrailingControl)
            {
                gcmVERIFY_OK(gcoOS_StrCatSafe(PP->logBuffer, _cldBUFFER_MAX, " "));
                PP->logCurrentSize = PP->logCurrentSize + 2;
            }
        }
        PP->ppLineNumber = PP->currentSourceFileLineNumber;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}


/*******************************************************************************
**
**  ppoPREPROCESSOR_DumpOutputStream
**
*/
gceSTATUS
ppoPREPROCESSOR_DumpOutputStream(
    ppoPREPROCESSOR PP
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("PP=0x%x",
                  PP);

    gcmASSERT(PP && PP->base.type == ppvOBJ_PREPROCESSOR);

    if (PP->outputTokenStreamHead)
    {
        status = ppoTOKEN_STREAM_Dump(PP, PP->outputTokenStreamHead);

        gcmFOOTER();
        return status;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}


/*******************************************************************************
**
**  ppoPREPROCESSOR_Report
**
*/
gceSTATUS
ppoPREPROCESSOR_Report(
    ppoPREPROCESSOR PP,
    sleREPORT_TYPE  Type,
    gctCONST_STRING Message,
    ...
    )
{
    gceSTATUS    status;
    gctARGUMENTS arguments;

    gcmHEADER_ARG("PP=0x%x Type=%d Message=0x%x",
                  PP, Type, Message);

    gcmARGUMENTS_START(arguments, Message);
    status = sloCOMPILER_VReport(
        PP->compiler,
        PP->currentSourceFileLineNumber,
        PP->currentSourceFileStringNumber,
        Type,
        Message,
        arguments);
    gcmARGUMENTS_END(arguments);

    gcmFOOTER();
    return status;
}

/*
x * x ...  x * x
----------------
total = y
*/
gctINT ppoPREPROCESSOR_Pow(
    gctINT x,
    gctINT y
    )
{
    gctINT i;
    gctINT rt   = 1;

    gcmHEADER_ARG("x=%d y=%d",
                  x, y);

    gcmASSERT( x >= 0 && y >= 0 );

    for ( i=1; i<=y; i++ )
    {
        rt = rt * x;
    }

    gcmFOOTER_ARG("rt=0x%x", rt);
    return rt;
}

gceSTATUS
ppoPREPROCESSOR_Construct_InitKeyword(
    sloCOMPILER     Compiler,
    ppoPREPROCESSOR *PP);

gceSTATUS
ppoPREPROCESSOR_Construct_InitOperator(
    sloCOMPILER     Compiler,
    ppoPREPROCESSOR *PP);

static gceSTATUS _ConstructExtensionString(
    sloCOMPILER     Compiler,
    ppoPREPROCESSOR *PP
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctPOINTER pointer = gcvNULL;
    gctCONST_STRING extraExtension = "GL_KHR_blend_equation_advanced GL_OES_sample_variables GL_OES_shader_image_atomic "
                                     "GL_OES_shader_multisample_interpolation GL_OES_texture_storage_multisample_2d_array "
                                     "GL_EXT_geometry_shader GL_EXT_gpu_shader5 GL_EXT_primitive_bounding_box GL_EXT_shader_io_blocks "
                                     "GL_EXT_tessellation_shader GL_EXT_texture_buffer GL_EXT_texture_cube_map_array GL_ARB_tessellation_shader ";
    gctSIZE_T extraExtensionLength = gcoOS_StrLen(extraExtension, gcvNULL);
    gctSIZE_T extensionStringLength = gcoOS_StrLen(GetGLExtensionString(), gcvNULL);
    gctSIZE_T length = 0;
    gctBOOL catExtraExtension = gcvFALSE;

    if (gcoOS_StrStr(GetGLExtensionString(), "GL_ANDROID_extension_pack_es31a", gcvNULL))
    {
        length = extraExtensionLength + extensionStringLength + 1;
        catExtraExtension = gcvTRUE;
    }
    else
    {
        length = extensionStringLength + 1;
    }

    gcmONERROR(sloCOMPILER_Allocate(Compiler, length, &pointer));
    (*PP)->extensionString = pointer;

    gcoOS_StrCopySafe((*PP)->extensionString, extensionStringLength + 1, GetGLExtensionString());
    if (catExtraExtension)
    {
        gcoOS_StrCatSafe((*PP)->extensionString, length, extraExtension);
    }

OnError:
    return status;
}

/*******************************************************************************
**
**  ppoPREPROCESSOR_Construct
**
**      Allocate memory, set undirty.
**
*/
gceSTATUS
ppoPREPROCESSOR_Construct(
    sloCOMPILER     Compiler,
    ppoPREPROCESSOR *PP
    )
{
    gceSTATUS status;
    gctPOINTER pointer = gcvNULL;

    gcmHEADER_ARG("Compiler=0x%x PP=0x%x",
                  Compiler, PP);

    /* Preprocessor. */
    gcmONERROR(
        sloCOMPILER_Allocate(
            Compiler,
            sizeof(struct _ppoPREPROCESSOR),
            &pointer
            ));

    *PP = pointer;

    gcoOS_MemFill(
        *PP,
        0,/*set value*/
        sizeof(struct _ppoPREPROCESSOR)
        );

    (*PP)->base.file = __FILE__;
    (*PP)->base.info = "Created in ppoPREPROCESSOR_Construct";
    (*PP)->base.line = __LINE__;
    (*PP)->base.node.next = gcvNULL;
    (*PP)->base.node.prev = gcvNULL;
    (*PP)->base.type = ppvOBJ_PREPROCESSOR;

    (*PP)->skipOPError = gcvFALSE;

    (*PP)->compiler = Compiler;

    /* Construct extension string. */
    _ConstructExtensionString(Compiler, PP);

    /* Keywords. */

    gcmONERROR(
        sloCOMPILER_Allocate(
            Compiler,
            sizeof (struct _ppsKEYWORD),
            &pointer
            ));

    (*PP)->keyword = pointer;

    gcmONERROR(
        ppoPREPROCESSOR_Construct_InitKeyword(Compiler, PP));

    /* Operators. */
    gcmONERROR(
        sloCOMPILER_Allocate(
            (*PP)->compiler,
            sizeof(gctSTRING*)*12,
            &pointer
            ) );
    (*PP)->operators = pointer;

    gcoOS_MemFill(
        (*PP)->operators,
        (gctUINT8)0,
        sizeof(gctSTRING*)*12
        );

    gcmONERROR(
        ppoPREPROCESSOR_Construct_InitOperator(Compiler, PP));

        gcmFOOTER_ARG("*PP=%d", *PP);
        return gcvSTATUS_OK;

OnError:
    gcmVERIFY_OK(sloCOMPILER_Report(
            Compiler,
            1,
            0,
            slvREPORT_FATAL_ERROR,
            "Failed to start preprocessing."));

    gcmFOOTER();
    return status;
}

gceSTATUS
ppoPREPROCESSOR_Construct_InitKeyword(
    sloCOMPILER     Compiler,
    ppoPREPROCESSOR *PP
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Compiler=0x%x PP=0x%x",
                  Compiler, PP);

    /*00    #*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "#",
                                       &((*PP)->keyword->sharp)
                                       ));

    /*01    define*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "define",
                                       &((*PP)->keyword->define)
                                       ));

    /*02    undef*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "undef",
                                       &((*PP)->keyword->undef)));

    /*03    if*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "if",
                                       &((*PP)->keyword->if_)));

    /*04    ifdef*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "ifdef",
                                       &((*PP)->keyword->ifdef)));

    /*05    ifndef*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "ifndef",
                                       &((*PP)->keyword->ifndef)));

    /*06    else*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "else",
                                       &((*PP)->keyword->else_)));

    /*07    elif*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "elif",
                                       &((*PP)->keyword->elif)));

    /*08    endif*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "endif",
                                       &((*PP)->keyword->endif)));

    /*09    error*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "error",
                                       &((*PP)->keyword->error)));

    /*10    pragma*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "pragma",
                                       &((*PP)->keyword->pragma)));

    /*11    extension*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "extension",
                                       &((*PP)->keyword->extension)));

    /*12    version*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "version",
                                       &((*PP)->keyword->version)));

    /*13    line*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "line",
                                       &((*PP)->keyword->line)));

    /*14    lpara*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "(",
                                       &((*PP)->keyword->lpara)));

    /*15    rpara*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       ")",
                                       &((*PP)->keyword->rpara)));

    /*16    newline*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "\n",
                                       &((*PP)->keyword->newline )));

    /*17    defined*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "defined",
                                       &((*PP)->keyword->defined)));

    /*18    minus*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "-",
                                       &((*PP)->keyword->minus)));

    /*19    plus*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "+",
                                       &((*PP)->keyword->plus)));

    /*20    ||*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "||",
                                       &((*PP)->keyword->lor)));

    /*21    &&*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "&&",
                                       &((*PP)->keyword->land)));

    /*22    |*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "|",
                                       &((*PP)->keyword->bor)));

    /*23    &*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "&",
                                       &((*PP)->keyword->band)));

    /*24    ==*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "==",
                                       &((*PP)->keyword->equal)));

    /*25    !=*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "!=",
                                       &((*PP)->keyword->not_equal )));

    /*26    >*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       ">",
                                       &((*PP)->keyword->more)));

    /*27    <*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "<",
                                       &((*PP)->keyword->less)));

    /*28    >=*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       ">=",
                                       &((*PP)->keyword->more_equal)));

    /*29    <=*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "<=",
                                       &((*PP)->keyword->less_equal)));

    /*30    <<*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "<<",
                                       &((*PP)->keyword->lshift)));

    /*31    >>*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       ">>",
                                       &((*PP)->keyword->rshift)));

    /*32    **/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "*",
                                       &((*PP)->keyword->mul)));

    /*33    / */
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "/",
                                       &((*PP)->keyword->div)));

    /*34    %*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "%",
                                       &((*PP)->keyword->perc)));

    /*35    +*/
    (*PP)->keyword->positive = (*PP)->keyword->plus;

    /*36    -*/
    (*PP)->keyword->negative = (*PP)->keyword->minus;

    /*37    ~*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "~",
                                       &((*PP)->keyword->banti )));

    /*38    !*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "!",
                                       &((*PP)->keyword->lanti)));

    /*39    ^*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "^",
                                       &((*PP)->keyword->bex)));

    /*40    EOF*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "<EOF>",
                                       &((*PP)->keyword->eof)));

    /*41    WS */
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "<WS>",
                                       &((*PP)->keyword->ws)));

    /*42    , */
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       ",",
                                       &((*PP)->keyword->comma)));

    /*43    version */
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "100",
                                       &((*PP)->keyword->version_100)));

    /*44    :   */
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       ":",
                                       &((*PP)->keyword->colon)));

    /*45    require */
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "require",
                                       &((*PP)->keyword->require)));

    /*46    enable  */
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "enable",
                                       &((*PP)->keyword->enable)));

    /*47    warn    */
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "warn",
                                       &((*PP)->keyword->warn)));

    /*48    disable */
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "disable",
                                       &((*PP)->keyword->disable)));

    /*49    __LINE__*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "__LINE__",
                                       &((*PP)->keyword->_line_)));

    /*50    __FILE__*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "__FILE__",
                                       &((*PP)->keyword->_file_)));

    /*51    __VERSION__*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "__VERSION__",
                                       &((*PP)->keyword->_version_)));

    /*52    GL_ES*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "GL_ES",
                                       &((*PP)->keyword->gl_es)));

    /*53    GL_*/
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "GL_",
                                       &((*PP)->keyword->gl_)));

    /*54    all     */
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "all",
                                       &((*PP)->keyword->all)));

    /*55    STDGL     */
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "STDGL",
                                       &((*PP)->keyword->STDGL)));

    /*56    debug     */
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "debug",
                                       &((*PP)->keyword->debug)));

    /*57    optimize  */
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "optimize",
                                       &((*PP)->keyword->optimize)));

    /*58    nul_str     */
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "",
                                       &((*PP)->keyword->nul_str)));

    /*59    gl_core_profile     */
    gcmONERROR(
        sloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "GL_core_profile",
                                       &((*PP)->keyword->gl_core_profile)));

    (*PP)->keyword->isVersionUndefined = gcvFALSE;
    gcmFOOTER_ARG("*PP=%d", *PP);
    return gcvSTATUS_OK;

OnError:
    gcmVERIFY_OK(sloCOMPILER_Report(
        Compiler,
        1,
        0,
        slvREPORT_FATAL_ERROR,
        "Failed to start preprocessing."));

    gcmFOOTER();
    return status;
}

gceSTATUS
ppoPREPROCESSOR_Construct_InitOperator(
    sloCOMPILER     Compiler,
    ppoPREPROCESSOR *PP
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Compiler=0x%x PP=0x%x",
                  Compiler, PP);

    /*(*PP)->operators 00:  Total: 3, Detail: 2, ||, 0*/
    gcmONERROR(
        sloCOMPILER_Allocate((*PP)->compiler,
                             sizeof(gctSTRING)*3,
                             (gctPOINTER*)&((*PP)->operators[0])));

    (*PP)->operators[0][0] = gcmINT2PTR(ppvBINARY_OP);
    (*PP)->operators[0][1] = (*PP)->keyword->lor;
    (*PP)->operators[0][2] = gcvNULL;

    /*(*PP)->operators 01:  Total: 3, Detail: 2, &&, 0*/

    gcmONERROR(
        sloCOMPILER_Allocate((*PP)->compiler,
                             sizeof(gctSTRING)*3,
                             (gctPOINTER*)&((*PP)->operators[1])));

    (*PP)->operators[1][0] = gcmINT2PTR(ppvBINARY_OP);
    (*PP)->operators[1][1] = (*PP)->keyword->land;
    (*PP)->operators[1][2] = gcvNULL;

    /*(*PP)->operators 02:  Total: 3, Detail: 2, |, 0*/
    gcmONERROR(
        sloCOMPILER_Allocate((*PP)->compiler,
                             sizeof(gctSTRING)*3,
                             (gctPOINTER*)&((*PP)->operators[2])));

    (*PP)->operators[2][0] = gcmINT2PTR(ppvBINARY_OP);
    (*PP)->operators[2][1] = (*PP)->keyword->bor;
    (*PP)->operators[2][2] = gcvNULL;

    /*(*PP)->operators 03:  Total: 3, Detail: 2, ^, 0*/
    gcmONERROR(
        sloCOMPILER_Allocate((*PP)->compiler,
                             sizeof(gctSTRING)*3,
                             (gctPOINTER*)&((*PP)->operators[3])));

    (*PP)->operators[3][0] = gcmINT2PTR(ppvBINARY_OP);
    (*PP)->operators[3][1] = (*PP)->keyword->bex;
    (*PP)->operators[3][2] = gcvNULL;

    /*(*PP)->operators 04:  Total: 3, Detail: 2, &, 0*/
    gcmONERROR(
        sloCOMPILER_Allocate((*PP)->compiler,
                             sizeof(gctSTRING)*3,
                             (gctPOINTER*)&((*PP)->operators[4])));

    (*PP)->operators[4][0] = gcmINT2PTR(ppvBINARY_OP);
    (*PP)->operators[4][1] = (*PP)->keyword->band;
    (*PP)->operators[4][2] = gcvNULL;

    /*(*PP)->operators 05:  Total: 4, Detail: 2, ==, !=, 0*/
    gcmONERROR(
        sloCOMPILER_Allocate((*PP)->compiler,
                             sizeof(gctSTRING)*4,
                             (gctPOINTER*)&((*PP)->operators[5])));

    (*PP)->operators[5][0] = gcmINT2PTR(ppvBINARY_OP);
    (*PP)->operators[5][1] = (*PP)->keyword->equal;
    (*PP)->operators[5][2] = (*PP)->keyword->not_equal;
    (*PP)->operators[5][3] = gcvNULL;

    /*(*PP)->operators 06:  Total: 6, Detail: 2, >, <, >=, <=, 0*/
    gcmONERROR(
        sloCOMPILER_Allocate((*PP)->compiler,
                             sizeof(gctSTRING)*6,
                             (gctPOINTER*)&((*PP)->operators[6])));

    (*PP)->operators[6][0] = gcmINT2PTR(ppvBINARY_OP);
    (*PP)->operators[6][1] = (*PP)->keyword->less;
    (*PP)->operators[6][2] = (*PP)->keyword->more;
    (*PP)->operators[6][3] = (*PP)->keyword->less_equal;
    (*PP)->operators[6][4] = (*PP)->keyword->more_equal;
    (*PP)->operators[6][5] = gcvNULL;

    /*(*PP)->operators 07:  Total: 4, Detail: 2, <<, >>, 0*/
    gcmONERROR(
        sloCOMPILER_Allocate((*PP)->compiler,
                             sizeof(gctSTRING)*4,
                             (gctPOINTER*)&((*PP)->operators[7])));

    (*PP)->operators[7][0] = gcmINT2PTR(ppvBINARY_OP);
    (*PP)->operators[7][1] = (*PP)->keyword->lshift;
    (*PP)->operators[7][2] = (*PP)->keyword->rshift;
    (*PP)->operators[7][3] = gcvNULL;

    /*(*PP)->operators 08:  Total: 4, Detail: 2, +, -, 0*/
    gcmONERROR(
        sloCOMPILER_Allocate((*PP)->compiler,
                             sizeof(gctSTRING)*4,
                             (gctPOINTER*)&((*PP)->operators[8])));

    (*PP)->operators[8][0] = gcmINT2PTR(ppvBINARY_OP);
    (*PP)->operators[8][1] = (*PP)->keyword->plus;
    (*PP)->operators[8][2] = (*PP)->keyword->minus;
    (*PP)->operators[8][3] = gcvNULL;

    /*(*PP)->operators 09:  Total: 5, Detail: 2, *, /, %, 0*/
    gcmONERROR(
        sloCOMPILER_Allocate((*PP)->compiler,
                             sizeof(gctSTRING)*5,
                             (gctPOINTER*)&((*PP)->operators[9])));

    (*PP)->operators[9][0] = gcmINT2PTR(ppvBINARY_OP);
    (*PP)->operators[9][1] = (*PP)->keyword->mul;
    (*PP)->operators[9][2] = (*PP)->keyword->div;
    (*PP)->operators[9][3] = (*PP)->keyword->perc;
    (*PP)->operators[9][4] = gcvNULL;

    /*(*PP)->operators 10:  Total: 8, Detail: 1, +, -, ~, !, defined, 0*/
    gcmONERROR(
        sloCOMPILER_Allocate((*PP)->compiler,
                             sizeof(gctSTRING)*8,
                             (gctPOINTER*)&((*PP)->operators[10])));

    (*PP)->operators[10][0] = gcmINT2PTR(ppvUNARY_OP);
    (*PP)->operators[10][1] = (*PP)->keyword->plus;
    (*PP)->operators[10][2] = (*PP)->keyword->minus;
    (*PP)->operators[10][3] = (*PP)->keyword->div;
    (*PP)->operators[10][4] = (*PP)->keyword->banti;
    (*PP)->operators[10][5] = (*PP)->keyword->lanti;
    (*PP)->operators[10][6] = (*PP)->keyword->defined;
    (*PP)->operators[10][7] = gcvNULL;

    /*(*PP)->operators 11:  Total: 0. This inputStream just a guarder.*/
    (*PP)->operators[11] = gcvNULL;

    gcmFOOTER_ARG("*PP=%d", *PP);
    return gcvSTATUS_OK;

OnError:
    gcmVERIFY_OK(sloCOMPILER_Report(
        Compiler,
        1,
        0,
        slvREPORT_FATAL_ERROR,
        "Failed to start preprocessing."));

    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  ppoPREPROCESSOR_Destroy
**
**      Release memory used by internal elements of *PP*, and *PP*.
**
*/
gceSTATUS
ppoPREPROCESSOR_Destroy(
    ppoPREPROCESSOR PP
    )
{
    gceSTATUS status;
    sloCOMPILER compiler = gcvNULL;
    gctUINT i;

    gcmHEADER_ARG("PP=0x%x",
                  PP);

    gcmASSERT(PP->base.type == ppvOBJ_PREPROCESSOR);

    compiler = PP->compiler;

    gcmONERROR(ppoPREPROCESSOR_Reset(PP));

    if (PP->extensionString)
    {
        gcmONERROR(sloCOMPILER_Free(PP->compiler, (gctPOINTER)PP->extensionString));
        PP->extensionString = gcvNULL;
    }

    /*release operators of every level*/
    i = 0;
    while (PP->operators[i] != gcvNULL)
    {
        gcmONERROR(sloCOMPILER_Free(
            compiler,
            PP->operators[i]
            ));

        i++;
    }

    gcmONERROR(sloCOMPILER_Free(
            compiler,
            PP->operators
            ));

    /*total number of group of operator, is 12.*/
    gcmASSERT(i == 11);

    /*release output stream*/
    gcmONERROR(ppoTOKEN_STREAM_Destroy(
        PP,
        PP->outputTokenStreamHead
        ));

    /*release keyword*/
    gcmONERROR(sloCOMPILER_Free(
        compiler,
        PP->keyword
        ));

    /*PP*/
    gcmONERROR(sloCOMPILER_Free(
        compiler,
        PP));

    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmVERIFY_OK(sloCOMPILER_Report(
        compiler,
        0,
        0,
        slvREPORT_INTERNAL_ERROR,
        "Error in destroy preprocessor."
        ));

    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  ppoPREPROCESSOR_Dump
**
**      Dump.
**
*/
gceSTATUS
ppoPREPROCESSOR_Dump(
    ppoPREPROCESSOR PP
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("PP=0x%x",
                  PP);

    gcmASSERT(PP && PP->base.type == ppvOBJ_PREPROCESSOR);

    gcmONERROR(sloCOMPILER_Dump(
        PP->compiler,
        slvDUMP_PREPROCESSOR,
        "<PP>"
        ));

    gcmONERROR(ppoBASE_Dump(
        PP,
        (ppoBASE)PP
        ));

    gcmONERROR(ppoMACRO_MANAGER_Dump(
        PP,
        PP->macroManager
        ));

    gcmONERROR(sloCOMPILER_Dump(
        PP->compiler,
        slvDUMP_PREPROCESSOR,
        "<TheInputStack>"
        ));

    if(PP->inputStream)
    {
        gcmONERROR(ppoINPUT_STREAM_Dump(
            PP,
            PP->inputStream
            ));
    }

    gcmONERROR(sloCOMPILER_Dump(
        PP->compiler,
        slvDUMP_PREPROCESSOR,
        "</TheInputStack>"
        ));

    gcmONERROR(sloCOMPILER_Dump(
        PP->compiler,
        slvDUMP_PREPROCESSOR,
        "<LastTokenString no=\"%d\" />",PP->currentSourceFileStringNumber
        ));

    gcmONERROR(sloCOMPILER_Dump(
        PP->compiler,
        slvDUMP_PREPROCESSOR,
        "<LastTokenLine no=\"%d\" />",
        PP->currentSourceFileLineNumber
        ));

    gcmONERROR(sloCOMPILER_Dump(
        PP->compiler,
        slvDUMP_PREPROCESSOR,
        "<OutputList>"
        ));

    if(PP->outputTokenStreamHead)
    {
        gcmONERROR(ppoTOKEN_STREAM_Dump(
            PP,
            PP->outputTokenStreamHead
            ));
    }

    gcmONERROR(sloCOMPILER_Dump(
        PP->compiler,
        slvDUMP_PREPROCESSOR,
        "<OutputList>"
        ));

    gcmONERROR(sloCOMPILER_Dump(
        PP->compiler,
        slvDUMP_PREPROCESSOR,
        "<Version version=\"%d\" />",
        PP->version
        ));

    gcmONERROR(sloCOMPILER_Dump(
        PP->compiler,
        slvDUMP_PREPROCESSOR,
        "<Legal doWeInValidArea=\"%d\" />", PP->doWeInValidArea
        ));

    gcmONERROR(sloCOMPILER_Dump(
        PP->compiler,
        slvDUMP_PREPROCESSOR,
        "<HasAnyStatementOtherThanVersionStatementHaveAppeared appeared=\"%d\" />",
        PP->otherStatementHasAlreadyAppeared
        ));

    gcmONERROR(sloCOMPILER_Dump(
        PP->compiler,
        slvDUMP_PREPROCESSOR,
        "<HasVersionStatementHaveAppeared appeared=\"%d\" />",
        PP->versionStatementHasAlreadyAppeared
        ));

    gcmONERROR(sloCOMPILER_Dump(
        PP->compiler,
        slvDUMP_PREPROCESSOR,
        "<NonPreprocessorStatementHasAlreadyAppeared appeared=\"%d\" />",
        PP->nonpreprocessorStatementHasAlreadyAppeared
        ));

    gcmONERROR(sloCOMPILER_Dump(
        PP->compiler,
        slvDUMP_PREPROCESSOR,
        "</PP>"
        ));

    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmVERIFY_OK(sloCOMPILER_Report(
            PP->compiler,
            0,
            0,
            slvREPORT_FATAL_ERROR,
            "Error in dumping preprocessor."
            ));

    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  ppoPREPROCESSOR_Parse
**
**      Parse the strings just set.
**
**
*/
gceSTATUS
ppoPREPROCESSOR_Parse(
    sloPREPROCESSOR     PP,
    char                *Buffer,
    gctUINT             Max,
    gctINT              *WriteInNumber
    )
{
    gceSTATUS           status;
    ppoTOKEN            ntoken  = gcvNULL;
    gctUINT             len     = 0;

    gcmHEADER_ARG("PP=0x%x Buffer=0x%x Max=0x%x WriteInNumber=0x%x",
                  PP, Buffer, Max, WriteInNumber);

    /*legal argument?*/
    gcmASSERT(PP && PP->base.type == ppvOBJ_PREPROCESSOR);

    *WriteInNumber = 0;

    /*end of input?*/
    if (PP->inputStream == gcvNULL)
    {
        /* flush the remaining log buffer to file and close the file */
        if (gcmOPT_DUMP_PPEDSTR2FILE())
        {
            if (PP->logBuffer[0] != '\0')
                ppoWriteBufferToFile(PP);
            if (PP->ppLogFile)
            {
                gcoOS_Close(gcvNULL, PP->ppLogFile);
                PP->ppLogFile = gcvNULL;
            }
        }
        gcmFOOTER_ARG("*WriteInNumber=%d", *WriteInNumber);
        return gcvSTATUS_OK;
    }

    if (PP->outputTokenStreamHead == gcvNULL)
    {
        gcmONERROR(PP->inputStream->GetToken(
            PP,
            &(PP->inputStream),
            &ntoken,
            !ppvICareWhiteSpace));

        if (ntoken->type == ppvTokenType_EOF)
        {
            /* flush the remaining log buffer to file and close the file */
            if (gcmOPT_DUMP_PPEDSTR2FILE())
            {
                if (PP->logBuffer[0] != '\0')
                    ppoWriteBufferToFile(PP);
                if (PP->ppLogFile)
                {
                    gcoOS_Close(gcvNULL, PP->ppLogFile);
                    PP->ppLogFile = gcvNULL;
                }
            }
            /* End of File. */
            gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
            gcmFOOTER_ARG("*WriteInNumber=%d", gcmOPT_VALUE(WriteInNumber) );
            return status;
        }

        gcmONERROR(ppoINPUT_STREAM_UnGetToken(
            PP,
            &(PP->inputStream),
            ntoken));

        gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));

        gcmONERROR(ppoPREPROCESSOR_PreprocessingFile(PP));

        if (PP->outputTokenStreamHead == gcvNULL)
        {
            gcmFOOTER_ARG("*WriteInNumber=%d", *WriteInNumber);

            return gcvSTATUS_OK;
        }
        else
        {
            /* Load the built-ins */
            gcmONERROR(sloCOMPILER_LoadBuiltIns(PP->compiler));
        }
    }

    gcmASSERT( PP->outputTokenStreamHead != gcvNULL );

    /*copy a string to *Buffer*.*/
    len = (gctUINT) gcoOS_StrLen(PP->outputTokenStreamHead->poolString, gcvNULL);

    len += PP->outputTokenStreamHead->hasLeadingWS ? 1 : 0;
    len += PP->outputTokenStreamHead->hasTrailingControl ? 1 : 0;
    if (len >= Max)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(
            PP->compiler,
            PP->outputTokenStreamHead->srcFileLine,
            PP->outputTokenStreamHead->srcFileString,
            slvREPORT_ERROR,
            "The token is too long for compiler : %s,"
            "max length : %u",
            PP->outputTokenStreamHead->poolString,
            Max));

        gcmONERROR(gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR);
    }
    else
    {
        *WriteInNumber = (gctINT) len;
    }

    if (PP->outputTokenStreamHead->hasLeadingWS)
    {
       gcmVERIFY_OK(gcoOS_StrCopySafe(Buffer, Max, " "));
       gcmONERROR(gcoOS_StrCatSafe(Buffer,
                                   Max - 1,
                                   PP->outputTokenStreamHead->poolString));
    }
    else
    {
       gcmONERROR(gcoOS_StrCopySafe(Buffer,
                                    Max,
                                    PP->outputTokenStreamHead->poolString));
    }

    if (PP->outputTokenStreamHead->hasTrailingControl)
    {
       gcmVERIFY_OK(gcoOS_StrCatSafe(Buffer, Max, " "));
    }

    /*set position*/
    gcmONERROR(sloCOMPILER_SetCurrentStringNo(
        PP->compiler,
        (gctUINT)PP->outputTokenStreamHead->srcFileString
        ));

    gcmONERROR(sloCOMPILER_SetCurrentLineNo(
        PP->compiler,
        PP->outputTokenStreamHead->srcFileLine
        ));

    /*remove head*/
    if (PP->outputTokenStreamHead == PP->outputTokenStreamEnd)
    {
        gcmONERROR(ppoTOKEN_Destroy(
            PP,
            PP->outputTokenStreamHead
            ));

        PP->outputTokenStreamHead = PP->outputTokenStreamEnd = gcvNULL;
    }
    else
    {
        ppoTOKEN prev = (void*)(PP->outputTokenStreamHead->inputStream.base.node.prev);

        gcmONERROR(ppoTOKEN_Destroy(
            PP,
            PP->outputTokenStreamHead
            ));

        PP->outputTokenStreamHead = prev;
    }

    gcmFOOTER_ARG("*WriteInNumber=%d", *WriteInNumber);
    return gcvSTATUS_OK;

OnError:
    gcmVERIFY_OK(sloCOMPILER_Report(
        PP->compiler,
        PP->currentSourceFileLineNumber,
        PP->currentSourceFileStringNumber,
        slvREPORT_ERROR,
        "Error in parsing."));

    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  ppoPREPROCESSOR_Reset
**
**      1. reset states.
**
*/

gceSTATUS
ppoPREPROCESSOR_Reset(
    ppoPREPROCESSOR PP
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("PP=0x%x",
                  PP);

    if (PP == gcvNULL)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

    if (PP->strings)
    {
        gcmONERROR(sloCOMPILER_Free(PP->compiler, (gctPOINTER)PP->strings));
        PP->strings = gcvNULL;
    }

    /*lens*/
    if (PP->lens)
    {
        gcmONERROR(sloCOMPILER_Free(PP->compiler, PP->lens));
        PP->lens = gcvNULL;
    }

    /*macro manager*/
    if (PP->macroManager)
    {
        gcmONERROR(ppoMACRO_MANAGER_Destroy(PP, PP->macroManager));
        PP->macroManager = gcvNULL;
    }

    /*inputstream*/
    while (PP->inputStream)
    {
        ppoINPUT_STREAM tmp;
        tmp = PP->inputStream;
        PP->inputStream = (ppoINPUT_STREAM)PP->inputStream->base.node.prev;

        gcmONERROR(sloCOMPILER_Free(PP->compiler, tmp));
    }

    /*release output stream*/
    if(PP->outputTokenStreamHead)
    {
        gcmONERROR(ppoTOKEN_STREAM_Destroy(PP, PP->outputTokenStreamHead));
        PP->outputTokenStreamHead = gcvNULL;
    }

    PP->outputTokenStreamEnd = gcvNULL;

    /*reset debug mode*/
    gcmONERROR(sloCOMPILER_SetDebug(PP->compiler, gcvFALSE));

    /*reset optimize mode*/
    gcmONERROR(sloCOMPILER_SetOptimize(PP->compiler, gcvTRUE));

    /*reset invariant mode*/
    gcmONERROR(sloCOMPILER_SetOutputInvariant(PP->compiler, gcvFALSE));

    /*reset compiler version*/
    PP->version = 100;

    /*reset states for version parsing.*/
    PP->otherStatementHasAlreadyAppeared = gcvFALSE;
    PP->versionStatementHasAlreadyAppeared = gcvFALSE;
    PP->nonpreprocessorStatementHasAlreadyAppeared = gcvFALSE;

    /*reset the file string number and the file line number*/
    PP->currentSourceFileStringNumber = 0;
    PP->currentSourceFileLineNumber = 1;

    /*reset the state maintained to restore chars read from input stream*/
    PP->lastGetcharPhase0IsFromThisBis = gcvNULL;

    /*reset the state to handle the line-continue following a comment.*/
    PP->iAmFollowingAComment = gcvFALSE;

    /*reset the state maintianed to parse the #if etc.*/
    PP->doWeInValidArea = gcvTRUE;

    PP->toLineEnd = gcvFALSE;

    PP->skipLine = -1;

    PP->skipOPError = gcvFALSE;

    /*apiState*/

    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmVERIFY_OK(sloCOMPILER_Report(
        PP->compiler,
        1,
        0,
        slvREPORT_INTERNAL_ERROR,
        "Failed in resetting."));

    gcmFOOTER();
    return status;
}

gctCONST_STRING vivImgRdWrNeedSWBordercolor(void)
{
    static int initialized = 0;
    static gctCONST_STRING needSWBordercolor = gcvNULL;
    if (initialized == 0)
    {
        needSWBordercolor = GetHWHasImageOutBoundaryFix() ? "0" : "1"; /* !!! DEBUG !!! */
        initialized = 1;
    }
    return needSWBordercolor;
}

typedef gctCONST_STRING (*GETVALUE_FPTR)(void);

typedef struct _slsPREDEFINED_MACRO
{
   gctSTRING        str;
   gctSTRING        rplStr;
   GETVALUE_FPTR    fptr;     /* function pointer to get the macro value string dynamically */
   gctBOOL          checkFeature; /* If it is FALSE, then it is always enabled. */
}
slsPREDEFINED_MACRO;

static slsPREDEFINED_MACRO _PredefinedMacros[] =
{
   {"GL_ES", "1", gcvNULL, gcvFALSE},
   {"GL_EXT_texture_array", gcvNULL, gcvNULL, gcvFALSE},
   {"GL_OES_texture_3D", gcvNULL, gcvNULL, gcvFALSE},
   {"GL_EXT_frag_depth", gcvNULL, gcvNULL, gcvFALSE},
   {"GL_OES_EGL_image_external", gcvNULL, gcvNULL, gcvFALSE},
   {"GL_OES_EGL_image_external_essl3", gcvNULL, gcvNULL, gcvFALSE},
   {"GL_FRAGMENT_PRECISION_HIGH", "1", gcvNULL, gcvFALSE},
   {"GL_EXT_shadow_samplers", gcvNULL, gcvNULL, gcvFALSE},
   {"GL_VIV_asm", gcvNULL , gcvNULL, gcvFALSE},
   {"_VIV_IMG_RD_WR_NEED_SW_BORDERCOLOR", gcvNULL, vivImgRdWrNeedSWBordercolor, gcvFALSE},

   /* The extensions below need to check if they are enabled. */
   {"GL_OES_standard_derivatives", gcvNULL, gcvNULL, gcvTRUE},
   {"GL_KHR_blend_equation_advanced", "1", gcvNULL, gcvTRUE},
   {"GL_OES_texture_storage_multisample_2d_array", "1", gcvNULL, gcvTRUE},
   {"GL_OES_sample_variables", "1", gcvNULL, gcvTRUE},
   {"GL_OES_shader_multisample_interpolation", "1", gcvNULL, gcvTRUE},
   {"GL_EXT_gpu_shader5", "1", gcvNULL, gcvTRUE},
   {"GL_EXT_texture_cube_map_array", "1", gcvNULL, gcvTRUE},
   {"GL_ARB_explicit_attrib_location", "1", gcvNULL, gcvTRUE},
   {"GL_ARB_tessellation_shader", "1", gcvNULL, gcvTRUE},
   {"GL_ARB_uniform_buffer_object", "1", gcvNULL, gcvTRUE},
   /* TS extension. */
   {"GL_EXT_tessellation_shader", "1", gcvNULL, gcvTRUE},
   {"GL_EXT_tessellation_point_size", "1", gcvNULL, gcvTRUE},
   /* GS extension. */
   {"GL_EXT_geometry_shader", "1", gcvNULL, gcvTRUE},
   {"GL_EXT_geometry_point_size", "1", gcvNULL, gcvTRUE},
   /* IO block extension. */
   {"GL_EXT_shader_io_blocks", "1", gcvNULL, gcvTRUE},
   /* shader implicit conversions extension. */
   {"GL_EXT_shader_implicit_conversions", "1", gcvNULL, gcvTRUE},
   /* texture buffer extension. */
   {"GL_EXT_texture_buffer", "1", gcvNULL, gcvTRUE},
   /* primitive bounding box extension. */
   {"GL_EXT_primitive_bounding_box", "1", gcvNULL, gcvTRUE},
   /* ANDROID_extension_pack_es31a extension*/
   {"GL_ANDROID_extension_pack_es31a", "1", gcvNULL, gcvTRUE},
};

#define _sldMaxPredefinedMacroNameLen 64   /*Hard coded maximum predefined macro name length */
#define _sldPredefinedMacroCount  (sizeof(_PredefinedMacros) / sizeof(slsPREDEFINED_MACRO))


/*******************************************************************************
**
**  ppoPREPROCESSOR_SetSourceStrings
**
**      Buffer source strings and lens, then set Dirty in *PP*
**
*/

gceSTATUS
ppoPREPROCESSOR_SetSourceStrings(
    ppoPREPROCESSOR     PP,
    gctCONST_STRING*    Strings,
    gctUINT_PTR         Lens,
    gctUINT             Count
    )
{
    gceSTATUS status = gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
    gctINT i;
    ppoINPUT_STREAM tmpbis = gcvNULL;
    ppoBYTE_INPUT_STREAM tmpbisCreated = gcvNULL;
    gctPOINTER pointer = gcvNULL;
    ppoTOKEN rplst = gcvNULL;
    gctSTRING rplStr = gcvNULL;

    gcmHEADER_ARG("PP=0x%x Strings=0x%x Lens=0x%x Count=%u",
                  PP, Strings, Lens, Count);

    /*check argument*/
    gcmASSERT(PP);
    gcmASSERT(ppvOBJ_PREPROCESSOR == PP->base.type);
    gcmASSERT(Strings);

    gcmONERROR(ppoPREPROCESSOR_Reset(PP));

    /*macro manager*/
    gcmONERROR(ppoMACRO_MANAGER_Construct(PP,
                               __FILE__,
                               __LINE__,
                               "ppoPREPROCESSOR_Construct : Create.",
                               &((PP)->macroManager)));

    /* pre-defined macros */
    do
    {
        ppoMACRO_SYMBOL ms = gcvNULL;
        gctSTRING msName   = gcvNULL;
#define _sldMessageBufferLen  (_sldMaxPredefinedMacroNameLen + 64)
        gctCHAR messageBuffer[_sldMessageBufferLen];
        gctUINT offset = 0;

        for(i = 0; i < (signed)_sldPredefinedMacroCount; i++)
        {
           gctCONST_STRING valueStr;

           if (_PredefinedMacros[i].checkFeature &&
               gcoOS_StrStr(PP->extensionString, _PredefinedMacros[i].str, gcvNULL) == gcvSTATUS_FALSE)
           {
               continue;
           }
           /* do not define GL_ES for desktop GL */
           if ((i== 0) && (sloCOMPILER_GetClientApiVersion(PP->compiler) == gcvAPI_OPENGL))
           {
               continue;
           }
           gcmONERROR(sloCOMPILER_AllocatePoolString(PP->compiler,
                                                     _PredefinedMacros[i].str,
                                                     &msName));
           valueStr = _PredefinedMacros[i].fptr ? _PredefinedMacros[i].fptr()
                                                : _PredefinedMacros[i].rplStr;
           if(valueStr)
           {
               gcmONERROR(sloCOMPILER_AllocatePoolString(PP->compiler,
                                                         valueStr,
                                                         &rplStr));

               gcmONERROR(
                   ppoTOKEN_Construct(PP, __FILE__, __LINE__, "Creat for GL_ES.", &rplst)
                   );

               rplst->hideSet       = gcvNULL;

               rplst->poolString    = rplStr;

               rplst->type          = ppvTokenType_INT;
           }
           else
           {
               rplst = gcvNULL;
           }

           offset = 0;
           gcoOS_PrintStrSafe(messageBuffer,
                              gcmSIZEOF(messageBuffer),
                              &offset,
                              "ppoPREPROCESSOR_Construct :add %s into macro symbol.",
                              _PredefinedMacros[i].str);

           gcmONERROR(ppoMACRO_SYMBOL_Construct(PP,
                                                __FILE__,
                                                __LINE__,
                                                  messageBuffer,
                                                 msName,
                                                 0,/*argc*/
                                                 gcvNULL,/*argv*/
                                                 rplst,/*replacement-list*/
                                                 &ms));

           if (ms == gcvNULL)
           {
               gcmASSERT(ms);
               gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
           }

           gcmONERROR(ppoMACRO_MANAGER_AddMacroSymbol(PP,
                                                      (PP)->macroManager,
                                                      ms));
        }
    }
    while(gcvFALSE);

    /*count*/
    PP->count = Count;

    /*lens*/
    gcmONERROR(sloCOMPILER_Allocate(
        PP->compiler,
        Count * sizeof(gctUINT),
        &pointer
        ));

    PP->lens = pointer;

    if (Lens == gcvNULL)
    {
        for (i = 0; i < (signed)Count; i++)
        {
            PP->lens[i] = (gctUINT) gcoOS_StrLen(Strings[i], gcvNULL);
        }
    }
    else
    {
        gcoOS_MemCopy(
            PP->lens,
            Lens,
            Count * sizeof(gctUINT)
            );
    }

    /*strings*/
    gcmONERROR(sloCOMPILER_Allocate(
        PP->compiler,
        Count * sizeof(gctCONST_STRING),
        &pointer));

    PP->strings = pointer;

    /*strings[i]*/
    for( i = 0; i < (signed)Count; i++ )
    {
        PP->strings[i] = Strings[i];
    }

    for( i = Count-1; i >= 0; --i )
    {
        if( PP->lens != 0 )
        {
            gcmONERROR(ppoBYTE_INPUT_STREAM_Construct(
                PP,
                gcvNULL,/*prev node*/
                gcvNULL,/*next node*/
                __FILE__,
                __LINE__,
                "ppoPREPROCESSOR_SetSourceStrings : Creat to init CPP input stream",
                PP->strings[i],
                i,/*file string number, used to report debug infomation.*/
                PP->lens[i],
                &tmpbisCreated
                ));

            tmpbis = (ppoINPUT_STREAM) tmpbisCreated;

            if( gcvNULL == PP->inputStream )
            {
                PP->inputStream = tmpbis;
                tmpbis->base.node.prev = gcvNULL;
                tmpbis->base.node.next = gcvNULL;
            }
            else
            {
                ppoINPUT_STREAM tmp_is = PP->inputStream;
                PP->inputStream = tmpbis;
                tmpbis->base.node.prev = (void*)tmp_is;
                tmpbis->base.node.next = (void*)gcvNULL;
                tmp_is->base.node.next = (void*)tmpbis;
            }/*if( gcvNULL == PP->inputStream )*/
        }
        else/*if( PP->lens > 0 )*/
        {
            gcmONERROR(ppoPREPROCESSOR_Report(
                PP,
                slvREPORT_WARN,
                "file string : %u's length is zero",
                i));
        }/*if( PP->lens > 0 )*/
    }/*for*/

    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmVERIFY_OK(sloCOMPILER_Report(
        PP->compiler,
        1,
        0,
        slvREPORT_FATAL_ERROR,
        "Failed in preprocessing."));

    gcmFOOTER();
    return status;
}

