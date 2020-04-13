/****************************************************************************
*
*    Copyright (c) 2005 - 2019 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/



#define __GLES11_API_ENTRIES(esApiMacro) \
    esApiMacro(ActiveTexture), \
    esApiMacro(AlphaFuncx), \
    esApiMacro(AlphaFuncxOES), \
    esApiMacro(BindBuffer), \
    esApiMacro(BindFramebufferOES), \
    esApiMacro(BindRenderbufferOES), \
    esApiMacro(BindTexture), \
    esApiMacro(BlendFunc), \
    esApiMacro(BlendFuncSeparateOES), \
    esApiMacro(BlendEquationOES), \
    esApiMacro(BlendEquationSeparateOES), \
    esApiMacro(BufferData), \
    esApiMacro(BufferSubData), \
    esApiMacro(CheckFramebufferStatusOES), \
    esApiMacro(Clear), \
    esApiMacro(ClearColorx), \
    esApiMacro(ClearColorxOES), \
    esApiMacro(ClearDepthx), \
    esApiMacro(ClearDepthxOES), \
    esApiMacro(ClearStencil), \
    esApiMacro(ClientActiveTexture), \
    esApiMacro(ClipPlanex), \
    esApiMacro(ClipPlanexOES), \
    esApiMacro(ClipPlanexIMG), \
    esApiMacro(Color4ub), \
    esApiMacro(Color4x), \
    esApiMacro(Color4xOES), \
    esApiMacro(ColorMask), \
    esApiMacro(ColorPointer), \
    esApiMacro(CompressedTexImage2D), \
    esApiMacro(CompressedTexSubImage2D), \
    esApiMacro(CopyTexImage2D), \
    esApiMacro(CopyTexSubImage2D), \
    esApiMacro(CullFace), \
    esApiMacro(CurrentPaletteMatrixOES), \
    esApiMacro(DeleteBuffers), \
    esApiMacro(DeleteFramebuffersOES), \
    esApiMacro(DeleteRenderbuffersOES), \
    esApiMacro(DeleteTextures), \
    esApiMacro(DepthFunc), \
    esApiMacro(DepthMask), \
    esApiMacro(DepthRangex), \
    esApiMacro(DepthRangexOES), \
    esApiMacro(Disable), \
    esApiMacro(DisableClientState), \
    esApiMacro(DrawArrays), \
    esApiMacro(DrawElements), \
    esApiMacro(DrawTexiOES), \
    esApiMacro(DrawTexivOES), \
    esApiMacro(DrawTexsOES), \
    esApiMacro(DrawTexsvOES), \
    esApiMacro(DrawTexxOES), \
    esApiMacro(DrawTexxvOES), \
    esApiMacro(EGLImageTargetRenderbufferStorageOES), \
    esApiMacro(EGLImageTargetTexture2DOES), \
    esApiMacro(Enable), \
    esApiMacro(EnableClientState), \
    esApiMacro(Finish), \
    esApiMacro(Flush), \
    esApiMacro(Fogx), \
    esApiMacro(FogxOES), \
    esApiMacro(Fogxv), \
    esApiMacro(FogxvOES), \
    esApiMacro(FramebufferRenderbufferOES), \
    esApiMacro(FramebufferTexture2DOES), \
    esApiMacro(FrontFace), \
    esApiMacro(Frustumx), \
    esApiMacro(FrustumxOES), \
    esApiMacro(GenBuffers), \
    esApiMacro(GenerateMipmapOES), \
    esApiMacro(GenFramebuffersOES), \
    esApiMacro(GenRenderbuffersOES), \
    esApiMacro(GenTextures), \
    esApiMacro(GetBooleanv), \
    esApiMacro(GetBufferParameteriv), \
    esApiMacro(GetClipPlanex), \
    esApiMacro(GetClipPlanexOES), \
    esApiMacro(GetError), \
    esApiMacro(GetFixedv), \
    esApiMacro(GetFixedvOES), \
    esApiMacro(GetFramebufferAttachmentParameterivOES), \
    esApiMacro(GetIntegerv), \
    esApiMacro(GetLightxv), \
    esApiMacro(GetLightxvOES), \
    esApiMacro(GetMaterialxv), \
    esApiMacro(GetMaterialxvOES), \
    esApiMacro(GetPointerv), \
    esApiMacro(GetRenderbufferParameterivOES), \
    esApiMacro(GetString), \
    esApiMacro(GetTexEnviv), \
    esApiMacro(GetTexEnvxv), \
    esApiMacro(GetTexEnvxvOES), \
    esApiMacro(GetTexGenivOES), \
    esApiMacro(GetTexGenxvOES), \
    esApiMacro(GetTexParameteriv), \
    esApiMacro(GetTexParameterxv), \
    esApiMacro(GetTexParameterxvOES), \
    esApiMacro(Hint), \
    esApiMacro(IsBuffer), \
    esApiMacro(IsEnabled), \
    esApiMacro(IsFramebufferOES), \
    esApiMacro(IsRenderbufferOES), \
    esApiMacro(IsTexture), \
    esApiMacro(LightModelx), \
    esApiMacro(LightModelxOES), \
    esApiMacro(LightModelxv), \
    esApiMacro(LightModelxvOES), \
    esApiMacro(Lightx), \
    esApiMacro(LightxOES), \
    esApiMacro(Lightxv), \
    esApiMacro(LightxvOES), \
    esApiMacro(LineWidthx), \
    esApiMacro(LineWidthxOES), \
    esApiMacro(LoadIdentity), \
    esApiMacro(LoadMatrixx), \
    esApiMacro(LoadMatrixxOES), \
    esApiMacro(LoadPaletteFromModelViewMatrixOES), \
    esApiMacro(LogicOp), \
    esApiMacro(Materialx), \
    esApiMacro(MaterialxOES), \
    esApiMacro(Materialxv), \
    esApiMacro(MaterialxvOES), \
    esApiMacro(MatrixIndexPointerOES), \
    esApiMacro(MatrixMode), \
    esApiMacro(MultMatrixx), \
    esApiMacro(MultMatrixxOES), \
    esApiMacro(MultiDrawArraysEXT), \
    esApiMacro(MultiDrawElementsEXT), \
    esApiMacro(MultiTexCoord4x), \
    esApiMacro(MultiTexCoord4xOES), \
    esApiMacro(Normal3x), \
    esApiMacro(Normal3xOES), \
    esApiMacro(NormalPointer), \
    esApiMacro(Orthox), \
    esApiMacro(OrthoxOES), \
    esApiMacro(PixelStorei), \
    esApiMacro(PointParameterx), \
    esApiMacro(PointParameterxOES), \
    esApiMacro(PointParameterxv), \
    esApiMacro(PointParameterxvOES), \
    esApiMacro(PointSizePointerOES), \
    esApiMacro(PointSizex), \
    esApiMacro(PointSizexOES), \
    esApiMacro(PolygonOffsetx), \
    esApiMacro(PolygonOffsetxOES), \
    esApiMacro(PopMatrix), \
    esApiMacro(PushMatrix), \
    esApiMacro(QueryMatrixxOES), \
    esApiMacro(ReadPixels), \
    esApiMacro(RenderbufferStorageOES), \
    esApiMacro(Rotatex), \
    esApiMacro(RotatexOES), \
    esApiMacro(SampleCoveragex), \
    esApiMacro(SampleCoveragexOES), \
    esApiMacro(Scalex), \
    esApiMacro(ScalexOES), \
    esApiMacro(Scissor), \
    esApiMacro(ShadeModel), \
    esApiMacro(StencilFunc), \
    esApiMacro(StencilMask), \
    esApiMacro(StencilOp), \
    esApiMacro(TexCoordPointer), \
    esApiMacro(TexDirectInvalidateVIV), \
    esApiMacro(TexDirectVIV), \
    esApiMacro(TexDirectVIVMap), \
    esApiMacro(TexDirectMapVIV), \
    esApiMacro(TexDirectTiledMapVIV), \
    esApiMacro(TexEnvi), \
    esApiMacro(TexEnviv), \
    esApiMacro(TexEnvx), \
    esApiMacro(TexEnvxOES), \
    esApiMacro(TexEnvxv), \
    esApiMacro(TexEnvxvOES), \
    esApiMacro(TexGeniOES), \
    esApiMacro(TexGenivOES), \
    esApiMacro(TexGenxOES), \
    esApiMacro(TexGenxvOES), \
    esApiMacro(TexImage2D), \
    esApiMacro(TexParameteri), \
    esApiMacro(TexParameteriv), \
    esApiMacro(TexParameterx), \
    esApiMacro(TexParameterxOES), \
    esApiMacro(TexParameterxv), \
    esApiMacro(TexParameterxvOES), \
    esApiMacro(TexSubImage2D), \
    esApiMacro(Translatex), \
    esApiMacro(TranslatexOES), \
    esApiMacro(VertexPointer), \
    esApiMacro(Viewport), \
    esApiMacro(WeightPointerOES), \
    esApiMacro(BindBufferARB), \
    esApiMacro(BufferDataARB), \
    esApiMacro(BufferSubDataARB), \
    esApiMacro(DeleteBuffersARB), \
    esApiMacro(GenBuffersARB), \
    esApiMacro(GetBufferParameterivARB), \
    esApiMacro(AlphaFunc), \
    esApiMacro(ClearColor), \
    esApiMacro(ClearDepthf), \
    esApiMacro(ClearDepthfOES), \
    esApiMacro(ClipPlanef), \
    esApiMacro(ClipPlanefOES), \
    esApiMacro(ClipPlanefIMG), \
    esApiMacro(Color4f), \
    esApiMacro(DepthRangef), \
    esApiMacro(DepthRangefOES), \
    esApiMacro(DrawTexfOES), \
    esApiMacro(DrawTexfvOES), \
    esApiMacro(Fogf), \
    esApiMacro(Fogfv), \
    esApiMacro(Frustumf), \
    esApiMacro(FrustumfOES), \
    esApiMacro(GetClipPlanef), \
    esApiMacro(GetClipPlanefOES), \
    esApiMacro(GetFloatv), \
    esApiMacro(GetLightfv), \
    esApiMacro(GetMaterialfv), \
    esApiMacro(GetTexEnvfv), \
    esApiMacro(GetTexGenfvOES), \
    esApiMacro(GetTexParameterfv), \
    esApiMacro(LightModelf), \
    esApiMacro(LightModelfv), \
    esApiMacro(Lightf), \
    esApiMacro(Lightfv), \
    esApiMacro(LineWidth), \
    esApiMacro(LoadMatrixf), \
    esApiMacro(Materialf), \
    esApiMacro(Materialfv), \
    esApiMacro(MultMatrixf), \
    esApiMacro(MultiTexCoord4f), \
    esApiMacro(Normal3f), \
    esApiMacro(Orthof), \
    esApiMacro(OrthofOES), \
    esApiMacro(PointParameterf), \
    esApiMacro(PointParameterfv), \
    esApiMacro(PointSize), \
    esApiMacro(PolygonOffset), \
    esApiMacro(Rotatef), \
    esApiMacro(SampleCoverage), \
    esApiMacro(Scalef), \
    esApiMacro(TexEnvf), \
    esApiMacro(TexEnvfv), \
    esApiMacro(TexGenfOES), \
    esApiMacro(TexGenfvOES), \
    esApiMacro(TexParameterf), \
    esApiMacro(TexParameterfv), \
    esApiMacro(Translatef), \
    esApiMacro(MapBufferOES), \
    esApiMacro(UnmapBufferOES), \
    esApiMacro(GetBufferPointervOES),


#ifndef VIV_EGL_BUILD

/* Each macro contains:
 *
 * - Function name without te "gl" prefix.
 * - Synchronization: ASYNC (asynchronous),
 *                    SYNC (synchronous),
 *                    SHARED (synchronous amoung all render threads).
 * - Types of each argument.
 */

glmDECLARE_2            (AlphaFunc, ASYNC, enum, clampf)
glmDECLARE_4            (ClearColor, ASYNC, clampf, clampf, clampf, clampf)
glmDECLARE_1            (ClearDepthf, ASYNC, clampf)
glmDECLARE_2            (ClipPlanef, ASYNC, enum, float_CP)
glmDECLARE_4            (Color4f, ASYNC, float, float, float, float)
glmDECLARE_2            (DepthRangef, ASYNC, clampf, clampf)
glmDECLARE_2            (Fogf, ASYNC, enum, float)
glmDECLARE_2            (Fogfv, ASYNC, enum, float_CP)
glmDECLARE_6            (Frustumf, ASYNC, float, float, float, float, float, float)
glmDECLARE_2            (GetClipPlanef, SYNC, enum, float_P)
glmDECLARE_2            (GetFloatv, SYNC, enum, float_P)
glmDECLARE_3            (GetLightfv, SYNC, enum, enum, float_P)
glmDECLARE_3            (GetMaterialfv, SYNC, enum, enum, float_P)
glmDECLARE_3            (GetTexEnvfv, SYNC, enum, enum, float_P)
glmDECLARE_3            (GetTexParameterfv, SYNC, enum, enum, float_P)
glmDECLARE_2            (LightModelf, ASYNC, enum, float)
glmDECLARE_2            (LightModelfv, ASYNC, enum, float_CP)
glmDECLARE_3            (Lightf, ASYNC, enum, enum, float)
glmDECLARE_3            (Lightfv, ASYNC, enum, enum, float_CP)
glmDECLARE_1            (LineWidth, ASYNC, float)
glmDECLARE_1            (LoadMatrixf, ASYNC, float_CP)
glmDECLARE_3            (Materialf, ASYNC, enum, enum, float)
glmDECLARE_3            (Materialfv, ASYNC, enum, enum, float_CP)
glmDECLARE_1            (MultMatrixf, ASYNC, float_CP)
glmDECLARE_5            (MultiTexCoord4f, ASYNC, enum, float, float, float, float)
glmDECLARE_3            (Normal3f, ASYNC, float, float, float)
glmDECLARE_6            (Orthof, ASYNC, float, float, float, float, float, float)
glmDECLARE_2            (PointParameterf, ASYNC, enum, float)
glmDECLARE_2            (PointParameterfv, ASYNC, enum, float_CP)
glmDECLARE_1            (PointSize, ASYNC, float)
glmDECLARE_2            (PolygonOffset, ASYNC, float, float)
glmDECLARE_4            (Rotatef, ASYNC, float, float, float, float)
glmDECLARE_3            (Scalef, ASYNC, float, float, float)
glmDECLARE_3            (TexEnvf, ASYNC, enum, enum, float)
glmDECLARE_3            (TexEnvfv, ASYNC, enum, enum, float_CP)
glmDECLARE_3            (TexParameterf, ASYNC, enum, enum, float)
glmDECLARE_3            (TexParameterfv, ASYNC, enum, enum, float_CP)
glmDECLARE_3            (Translatef, ASYNC, float, float, float)
glmDECLARE_1            (ActiveTexture, ASYNC, enum)
glmDECLARE_2            (AlphaFuncx, ASYNC, enum, clampx)
glmDECLARE_2            (BindBuffer, ASYNC, enum, uint)
glmDECLARE_2            (BindTexture, ASYNC, enum, uint)
glmDECLARE_2            (BlendFunc, ASYNC, enum, enum)
glmDECLARE_4            (BufferData, SHARED, enum, sizeiptr, void_CP, enum)
glmDECLARE_4            (BufferSubData, SHARED, enum, intptr, sizeiptr, void_CP)
glmDECLARE_1            (Clear, ASYNC, bitfield)
glmDECLARE_4            (ClearColorx, ASYNC, clampx, clampx, clampx, clampx)
glmDECLARE_1            (ClearDepthx, ASYNC, clampx)
glmDECLARE_1            (ClearStencil, ASYNC, int)
glmDECLARE_1            (ClientActiveTexture, ASYNC, enum)
glmDECLARE_2            (ClipPlanex, ASYNC, enum, fixed_CP)
glmDECLARE_4            (Color4ub, ASYNC, ubyte, ubyte, ubyte, ubyte)
glmDECLARE_4            (Color4x, ASYNC, fixed, fixed, fixed, fixed)
glmDECLARE_4            (ColorMask, ASYNC, boolean, boolean, boolean, boolean)
glmDECLARE_4            (ColorPointer, ASYNC, int, enum, sizei, void_CP)
glmDECLARE_8            (CompressedTexImage2D, SHARED, enum, int, enum, sizei, sizei, int, sizei, void_CP)
glmDECLARE_9            (CompressedTexSubImage2D, SHARED, enum, int, int, int, sizei, sizei, enum, sizei, void_CP)
glmDECLARE_8            (CopyTexImage2D, SHARED, enum, int, enum, int, int, sizei, sizei, int)
glmDECLARE_8            (CopyTexSubImage2D, SHARED, enum, int, int, int, int, int, sizei, sizei)
glmDECLARE_1            (CullFace, ASYNC, enum)
glmDECLARE_2            (DeleteBuffers, SHARED, sizei, uint_CP)
glmDECLARE_2            (DeleteTextures, SHARED, sizei, uint_CP)
glmDECLARE_1            (DepthFunc, ASYNC, enum)
glmDECLARE_1            (DepthMask, ASYNC, boolean)
glmDECLARE_2            (DepthRangex, ASYNC, clampx, clampx)
glmDECLARE_1            (Disable, ASYNC, enum)
glmDECLARE_1            (DisableClientState, ASYNC, enum)
glmDECLARE_3            (DrawArrays, ASYNC, enum, int, sizei)
glmDECLARE_4            (DrawElements, ASYNC, enum, sizei, enum, void_CP)
glmDECLARE_1            (Enable, ASYNC, enum)
glmDECLARE_1            (EnableClientState, ASYNC, enum)
glmDECLARE              (Finish, ASYNC)
glmDECLARE              (Flush, ASYNC)
glmDECLARE_2            (Fogx, ASYNC, enum, fixed)
glmDECLARE_2            (Fogxv, ASYNC, enum, fixed_CP)
glmDECLARE_1            (FrontFace, ASYNC, enum)
glmDECLARE_6            (Frustumx, ASYNC, fixed, fixed, fixed, fixed, fixed, fixed)
glmDECLARE_2            (GetBooleanv, SYNC, enum, boolean_P)
glmDECLARE_3            (GetBufferParameteriv, SYNC, enum, enum, int_P)
glmDECLARE_2            (GetClipPlanex, SYNC, enum, fixed_P)
glmDECLARE_2            (GenBuffers, SHARED, sizei, uint_P)
glmDECLARE_2            (GenTextures, SHARED, sizei, uint_P)
glmDECLARE_ENUM         (GetError, SYNC)
glmDECLARE_2            (GetFixedv, SYNC, enum, fixed_P)
glmDECLARE_2            (GetIntegerv, SYNC, enum, int_P)
glmDECLARE_3            (GetLightxv, SYNC, enum, enum, fixed_P)
glmDECLARE_3            (GetMaterialxv, SYNC, enum, enum, fixed_P)
glmDECLARE_2            (GetPointerv, SYNC, enum, void_PP)
glmDECLARE_UBYTE_CP_1   (GetString, SYNC, enum)
glmDECLARE_3            (GetTexEnviv, SYNC, enum, enum, int_P)
glmDECLARE_3            (GetTexEnvxv, SYNC, enum, enum, fixed_P)
glmDECLARE_3            (GetTexParameteriv, SYNC, enum, enum, int_P)
glmDECLARE_3            (GetTexParameterxv, SYNC, enum, enum, fixed_P)
glmDECLARE_2            (Hint, ASYNC, enum, enum)
glmDECLARE_BOOLEAN_1    (IsBuffer, SYNC, uint)
glmDECLARE_BOOLEAN_1    (IsEnabled, SYNC, enum)
glmDECLARE_BOOLEAN_1    (IsTexture, SYNC, uint)
glmDECLARE_2            (LightModelx, ASYNC, enum, fixed)
glmDECLARE_2            (LightModelxv, ASYNC, enum, fixed_CP)
glmDECLARE_3            (Lightx, ASYNC, enum, enum, fixed)
glmDECLARE_3            (Lightxv, ASYNC, enum, enum, fixed_CP)
glmDECLARE_1            (LineWidthx, ASYNC, fixed)
glmDECLARE              (LoadIdentity, ASYNC)
glmDECLARE_1            (LoadMatrixx, ASYNC, fixed_CP)
glmDECLARE_1            (LogicOp, ASYNC, enum)
glmDECLARE_3            (Materialx, ASYNC, enum, enum, fixed)
glmDECLARE_3            (Materialxv, ASYNC, enum, enum, fixed_CP)
glmDECLARE_1            (MatrixMode, ASYNC, enum)
glmDECLARE_1            (MultMatrixx, ASYNC, fixed_CP)
glmDECLARE_5            (MultiTexCoord4x, ASYNC, enum, fixed, fixed, fixed, fixed)
glmDECLARE_3            (Normal3x, ASYNC, fixed, fixed, fixed)
glmDECLARE_3            (NormalPointer, ASYNC, enum, sizei, void_CP)
glmDECLARE_6            (Orthox, ASYNC, fixed, fixed, fixed, fixed, fixed, fixed)
glmDECLARE_2            (PixelStorei, ASYNC, enum, int)
glmDECLARE_2            (PointParameterx, ASYNC, enum, fixed)
glmDECLARE_2            (PointParameterxv, ASYNC, enum, fixed_CP)
glmDECLARE_1            (PointSizex, ASYNC, fixed)
glmDECLARE_2            (PolygonOffsetx, ASYNC, fixed, fixed)
glmDECLARE              (PopMatrix, ASYNC)
glmDECLARE              (PushMatrix, ASYNC)
glmDECLARE_7            (ReadPixels, SYNC, int, int, sizei, sizei, enum, enum, void_P)
glmDECLARE_4            (Rotatex, ASYNC, fixed, fixed, fixed, fixed)
glmDECLARE_2            (SampleCoverage, ASYNC, clampf, boolean)
glmDECLARE_2            (SampleCoveragex, ASYNC, clampx, boolean)
glmDECLARE_3            (Scalex, ASYNC, fixed, fixed, fixed)
glmDECLARE_4            (Scissor, ASYNC, int, int, sizei, sizei)
glmDECLARE_1            (ShadeModel, ASYNC, enum)
glmDECLARE_3            (StencilFunc, ASYNC, enum, int, uint)
glmDECLARE_1            (StencilMask, ASYNC, uint)
glmDECLARE_3            (StencilOp, ASYNC, enum, enum, enum)
glmDECLARE_4            (TexCoordPointer, ASYNC, int, enum, sizei, void_CP)
glmDECLARE_3            (TexEnvi, ASYNC, enum, enum, int)
glmDECLARE_3            (TexEnvx, ASYNC, enum, enum, fixed)
glmDECLARE_3            (TexEnviv, ASYNC, enum, enum, int_CP)
glmDECLARE_3            (TexEnvxv, ASYNC, enum, enum, fixed_CP)
glmDECLARE_9            (TexImage2D, SHARED, enum, int, int, sizei, sizei, int, enum, enum, void_CP)
glmDECLARE_3            (TexParameteri, ASYNC, enum, enum, int)
glmDECLARE_3            (TexParameterx, ASYNC, enum, enum, fixed)
glmDECLARE_3            (TexParameteriv, ASYNC, enum, enum, int_CP)
glmDECLARE_3            (TexParameterxv, ASYNC, enum, enum, fixed_CP)
glmDECLARE_9            (TexSubImage2D, SHARED, enum, int, int, int, sizei, sizei, enum, enum, void_CP)
glmDECLARE_3            (Translatex, ASYNC, fixed, fixed, fixed)
glmDECLARE_4            (VertexPointer, ASYNC, int, enum, sizei, void_CP)
glmDECLARE_4            (Viewport, ASYNC, int, int, sizei, sizei)
glmDECLARE_3            (PointSizePointerOES, ASYNC, enum, sizei, void_CP)
glmDECLARE_2            (BlendEquationSeparateOES, ASYNC, enum, enum)
glmDECLARE_4            (BlendFuncSeparateOES, ASYNC, enum, enum, enum, enum)
glmDECLARE_1            (BlendEquationOES, ASYNC, enum)
glmDECLARE_5            (DrawTexsOES, SHARED, short, short, short, short, short)
glmDECLARE_5            (DrawTexiOES, SHARED, int, int, int, int, int)
glmDECLARE_5            (DrawTexxOES, SHARED, fixed, fixed, fixed, fixed, fixed)
glmDECLARE_1            (DrawTexsvOES, SHARED, short_CP)
glmDECLARE_1            (DrawTexivOES, SHARED, int_CP)
glmDECLARE_1            (DrawTexxvOES, SHARED, fixed_CP)
glmDECLARE_5            (DrawTexfOES, SHARED, float, float, float, float, float)
glmDECLARE_1            (DrawTexfvOES, SHARED, float_CP)
glmDECLARE_2            (EGLImageTargetTexture2DOES, SHARED, enum, eglImageOES)
glmDECLARE_2            (EGLImageTargetRenderbufferStorageOES, SHARED, enum, eglImageOES)
glmDECLARE_2            (AlphaFuncxOES, ASYNC, enum, clampx)
glmDECLARE_4            (ClearColorxOES, ASYNC, clampx, clampx, clampx, clampx)
glmDECLARE_1            (ClearDepthxOES, ASYNC, clampx)
glmDECLARE_2            (ClipPlanexOES, ASYNC, enum, fixed_CP)
glmDECLARE_4            (Color4xOES, ASYNC, fixed, fixed, fixed, fixed)
glmDECLARE_2            (DepthRangexOES, ASYNC, clampx, clampx)
glmDECLARE_2            (FogxOES, ASYNC, enum, fixed)
glmDECLARE_2            (FogxvOES, ASYNC, enum, fixed_CP)
glmDECLARE_6            (FrustumxOES, ASYNC, fixed, fixed, fixed, fixed, fixed, fixed)
glmDECLARE_2            (GetClipPlanexOES, SYNC, enum, fixed_P)
glmDECLARE_2            (GetFixedvOES, SYNC, enum, fixed_P)
glmDECLARE_3            (GetLightxvOES, SYNC, enum, enum, fixed_P)
glmDECLARE_3            (GetMaterialxvOES, SYNC, enum, enum, fixed_P)
glmDECLARE_3            (GetTexEnvxvOES, SYNC, enum, enum, fixed_P)
glmDECLARE_3            (GetTexParameterxvOES, SYNC, enum, enum, fixed_P)
glmDECLARE_2            (LightModelxOES, ASYNC, enum, fixed)
glmDECLARE_2            (LightModelxvOES, ASYNC, enum, fixed_CP)
glmDECLARE_3            (LightxOES, ASYNC, enum, enum, fixed)
glmDECLARE_3            (LightxvOES, ASYNC, enum, enum, fixed_CP)
glmDECLARE_1            (LineWidthxOES, ASYNC, fixed)
glmDECLARE_1            (LoadMatrixxOES, ASYNC, fixed_CP)
glmDECLARE_3            (MaterialxOES, ASYNC, enum, enum, fixed)
glmDECLARE_3            (MaterialxvOES, ASYNC, enum, enum, fixed_CP)
glmDECLARE_1            (MultMatrixxOES, ASYNC, fixed_CP)
glmDECLARE_5            (MultiTexCoord4xOES, ASYNC, enum, fixed, fixed, fixed, fixed)
glmDECLARE_3            (Normal3xOES, ASYNC, fixed, fixed, fixed)
glmDECLARE_6            (OrthoxOES, ASYNC, fixed, fixed, fixed, fixed, fixed, fixed)
glmDECLARE_2            (PointParameterxOES, ASYNC, enum, fixed)
glmDECLARE_2            (PointParameterxvOES, ASYNC, enum, fixed_CP)
glmDECLARE_1            (PointSizexOES, ASYNC, fixed)
glmDECLARE_2            (PolygonOffsetxOES, ASYNC, fixed, fixed)
glmDECLARE_4            (RotatexOES, ASYNC, fixed, fixed, fixed, fixed)
glmDECLARE_2            (SampleCoveragexOES, ASYNC, clampx, boolean)
glmDECLARE_3            (ScalexOES, ASYNC, fixed, fixed, fixed)
glmDECLARE_3            (TexEnvxOES, ASYNC, enum, enum, fixed)
glmDECLARE_3            (TexEnvxvOES, ASYNC, enum, enum, fixed_CP)
glmDECLARE_3            (TexParameterxOES, ASYNC, enum, enum, fixed)
glmDECLARE_3            (TexParameterxvOES, ASYNC, enum, enum, fixed_CP)
glmDECLARE_3            (TranslatexOES, ASYNC, fixed, fixed, fixed)
glmDECLARE_BOOLEAN_1    (IsRenderbufferOES, SYNC, uint)
glmDECLARE_2            (BindRenderbufferOES, ASYNC, enum, uint)
glmDECLARE_2            (DeleteRenderbuffersOES, SHARED, sizei, uint_CP)
glmDECLARE_2            (GenRenderbuffersOES, SHARED, sizei, uint_P)
glmDECLARE_4            (RenderbufferStorageOES, SHARED, enum, enum, sizei, sizei)
glmDECLARE_3            (GetRenderbufferParameterivOES, SYNC, enum, enum, int_P)
glmDECLARE_BOOLEAN_1    (IsFramebufferOES, SYNC, uint)
glmDECLARE_2            (BindFramebufferOES, ASYNC, enum, uint)
glmDECLARE_2            (DeleteFramebuffersOES, SHARED, sizei, uint_CP)
glmDECLARE_2            (GenFramebuffersOES, SHARED, sizei, uint_P)
glmDECLARE_ENUM_1       (CheckFramebufferStatusOES, SYNC, enum)
glmDECLARE_4            (FramebufferRenderbufferOES, SHARED, enum, enum, enum, uint)
glmDECLARE_5            (FramebufferTexture2DOES, SHARED, enum, enum, enum, uint, int)
glmDECLARE_4            (GetFramebufferAttachmentParameterivOES, SYNC, enum, enum, enum, int_P)
glmDECLARE_1            (GenerateMipmapOES, SHARED, enum)
glmDECLARE_VOID_P_2     (MapBufferOES, SYNC, enum, enum)
glmDECLARE_BOOLEAN_1    (UnmapBufferOES, SHARED, enum)
glmDECLARE_3            (GetBufferPointervOES, SYNC, enum, enum, void_PP)
glmDECLARE_1            (CurrentPaletteMatrixOES, ASYNC, uint)
glmDECLARE              (LoadPaletteFromModelViewMatrixOES, ASYNC)
glmDECLARE_4            (MatrixIndexPointerOES, ASYNC, int, enum, sizei, void_CP)
glmDECLARE_4            (WeightPointerOES, ASYNC, int, enum, sizei, void_CP)
glmDECLARE_BITFIELD_2   (QueryMatrixxOES, SYNC, fixed_P, int_P)
glmDECLARE_2            (DepthRangefOES, ASYNC, clampf, clampf)
glmDECLARE_6            (FrustumfOES, ASYNC, float, float, float, float, float, float)
glmDECLARE_6            (OrthofOES, ASYNC, float, float, float, float, float, float)
glmDECLARE_2            (ClipPlanefOES, ASYNC, enum, float_CP)
glmDECLARE_2            (GetClipPlanefOES, ASYNC, enum, float_P)
glmDECLARE_1            (ClearDepthfOES, ASYNC, clampf)
glmDECLARE_3            (TexGenfOES, ASYNC, enum, enum, float)
glmDECLARE_3            (TexGenfvOES, ASYNC, enum, enum, float_CP)
glmDECLARE_3            (TexGeniOES, ASYNC, enum, enum, int)
glmDECLARE_3            (TexGenivOES, ASYNC, enum, enum, int_CP)
glmDECLARE_3            (TexGenxOES, ASYNC, enum, enum, fixed)
glmDECLARE_3            (TexGenxvOES, ASYNC, enum, enum, fixed_CP)
glmDECLARE_3            (GetTexGenfvOES, SYNC, enum, enum, float_P)
glmDECLARE_3            (GetTexGenivOES, SYNC, enum, enum, int_P)
glmDECLARE_3            (GetTexGenxvOES, SYNC, enum, enum, fixed_P)
glmDECLARE_MULTIDRAWA   (MultiDrawArraysEXT, ASYNC, enum, int_P, sizei_P, sizei)
glmDECLARE_MULTIDRAWE   (MultiDrawElementsEXT, ASYNC, enum, sizei_CP, enum, void_CPP, sizei)
glmDECLARE_2            (ClipPlanefIMG, ASYNC, enum, float_CP)
glmDECLARE_2            (ClipPlanexIMG, ASYNC, enum, fixed_CP)
glmDECLARE_5            (TexDirectVIV, SHARED, enum, sizei, sizei, enum, void_PP)
glmDECLARE_6            (TexDirectVIVMap, SHARED, enum, sizei, sizei, enum, void_PP, uint_CP)
glmDECLARE_1            (TexDirectInvalidateVIV, SHARED, enum)
glmDECLARE_2            (BindBufferARB, ASYNC, enum, uint)
glmDECLARE_4            (BufferDataARB, SHARED, enum, sizeiptr, void_CP, enum)
glmDECLARE_4            (BufferSubDataARB, SHARED, enum, intptr, sizeiptr, void_CP)
glmDECLARE_2            (DeleteBuffersARB, SHARED, sizei, uint_CP)
glmDECLARE_2            (GenBuffersARB, SHARED, sizei, uint_P)
glmDECLARE_3            (GetBufferParameterivARB, SYNC, enum, enum, int_P)

#endif  /* VIV_EGL_BUILD */
