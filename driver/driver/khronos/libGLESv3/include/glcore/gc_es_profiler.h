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

#define GLES3_APICALLBASE 100

/* ES2.0 old APIs */
#define GLES3_ACTIVETEXTURE 100
#define GLES3_ATTACHSHADER 101
#define GLES3_BINDATTRIBLOCATION 102
#define GLES3_BINDBUFFER 103
#define GLES3_BINDFRAMEBUFFER 104
#define GLES3_BINDRENDERBUFFER 105
#define GLES3_BINDTEXTURE 106
#define GLES3_BLENDCOLOR 107
#define GLES3_BLENDEQUATION 108
#define GLES3_BLENDEQUATIONSEPARATE 109
#define GLES3_BLENDFUNC 110
#define GLES3_BLENDFUNCSEPARATE 111
#define GLES3_BUFFERDATA 112
#define GLES3_BUFFERSUBDATA 113
#define GLES3_CHECKFRAMEBUFFERSTATUS 114
#define GLES3_CLEAR 115
#define GLES3_CLEARCOLOR 116
#define GLES3_CLEARDEPTHF 117
#define GLES3_CLEARSTENCIL 118
#define GLES3_COLORMASK 119
#define GLES3_COMPILESHADER 120
#define GLES3_COMPRESSEDTEXIMAGE2D 121
#define GLES3_COMPRESSEDTEXSUBIMAGE2D 122
#define GLES3_COPYTEXIMAGE2D 123
#define GLES3_COPYTEXSUBIMAGE2D 124
#define GLES3_CREATEPROGRAM 125
#define GLES3_CREATESHADER 126
#define GLES3_CULLFACE 127
#define GLES3_DELETEBUFFERS 128
#define GLES3_DELETEFRAMEBUFFERS 129
#define GLES3_DELETEPROGRAM 130
#define GLES3_DELETERENDERBUFFERS 131
#define GLES3_DELETESHADER 132
#define GLES3_DELETETEXTURES 133
#define GLES3_DEPTHFUNC 134
#define GLES3_DEPTHMASK 135
#define GLES3_DEPTHRANGEF 136
#define GLES3_DETACHSHADER 137
#define GLES3_DISABLE 138
#define GLES3_DISABLEVERTEXATTRIBARRAY 139
#define GLES3_DRAWARRAYS 140
#define GLES3_DRAWELEMENTS 141
#define GLES3_ENABLE 142
#define GLES3_ENABLEVERTEXATTRIBARRAY 143
#define GLES3_FINISH 144
#define GLES3_FLUSH 145
#define GLES3_FRAMEBUFFERRENDERBUFFER 146
#define GLES3_FRAMEBUFFERTEXTURE2D 147
#define GLES3_FRONTFACE 148
#define GLES3_GENBUFFERS 149
#define GLES3_GENERATEMIPMAP 150
#define GLES3_GENFRAMEBUFFERS 151
#define GLES3_GENRENDERBUFFERS 152
#define GLES3_GENTEXTURES 153
#define GLES3_GETACTIVEATTRIB 154
#define GLES3_GETACTIVEUNIFORM 155
#define GLES3_GETATTACHEDSHADERS 156
#define GLES3_GETATTRIBLOCATION 157
#define GLES3_GETBOOLEANV 158
#define GLES3_GETBUFFERPARAMETERIV 159
#define GLES3_GETERROR 160
#define GLES3_GETFLOATV 161
#define GLES3_GETFRAMEBUFFERATTACHMENTPARAMETERIV 162
#define GLES3_GETINTEGERV 163
#define GLES3_GETPROGRAMIV 164
#define GLES3_GETPROGRAMINFOLOG 165
#define GLES3_GETRENDERBUFFERPARAMETERIV 166
#define GLES3_GETSHADERIV 167
#define GLES3_GETSHADERINFOLOG 168
#define GLES3_GETSHADERPRECISIONFORMAT 169
#define GLES3_GETSHADERSOURCE 170
#define GLES3_GETSTRING 171
#define GLES3_GETTEXPARAMETERFV 172
#define GLES3_GETTEXPARAMETERIV 173
#define GLES3_GETUNIFORMFV 174
#define GLES3_GETUNIFORMIV 175
#define GLES3_GETUNIFORMLOCATION 176
#define GLES3_GETVERTEXATTRIBFV 177
#define GLES3_GETVERTEXATTRIBIV 178
#define GLES3_GETVERTEXATTRIBPOINTERV 179
#define GLES3_HINT 180
#define GLES3_ISBUFFER 181
#define GLES3_ISENABLED 182
#define GLES3_ISFRAMEBUFFER 183
#define GLES3_ISPROGRAM 184
#define GLES3_ISRENDERBUFFER 185
#define GLES3_ISSHADER 186
#define GLES3_ISTEXTURE 187
#define GLES3_LINEWIDTH 188
#define GLES3_LINKPROGRAM 189
#define GLES3_PIXELSTOREI 190
#define GLES3_POLYGONOFFSET 191
#define GLES3_READPIXELS 192
#define GLES3_RELEASESHADERCOMPILER 193
#define GLES3_RENDERBUFFERSTORAGE 194
#define GLES3_SAMPLECOVERAGE 195
#define GLES3_SCISSOR 196
#define GLES3_SHADERBINARY 197
#define GLES3_SHADERSOURCE 198
#define GLES3_STENCILFUNC 199
#define GLES3_STENCILFUNCSEPARATE 200
#define GLES3_STENCILMASK 201
#define GLES3_STENCILMASKSEPARATE 202
#define GLES3_STENCILOP 203
#define GLES3_STENCILOPSEPARATE 204
#define GLES3_TEXIMAGE2D 205
#define GLES3_TEXPARAMETERF 206
#define GLES3_TEXPARAMETERFV 207
#define GLES3_TEXPARAMETERI 208
#define GLES3_TEXPARAMETERIV 209
#define GLES3_TEXSUBIMAGE2D 210
#define GLES3_UNIFORM1F 211
#define GLES3_UNIFORM1FV 212
#define GLES3_UNIFORM1I 213
#define GLES3_UNIFORM1IV 214
#define GLES3_UNIFORM2F 215
#define GLES3_UNIFORM2FV 216
#define GLES3_UNIFORM2I 217
#define GLES3_UNIFORM2IV 218
#define GLES3_UNIFORM3F 219
#define GLES3_UNIFORM3FV 220
#define GLES3_UNIFORM3I 221
#define GLES3_UNIFORM3IV 222
#define GLES3_UNIFORM4F 223
#define GLES3_UNIFORM4FV 224
#define GLES3_UNIFORM4I 225
#define GLES3_UNIFORM4IV 226
#define GLES3_UNIFORMMATRIX2FV 227
#define GLES3_UNIFORMMATRIX3FV 228
#define GLES3_UNIFORMMATRIX4FV 229
#define GLES3_USEPROGRAM 230
#define GLES3_VALIDATEPROGRAM 231
#define GLES3_VERTEXATTRIB1F 232
#define GLES3_VERTEXATTRIB1FV 233
#define GLES3_VERTEXATTRIB2F 234
#define GLES3_VERTEXATTRIB2FV 235
#define GLES3_VERTEXATTRIB3F 236
#define GLES3_VERTEXATTRIB3FV 237
#define GLES3_VERTEXATTRIB4F 238
#define GLES3_VERTEXATTRIB4FV 239
#define GLES3_VERTEXATTRIBPOINTER 240
#define GLES3_VIEWPORT 241
#define GLES3_GETPROGRAMBINARYOES 242
#define GLES3_PROGRAMBINARYOES 243
#define GLES3_TEXIMAGE3DOES 244
#define GLES3_TEXSUBIMAGE3DOES 245
#define GLES3_COPYSUBIMAGE3DOES 246
#define GLES3_COMPRESSEDTEXIMAGE3DOES 247
#define GLES3_COMPRESSEDTEXSUBIMAGE3DOES 248
#define GLES3_FRAMEBUFFERTEXTURE3DOES 249
#define GLES3_BINDVERTEXARRAYOES 250
#define GLES3_GENVERTEXARRAYOES 251
#define GLES3_ISVERTEXARRAYOES 252
#define GLES3_DELETEVERTEXARRAYOES 253
#define GLES3_MAPBUFFEROES 254
#define GLES3_UNMAPBUFFEROES 255
#define GLES3_GETBUFFERPOINTERVOES 256
#define GLES3_DISCARDFRAMEBUFFEREXT 257

/* skip 6 counters for API statistics */

/* ES3.0 APIs */
#define GLES3_READBUFFER 264
#define GLES3_DRAWRANGEELEMENTS 265
#define GLES3_TEXIMAGE3D 266
#define GLES3_TEXSUBIMAGE3D 267
#define GLES3_COPYTEXSUBIMAGE3D 268
#define GLES3_COMPRESSEDTEXIMAGE3D 269
#define GLES3_COMPRESSEDTEXSUBIMAGE3D 270
#define GLES3_GENQUERIES 271
#define GLES3_DELETEQUERIES 272
#define GLES3_ISQUERY 273
#define GLES3_BEGINQUERY 274
#define GLES3_ENDQUERY 275
#define GLES3_GETQUERYIV 276
#define GLES3_GETQUERYOBJECTUIV 277
#define GLES3_UNMAPBUFFER 278
#define GLES3_GETBUFFERPOINTERV 279
#define GLES3_DRAWBUFFERS 280
#define GLES3_UNIFORMMATRIX2X3FV 281
#define GLES3_UNIFORMMATRIX3X2FV 282
#define GLES3_UNIFORMMATRIX2X4FV 283
#define GLES3_UNIFORMMATRIX4X2FV 284
#define GLES3_UNIFORMMATRIX3X4FV 285
#define GLES3_UNIFORMMATRIX4X3FV 286
#define GLES3_BLITFRAMEBUFFER 287
#define GLES3_RENDERBUFFERSTORAGEMULTISAMPLE 288
#define GLES3_FRAMEBUFFERTEXTURELAYER 289
#define GLES3_MAPBUFFERRANGE 290
#define GLES3_FLUSHMAPPEDBUFFERRANGE 291
#define GLES3_BINDVERTEXARRAY 292
#define GLES3_DELETEVERTEXARRAYS 293
#define GLES3_GENVERTEXARRAYS 294
#define GLES3_ISVERTEXARRAY 295
#define GLES3_GETINTEGERI_V 296
#define GLES3_BEGINTRANSFORMFEEDBACK 297
#define GLES3_ENDTRANSFORMFEEDBACK 298
#define GLES3_BINDBUFFERRANGE 299
#define GLES3_BINDBUFFERBASE 300
#define GLES3_TRANSFORMFEEDBACKVARYINGS 301
#define GLES3_GETTRANSFORMFEEDBACKVARYING 302
#define GLES3_VERTEXATTRIBIPOINTER 303
#define GLES3_GETVERTEXATTRIBIIV 304
#define GLES3_GETVERTEXATTRIBIUIV 305
#define GLES3_VERTEXATTRIBI4I 306
#define GLES3_VERTEXATTRIBI4UI 307
#define GLES3_VERTEXATTRIBI4IV 308
#define GLES3_VERTEXATTRIBI4UIV 309
#define GLES3_GETUNIFORMUIV 310
#define GLES3_GETFRAGDATALOCATION 311
#define GLES3_UNIFORM1UI 312
#define GLES3_UNIFORM2UI 313
#define GLES3_UNIFORM3UI 314
#define GLES3_UNIFORM4UI 315
#define GLES3_UNIFORM1UIV 316
#define GLES3_UNIFORM2UIV 317
#define GLES3_UNIFORM3UIV 318
#define GLES3_UNIFORM4UIV 319
#define GLES3_CLEARBUFFERIV 320
#define GLES3_CLEARBUFFERUIV 321
#define GLES3_CLEARBUFFERFV 322
#define GLES3_CLEARBUFFERFI 323
#define GLES3_GETSTRINGI 324
#define GLES3_COPYBUFFERSUBDATA 325
#define GLES3_GETUNIFORMINDICES 326
#define GLES3_GETACTIVEUNIFORMSIV 327
#define GLES3_GETUNIFORMBLOCKINDEX 328
#define GLES3_GETACTIVEUNIFORMBLOCKIV 329
#define GLES3_GETACTIVEUNIFORMBLOCKNAME 330
#define GLES3_UNIFORMBLOCKBINDING 331
#define GLES3_DRAWARRAYSINSTANCED 332
#define GLES3_DRAWELEMENTSINSTANCED 333
#define GLES3_FENCESYNC 334
#define GLES3_ISSYNC 335
#define GLES3_DELETESYNC 336
#define GLES3_CLIENTWAITSYNC 337
#define GLES3_WAITSYNC 338
#define GLES3_GETINTEGER64V 339
#define GLES3_GETSYNCIV 340
#define GLES3_GETINTEGER64I_V 341
#define GLES3_GETBUFFERPARAMETERI64V 342
#define GLES3_GENSAMPLERS 343
#define GLES3_DELETESAMPLERS 344
#define GLES3_ISSAMPLER 345
#define GLES3_BINDSAMPLER 346
#define GLES3_SAMPLERPARAMETERI 347
#define GLES3_SAMPLERPARAMETERIV 348
#define GLES3_SAMPLERPARAMETERF 349
#define GLES3_SAMPLERPARAMETERFV 350
#define GLES3_GETSAMPLERPARAMETERIV 351
#define GLES3_GETSAMPLERPARAMETERFV 352
#define GLES3_VERTEXATTRIBDIVISOR 353
#define GLES3_BINDTRANSFORMFEEDBACK 354
#define GLES3_DELETETRANSFORMFEEDBACKS 355
#define GLES3_GENTRANSFORMFEEDBACKS 356
#define GLES3_ISTRANSFORMFEEDBACK 357
#define GLES3_PAUSETRANSFORMFEEDBACK 358
#define GLES3_RESUMETRANSFORMFEEDBACK 359
#define GLES3_GETPROGRAMBINARY 360
#define GLES3_PROGRAMBINARY 361
#define GLES3_PROGRAMPARAMETERI 362
#define GLES3_INVALIDATEFRAMEBUFFER 363
#define GLES3_INVALIDATESUBFRAMEBUFFER 364
#define GLES3_TEXSTORAGE2D 365
#define GLES3_TEXSTORAGE3D 366
#define GLES3_GETINTERNALFORMATIV 367
#define GLES3_EGLIMAGETARGETTEXTURE2DOES 368
#define GLES3_EGLIMAGETARGETRENDERBUFFERSTORAGEOES 369
#define GLES3_MULTIDRAWARRAYSEXT 370
#define GLES3_MULTIDRAWELEMENTSEXT 371
#define GLES3_FRAMEBUFFERTEXTURE2DMULTISAMPLEEXT 372
#define GLES3_TEXDIRECTVIV 373
#define GLES3_TEXDIRECTINVALIDATEVIV 374
#define GLES3_TEXDIRECTVIVMAP 375
#define GLES3_TEXDIRECTTILEDMAPVIV 376
#define GLES3_GETGRAPHICSRESETSTATUS 377
#define GLES3_READNPIXELS 378
#define GLES3_GETNUNIFORMFV 379
#define GLES3_GETNUNIFORMIV 380

#define GLES31_TEMP 381
#define GLES31_DISPATCHCOMPUTE 382
#define GLES31_DISPATCHCOMPUTEINDIRECT 383
#define GLES31_DRAWARRAYSINDIRECT 384
#define GLES31_DRAWELEMENTSINDIRECT 385
#define GLES31_MULTIDRAWARRAYSINDIRECTEXT 386
#define GLES31_MULTIDRAWELEMENTSINDIRECTEXT 387
#define GLES31_DRAWELEMENTSBASEVERTEX 388
#define GLES31_DRAWRANGEELEMENTSBASEVERTEX 389
#define GLES31_DRAWELEMENTSINSTANCEDBASEVERTEX 390
#define GLES31_MULTIDRAWELEMENTSBASEVERTEXEXT 391
#define GLES31_FRAMEBUFFERPARAMETERI 392
#define GLES31_GETFRAMEBUFFERPARAMETERIV 393
#define GLES31_GETPROGRAMINTERFACEIV 394
#define GLES31_GETPROGRAMRESOURCEINDEX 395
#define GLES31_GETPROGRAMRESOURCENAME 396
#define GLES31_GETPROGRAMRESOURCEIV 397
#define GLES31_GETPROGRAMRESOURCELOCATION 398
#define GLES31_USEPROGRAMSTAGES 399
#define GLES31_ACTIVESHADERPROGRAM 400
#define GLES31_CREATESHADERPROGRAMV 401
#define GLES31_BINDPROGRAMPIPELINE 402
#define GLES31_DELETEPROGRAMPIPELINES 403
#define GLES31_GENPROGRAMPIPELINES 404
#define GLES31_ISPROGRAMPIPELINE 405
#define GLES31_GETPROGRAMPIPELINEIV 406
#define GLES31_PROGRAMUNIFORM1I 407
#define GLES31_PROGRAMUNIFORM2I 408
#define GLES31_PROGRAMUNIFORM3I 409
#define GLES31_PROGRAMUNIFORM4I 410
#define GLES31_PROGRAMUNIFORM1UI 411
#define GLES31_PROGRAMUNIFORM2UI 412
#define GLES31_PROGRAMUNIFORM3UI 413
#define GLES31_PROGRAMUNIFORM4UI 414
#define GLES31_PROGRAMUNIFORM1F 415
#define GLES31_PROGRAMUNIFORM2F 416
#define GLES31_PROGRAMUNIFORM3F 417
#define GLES31_PROGRAMUNIFORM4F 418
#define GLES31_PROGRAMUNIFORM1IV 419
#define GLES31_PROGRAMUNIFORM2IV 420
#define GLES31_PROGRAMUNIFORM3IV 421
#define GLES31_PROGRAMUNIFORM4IV 422
#define GLES31_PROGRAMUNIFORM1UIV 423
#define GLES31_PROGRAMUNIFORM2UIV 424
#define GLES31_PROGRAMUNIFORM3UIV 425
#define GLES31_PROGRAMUNIFORM4UIV 426
#define GLES31_PROGRAMUNIFORM1FV 427
#define GLES31_PROGRAMUNIFORM2FV 428
#define GLES31_PROGRAMUNIFORM3FV 429
#define GLES31_PROGRAMUNIFORM4FV 430
#define GLES31_PROGRAMUNIFORMMATRIX2FV 431
#define GLES31_PROGRAMUNIFORMMATRIX3FV 432
#define GLES31_PROGRAMUNIFORMMATRIX4FV 433
#define GLES31_PROGRAMUNIFORMMATRIX2X3FV 434
#define GLES31_PROGRAMUNIFORMMATRIX3X2FV 435
#define GLES31_PROGRAMUNIFORMMATRIX2X4FV 436
#define GLES31_PROGRAMUNIFORMMATRIX4X2FV 437
#define GLES31_PROGRAMUNIFORMMATRIX3X4FV 438
#define GLES31_PROGRAMUNIFORMMATRIX4X3FV 439
#define GLES31_VALIDATEPROGRAMPIPELINE 440
#define GLES31_GETPROGRAMPIPELINEINFOLOG 441
#define GLES31_BINDIMAGETEXTURE 442
#define GLES31_GETBOOLEANI_V 443
#define GLES31_MEMORYBARRIER 444
#define GLES31_MEMORYBARRIERBYREGION 445
#define GLES31_TEXSTORAGE2DMULTISAMPLE 446
#define GLES31_GETMULTISAMPLEFV 447
#define GLES31_SAMPLEMASKI 448
#define GLES31_GETTEXLEVELPARAMETERIV 449
#define GLES31_GETTEXLEVELPARAMETERFV 450
#define GLES31_BINDVERTEXBUFFER 451
#define GLES31_VERTEXATTRIBFORMAT 452
#define GLES31_VERTEXATTRIBIFORMAT 453
#define GLES31_VERTEXATTRIBBINDING 454
#define GLES31_VERTEXBINDINGDIVISOR 455
#define GLES31_TEXSTORAGE3DMULTISAMPLE 456
#define GLES31_BLENDBARRIER 457
#define GLES31_DEBUGMESSAGECONTROL 458
#define GLES31_DEBUGMESSAGEINSERT 459
#define GLES31_DEBUGMESSAGECALLBACK 460
#define GLES31_GETDEBUGMESSAGELOG 461
#define GLES31_GETPOINTERV 462
#define GLES31_PUSHDEBUGGROUP 463
#define GLES31_POPDEBUGGROUP 464
#define GLES31_OBJECTLABEL 465
#define GLES31_GETOBJECTLABEL 466
#define GLES31_OBJECTPTRLABEL 467
#define GLES31_GETOBJECTPTRLABEL 468
#define GLES31_BLENDEQUATIONI 469
#define GLES31_BLENDEQUATIONSEPARATEI 470
#define GLES31_BLENDFUNCI 471
#define GLES31_BLENDFUNCSEPARATEI 472
#define GLES31_COLORMASKI 473
#define GLES31_ENABLEI 474
#define GLES31_DISABLEI 475
#define GLES31_ISENABLEDI 476
#define GLES31_TEXPARAMETERIIV 477
#define GLES31_TEXPARAMETERIUIV 478
#define GLES31_GETTEXPARAMETERIIV 479
#define GLES31_GETTEXPARAMETERIUIV 480
#define GLES31_SAMPLERPARAMETERIIV 481
#define GLES31_SAMPLERPARAMETERIUIV 482
#define GLES31_GETSAMPLERPARAMETERIIV 483
#define GLES31_GETSAMPLERPARAMETERIUIV 484
#define GLES31_TEXBUFFER 485
#define GLES31_TEXBUFFERRANGE 486
#define GLES31_PATCHPARAMETERI 487
#define GLES31_FRAMEBUFFERTEXTURE 488
#define GLES31_MINSAMPLESHADING 489
#define GLES31_COPYIMAGESUBDATA 490
#define GLES3_GETNUNIFORMUIV    491

#define GLES3_NUM_API_CALLS     (GLES3_GETNUNIFORMUIV + 1 - 100)

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
    gctBOOL         frameBegun;
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

    gctUINT32       apiCalls[GLES3_NUM_API_CALLS];
    gctUINT64       apiTimes[GLES3_NUM_API_CALLS];
    gctUINT64       totalDriverTime;

    gctUINT32       drawCount;

}
glsPROFILER;

extern gctBOOL __glProfiler(gctPOINTER Profiler, gctUINT32 Enum, gctHANDLE Value);

#endif /* __gc_es_profiler_h__ */
