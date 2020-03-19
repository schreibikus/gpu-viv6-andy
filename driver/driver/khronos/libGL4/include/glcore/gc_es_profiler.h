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


#ifndef __gc_es_profiler_h__
#define __gc_es_profiler_h__

#include "shared/gc_hal_types.h"

#define GL4_APICALLBASE 100

/* ES2.0 old APIs */
#define GL4_ACTIVETEXTURE 100
#define GL4_ATTACHSHADER 101
#define GL4_BINDATTRIBLOCATION 102
#define GL4_BINDBUFFER 103
#define GL4_BINDFRAMEBUFFER 104
#define GL4_BINDRENDERBUFFER 105
#define GL4_BINDTEXTURE 106
#define GL4_BLENDCOLOR 107
#define GL4_BLENDEQUATION 108
#define GL4_BLENDEQUATIONSEPARATE 109
#define GL4_BLENDFUNC 110
#define GL4_BLENDFUNCSEPARATE 111
#define GL4_BUFFERDATA 112
#define GL4_BUFFERSUBDATA 113
#define GL4_CHECKFRAMEBUFFERSTATUS 114
#define GL4_CLEAR 115
#define GL4_CLEARCOLOR 116
#define GL4_CLEARDEPTHF 117
#define GL4_CLEARSTENCIL 118
#define GL4_COLORMASK 119
#define GL4_COMPILESHADER 120
#define GL4_COMPRESSEDTEXIMAGE2D 121
#define GL4_COMPRESSEDTEXSUBIMAGE2D 122
#define GL4_COPYTEXIMAGE2D 123
#define GL4_COPYTEXSUBIMAGE2D 124
#define GL4_CREATEPROGRAM 125
#define GL4_CREATESHADER 126
#define GL4_CULLFACE 127
#define GL4_DELETEBUFFERS 128
#define GL4_DELETEFRAMEBUFFERS 129
#define GL4_DELETEPROGRAM 130
#define GL4_DELETERENDERBUFFERS 131
#define GL4_DELETESHADER 132
#define GL4_DELETETEXTURES 133
#define GL4_DEPTHFUNC 134
#define GL4_DEPTHMASK 135
#define GL4_DEPTHRANGEF 136
#define GL4_DETACHSHADER 137
#define GL4_DISABLE 138
#define GL4_DISABLEVERTEXATTRIBARRAY 139
#define GL4_DRAWARRAYS 140
#define GL4_DRAWELEMENTS 141
#define GL4_ENABLE 142
#define GL4_ENABLEVERTEXATTRIBARRAY 143
#define GL4_FINISH 144
#define GL4_FLUSH 145
#define GL4_FRAMEBUFFERRENDERBUFFER 146
#define GL4_FRAMEBUFFERTEXTURE2D 147
#define GL4_FRONTFACE 148
#define GL4_GENBUFFERS 149
#define GL4_GENERATEMIPMAP 150
#define GL4_GENFRAMEBUFFERS 151
#define GL4_GENRENDERBUFFERS 152
#define GL4_GENTEXTURES 153
#define GL4_GETACTIVEATTRIB 154
#define GL4_GETACTIVEUNIFORM 155
#define GL4_GETATTACHEDSHADERS 156
#define GL4_GETATTRIBLOCATION 157
#define GL4_GETBOOLEANV 158
#define GL4_GETBUFFERPARAMETERIV 159
#define GL4_GETERROR 160
#define GL4_GETFLOATV 161
#define GL4_GETFRAMEBUFFERATTACHMENTPARAMETERIV 162
#define GL4_GETINTEGERV 163
#define GL4_GETPROGRAMIV 164
#define GL4_GETPROGRAMINFOLOG 165
#define GL4_GETRENDERBUFFERPARAMETERIV 166
#define GL4_GETSHADERIV 167
#define GL4_GETSHADERINFOLOG 168
#define GL4_GETSHADERPRECISIONFORMAT 169
#define GL4_GETSHADERSOURCE 170
#define GL4_GETSTRING 171
#define GL4_GETTEXPARAMETERFV 172
#define GL4_GETTEXPARAMETERIV 173
#define GL4_GETUNIFORMFV 174
#define GL4_GETUNIFORMIV 175
#define GL4_GETUNIFORMLOCATION 176
#define GL4_GETVERTEXATTRIBFV 177
#define GL4_GETVERTEXATTRIBIV 178
#define GL4_GETVERTEXATTRIBPOINTERV 179
#define GL4_HINT 180
#define GL4_ISBUFFER 181
#define GL4_ISENABLED 182
#define GL4_ISFRAMEBUFFER 183
#define GL4_ISPROGRAM 184
#define GL4_ISRENDERBUFFER 185
#define GL4_ISSHADER 186
#define GL4_ISTEXTURE 187
#define GL4_LINEWIDTH 188
#define GL4_LINKPROGRAM 189
#define GL4_PIXELSTOREI 190
#define GL4_POLYGONOFFSET 191
#define GL4_READPIXELS 192
#define GL4_RELEASESHADERCOMPILER 193
#define GL4_RENDERBUFFERSTORAGE 194
#define GL4_SAMPLECOVERAGE 195
#define GL4_SCISSOR 196
#define GL4_SHADERBINARY 197
#define GL4_SHADERSOURCE 198
#define GL4_STENCILFUNC 199
#define GL4_STENCILFUNCSEPARATE 200
#define GL4_STENCILMASK 201
#define GL4_STENCILMASKSEPARATE 202
#define GL4_STENCILOP 203
#define GL4_STENCILOPSEPARATE 204
#define GL4_TEXIMAGE2D 205
#define GL4_TEXPARAMETERF 206
#define GL4_TEXPARAMETERFV 207
#define GL4_TEXPARAMETERI 208
#define GL4_TEXPARAMETERIV 209
#define GL4_TEXSUBIMAGE2D 210
#define GL4_UNIFORM1F 211
#define GL4_UNIFORM1FV 212
#define GL4_UNIFORM1I 213
#define GL4_UNIFORM1IV 214
#define GL4_UNIFORM2F 215
#define GL4_UNIFORM2FV 216
#define GL4_UNIFORM2I 217
#define GL4_UNIFORM2IV 218
#define GL4_UNIFORM3F 219
#define GL4_UNIFORM3FV 220
#define GL4_UNIFORM3I 221
#define GL4_UNIFORM3IV 222
#define GL4_UNIFORM4F 223
#define GL4_UNIFORM4FV 224
#define GL4_UNIFORM4I 225
#define GL4_UNIFORM4IV 226
#define GL4_UNIFORMMATRIX2FV 227
#define GL4_UNIFORMMATRIX3FV 228
#define GL4_UNIFORMMATRIX4FV 229
#define GL4_USEPROGRAM 230
#define GL4_VALIDATEPROGRAM 231
#define GL4_VERTEXATTRIB1F 232
#define GL4_VERTEXATTRIB1FV 233
#define GL4_VERTEXATTRIB2F 234
#define GL4_VERTEXATTRIB2FV 235
#define GL4_VERTEXATTRIB3F 236
#define GL4_VERTEXATTRIB3FV 237
#define GL4_VERTEXATTRIB4F 238
#define GL4_VERTEXATTRIB4FV 239
#define GL4_VERTEXATTRIBPOINTER 240
#define GL4_VIEWPORT 241
#define GL4_GETPROGRAMBINARYOES 242
#define GL4_PROGRAMBINARYOES 243
#define GL4_TEXIMAGE3DOES 244
#define GL4_TEXSUBIMAGE3DOES 245
#define GL4_COPYSUBIMAGE3DOES 246
#define GL4_COMPRESSEDTEXIMAGE3DOES 247
#define GL4_COMPRESSEDTEXSUBIMAGE3DOES 248
#define GL4_FRAMEBUFFERTEXTURE3DOES 249
#define GL4_BINDVERTEXARRAYOES 250
#define GL4_GENVERTEXARRAYOES 251
#define GL4_ISVERTEXARRAYOES 252
#define GL4_DELETEVERTEXARRAYOES 253
#define GL4_DISCARDFRAMEBUFFEREXT 257

/* skip 6 counters for API statistics */

/* ES3.0 APIs */
#define GL4_READBUFFER 264
#define GL4_DRAWRANGEELEMENTS 265
#define GL4_TEXIMAGE3D 266
#define GL4_TEXSUBIMAGE3D 267
#define GL4_COPYTEXSUBIMAGE3D 268
#define GL4_COMPRESSEDTEXIMAGE3D 269
#define GL4_COMPRESSEDTEXSUBIMAGE3D 270
#define GL4_GENQUERIES 271
#define GL4_DELETEQUERIES 272
#define GL4_ISQUERY 273
#define GL4_BEGINQUERY 274
#define GL4_ENDQUERY 275
#define GL4_GETQUERYIV 276
#define GL4_GETQUERYOBJECTUIV 277
#define GL4_UNMAPBUFFER 278
#define GL4_GETBUFFERPOINTERV 279
#define GL4_DRAWBUFFERS 280
#define GL4_UNIFORMMATRIX2X3FV 281
#define GL4_UNIFORMMATRIX3X2FV 282
#define GL4_UNIFORMMATRIX2X4FV 283
#define GL4_UNIFORMMATRIX4X2FV 284
#define GL4_UNIFORMMATRIX3X4FV 285
#define GL4_UNIFORMMATRIX4X3FV 286
#define GL4_BLITFRAMEBUFFER 287
#define GL4_RENDERBUFFERSTORAGEMULTISAMPLE 288
#define GL4_FRAMEBUFFERTEXTURELAYER 289
#define GL4_MAPBUFFERRANGE 290
#define GL4_FLUSHMAPPEDBUFFERRANGE 291
#define GL4_BINDVERTEXARRAY 292
#define GL4_DELETEVERTEXARRAYS 293
#define GL4_GENVERTEXARRAYS 294
#define GL4_ISVERTEXARRAY 295
#define GL4_GETINTEGERI_V 296
#define GL4_BEGINTRANSFORMFEEDBACK 297
#define GL4_ENDTRANSFORMFEEDBACK 298
#define GL4_BINDBUFFERRANGE 299
#define GL4_BINDBUFFERBASE 300
#define GL4_TRANSFORMFEEDBACKVARYINGS 301
#define GL4_GETTRANSFORMFEEDBACKVARYING 302
#define GL4_VERTEXATTRIBIPOINTER 303
#define GL4_GETVERTEXATTRIBIIV 304
#define GL4_GETVERTEXATTRIBIUIV 305
#define GL4_VERTEXATTRIBI4I 306
#define GL4_VERTEXATTRIBI4UI 307
#define GL4_VERTEXATTRIBI4IV 308
#define GL4_VERTEXATTRIBI4UIV 309
#define GL4_GETUNIFORMUIV 310
#define GL4_GETFRAGDATALOCATION 311
#define GL4_UNIFORM1UI 312
#define GL4_UNIFORM2UI 313
#define GL4_UNIFORM3UI 314
#define GL4_UNIFORM4UI 315
#define GL4_UNIFORM1UIV 316
#define GL4_UNIFORM2UIV 317
#define GL4_UNIFORM3UIV 318
#define GL4_UNIFORM4UIV 319
#define GL4_CLEARBUFFERIV 320
#define GL4_CLEARBUFFERUIV 321
#define GL4_CLEARBUFFERFV 322
#define GL4_CLEARBUFFERFI 323
#define GL4_GETSTRINGI 324
#define GL4_COPYBUFFERSUBDATA 325
#define GL4_GETUNIFORMINDICES 326
#define GL4_GETACTIVEUNIFORMSIV 327
#define GL4_GETUNIFORMBLOCKINDEX 328
#define GL4_GETACTIVEUNIFORMBLOCKIV 329
#define GL4_GETACTIVEUNIFORMBLOCKNAME 330
#define GL4_UNIFORMBLOCKBINDING 331
#define GL4_DRAWARRAYSINSTANCED 332
#define GL4_DRAWELEMENTSINSTANCED 333
#define GL4_FENCESYNC 334
#define GL4_ISSYNC 335
#define GL4_DELETESYNC 336
#define GL4_CLIENTWAITSYNC 337
#define GL4_WAITSYNC 338
#define GL4_GETINTEGER64V 339
#define GL4_GETSYNCIV 340
#define GL4_GETINTEGER64I_V 341
#define GL4_GETBUFFERPARAMETERI64V 342
#define GL4_GENSAMPLERS 343
#define GL4_DELETESAMPLERS 344
#define GL4_ISSAMPLER 345
#define GL4_BINDSAMPLER 346
#define GL4_SAMPLERPARAMETERI 347
#define GL4_SAMPLERPARAMETERIV 348
#define GL4_SAMPLERPARAMETERF 349
#define GL4_SAMPLERPARAMETERFV 350
#define GL4_GETSAMPLERPARAMETERIV 351
#define GL4_GETSAMPLERPARAMETERFV 352
#define GL4_VERTEXATTRIBDIVISOR 353
#define GL4_BINDTRANSFORMFEEDBACK 354
#define GL4_DELETETRANSFORMFEEDBACKS 355
#define GL4_GENTRANSFORMFEEDBACKS 356
#define GL4_ISTRANSFORMFEEDBACK 357
#define GL4_PAUSETRANSFORMFEEDBACK 358
#define GL4_RESUMETRANSFORMFEEDBACK 359
#define GL4_GETPROGRAMBINARY 360
#define GL4_PROGRAMBINARY 361
#define GL4_PROGRAMPARAMETERI 362
#define GL4_INVALIDATEFRAMEBUFFER 363
#define GL4_INVALIDATESUBFRAMEBUFFER 364
#define GL4_TEXSTORAGE2D 365
#define GL4_TEXSTORAGE3D 366
#define GL4_GETINTERNALFORMATIV 367
#define GL4_EGLIMAGETARGETTEXTURE2DOES 368
#define GL4_EGLIMAGETARGETRENDERBUFFERSTORAGEOES 369
#define GL4_MULTIDRAWARRAYS 370
#define GL4_MULTIDRAWELEMENTS 371
#define GL4_FRAMEBUFFERTEXTURE2DMULTISAMPLEEXT 372
#define GL4_TEXDIRECTVIV 373
#define GL4_TEXDIRECTINVALIDATEVIV 374
#define GL4_TEXDIRECTVIVMAP 375
#define GL4_TEXDIRECTTILEDMAPVIV 376
#define GL4_GETGRAPHICSRESETSTATUS 377
#define GL4_READNPIXELS 378
#define GL4_GETNUNIFORMFV 379
#define GL4_GETNUNIFORMIV 380

#define GL4_TEMP 381
#define GL4_DISPATCHCOMPUTE 382
#define GL4_DISPATCHCOMPUTEINDIRECT 383
#define GL4_DRAWARRAYSINDIRECT 384
#define GL4_DRAWELEMENTSINDIRECT 385
#define GL4_MULTIDRAWARRAYSINDIRECT 386
#define GL4_MULTIDRAWELEMENTSINDIRECT 387
#define GL4_DRAWELEMENTSBASEVERTEX 388
#define GL4_DRAWRANGEELEMENTSBASEVERTEX 389
#define GL4_DRAWELEMENTSINSTANCEDBASEVERTEX 390
#define GL4_MULTIDRAWELEMENTSBASEVERTEX 391
#define GL4_FRAMEBUFFERPARAMETERI 392
#define GL4_GETFRAMEBUFFERPARAMETERIV 393
#define GL4_GETPROGRAMINTERFACEIV 394
#define GL4_GETPROGRAMRESOURCEINDEX 395
#define GL4_GETPROGRAMRESOURCENAME 396
#define GL4_GETPROGRAMRESOURCEIV 397
#define GL4_GETPROGRAMRESOURCELOCATION 398
#define GL4_USEPROGRAMSTAGES 399
#define GL4_ACTIVESHADERPROGRAM 400
#define GL4_CREATESHADERPROGRAMV 401
#define GL4_BINDPROGRAMPIPELINE 402
#define GL4_DELETEPROGRAMPIPELINES 403
#define GL4_GENPROGRAMPIPELINES 404
#define GL4_ISPROGRAMPIPELINE 405
#define GL4_GETPROGRAMPIPELINEIV 406
#define GL4_PROGRAMUNIFORM1I 407
#define GL4_PROGRAMUNIFORM2I 408
#define GL4_PROGRAMUNIFORM3I 409
#define GL4_PROGRAMUNIFORM4I 410
#define GL4_PROGRAMUNIFORM1UI 411
#define GL4_PROGRAMUNIFORM2UI 412
#define GL4_PROGRAMUNIFORM3UI 413
#define GL4_PROGRAMUNIFORM4UI 414
#define GL4_PROGRAMUNIFORM1F 415
#define GL4_PROGRAMUNIFORM2F 416
#define GL4_PROGRAMUNIFORM3F 417
#define GL4_PROGRAMUNIFORM4F 418
#define GL4_PROGRAMUNIFORM1IV 419
#define GL4_PROGRAMUNIFORM2IV 420
#define GL4_PROGRAMUNIFORM3IV 421
#define GL4_PROGRAMUNIFORM4IV 422
#define GL4_PROGRAMUNIFORM1UIV 423
#define GL4_PROGRAMUNIFORM2UIV 424
#define GL4_PROGRAMUNIFORM3UIV 425
#define GL4_PROGRAMUNIFORM4UIV 426
#define GL4_PROGRAMUNIFORM1FV 427
#define GL4_PROGRAMUNIFORM2FV 428
#define GL4_PROGRAMUNIFORM3FV 429
#define GL4_PROGRAMUNIFORM4FV 430
#define GL4_PROGRAMUNIFORMMATRIX2FV 431
#define GL4_PROGRAMUNIFORMMATRIX3FV 432
#define GL4_PROGRAMUNIFORMMATRIX4FV 433
#define GL4_PROGRAMUNIFORMMATRIX2X3FV 434
#define GL4_PROGRAMUNIFORMMATRIX3X2FV 435
#define GL4_PROGRAMUNIFORMMATRIX2X4FV 436
#define GL4_PROGRAMUNIFORMMATRIX4X2FV 437
#define GL4_PROGRAMUNIFORMMATRIX3X4FV 438
#define GL4_PROGRAMUNIFORMMATRIX4X3FV 439
#define GL4_VALIDATEPROGRAMPIPELINE 440
#define GL4_GETPROGRAMPIPELINEINFOLOG 441
#define GL4_BINDIMAGETEXTURE 442
#define GL4_GETBOOLEANI_V 443
#define GL4_MEMORYBARRIER 444
#define GL4_MEMORYBARRIERBYREGION 445
#define GL4_TEXSTORAGE2DMULTISAMPLE 446
#define GL4_GETMULTISAMPLEFV 447
#define GL4_SAMPLEMASKI 448
#define GL4_GETTEXLEVELPARAMETERIV 449
#define GL4_GETTEXLEVELPARAMETERFV 450
#define GL4_BINDVERTEXBUFFER 451
#define GL4_VERTEXATTRIBFORMAT 452
#define GL4_VERTEXATTRIBIFORMAT 453
#define GL4_VERTEXATTRIBBINDING 454
#define GL4_VERTEXBINDINGDIVISOR 455
#define GL4_TEXSTORAGE3DMULTISAMPLE 456
#define GL4_BLENDBARRIER 457
#define GL4_DEBUGMESSAGECONTROL 458
#define GL4_DEBUGMESSAGEINSERT 459
#define GL4_DEBUGMESSAGECALLBACK 460
#define GL4_GETDEBUGMESSAGELOG 461
#define GL4_GETPOINTERV 462
#define GL4_PUSHDEBUGGROUP 463
#define GL4_POPDEBUGGROUP 464
#define GL4_OBJECTLABEL 465
#define GL4_GETOBJECTLABEL 466
#define GL4_OBJECTPTRLABEL 467
#define GL4_GETOBJECTPTRLABEL 468
#define GL4_BLENDEQUATIONI 469
#define GL4_BLENDEQUATIONSEPARATEI 470
#define GL4_BLENDFUNCI 471
#define GL4_BLENDFUNCSEPARATEI 472
#define GL4_COLORMASKI 473
#define GL4_ENABLEI 474
#define GL4_DISABLEI 475
#define GL4_ISENABLEDI 476
#define GL4_TEXPARAMETERIIV 477
#define GL4_TEXPARAMETERIUIV 478
#define GL4_GETTEXPARAMETERIIV 479
#define GL4_GETTEXPARAMETERIUIV 480
#define GL4_SAMPLERPARAMETERIIV 481
#define GL4_SAMPLERPARAMETERIUIV 482
#define GL4_GETSAMPLERPARAMETERIIV 483
#define GL4_GETSAMPLERPARAMETERIUIV 484
#define GL4_TEXBUFFER 485
#define GL4_TEXBUFFERRANGE 486
#define GL4_PATCHPARAMETERI 487
#define GL4_FRAMEBUFFERTEXTURE 488
#define GL4_MINSAMPLESHADING 489
#define GL4_COPYIMAGESUBDATA 490
#define GL4_GETNUNIFORMUIV    491
#define GL4_GETTEXIMAGE       492

#define GL4_NUM_API_CALLS     (GL4_GETNUNIFORMUIV + 1 - 100)

#define GL3_PROFILER_FRAME_END 10
#define GL3_PROFILER_FRAME_TYPE 11
#define GL3_PROFILER_FRAME_COUNT 12
#define GL3_PROFILER_FINISH_BEGIN 13
#define GL3_PROFILER_FINISH_END 14

#define GL3_PROFILER_PRIMITIVE_END 20
#define GL3_PROFILER_PRIMITIVE_TYPE 21
#define GL3_PROFILER_PRIMITIVE_COUNT 22

#define GL3_TEXUPLOAD_SIZE 30

#define GL3_PROFILER_DRAW_BEGIN    40
#define GL3_PROFILER_DRAW_END      41
#define GL3_FBO_USE                42

#define GL3_COMPILER_SHADER_LENGTH 50
#define GL3_COMPILER_START_TIME 51
#define GL3_COMPILER_END_TIME 52

#define GL3_SHADER_OBJECT 60

#define GL3_PROGRAM_IN_USE_BEGIN 70
#define GL3_PROGRAM_IN_USE_END 71
#define GL3_PROGRAM_ATTRIB_CNT 72
#define GL3_PROGRAM_UNIFORM_CNT 73
#define GL3_PROGRAM_VERTEX_SHADER 74
#define GL3_PROGRAM_FRAGMENT_SHADER 75
#define GL3_PROGRAM_LINK_FUNCTION 76

#define GL3_PROFILER_COMPUTE_BEGIN 80
#define GL3_PROFILER_COMPUTE_END 81

#define GL3_PROFILER_WRITE_HEADER 90
#define GL3_PROFILER_WRITE_FRAME_BEGIN 91
#define GL3_PROFILER_WRITE_FRAME_END 92
#define GL3_PROFILER_WRITE_FRAME_RESET 93
/* Profile information. */
typedef struct _glsPROFILER
{
    gctBOOL         enable;
    gctBOOL         useGlfinish;
    gctBOOL         perDrawMode;
    gctBOOL         enableOutputCounters;  /* for VIV_PROFILE = 2 */
    gctBOOL         writeDrawable;

    gctBOOL         need_dump;
    gctBOOL       frameBegun;
    gctUINT32       frameCount;       /* for VIV_PROFILE = 1 */
    gctUINT32       frameStartNumber; /* for VIV_PROFILE = 3 */
    gctUINT32       frameEndNumber;   /* for VIV_PROFILE = 3 */
    gctUINT32       curFrameNumber;

    /* Current frame information */
    gctUINT32       frameNumber;
    gctUINT32       finishNumber;
    gctUINT64       frameStartTimeusec;
    gctUINT64       frameEndTimeusec;

    gctUINT32       drawPointCount;
    gctUINT32       drawLineCount;
    gctUINT32       drawTriangleCount;

    /* Current primitive information */
    gctUINT32       primitiveNumber;
    gctUINT32       primitiveType;
    gctUINT32       primitiveCount;

    gctUINT32       apiCalls[800];
    gctUINT64       apiTimes[800];
    gctUINT64       totalDriverTime;

    gctUINT32       drawCount;

}
glsPROFILER;

extern gctBOOL __glProfiler(gctPOINTER Profiler, gctUINT32 Enum, gctHANDLE Value);

#endif /* __gc_es_profiler_h__ */

