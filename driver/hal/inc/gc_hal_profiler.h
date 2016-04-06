/****************************************************************************
*
*    Copyright (c) 2005 - 2016 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_hal_profiler_h_
#define __gc_hal_profiler_h_

#ifdef __cplusplus
extern "C" {
#endif

#define GLVERTEX_OBJECT 10
#define GLVERTEX_OBJECT_BYTES 11

#define GLINDEX_OBJECT 20
#define GLINDEX_OBJECT_BYTES 21

#define GLTEXTURE_OBJECT 30
#define GLTEXTURE_OBJECT_BYTES 31

#define GLBUFOBJ_OBJECT 40
#define GLBUFOBJ_OBJECT_BYTES 41

#if VIVANTE_PROFILER
#define gcmPROFILE_GC(Enum, Value)  gcoPROFILER_Count(gcvNULL, Enum, Value)
#else
#define gcmPROFILE_GC(Enum, Value)  do { } while (gcvFALSE)
#endif

#ifndef gcdNEW_PROFILER_FILE
#define gcdNEW_PROFILER_FILE    1
#endif

#define    ES11_CALLS              151
#define    ES11_DRAWCALLS          (ES11_CALLS             + 1)
#define    ES11_STATECHANGECALLS   (ES11_DRAWCALLS         + 1)
#define    ES11_POINTCOUNT         (ES11_STATECHANGECALLS  + 1)
#define    ES11_LINECOUNT          (ES11_POINTCOUNT        + 1)
#define    ES11_TRIANGLECOUNT      (ES11_LINECOUNT         + 1)

#define    ES30_CALLS              159
#define    ES30_DRAWCALLS          (ES30_CALLS             + 1)
#define    ES30_STATECHANGECALLS   (ES30_DRAWCALLS         + 1)
#define    ES30_POINTCOUNT         (ES30_STATECHANGECALLS  + 1)
#define    ES30_LINECOUNT          (ES30_POINTCOUNT        + 1)
#define    ES30_TRIANGLECOUNT      (ES30_LINECOUNT         + 1)

#define    VG11_CALLS              88
#define    VG11_DRAWCALLS          (VG11_CALLS              + 1)
#define    VG11_STATECHANGECALLS   (VG11_DRAWCALLS          + 1)
#define    VG11_FILLCOUNT          (VG11_STATECHANGECALLS   + 1)
#define    VG11_STROKECOUNT        (VG11_FILLCOUNT          + 1)
/* End of Driver API ID Definitions. */

/* HAL & MISC IDs. */
#define HAL_VERTBUFNEWBYTEALLOC    1
#define HAL_VERTBUFTOTALBYTEALLOC  (HAL_VERTBUFNEWBYTEALLOC     + 1)
#define HAL_VERTBUFNEWOBJALLOC     (HAL_VERTBUFTOTALBYTEALLOC   + 1)
#define HAL_VERTBUFTOTALOBJALLOC   (HAL_VERTBUFNEWOBJALLOC      + 1)
#define HAL_INDBUFNEWBYTEALLOC     (HAL_VERTBUFTOTALOBJALLOC    + 1)
#define HAL_INDBUFTOTALBYTEALLOC   (HAL_INDBUFNEWBYTEALLOC      + 1)
#define HAL_INDBUFNEWOBJALLOC      (HAL_INDBUFTOTALBYTEALLOC    + 1)
#define HAL_INDBUFTOTALOBJALLOC    (HAL_INDBUFNEWOBJALLOC       + 1)
#define HAL_TEXBUFNEWBYTEALLOC     (HAL_INDBUFTOTALOBJALLOC     + 1)
#define HAL_TEXBUFTOTALBYTEALLOC   (HAL_TEXBUFNEWBYTEALLOC      + 1)
#define HAL_TEXBUFNEWOBJALLOC      (HAL_TEXBUFTOTALBYTEALLOC    + 1)
#define HAL_TEXBUFTOTALOBJALLOC    (HAL_TEXBUFNEWOBJALLOC       + 1)

#define GPU_CYCLES           1
#define GPU_READ64BYTE       (GPU_CYCLES         + 1)
#define GPU_WRITE64BYTE      (GPU_READ64BYTE     + 1)
#define GPU_TOTALCYCLES      (GPU_WRITE64BYTE    + 1)
#define GPU_IDLECYCLES       (GPU_TOTALCYCLES    + 1)

#define VS_INSTCOUNT          1
#define VS_BRANCHINSTCOUNT    (VS_INSTCOUNT          + 1)
#define VS_TEXLDINSTCOUNT     (VS_BRANCHINSTCOUNT    + 1)
#define VS_RENDEREDVERTCOUNT  (VS_TEXLDINSTCOUNT     + 1)
#define VS_SOURCE             (VS_RENDEREDVERTCOUNT  + 1)
#define VS_NONIDLESTARVECOUNT (VS_SOURCE             + 1)
#define VS_STARVELCOUNT       (VS_NONIDLESTARVECOUNT + 1)
#define VS_STALLCOUNT         (VS_STARVELCOUNT       + 1)
#define VS_PROCESSCOUNT       (VS_STALLCOUNT         + 1)

#define PS_INSTCOUNT          1
#define PS_BRANCHINSTCOUNT    (PS_INSTCOUNT          + 1)
#define PS_TEXLDINSTCOUNT     (PS_BRANCHINSTCOUNT    + 1)
#define PS_RENDEREDPIXCOUNT   (PS_TEXLDINSTCOUNT     + 1)
#define PS_SOURCE             (PS_RENDEREDPIXCOUNT   + 1)
#define PS_NONIDLESTARVECOUNT (PS_SOURCE             + 1)
#define PS_STARVELCOUNT       (PS_NONIDLESTARVECOUNT + 1)
#define PS_STALLCOUNT         (PS_STARVELCOUNT       + 1)
#define PS_PROCESSCOUNT       (PS_STALLCOUNT         + 1)
#define PS_SHADERCYCLECOUNT   (PS_PROCESSCOUNT       + 1)

#define PA_INVERTCOUNT        1
#define PA_INPRIMCOUNT        (PA_INVERTCOUNT      + 1)
#define PA_OUTPRIMCOUNT       (PA_INPRIMCOUNT      + 1)
#define PA_DEPTHCLIPCOUNT     (PA_OUTPRIMCOUNT     + 1)
#define PA_TRIVIALREJCOUNT    (PA_DEPTHCLIPCOUNT   + 1)
#define PA_CULLCOUNT          (PA_TRIVIALREJCOUNT  + 1)
#define PA_NONIDLESTARVECOUNT (PA_CULLCOUNT        + 1)
#define PA_STARVELCOUNT       (PA_NONIDLESTARVECOUNT + 1)
#define PA_STALLCOUNT         (PA_STARVELCOUNT     + 1)
#define PA_PROCESSCOUNT       (PA_STALLCOUNT       + 1)

#define SE_TRIANGLECOUNT        1
#define SE_LINECOUNT            (SE_TRIANGLECOUNT        + 1)
#define SE_STARVECOUNT          (SE_LINECOUNT            + 1)
#define SE_STALLCOUNT           (SE_STARVECOUNT          + 1)
#define SE_RECEIVETRIANGLECOUNT (SE_STALLCOUNT           + 1)
#define SE_SENDTRIANGLECOUNT    (SE_RECEIVETRIANGLECOUNT + 1)
#define SE_RECEIVELINESCOUNT    (SE_SENDTRIANGLECOUNT    + 1)
#define SE_SENDLINESCOUNT       (SE_RECEIVELINESCOUNT    + 1)
#define SE_NONIDLESTARVECOUNT   (SE_SENDLINESCOUNT       + 1)
#define SE_PROCESSCOUNT         (SE_NONIDLESTARVECOUNT   + 1)

#define RA_VALIDPIXCOUNT      1
#define RA_TOTALQUADCOUNT     (RA_VALIDPIXCOUNT      + 1)
#define RA_VALIDQUADCOUNTEZ   (RA_TOTALQUADCOUNT     + 1)
#define RA_TOTALPRIMCOUNT     (RA_VALIDQUADCOUNTEZ   + 1)
#define RA_PIPECACHEMISSCOUNT (RA_TOTALPRIMCOUNT     + 1)
#define RA_PREFCACHEMISSCOUNT (RA_PIPECACHEMISSCOUNT + 1)
#define RA_EEZCULLCOUNT       (RA_PREFCACHEMISSCOUNT + 1)
#define RA_NONIDLESTARVECOUNT (RA_EEZCULLCOUNT       + 1)
#define RA_STARVELCOUNT       (RA_NONIDLESTARVECOUNT + 1)
#define RA_STALLCOUNT         (RA_STARVELCOUNT       + 1)
#define RA_PROCESSCOUNT       (RA_STALLCOUNT         + 1)

#define TX_TOTBILINEARREQ     1
#define TX_TOTTRILINEARREQ    (TX_TOTBILINEARREQ      + 1)
#define TX_TOTDISCARDTEXREQ   (TX_TOTTRILINEARREQ     + 1)
#define TX_TOTTEXREQ          (TX_TOTDISCARDTEXREQ    + 1)
#define TX_MEMREADCOUNT       (TX_TOTTEXREQ           + 1)
#define TX_MEMREADIN8BCOUNT   (TX_MEMREADCOUNT        + 1)
#define TX_CACHEMISSCOUNT     (TX_MEMREADIN8BCOUNT    + 1)
#define TX_CACHEHITTEXELCOUNT (TX_CACHEMISSCOUNT      + 1)
#define TX_CACHEMISSTEXELCOUNT (TX_CACHEHITTEXELCOUNT + 1)
#define TX_NONIDLESTARVECOUNT  (TX_CACHEMISSTEXELCOUNT+ 1)
#define TX_STARVELCOUNT        (TX_NONIDLESTARVECOUNT + 1)
#define TX_STALLCOUNT          (TX_STARVELCOUNT       + 1)
#define TX_PROCESSCOUNT        (TX_STALLCOUNT         + 1)

#define PE_KILLEDBYCOLOR      1
#define PE_KILLEDBYDEPTH      (PE_KILLEDBYCOLOR    + 1)
#define PE_DRAWNBYCOLOR       (PE_KILLEDBYDEPTH    + 1)
#define PE_DRAWNBYDEPTH       (PE_DRAWNBYCOLOR     + 1)

#define MC_READREQ8BPIPE      1
#define MC_READREQ8BIP        (MC_READREQ8BPIPE    + 1)
#define MC_WRITEREQ8BPIPE     (MC_READREQ8BIP      + 1)
#define MC_AXIMINLATENCY      (MC_WRITEREQ8BPIPE   + 1)
#define MC_AXIMAXLATENCY      (MC_AXIMINLATENCY    + 1)
#define MC_AXITOTALLATENCY    (MC_AXIMAXLATENCY    + 1)
#define MC_AXISAMPLECOUNT     (MC_AXITOTALLATENCY  + 1)

#define AXI_READREQSTALLED    1
#define AXI_WRITEREQSTALLED   (AXI_READREQSTALLED  + 1)
#define AXI_WRITEDATASTALLED  (AXI_WRITEREQSTALLED + 1)

#define FE_DRAWCOUNT    1
#define FE_OUTVERTEXCOUNT     (FE_DRAWCOUNT  + 1)
#define FE_STALLCOUNT         (FE_OUTVERTEXCOUNT + 1)
#define FE_STARVECOUNT        (FE_STALLCOUNT + 1)

#define PVS_INSTRCOUNT        1
#define PVS_ALUINSTRCOUNT     (PVS_INSTRCOUNT      + 1)
#define PVS_TEXINSTRCOUNT     (PVS_ALUINSTRCOUNT   + 1)
#define PVS_ATTRIBCOUNT       (PVS_TEXINSTRCOUNT   + 1)
#define PVS_UNIFORMCOUNT      (PVS_ATTRIBCOUNT     + 1)
#define PVS_FUNCTIONCOUNT     (PVS_UNIFORMCOUNT    + 1)
#define PVS_SOURCE            (PVS_FUNCTIONCOUNT   + 1)

#define PPS_INSTRCOUNT       1
#define PPS_ALUINSTRCOUNT    (PPS_INSTRCOUNT       + 1)
#define PPS_TEXINSTRCOUNT    (PPS_ALUINSTRCOUNT    + 1)
#define PPS_ATTRIBCOUNT      (PPS_TEXINSTRCOUNT    + 1)
#define PPS_UNIFORMCOUNT     (PPS_ATTRIBCOUNT      + 1)
#define PPS_FUNCTIONCOUNT    (PPS_UNIFORMCOUNT     + 1)
#define PPS_SOURCE           (PPS_FUNCTIONCOUNT    + 1)
/* End of MISC Counter IDs. */

#ifdef gcdNEW_PROFILER_FILE

/* Category Constants. */
#define VPHEADER        0x010000
#define VPG_INFO        0x020000
#define VPG_TIME        0x030000
#define VPG_MEM         0x040000
#define VPG_ES11        0x050000
#define VPG_ES30        0x060000
#define VPG_VG11        0x070000
#define VPG_HAL         0x080000
#define VPG_HW          0x090000
#define VPG_GPU         0x0a0000
#define VPG_VS          0x0b0000
#define VPG_PS          0x0c0000
#define VPG_PA          0x0d0000
#define VPG_SETUP       0x0e0000
#define VPG_RA          0x0f0000
#define VPG_TX          0x100000
#define VPG_PE          0x110000
#define VPG_MC          0x120000
#define VPG_AXI         0x130000
#define VPG_PROG        0x140000
#define VPG_PVS         0x150000
#define VPG_PPS         0x160000
#define VPG_ES11_TIME   0x170000
#define VPG_ES30_TIME   0x180000
#define VPG_FRAME       0x190000
#define VPG_ES11_DRAW   0x200000
#define VPG_ES30_DRAW   0x210000
#define VPG_VG11_TIME   0x220000
#define VPG_FE          0x230000
#define VPG_MULTI_GPU   0x240000
#define VPG_END         0xff0000

/* Info. */
#define VPC_INFOCOMPANY         (VPG_INFO + 1)
#define VPC_INFOVERSION         (VPC_INFOCOMPANY + 1)
#define VPC_INFORENDERER        (VPC_INFOVERSION + 1)
#define VPC_INFOREVISION        (VPC_INFORENDERER + 1)
#define VPC_INFODRIVER          (VPC_INFOREVISION + 1)
#define VPC_INFODRIVERMODE      (VPC_INFODRIVER + 1)
#define VPC_INFOSCREENSIZE      (VPC_INFODRIVERMODE + 1)

/* Counter Constants. */
#define VPC_ELAPSETIME          (VPG_TIME + 1)
#define VPC_CPUTIME             (VPC_ELAPSETIME + 1)

#define VPC_MEMMAXRES           (VPG_MEM + 1)
#define VPC_MEMSHARED           (VPC_MEMMAXRES + 1)
#define VPC_MEMUNSHAREDDATA     (VPC_MEMSHARED + 1)
#define VPC_MEMUNSHAREDSTACK    (VPC_MEMUNSHAREDDATA + 1)

/* OpenGL ES11 Statics Counter IDs. */
#define    VPC_ES11CALLS            (VPG_ES11 +    ES11_CALLS)
#define    VPC_ES11DRAWCALLS        (VPG_ES11 +    ES11_DRAWCALLS)
#define    VPC_ES11STATECHANGECALLS (VPG_ES11 +    ES11_STATECHANGECALLS)
#define    VPC_ES11POINTCOUNT       (VPG_ES11 +    ES11_POINTCOUNT)
#define    VPC_ES11LINECOUNT        (VPG_ES11 +    ES11_LINECOUNT)
#define    VPC_ES11TRIANGLECOUNT    (VPG_ES11 +    ES11_TRIANGLECOUNT)

/* OpenGL ES30 Statistics Counter IDs. */
#define    VPC_ES30CALLS            (VPG_ES30 +    ES30_CALLS)
#define    VPC_ES30DRAWCALLS        (VPG_ES30 +    ES30_DRAWCALLS)
#define    VPC_ES30STATECHANGECALLS (VPG_ES30 +    ES30_STATECHANGECALLS)
#define    VPC_ES30POINTCOUNT       (VPG_ES30 +    ES30_POINTCOUNT)
#define    VPC_ES30LINECOUNT        (VPG_ES30 +    ES30_LINECOUNT)
#define    VPC_ES30TRIANGLECOUNT    (VPG_ES30 +    ES30_TRIANGLECOUNT)

/* OpenVG Statistics Counter IDs. */
#define    VPC_VG11CALLS            (VPG_VG11 +    VG11_CALLS)
#define    VPC_VG11DRAWCALLS        (VPG_VG11 +    VG11_DRAWCALLS)
#define    VPC_VG11STATECHANGECALLS (VPG_VG11 +    VG11_STATECHANGECALLS)
#define    VPC_VG11FILLCOUNT        (VPG_VG11 +    VG11_FILLCOUNT)
#define    VPC_VG11STROKECOUNT      (VPG_VG11 +    VG11_STROKECOUNT)

/* HAL Counters. */
#define VPC_HALVERTBUFNEWBYTEALLOC      (VPG_HAL + HAL_VERTBUFNEWBYTEALLOC)
#define VPC_HALVERTBUFTOTALBYTEALLOC    (VPG_HAL + HAL_VERTBUFTOTALBYTEALLOC)
#define VPC_HALVERTBUFNEWOBJALLOC       (VPG_HAL + HAL_VERTBUFNEWOBJALLOC)
#define VPC_HALVERTBUFTOTALOBJALLOC     (VPG_HAL + HAL_VERTBUFTOTALOBJALLOC)
#define VPC_HALINDBUFNEWBYTEALLOC       (VPG_HAL + HAL_INDBUFNEWBYTEALLOC)
#define VPC_HALINDBUFTOTALBYTEALLOC     (VPG_HAL + HAL_INDBUFTOTALBYTEALLOC)
#define VPC_HALINDBUFNEWOBJALLOC        (VPG_HAL + HAL_INDBUFNEWOBJALLOC)
#define VPC_HALINDBUFTOTALOBJALLOC      (VPG_HAL + HAL_INDBUFTOTALOBJALLOC)
#define VPC_HALTEXBUFNEWBYTEALLOC       (VPG_HAL + HAL_TEXBUFNEWBYTEALLOC)
#define VPC_HALTEXBUFTOTALBYTEALLOC     (VPG_HAL + HAL_TEXBUFTOTALBYTEALLOC)
#define VPC_HALTEXBUFNEWOBJALLOC        (VPG_HAL + HAL_TEXBUFNEWOBJALLOC)
#define VPC_HALTEXBUFTOTALOBJALLOC      (VPG_HAL + HAL_TEXBUFTOTALOBJALLOC)

/* HW: GPU Counters. */
#define VPC_GPUCYCLES                   (VPG_GPU + GPU_CYCLES)
#define VPC_GPUREAD64BYTE               (VPG_GPU + GPU_READ64BYTE)
#define VPC_GPUWRITE64BYTE              (VPG_GPU + GPU_WRITE64BYTE)
#define VPC_GPUTOTALCYCLES              (VPG_GPU + GPU_TOTALCYCLES)
#define VPC_GPUIDLECYCLES               (VPG_GPU + GPU_IDLECYCLES)

/* HW: Shader Counters. */
#define VPC_VSINSTCOUNT                 (VPG_VS + VS_INSTCOUNT)
#define VPC_VSBRANCHINSTCOUNT           (VPG_VS + VS_BRANCHINSTCOUNT)
#define VPC_VSTEXLDINSTCOUNT            (VPG_VS + VS_TEXLDINSTCOUNT)
#define VPC_VSRENDEREDVERTCOUNT         (VPG_VS + VS_RENDEREDVERTCOUNT)
#define VPC_VSNONIDLESTARVECOUNT        (VPG_VS + VS_NONIDLESTARVECOUNT)
#define VPC_VSSTARVELCOUNT              (VPG_VS + VS_STARVELCOUNT)
#define VPC_VSSTALLCOUNT                (VPG_VS + VS_STALLCOUNT)
#define VPC_VSPROCESSCOUNT              (VPG_VS + VS_PROCESSCOUNT)
/* HW: PS Count. */
#define VPC_PSINSTCOUNT                 (VPG_PS + PS_INSTCOUNT)
#define VPC_PSBRANCHINSTCOUNT           (VPG_PS + PS_BRANCHINSTCOUNT)
#define VPC_PSTEXLDINSTCOUNT            (VPG_PS + PS_TEXLDINSTCOUNT)
#define VPC_PSRENDEREDPIXCOUNT          (VPG_PS + PS_RENDEREDPIXCOUNT)
#define VPC_PSNONIDLESTARVECOUNT        (VPG_PS + PS_NONIDLESTARVECOUNT)
#define VPC_PSSTARVELCOUNT              (VPG_PS + PS_STARVELCOUNT)
#define VPC_PSSTALLCOUNT                (VPG_PS + PS_STALLCOUNT)
#define VPC_PSPROCESSCOUNT              (VPG_PS + PS_PROCESSCOUNT)
#define VPC_PSSHADERCYCLECOUNT          (VPG_PS + PS_SHADERCYCLECOUNT)

/* HW: PA Counters. */
#define VPC_PAINVERTCOUNT               (VPG_PA + PA_INVERTCOUNT)
#define VPC_PAINPRIMCOUNT               (VPG_PA + PA_INPRIMCOUNT)
#define VPC_PAOUTPRIMCOUNT              (VPG_PA + PA_OUTPRIMCOUNT)
#define VPC_PADEPTHCLIPCOUNT            (VPG_PA + PA_DEPTHCLIPCOUNT)
#define VPC_PATRIVIALREJCOUNT           (VPG_PA + PA_TRIVIALREJCOUNT)
#define VPC_PACULLCOUNT                 (VPG_PA + PA_CULLCOUNT)
#define VPC_PANONIDLESTARVECOUNT        (VPG_PA + PA_NONIDLESTARVECOUNT)
#define VPC_PASTARVELCOUNT              (VPG_PA + PA_STARVELCOUNT)
#define VPC_PASTALLCOUNT                (VPG_PA + PA_STALLCOUNT)
#define VPC_PAPROCESSCOUNT              (VPG_PA + PA_PROCESSCOUNT)

/* HW: Setup Counters. */
#define VPC_SETRIANGLECOUNT             (VPG_SETUP + SE_TRIANGLECOUNT)
#define VPC_SELINECOUNT                 (VPG_SETUP + SE_LINECOUNT)
#define VPC_SESTARVECOUNT               (VPG_SETUP + SE_STARVECOUNT)
#define VPC_SESTALLCOUNT                (VPG_SETUP + SE_STALLCOUNT)
#define VPC_SERECEIVETRIANGLECOUNT      (VPG_SETUP + SE_RECEIVETRIANGLECOUNT)
#define VPC_SESENDTRIANGLECOUNT         (VPG_SETUP + SE_SENDTRIANGLECOUNT)
#define VPC_SERECEIVELINESCOUNT         (VPG_SETUP + SE_RECEIVELINESCOUNT)
#define VPC_SESENDLINESCOUNT            (VPG_SETUP + SE_SENDLINESCOUNT)
#define VPC_SENONIDLESTARVECOUNT        (VPG_SETUP + SE_NONIDLESTARVECOUNT)
#define VPC_SEPROCESSCOUNT              (VPG_SETUP + SE_PROCESSCOUNT)

/* HW: RA Counters. */
#define VPC_RAVALIDPIXCOUNT             (VPG_RA + RA_VALIDPIXCOUNT)
#define VPC_RATOTALQUADCOUNT            (VPG_RA + RA_TOTALQUADCOUNT)
#define VPC_RAVALIDQUADCOUNTEZ          (VPG_RA + RA_VALIDQUADCOUNTEZ)
#define VPC_RATOTALPRIMCOUNT            (VPG_RA + RA_TOTALPRIMCOUNT)
#define VPC_RAPIPECACHEMISSCOUNT        (VPG_RA + RA_PIPECACHEMISSCOUNT)
#define VPC_RAPREFCACHEMISSCOUNT        (VPG_RA + RA_PREFCACHEMISSCOUNT)
#define VPC_RAEEZCULLCOUNT              (VPG_RA + RA_EEZCULLCOUNT)
#define VPC_RANONIDLESTARVECOUNT        (VPG_RA + RA_NONIDLESTARVECOUNT)
#define VPC_RASTARVELCOUNT              (VPG_RA + RA_STARVELCOUNT)
#define VPC_RASTALLCOUNT                (VPG_RA + RA_STALLCOUNT)
#define VPC_RAPROCESSCOUNT              (VPG_RA + RA_PROCESSCOUNT)

/* HW: TEX Counters. */
#define VPC_TXTOTBILINEARREQ            (VPG_TX + TX_TOTBILINEARREQ)
#define VPC_TXTOTTRILINEARREQ           (VPG_TX + TX_TOTTRILINEARREQ)
#define VPC_TXTOTDISCARDTEXREQ          (VPG_TX + TX_TOTDISCARDTEXREQ)
#define VPC_TXTOTTEXREQ                 (VPG_TX + TX_TOTTEXREQ)
#define VPC_TXMEMREADCOUNT              (VPG_TX + TX_MEMREADCOUNT)
#define VPC_TXMEMREADIN8BCOUNT          (VPG_TX + TX_MEMREADIN8BCOUNT)
#define VPC_TXCACHEMISSCOUNT            (VPG_TX + TX_CACHEMISSCOUNT)
#define VPC_TXCACHEHITTEXELCOUNT        (VPG_TX + TX_CACHEHITTEXELCOUNT)
#define VPC_TXCACHEMISSTEXELCOUNT       (VPG_TX + TX_CACHEMISSTEXELCOUNT)
#define VPC_TXNONIDLESTARVECOUNT        (VPG_TX + TX_NONIDLESTARVECOUNT)
#define VPC_TXSTARVELCOUNT              (VPG_TX + TX_STARVELCOUNT)
#define VPC_TXSTALLCOUNT                (VPG_TX + TX_STALLCOUNT)
#define VPC_TXPROCESSCOUNT              (VPG_TX + TX_PROCESSCOUNT)

/* HW: PE Counters. */
#define VPC_PEKILLEDBYCOLOR             (VPG_PE + PE_KILLEDBYCOLOR)
#define VPC_PEKILLEDBYDEPTH             (VPG_PE + PE_KILLEDBYDEPTH)
#define VPC_PEDRAWNBYCOLOR              (VPG_PE + PE_DRAWNBYCOLOR)
#define VPC_PEDRAWNBYDEPTH              (VPG_PE + PE_DRAWNBYDEPTH)

/* HW: MC Counters. */
#define VPC_MCREADREQ8BPIPE             (VPG_MC + MC_READREQ8BPIPE)
#define VPC_MCREADREQ8BIP               (VPG_MC + MC_READREQ8BIP)
#define VPC_MCWRITEREQ8BPIPE            (VPG_MC + MC_WRITEREQ8BPIPE)
#define VPC_MCAXIMINLATENCY             (VPG_MC + MC_AXIMINLATENCY)
#define VPC_MCAXIMAXLATENCY             (VPG_MC + MC_AXIMAXLATENCY)
#define VPC_MCAXITOTALLATENCY           (VPG_MC + MC_AXITOTALLATENCY)
#define VPC_MCAXISAMPLECOUNT            (VPG_MC + MC_AXISAMPLECOUNT)

/* HW: AXI Counters. */
#define VPC_AXIREADREQSTALLED           (VPG_AXI + AXI_READREQSTALLED)
#define VPC_AXIWRITEREQSTALLED          (VPG_AXI + AXI_WRITEREQSTALLED)
#define VPC_AXIWRITEDATASTALLED         (VPG_AXI + AXI_WRITEDATASTALLED)

/* HW: FE Counters. */
#define VPC_FEDRAWCOUNT                 (VPG_FE + FE_DRAWCOUNT)
#define VPC_FEOUTVERTEXCOUNT            (VPG_FE + FE_OUTVERTEXCOUNT)
#define VPC_FESTALLCOUNT                (VPG_FE + FE_STALLCOUNT)
#define VPC_FESTARVECOUNT               (VPG_FE + FE_STARVECOUNT)

/* PROGRAM: Shader program counters. */
#define VPC_PVSINSTRCOUNT           (VPG_PVS + PVS_INSTRCOUNT)
#define VPC_PVSALUINSTRCOUNT        (VPG_PVS + PVS_ALUINSTRCOUNT)
#define VPC_PVSTEXINSTRCOUNT        (VPG_PVS + PVS_TEXINSTRCOUNT)
#define VPC_PVSATTRIBCOUNT          (VPG_PVS + PVS_ATTRIBCOUNT)
#define VPC_PVSUNIFORMCOUNT         (VPG_PVS + PVS_UNIFORMCOUNT)
#define VPC_PVSFUNCTIONCOUNT        (VPG_PVS + PVS_FUNCTIONCOUNT)
#define VPC_PVSSOURCE               (VPG_PVS + PVS_SOURCE)

#define VPC_PPSINSTRCOUNT           (VPG_PPS + PPS_INSTRCOUNT)
#define VPC_PPSALUINSTRCOUNT        (VPG_PPS + PPS_ALUINSTRCOUNT)
#define VPC_PPSTEXINSTRCOUNT        (VPG_PPS + PPS_TEXINSTRCOUNT)
#define VPC_PPSATTRIBCOUNT          (VPG_PPS + PPS_ATTRIBCOUNT)
#define VPC_PPSUNIFORMCOUNT         (VPG_PPS + PPS_UNIFORMCOUNT)
#define VPC_PPSFUNCTIONCOUNT        (VPG_PPS + PPS_FUNCTIONCOUNT)
#define VPC_PPSSOURCE               (VPG_PPS + PPS_SOURCE)

#define VPC_PROGRAMHANDLE           (VPG_PROG + 1)

#define VPC_ES30_DRAW_NO            (VPG_ES30_DRAW + 1)
#define VPC_ES11_DRAW_NO            (VPG_ES11_DRAW + 1)
#define VPC_ES30_GPU_NO             (VPG_MULTI_GPU + 1)
#endif

#if VIVANTE_PROFILER_ALL_COUNTER
#define   MODULE_FRONT_END_COUNTER_NUM                    30
#define   MODULE_SHADER_COUNTER_NUM                       30
#define   MODULE_PRIMITIVE_ASSEMBLY_COUNTER_NUM           30
#define   MODULE_SETUP_COUNTER_NUM                        30
#define   MODULE_RASTERIZER_COUNTER_NUM                   30
#define   MODULE_TEXTURE_COUNTER_NUM                      30
#define   MODULE_PIXEL_ENGINE_COUNTER_NUM                 30
#define   MODULE_MEMORY_CONTROLLER_COUNTER_NUM            30
#define   MODULE_HOST_INTERFACE_COUNTER_NUM               30
#endif

#if VIVANTE_PROFILER_PROBE
#define   MODULE_FRONT_END_COUNTER_NUM                    0x5
#define   MODULE_VERTEX_SHADER_COUNTER_NUM                0x9
#define   MODULE_PRIMITIVE_ASSEMBLY_COUNTER_NUM           0xC
#define   MODULE_SETUP_COUNTER_NUM                        0xD
#define   MODULE_RASTERIZER_COUNTER_NUM                   0xE
#define   MODULE_PIXEL_SHADER_COUNTER_NUM                 0x9
#define   MODULE_TEXTURE_COUNTER_NUM                      0x8
#define   MODULE_PIXEL_ENGINE_COUNTER_NUM                 0x8
#define   MODULE_MEMORY_CONTROLLER_COLOR_COUNTER_NUM      0xC
#define   MODULE_MEMORY_CONTROLLER_DEPTH_COUNTER_NUM      0xC
#define   MODULE_HOST_INTERFACE0_COUNTER_NUM              0x9
#define   MODULE_HOST_INTERFACE1_COUNTER_NUM              0x7
#define   MODULE_GPUL2_CACHE_COUNTER_NUM                  0xE

typedef enum _gceCOUNTER
{
    gcvCOUNTER_FRONT_END,
    gcvCOUNTER_VERTEX_SHADER,
    gcvCOUNTER_PRIMITIVE_ASSEMBLY,
    gcvCOUNTER_SETUP,
    gcvCOUNTER_RASTERIZER,
    gcvCOUNTER_PIXEL_SHADER,
    gcvCOUNTER_TEXTURE,
    gcvCOUNTER_PIXEL_ENGINE,
    gcvCOUNTER_MEMORY_CONTROLLER_COLOR,
    gcvCOUNTER_MEMORY_CONTROLLER_DEPTH,
    gcvCOUNTER_HOST_INTERFACE0,
    gcvCOUNTER_HOST_INTERFACE1,
    gcvCOUNTER_GPUL2_CACHE,
    gcvCOUNTER_COUNT
}
gceCOUNTER;
#endif

/* HW profile information. */
typedef struct _gcsPROFILER_COUNTERS
{
    /* HW static counters. */
    gctUINT32       gpuClock;
    gctUINT32       axiClock;
    gctUINT32       shaderClock;

    /* HW vairable counters. */
    gctUINT32       gpuClockStart;
    gctUINT32       gpuClockEnd;

    /* HW vairable counters. */
    gctUINT32       gpuCyclesCounter;
    gctUINT32       gpuTotalCyclesCounter;
    gctUINT32       gpuIdleCyclesCounter;
    gctUINT32       gpuTotalRead64BytesPerFrame;
    gctUINT32       gpuTotalWrite64BytesPerFrame;

    /* PE */
    gctUINT32       pe_pixel_count_killed_by_color_pipe;
    gctUINT32       pe_pixel_count_killed_by_depth_pipe;
    gctUINT32       pe_pixel_count_drawn_by_color_pipe;
    gctUINT32       pe_pixel_count_drawn_by_depth_pipe;

    /* SH */
    gctUINT32       ps_inst_counter;
    gctUINT32       rendered_pixel_counter;
    gctUINT32       vs_inst_counter;
    gctUINT32       rendered_vertice_counter;
    gctUINT32       vtx_branch_inst_counter;
    gctUINT32       vtx_texld_inst_counter;
    gctUINT32       pxl_branch_inst_counter;
    gctUINT32       pxl_texld_inst_counter;
    gctUINT32       vs_non_idle_starve_count;
    gctUINT32       vs_starve_count;
    gctUINT32       vs_stall_count;
    gctUINT32       vs_process_count;
    gctUINT32       ps_non_idle_starve_count;
    gctUINT32       ps_starve_count;
    gctUINT32       ps_stall_count;
    gctUINT32       ps_process_count;
    gctUINT32       shader_cycle_count;

    /* PA */
    gctUINT32       pa_input_vtx_counter;
    gctUINT32       pa_input_prim_counter;
    gctUINT32       pa_output_prim_counter;
    gctUINT32       pa_depth_clipped_counter;
    gctUINT32       pa_trivial_rejected_counter;
    gctUINT32       pa_culled_counter;
    gctUINT32       pa_non_idle_starve_count;
    gctUINT32       pa_starve_count;
    gctUINT32       pa_stall_count;
    gctUINT32       pa_process_count;

    /* SE */
    gctUINT32       se_culled_triangle_count;
    gctUINT32       se_culled_lines_count;
    gctUINT32       se_starve_count;
    gctUINT32       se_stall_count;
    gctUINT32       se_receive_triangle_count;
    gctUINT32       se_send_triangle_count;
    gctUINT32       se_receive_lines_count;
    gctUINT32       se_send_lines_count;
    gctUINT32       se_process_count;
    gctUINT32       se_non_idle_starve_count;

    /* RA */
    gctUINT32       ra_valid_pixel_count;
    gctUINT32       ra_total_quad_count;
    gctUINT32       ra_valid_quad_count_after_early_z;
    gctUINT32       ra_total_primitive_count;
    gctUINT32       ra_pipe_cache_miss_counter;
    gctUINT32       ra_prefetch_cache_miss_counter;
    gctUINT32       ra_eez_culled_counter;
    gctUINT32       ra_non_idle_starve_count;
    gctUINT32       ra_starve_count;
    gctUINT32       ra_stall_count;
    gctUINT32       ra_process_count;

    /* TX */
    gctUINT32       tx_total_bilinear_requests;
    gctUINT32       tx_total_trilinear_requests;
    gctUINT32       tx_total_discarded_texture_requests;
    gctUINT32       tx_total_texture_requests;
    gctUINT32       tx_mem_read_count;
    gctUINT32       tx_mem_read_in_8B_count;
    gctUINT32       tx_cache_miss_count;
    gctUINT32       tx_cache_hit_texel_count;
    gctUINT32       tx_cache_miss_texel_count;
    gctUINT32       tx_non_idle_starve_count;
    gctUINT32       tx_starve_count;
    gctUINT32       tx_stall_count;
    gctUINT32       tx_process_count;

    /* MC */
    gctUINT32       mc_total_read_req_8B_from_pipeline;
    gctUINT32       mc_total_read_req_8B_from_IP;
    gctUINT32       mc_total_write_req_8B_from_pipeline;
    gctUINT32       mc_axi_total_latency;
    gctUINT32       mc_axi_sample_count;
    gctUINT32       mc_axi_max_latency;
    gctUINT32       mc_axi_min_latency;

    /* HI */
    gctUINT32       hi_axi_cycles_read_request_stalled;
    gctUINT32       hi_axi_cycles_write_request_stalled;
    gctUINT32       hi_axi_cycles_write_data_stalled;

    /* FE */
    gctUINT32       fe_draw_count;
    gctUINT32       fe_out_vertex_count;
    gctUINT32       fe_stall_count;
    gctUINT32       fe_starve_count;

#if VIVANTE_PROFILER_ALL_COUNTER
    gctUINT32       feCounters[MODULE_FRONT_END_COUNTER_NUM];
    gctUINT32       paCounters[MODULE_PRIMITIVE_ASSEMBLY_COUNTER_NUM];
    gctUINT32       shCounters[MODULE_SHADER_COUNTER_NUM];
    gctUINT32       seCounters[MODULE_SETUP_COUNTER_NUM];
    gctUINT32       raCounters[MODULE_RASTERIZER_COUNTER_NUM];
    gctUINT32       txCounters[MODULE_TEXTURE_COUNTER_NUM];
    gctUINT32       peCounters[MODULE_PIXEL_ENGINE_COUNTER_NUM];
    gctUINT32       mcCounters[MODULE_MEMORY_CONTROLLER_COUNTER_NUM];
    gctUINT32       hiCounters[MODULE_HOST_INTERFACE_COUNTER_NUM];

#endif
}
gcsPROFILER_COUNTERS;

#if VIVANTE_PROFILER_PROBE
#define NumOfDrawBuf 1024
#endif

/* HAL profile information. */
typedef struct _gcsPROFILER
{
    gctUINT32       enable;
    gctBOOL         enableHal;
    gctBOOL         enableHW;
    gctBOOL         enableSH;
    gctBOOL         isSyncMode;
    gctBOOL         enablePrint;
    gctBOOL         disableOutputCounter;

    gctBOOL         useSocket;
    gctINT          sockFd;

    gctFILE         file;

    /* Aggregate Information */

    /* Clock Info */
    gctUINT64       frameStart;
    gctUINT64       frameEnd;

    /* Current frame information */
    gctUINT32       frameNumber;
    gctUINT64       frameStartTimeusec;
    gctUINT64       frameEndTimeusec;
    gctUINT64       frameStartCPUTimeusec;
    gctUINT64       frameEndCPUTimeusec;

#if PROFILE_HAL_COUNTERS
    gctUINT32       vertexBufferTotalBytesAlloc;
    gctUINT32       vertexBufferNewBytesAlloc;
    int             vertexBufferTotalObjectsAlloc;
    int             vertexBufferNewObjectsAlloc;

    gctUINT32       indexBufferTotalBytesAlloc;
    gctUINT32       indexBufferNewBytesAlloc;
    int             indexBufferTotalObjectsAlloc;
    int             indexBufferNewObjectsAlloc;

    gctUINT32       textureBufferTotalBytesAlloc;
    gctUINT32       textureBufferNewBytesAlloc;
    int             textureBufferTotalObjectsAlloc;
    int             textureBufferNewObjectsAlloc;

    gctUINT32       numCommits;
    gctUINT32       drawPointCount;
    gctUINT32       drawLineCount;
    gctUINT32       drawTriangleCount;
    gctUINT32       drawVertexCount;
    gctUINT32       redundantStateChangeCalls;
#endif

#if VIVANTE_PROFILER_PROBE
    gctHANDLE       newCounterBuf[NumOfDrawBuf];
    gctUINT32       curBufId;
    gctFILE         probeFile;
#endif

}
gcsPROFILER;

/* Memory profile information. */
struct _gcsMemProfile
{
    /* Memory Usage */
    gctUINT32       videoMemUsed;
    gctUINT32       systemMemUsed;
    gctUINT32       commitBufferSize;
    gctUINT32       contextBufferCopyBytes;
};

/* Shader profile information. */
struct _gcsSHADER_PROFILER
{
    gctUINT32       shaderLength;
    gctUINT32       shaderALUCycles;
    gctUINT32       shaderTexLoadCycles;
    gctUINT32       shaderTempRegCount;
    gctUINT32       shaderSamplerRegCount;
    gctUINT32       shaderInputRegCount;
    gctUINT32       shaderOutputRegCount;
};

/* Initialize the gcsProfiler. */
gceSTATUS
gcoPROFILER_Initialize(
    IN gcoHAL Hal,
    IN gctBOOL Enable
    );

/* Destroy the gcProfiler. */
gceSTATUS
gcoPROFILER_Destroy(
    IN gcoHAL Hal
    );

/* Write data to profiler. */
gceSTATUS
gcoPROFILER_Write(
    IN gcoHAL Hal,
    IN gctSIZE_T ByteCount,
    IN gctCONST_POINTER Data
    );

/* Flush data out. */
gceSTATUS
gcoPROFILER_Flush(
    IN gcoHAL Hal
    );

/* Call to signal end of frame. */
gceSTATUS
gcoPROFILER_EndFrame(
    IN gcoHAL Hal
    );

gceSTATUS
gcoPROFILER_BeginDraw(
IN gcoHAL Hal
);

/* Call to signal end of draw. */
gceSTATUS
gcoPROFILER_EndDraw(
    IN gcoHAL Hal,
    IN gctBOOL FirstDraw
    );

/* Increase profile counter Enum by Value. */
gceSTATUS
gcoPROFILER_Count(
    IN gcoHAL Hal,
    IN gctUINT32 Enum,
    IN gctINT Value
    );

/* Profile input vertex shader. */
gceSTATUS
gcoPROFILER_ShaderVS(
    IN gcoHAL Hal,
    IN gctPOINTER Vs
    );

/* Profile input fragment shader. */
gceSTATUS
gcoPROFILER_ShaderFS(
    IN gcoHAL Hal,
    IN gctPOINTER Fs
    );

#ifdef __cplusplus
}
#endif

#endif /* __gc_hal_profiler_h_ */
