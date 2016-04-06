##############################################################################
#
#    Copyright (c) 2005 - 2016 by Vivante Corp.  All rights reserved.
#
#    The material in this file is confidential and contains trade secrets
#    of Vivante Corporation. This is proprietary information owned by
#    Vivante Corporation. No part of this work may be disclosed,
#    reproduced, copied, transmitted, or used in any way for any purpose,
#    without the express written permission of Vivante Corporation.
#
##############################################################################


"$(DEFFILE)" : $(AQROOT)\hal\os\libGAL.def.mak
    @copy << "$(DEFFILE)"
;
; !! Do not edit this file - it is automatically generated !!
;

LIBRARY libGAL

EXPORTS
    ;gcoOS
    gcoOS_AcquireMutex
    gcoOS_AddSignalHandler
    gcoOS_Allocate
    gcoOS_AllocateMemory
    gcoOS_AllocateNonPagedMemory
    gcoOS_AllocateSharedMemory
    gcoOS_AtomConstruct
    gcoOS_AtomDestroy
    gcoOS_AtomGet
    gcoOS_AtomSet
    gcoOS_AtomDecrement
    gcoOS_AtomIncrement
    gcoOS_Close
    gcoOS_CloseSocket
    gcoOS_CloseThread
    gcoOS_Connect
    gcoOS_Construct
    gcoOS_CreateMutex
    gcoOS_CreateSignal
    gcoOS_CreateThread
    gcoOS_DebugBreak
    gcoOS_DebugFatal
    gcoOS_DebugTrace
    gcoOS_DebugTraceZone
    gcoOS_Delay
    gcoOS_DeleteMutex
    gcoOS_DebugStatus2Name
    gcoOS_Destroy
    gcoOS_DestroySignal
    gcoOS_DeviceControl
    gcoOS_EnableDebugBuffer
    gcoOS_Flush
    gcoOS_Free
    gcoOS_FreeContiguous
    gcoOS_FreeLibrary
    gcoOS_FreeNonPagedMemory
    gcoOS_FreeSharedMemory
    gcoOS_FreeThreadData
    gcoOS_FreeMemory
    gcoOS_GetBaseAddress
    gcoOS_GetCPUTime
    gcoOS_GetCurrentProcessID
    gcoOS_GetCurrentThreadID
    gcoOS_GetCwd
    gcoOS_GetDebugLevel
    gcoOS_GetDebugZone
    gcoOS_GetEnv
    gcoOS_SetEnv
    gcoOS_GetMemoryUsage
    gcoOS_GetMemorySize
    gcoOS_GetPLSValue
    gcoOS_GetPos
    gcoOS_GetProcAddress
    gcoOS_GetProfileTick
    gcoOS_GetTicks
    gcoOS_GetTime
    gcoOS_GetTLS
    gcoOS_CopyTLS
    gcoOS_QueryTLS
    gcoOS_HexStrToInt
    gcoOS_HexStrToFloat
    gcoOS_LoadLibrary
    gcoOS_MapUserMemory
    gcoOS_MemCmp
    gcoOS_Open
    gcoOS_Print
    gcoOS_PrintStrSafe
    gcoOS_PrintStrVSafe
    gcoOS_QueryProfileTickRate
    gcoOS_QueryVideoMemory
    gcoOS_LockPLS
    gcoOS_Read
    gcoOS_ReadRegister
    gcoOS_ReleaseMutex
    gcoOS_Seek
    gcoOS_Send
    gcoOS_WaitForSend
    gcoOS_SetDebugFile
    gcoOS_ReplaceDebugFile
    gcoOS_SetDebugLevel
    gcoOS_SetDebugLevelZone
    gcoOS_SetDebugShaderFiles
    gcoOS_SetDebugZone
    gcoOS_SetDebugZones
    gcoOS_SetPLSValue
    gcoOS_SetPos
    gcoOS_Shutdown
    gcoOS_Signal
    gcoOS_Socket
    gcoOS_StackPush
    gcoOS_StackPop
    gcoOS_StackDump
    gcoOS_Stat
    gcoOS_StrCatSafe
    gcoOS_StrCmp
    gcoOS_StrCopySafe
    gcoOS_StrDup
    gcoOS_StrStr
    gcoOS_StrFindReverse
    gcoOS_StrNCmp
    gcoOS_StrToFloat
    gcoOS_StrToInt
    gcoOS_UnLockPLS
    gcoOS_UnmapUserMemory
    gcoOS_Verify
    gcoOS_WaitSignal
    gcoOS_Write
    gcoOS_WriteRegister
    gcoOS_CacheFlush
    gcoOS_CacheInvalidate
    gcoOS_CacheClean
    gcoOS_DetectProcessByName
    gcoOS_DetectProcessByEncryptedName
    gcoOS_MapUserMemoryEx
    gcoOS_FscanfI

    gcoOS_SysTraceBegin
    gcoOS_SysTraceEnd


!IF "$(VIVANTE_ENABLE_3D)_$(VIVANTE_ENABLE_VG)" != "0_0"
; This part is owned by Both 3D & VG.
    gcoOS_GetDisplay
    gcoOS_GetDisplayByIndex
    gcoOS_GetDisplayInfo
    gcoOS_GetDisplayInfoEx
    gcoOS_GetDisplayVirtual
    gcoOS_GetDisplayBackbuffer
    gcoOS_CancelDisplayBackbuffer;
    gcoOS_SetDisplayVirtual
    gcoOS_SetSwapInterval
    gcoOS_GetSwapInterval
    gcoOS_DestroyDisplay
    gcoOS_CreateWindow
    gcoOS_GetWindowInfo
    gcoOS_DestroyWindow
    gcoOS_DrawImage
    gcoOS_GetImage
    gcoOS_CreatePixmap
    gcoOS_GetPixmapInfo
    gcoOS_DrawPixmap
    gcoOS_DestroyPixmap
    gcoOS_LoadEGLLibrary
    gcoOS_FreeEGLLibrary
    gcoOS_ShowWindow
    gcoOS_HideWindow
    gcoOS_SetWindowTitle
    gcoOS_CapturePointer
    gcoOS_GetEvent
    gcoOS_CreateClientBuffer
    gcoOS_GetClientBufferInfo
    gcoOS_DestroyClientBuffer
    gcoOS_InitLocalDisplayInfo
    gcoOS_DeinitLocalDisplayInfo
    gcoOS_GetDisplayInfoEx2
    gcoOS_GetDisplayBackbufferEx
    gcoOS_IsValidDisplay
    gcoOS_SynchronousFlip
    gcoOS_GetNativeVisualId
    gcoOS_GetWindowInfoEx
    gcoOS_DrawImageEx
    gcoOS_SetWindowFormat;
    gcoOS_GetPixmapInfoEx
    gcoOS_CopyPixmapBits
    gcoOS_CreateContext
    gcoOS_DestroyContext
    gcoOS_MakeCurrent
    gcoOS_CreateDrawable
    gcoOS_DestroyDrawable
    gcoOS_SwapBuffers
    gcoOS_SetDisplayVirtualEx
    gcoOS_QuerySystemInfo

    ; gcsMEM
    gcfMEM_InitFSMemPool
    gcfMEM_FreeFSMemPool
    gcfMEM_FSMemPoolGetANode
    gcfMEM_FSMemPoolFreeANode
    gcfMEM_FSMemPoolFreeAList
    gcfMEM_InitAFSMemPool
    gcfMEM_FreeAFSMemPool
    gcfMEM_AFSMemPoolGetANode
    gcfMEM_AFSMemPoolFreeANode

    gcoHAL_QueryTargetCaps
!ENDIF

    ; gcoHAL
    gcoHAL_GetHardware
    gcoHAL_AddFrameDB
    gcoHAL_Call
    gcoHAL_Commit
    gcoHAL_Compact
    gcoHAL_Construct
    gcoHAL_Destroy
    gcoHAL_DumpFrameDB
    gcoHAL_GetDump

!IF "$(VIVANTE_ENABLE_2D)" == "1"
    gcoHAL_Get2DEngine
!ENDIF

!IF "$(VIVANTE_ENABLE_3D)" == "1"
    gcoHAL_GetSpecialHintData
    gcoHAL_GetPatchID
    gcoHAL_SetPatchID
    gcoHAL_SetGlobalPatchID
!ENDIF

    gcoHAL_GetTimerTime
    gcoHAL_IsFeatureAvailable
    gcoHAL_IsFeatureAvailable1
    gcoHAL_MapMemory
    gcoHAL_ProfileEnd
    gcoHAL_ProfileStart
    gcoHAL_QueryChipCount
    gcoHAL_QueryChipFeature
    gcoHAL_QueryChipIdentity
    gcoHAL_QueryChipLimits
    gcoHAL_QueryPowerManagementState
    gcoHAL_QueryTiled
    gcoHAL_QueryVideoMemory
    gcoHAL_ScheduleEvent
    gcoHAL_ScheduleUnmapMemory
    gcoHAL_ScheduleUnmapUserMemory
    gcoHAL_SetPowerManagementState
    gcoHAL_SetTimer
    gcoHAL_UnmapMemory
    gcoHAL_MapUserMemory
    gcoHAL_UnmapUserMemory
    gcoHAL_QuerySeparated2D
    gcoHAL_SetTimeOut
    gcoHAL_GetOption
    gcoHAL_FrameInfoOps
    gcoHAL_GetProductName
    gcoHAL_QueryResetTimeStamp
    gcoHAL_WrapUserMemory
    gcoHAL_LockVideoMemory
    gcoHAL_UnlockVideoMemory
    gcoHAL_ReleaseVideoMemory

!IF "$(VIVANTE_ENABLE_3D)" == "1"
    gcoHAL_QueryShaderCaps
    gcoHAL_QuerySamplerBase
    gcoHAL_QueryUniformBase
    gcoHAL_QueryStreamCaps
    gcoHAL_QueryTextureCaps
    gcoHAL_QueryTextureMaxAniso
    gcoHAL_SendFence
    gcoHAL_SetBltNP2Texture
    gcoHAL_Get3DEngine
    gcoHAL_SetCompilerFuncTable
!ENDIF
    gcoHAL_SetHardwareType

    ; gcoDUMP
    gcoDUMP_AddSurface
    gcoDUMP_Construct
    gcoDUMP_Control
    gcoDUMP_Delete
    gcoDUMP_Destroy
    gcoDUMP_DumpData
    gcoDUMP_FrameBegin
    gcoDUMP_FrameEnd
    gcoDUMP_IsEnabled
    gcoDUMP_SetDumpFlag
    gcfDump
    gcfDumpApi
    gcfDumpArray
    gcfDumpArrayToken
    gcfDumpApiData
    gcfDumpBuffer
    gcfDumpFrameRate
    gcfDump2DCommand
    gcfDump2DSurface
    gcfAddMemoryInfo
    gcfDelMemoryInfo

    ; gcoSURF
    ; 2D
!IF "$(VIVANTE_ENABLE_2D)" == "1"
    gcoSURF_MonoBlit
    gcoSURF_SetClipping
    gcoSURF_Line
    gcoSURF_FilterBlit
    gcoSURF_EnableAlphaBlend
    gcoSURF_DisableAlphaBlend
    gcoSURF_Blit
    gcoSURF_Clear2D
!ENDIF

    gcoSURF_Blend
    gcoSURF_Construct
    gcoSURF_ConstructWrapper
    gcoSURF_CPUCacheOperation
    gcoSURF_Destroy
    gcoSURF_Fill
    gcoSURF_Flush
    gcoSURF_GetAlignedSize
    gcoSURF_GetAlignment
    gcoSURF_GetBottomBufferOffset
    gcoSURF_GetColorType
    gcoSURF_GetColorSpace
    gcoSURF_GetFormat
    gcoSURF_GetFormatInfo
    gcoSURF_GetPackedFormat
    gcoSURF_GetSize
    gcoSURF_IsValid
    gcoSURF_Lock
    gcoSURF_MapUserSurface
    gcoSURF_QueryFlags
    gcoSURF_QueryFormat
    gcoSURF_QueryOrientation
    gcoSURF_QueryReferenceCount
    gcoSURF_ReferenceSurface
    gcoSURF_SetFlags
    gcoSURF_SetBuffer
    gcoSURF_SetColorType
    gcoSURF_SetColorSpace
    gcoSURF_SetDither
    gcoSURF_SetOrientation
    gcoSURF_SetRotation
    gcoSURF_SetWindow
    gcoSURF_SetImage
    gcoSURF_Unlock
    gcoSURF_GetTiling
    gcoSURF_LockNode
    gcoSURF_UnLockNode
    gcoSURF_WrapSurface
    gcoSURF_GetInfo
    gcoSURF_QueryHints

!IF "$(VIVANTE_ENABLE_3D)_$(VIVANTE_ENABLE_VG)" != "0_0"
;   Both 3D & VG have these functions.
    gcoSURF_GetSamples
    gcoSURF_SetSamples
    gcoSURF_Copy
!ENDIF

!IF "$(VIVANTE_ENABLE_3D)" == "1"
    depr_gcoSURF_Resolve
    depr_gcoSURF_ResolveRect

    gcoSURF_Clear
    gcoSURF_CopyPixels
    gcoSURF_DisableTileStatus
    gcoSURF_EnableTileStatus
    gcoSURF_FillFromTile
    gcoSURF_IsTileStatusSupported
    gcoSURF_IsTileStatusEnabled
    gcoSURF_IsCompressed
    gcoSURF_IsRenderable
    gcoSURF_IsFormatRenderableAsRT
    gcoSURF_Resample
    gcoSURF_Preserve
    gcoSURF_GetResolveAlignment
    gcoSURF_ResolveRect
    gcoSURF_IsHWResolveable
    gcoSURF_BlitCPU
    gcoSURF_SetResolvability
    gcoSURF_Swap
    gcoSURF_ResetSurWH
    gcoSURF_SetSharedLock
    gcoSURF_GetFence
    gcoSURF_WaitFence
    gcoSURF_AlignResolveRect
 	gcoSURF_DrawBlit
    gcoSURF_3DBlitCopy
    gcsSURF_NODE_Construct
    gcsSURF_NODE_Destroy
    gcsSURF_NODE_GetHardwareAddress
    gcsSURF_NODE_SetHardwareAddress

    gcoSURF_FlushTileStatus
    gcoSURF_AppendTileStatus
!ENDIF

!IF "$(VIVANTE_ENABLE_3D)" == "1"
    ; gcoINDEX
    gcoINDEX_Bind
    gcoINDEX_BindOffset
    gcoINDEX_SetSharedLock
    gcoINDEX_Construct
    gcoINDEX_Destroy
    gcoINDEX_Free
    gcoINDEX_GetIndexRange
    gcoINDEX_Load
    gcoINDEX_Lock
    gcoINDEX_QueryCaps
    gcoINDEX_SetDynamic
    gcoINDEX_Unlock
    gcoINDEX_Upload
    gcoINDEX_UploadOffset
    gcoINDEX_Merge
    gcoINDEX_CheckRange
    gcoINDEX_GetFence
    gcoINDEX_WaitFence
!ENDIF

!IF "$(VIVANTE_ENABLE_2D)" == "1"
    ; gco2D
    gco2D_Blit
    gco2D_BatchBlit
    gco2D_Clear
    gco2D_ColorLine
    gco2D_Construct
    gco2D_ConstructColorBrush
    gco2D_ConstructMonochromeBrush
    gco2D_ConstructSingleColorBrush
    gco2D_Destroy
    gco2D_DisableAlphaBlend
    gco2D_EnableAlphaBlend
    gco2D_EnableAlphaBlendAdvanced
    gco2D_EnableUserFilterPasses
    gco2D_FilterBlit
    gco2D_FilterBlitEx
    gco2D_Flush
    gco2D_FlushBrush
    gco2D_FreeFilterBuffer
    gco2D_GetBrushCache
    gco2D_GetMaximumDataCount
    gco2D_GetMaximumRectCount
    gco2D_GetPackSize
    gco2D_GetPixelAlignment
    gco2D_Line
    gco2D_LoadPalette
    gco2D_LoadSolidBrush
    gco2D_MonoBlit
    gco2D_ProfileEngine
    gco2D_SetAutoFlushCycles
    gco2D_SetBitBlitMirror
    gco2D_SetBrushLimit
    gco2D_SetClipping
    gco2D_SetColorSource
    gco2D_SetColorSourceAdvanced
    gco2D_SetColorSourceEx
    gco2D_SetFilterType
    gco2D_SetKernelSize
    gco2D_SetMaskedSource
    gco2D_SetMaskedSourceEx
    gco2D_SetMonochromeSource
    gco2D_SetPixelMultiplyModeAdvanced
    gco2D_SetPorterDuffBlending
    gco2D_SetSource
    gco2D_SetSourceColorKeyAdvanced
    gco2D_SetSourceColorKeyRangeAdvanced
    gco2D_SetSourceGlobalColorAdvanced
    gco2D_SetStretchFactors
    gco2D_SetStretchRectFactors
    gco2D_SetTarget
    gco2D_SetTargetColorKeyAdvanced
    gco2D_SetTargetColorKeyRangeAdvanced
    gco2D_SetTargetEx
    gco2D_SetTargetGlobalColorAdvanced
    gco2D_SetTransparencyAdvanced
    gco2D_SetTransparencyAdvancedEx
    gco2D_SetUserFilterKernel
    gco2D_SetYUVColorMode
    gco2D_StretchBlit
    gco2D_EnableDither
    gco2D_SetGenericSource
    gco2D_SetGenericTarget
    gco2D_FilterBlitEx2
    gco2D_SetCurrentSourceIndex
    gco2D_MultiSourceBlit
    gco2D_SetROP
    gco2D_SetGdiStretchMode
    gco2D_SetSourceTileStatus
    gco2D_CalcStretchFactor
    gco2D_QueryU32
    gco2D_SetStateU32
    gco2D_SetStateArrayU32
    gco2D_SetStateArrayI32
    gco2D_SetTargetRect
    gco2D_MonoBlitEx
    gco2D_SetTargetTileStatus

    gco2D_Set2DEngine
    gco2D_Get2DEngine
    gco2D_UnSet2DEngine
    gco2D_Commit
    gco2D_NatureRotateTranslation
!ENDIF

!IF "$(VIVANTE_ENABLE_3D)" == "1"
    gco3D_Set3DEngine
    gco3D_Get3DEngine
    gco3D_UnSet3DEngine
    gco3D_ComposeLayer
    gco3D_CompositionBegin
    gco3D_CompositionEnd
    gco3D_CompositionSignals
    gco3D_Construct
    gco3D_Destroy
    gco3D_DrawIndexedPrimitives
    gco3D_DrawIndexedPrimitivesOffset
    gco3D_DrawPrimitives
    gco3D_DrawIndirectPrimitives
    gco3D_MultiDrawIndirectPrimitives
    gco3D_DrawInstancedPrimitives
    gco3D_DrawNullPrimitives
    gco3D_DrawPrimitivesCount
    gco3D_DrawPrimitivesOffset
    gco3D_DrawPattern
    gco3D_EnableBlending
    gco3D_EnableDepthWrite
    gco3D_EnableDither
    gco3D_EnableTextureStage
    gco3D_InvokeThreadWalker
    gco3D_ProbeComposition
    gco3D_Semaphore
    gco3D_FlushSHL1Cache
    gco3D_SetAALineTexSlot
    gco3D_SetAALineWidth
    gco3D_SetAllEarlyDepthModes
    gco3D_SetEarlyDepthFromAPP
    gco3D_SetRADepthWrite
    gco3D_SwitchDynamicEarlyDepthMode
    gco3D_DisableDynamicEarlyDepthMode
    gco3D_SetAlphaCompare
    gco3D_SetAlphaReference
    gco3D_SetAlphaReferenceF
    gco3D_SetAlphaReferenceX
    gco3D_SetAlphaTest
    gco3D_SetAlphaTextureFunction
    gco3D_SetAntiAlias
    gco3D_SetSamples
    gco3D_SetAntiAliasLine
    gco3D_SetAPI
    gco3D_GetAPI
    gco3D_SetBlendColor
    gco3D_SetBlendColorF
    gco3D_SetBlendColorX
    gco3D_SetBlendFunction
    gco3D_SetBlendMode
    gco3D_SetCentroids
    gco3D_GetSampleCoords
    gco3D_SetClearColor
    gco3D_SetClearColorF
    gco3D_SetClearColorX
    gco3D_SetClearDepthF
    gco3D_SetClearDepthX
    gco3D_SetClearStencil
    gco3D_SetColorTextureFunction
    gco3D_SetColorWrite
    gco3D_SetCulling
    gco3D_SetDepth
    gco3D_SetDepthCompare
    gco3D_SetDepthMode
    gco3D_SetDepthOnly
    gco3D_SetDepthRangeF
    gco3D_SetDepthRangeX
    gco3D_SetDepthScaleBiasF
    gco3D_SetDepthScaleBiasX
    gco3D_SetDepthPlaneF
    gco3D_SetEarlyDepth
    gco3D_SetFill
    gco3D_SetFogColorF
    gco3D_SetFogColorX
    gco3D_SetFragmentColorF
    gco3D_SetFragmentColorX
    gco3D_SetFragmentConfiguration
    gco3D_SetLastPixelEnable
    gco3D_SetLogicOp
    gco3D_SetPointSizeEnable
    gco3D_SetPointSprite
    gco3D_SetPrimitiveIdEnable
    gco3D_SetScissors
    gco3D_SetShading
    gco3D_SetStencilAll
    gco3D_SetStencilCompare
    gco3D_SetStencilDepthFail
    gco3D_SetStencilFail
    gco3D_SetStencilMask
    gco3D_SetStencilMaskBack
    gco3D_SetStencilMode
    gco3D_SetStencilPass
    gco3D_SetStencilReference
    gco3D_SetStencilWriteMask
    gco3D_SetStencilWriteMaskBack
    gco3D_SetTarget
    gco3D_SetTetxureColorF
    gco3D_SetTetxureColorX
    gco3D_SetTextureAlphaMask
    gco3D_SetTextureColorMask
    gco3D_SetViewport
    gco3D_WriteBuffer
    gco3D_UnsetTarget
    gco3D_UnsetDepth
    gco3D_SetWClipEnable
    gco3D_GetWClipEnable
    gco3D_GetClosestRenderFormat
    gco3D_SetWPlaneLimitF
    gco3D_SetWPlaneLimitX
    gco3D_SetWPlaneLimit
    gco3D_SetQuery
    gco3D_GetQuery
    gco3D_SetColorOutCount
    gco3D_SetColorCacheMode
    gco3D_PrimitiveRestart
    gco3D_SetPSOutputMapping
    gco3D_LoadProgram
    gco3D_EnableAlphaToCoverage
    gco3D_EnableSampleCoverage
    gco3D_SetSampleCoverageValue
    gco3D_EnableSampleMask
    gco3D_SetSampleMask
    gco3D_EnableSampleShading
    gco3D_EnableSampleMaskOut
    gco3D_SetMinSampleShadingValue
    gco3D_SetSampleShading
    gco3D_SetRenderLayered
    gco3D_SetShaderLayered
    gco3D_IsProgramSwitched

    gco3D_EnableBlendingIndexed
    gco3D_SetBlendFunctionIndexed
    gco3D_SetBlendModeIndexed
    gco3D_SetColorWriteIndexed

    gco3D_SetPatchVertices

    gco3D_SetXfbCmd
    gco3D_SetXfbHeader
    gco3D_SetXfbBuffer
    gco3D_SetRasterDiscard

    gco3D_QueryReset

    ; gcoTEXTURE
    gcoTEXTURE_AddMipMap
    gcoTEXTURE_AddMipMapEx
    gcoTEXTURE_AddMipMapFromClient
    gcoTEXTURE_AddMipMapFromSurface
    gcoTEXTURE_BindTexture
    gcoTEXTURE_BindTextureEx
    gcoTEXTURE_Construct
    gcoTEXTURE_ConstructEx
    gcoTEXTURE_ConstructSized
    gcoTEXTURE_Destroy
    gcoTEXTURE_Disable
    gcoTEXTURE_Flush
    gcoTEXTURE_FlushVS
    gcoTEXTURE_GetClosestFormat
    gcoTEXTURE_GetClosestFormatEx
    gcoTEXTURE_GetFormatInfo
    gcoTEXTURE_GetMipMap
    gcoTEXTURE_GetMipMapFace
    gcoTEXTURE_GetMipMapSlice
    gcoTEXTURE_InitParams
    gcoTEXTURE_IsComplete
    gcoTEXTURE_IsRenderable
    gcoTEXTURE_QueryCaps
    gcoTEXTURE_RenderIntoMipMap
    gcoTEXTURE_RenderIntoMipMap2
    gcoTEXTURE_SetEndianHint
    gcoTEXTURE_Upload
    gcoTEXTURE_UploadCompressed
    gcoTEXTURE_UploadCompressedSub
    gcoTEXTURE_UploadSub
    gcoTEXTURE_SetDepthTextureFlag
    gcoTEXTURE_BindTextureTS
    gcoTEXTURE_GenerateMipMap
    gcoTEXTURE_UploadYUV
    gcoTEXTURE_BindTextureDesc
    gcoTEXTURE_SetDescDirty
    gcoTEXTURE_LockMipMap
!ENDIF

    ; gcsRECT
    gcsRECT_Height
    gcsRECT_Set
    gcsRECT_Width
    gcsRECT_Rotate
    gcsRECT_RelativeRotation

!IF "$(VIVANTE_ENABLE_3D)_$(VIVANTE_ENABLE_2D)" != "0_0"
    ;gcsBITMASK
    gcsBITMASK_InitAllOne
    gcsBITMASK_InitAllZero
    gcsBITMASK_InitOR
    gcsBITMASK_Test
    gcsBITMASK_TestAndClear
    gcsBITMASK_IsAllZero
    gcsBITMASK_Set
    gcsBITMASK_Clear
    gcsBITMASK_SetAll
    gcsBITMASK_MergeBitMaskArray
    gcsBITMASK_OR
!ENDIF


!IF "$(VIVANTE_ENABLE_2D)" == "1"
    ; gcoBRUSH
    gcoBRUSH_Destroy
!ENDIF

!IF "$(VIVANTE_ENABLE_3D)" == "1"
    ; gcSHADER
    gcQueryShaderCompilerHwCfg
    gcLoadShaders
    gcLoadKernel
    gcInvokeThreadWalker
    gcSHADER_SpecialHint
    gcoSHADER_ProgramUniform
    gcoSHADER_ProgramUniformEx
    gcoSHADER_BindBufferBlock
    gcCreateInstVidMem
    gcDestroyInstVidMem
    gcCreateSpillVidMem
    gcDestroySpillVidMem

    ; gcoSTREAM
    gcoSTREAM_SetSharedLock
    gcoSTREAM_Construct
    gcoSTREAM_Destroy
    gcoSTREAM_Flush
    gcoSTREAM_Lock
    gcoSTREAM_Reserve
    gcoSTREAM_SetDynamic
    gcoSTREAM_SetStride
    gcoSTREAM_Unlock
    gcoSTREAM_Upload
    gcoSTREAM_UploadDynamic
    gcoSTREAM_CPUCacheOperation
    gcoSTREAM_CPUCacheOperation_Range
    gcoSTREAM_GetFence
    gcoSTREAM_WaitFence
    gcoSTREAM_GetSize

    ; gcoVERTEX
    gcoVERTEX_Bind
    gcoVERTEX_Construct
    gcoVERTEX_Destroy
    gcoVERTEX_DisableAttribute
    gcoVERTEX_EnableAttribute
    gcoVERTEX_Reset

    gcoVERTEXARRAY_Construct
    gcoVERTEXARRAY_Destroy
    gcoVERTEXARRAY_IndexUpdate
    gcoVERTEXARRAY_Bind
    gcoVERTEXARRAY_Bind_Ex
    gcoVERTEXARRAY_Bind_Ex2

    ; gcoBUFFER
    gcoBUFFER_Construct
    gcoBUFFER_Destroy
    gcoBUFFER_Reserve
    gcoBUFFER_Write
    gcoBUFFER_Commit
    gcoBUFFER_StartTEMPCMDBUF
    gcoBUFFER_EndTEMPCMDBUF

    ; gcoCL
    gcoHARDWARE_InitializeCL
    gcoCLHardware_Construct
    gcoCL_InitializeHardware
    gcoCL_SetHardware
    gcoCL_AllocateMemory
    gcoCL_FreeMemory
    gcoCL_FlushMemory
    gcoCL_InvalidateMemoryCache
    gcoCL_ShareMemoryWithStream
    gcoCL_ShareMemoryWithBufObj
    gcoCL_UnshareMemory
    gcoCL_FlushSurface
    gcoCL_LockSurface
    gcoCL_UnlockSurface
    gcoCL_CreateTexture
    gcoCL_DestroyTexture
    gcoCL_SetupTexture
    gcoCL_QueryDeviceInfo
    gcoCL_QueryDeviceCount
    gcoCL_SelectDevice
    gcoCL_Commit
    gcoCL_CreateSignal
    gcoCL_DestroySignal
    gcoCL_SubmitSignal
    gcoCL_WaitSignal
    gcoCL_Flush
    gcoCL_LoadKernel
    gcoCL_InvokeKernel
    gcoCL_InvokeThreadWalker
    gcoCL_SetSignal

    ; gcoPROFILER
    gcoPROFILER_Initialize
    gcoPROFILER_Destroy
    gcoPROFILER_Write
    gcoPROFILER_Flush
    gcoPROFILER_EndFrame
    gcoPROFILER_BeginDraw
    gcoPROFILER_EndDraw
    gcoPROFILER_ShaderFS
    gcoPROFILER_ShaderVS

    ; gcoMATH
    gcoMATH_Log2in5dot5
    gcoMATH_UIntAsFloat
    gcoMATH_FloatAsUInt
    gcoMATH_UInt8AsFloat16
    gcoMATH_CompareEqualF
    gcoMATH_Float16ToFloat
    gcoMATH_FloatToFloat16
    gcoMATH_Float14ToFloat
    gcoMATH_Float11ToFloat
    gcoMATH_FloatToFloat11
    gcoMATH_Float10ToFloat
    gcoMATH_FloatToFloat10
!ENDIF

!IF "$(VIVANTE_ENABLE_VG)" == "1"
    gcoHAL_GetVGEngine
!IF "$(GC355_PROFILER)" == "1"
    gcoVG_ProfilerEnableDisable
    gcoVG_ProfilerTreeDepth
    gcoVG_ProfilerSetStates
!ENDIF
    gcoVG_IsMaskSupported
    gcoVG_IsTargetSupported
    gcoVG_IsImageSupported
    gcoVG_PackColorComponent
    gcoVG_Construct
    gcoVG_Destroy
    gcoVG_SetTarget
    gcoVG_UnsetTarget
    gcoVG_SetUserToSurface
    gcoVG_SetSurfaceToImage
    gcoVG_EnableMask
    gcoVG_SetMask
    gcoVG_UnsetMask
    gcoVG_FlushMask
    gcoVG_EnableScissor
    gcoVG_SetScissor
    gcoVG_SetSolidPaint
    gcoVG_SetLinearPaint
    gcoVG_SetRadialPaint
    gcoVG_SetPatternPaint
    gcoVG_SetColorRamp
    gcoVG_SetPattern
    gcoVG_SetBlendMode
    gcoVG_FinalizePath
    gcoVG_Clear
    gcoVG_DrawPath
    gcoVG_DrawImage
    gcoVG_TesselateImage
    gcoVG_Blit
    gcoVG_ColorMatrix
    gcoVG_SeparableConvolve
    gcoVG_GaussianBlur
    gcoVG_SetTileFillColor
    gcoVG_EnableColorTransform
    gcoVG_SetColorTransform
    gcoVG_SetImageMode
    gcoVG_SetRenderingQuality
    gcoVG_SetFillRule
    gcoVG_EnableDither
    gcoVG_DrawSurfaceToImage
!IF "$(VIVANTE_ENABLE_3D)_$(VIVANTE_ENABLE_2D)_$(VIVANTE_ENABLE_VG)" == "0_0_1"
    gcoVG_SetColorKey
    gcoVG_SetColorIndexTable
    gcoVG_Resolve
!ENDIF

    gcoVGHARDWARE_QueryPathStorage
    gcoVGHARDWARE_AssociateCompletion
    gcoVGHARDWARE_DeassociateCompletion
    gcoVGHARDWARE_CheckCompletion
    gcoVGHARDWARE_WaitCompletion
    gcoVGHARDWARE_FlushPipe
    gcoVGHARDWARE_AllocateLinearVideoMemory
    gcoVGHARDWARE_FreeVideoMemory
    gcoVGHARDWARE_ScheduleVideoMemory
    gcoVGHARDWARE_SplitAddress
    gcoVGHARDWARE_CombineAddress
    gcoVGHARDWARE_QueryCommandBuffer
    gcoVGHARDWARE_AlignToTile
    gcoVGHARDWARE_ReserveTask
    gcoVGHARDWARE_IsMaskSupported
    gcoVGHARDWARE_IsTargetSupported
    gcoVGHARDWARE_IsImageSupported
    gcoVGHARDWARE_IsFeatureAvailable
    gcoVGHARDWARE_SetVgTarget
    gcoVGHARDWARE_EnableMask
    gcoVGHARDWARE_SetVgMask
    gcoVGHARDWARE_FlushVgMask
    gcoVGHARDWARE_EnableScissor
    gcoVGHARDWARE_SetScissor
    gcoVGHARDWARE_PackColor32
    gcoVGHARDWARE_EnableColorTransform
    gcoVGHARDWARE_SetColorTransform
    gcoVGHARDWARE_SetPaintSolid
    gcoVGHARDWARE_SetPaintLinear
    gcoVGHARDWARE_SetPaintRadial
    gcoVGHARDWARE_SetPaintPattern
    gcoVGHARDWARE_SetPaintImage
    gcoVGHARDWARE_SetVgBlendMode
    gcoVGHARDWARE_VgClear
    gcoVGHARDWARE_DrawPath
    gcoVGHARDWARE_SetTessellation
    gcoVGHARDWARE_SetPathDataType
    gcoVGHARDWARE_DrawImage
    gcoVGHARDWARE_TesselateImage
    gcoVGHARDWARE_VgBlit
    gcoVGHARDWARE_ColorMatrix
    gcoVGHARDWARE_SeparableConvolve
    gcoVGHARDWARE_GaussianBlur
    gcoVGHARDWARE_SetVgImageMode
    gcoVGHARDWARE_SetRenderingQuality
    gcoVGHARDWARE_SetFillRule
    gcoVGHARDWARE_EnableDither
    gcoVGHARDWARE_PackColorComponent
!IF "$(VIVANTE_ENABLE_3D)_$(VIVANTE_ENABLE_2D)_$(VIVANTE_ENABLE_VG)" == "0_0_1"
    gcoVGHARDWARE_SetColorKey
    gcoVGHARDWARE_SetColorIndexTable
    gcoVGHARDWARE_ResolveRect
!ENDIF

    gcoHAL_Flush
    gcoHAL_AssociateCompletion
    gcoHAL_DeassociateCompletion
    gcoHAL_CheckCompletion
    gcoHAL_WaitCompletion
    gcoHAL_ScheduleVideoMemory
    gcoHAL_AllocateLinearVideoMemory
    gcoHAL_QueryPathStorage
    gcoHAL_FreeVideoMemory
    gcoHAL_CombineAddress
    gcoHAL_SplitAddress
    gcoHAL_GetAlignedSurfaceSize
    gcoHAL_QueryCommandBuffer
    gcoHAL_ReserveTask
    gcoHAL_Query3DCoreCount
!ENDIF
    gcoHAL_GetHardwareType

!IF "$(VIVANTE_ENABLE_3D)" == "1"
    gcoBUFOBJ_Construct
    gcoBUFOBJ_Destroy
    gcoBUFOBJ_Lock
    gcoBUFOBJ_Unlock
    gcoBUFOBJ_Free
    gcoBUFOBJ_Upload
    gcoBUFOBJ_IndexBind
    gcoBUFOBJ_IndexGetRange
    gcoBUFOBJ_CPUCacheOperation_Range
    gcoBUFOBJ_CPUCacheOperation
    gcoBUFOBJ_GetSize
    gcoBUFOBJ_GPUCacheOperation
    gcoBUFOBJ_GetFence
    gcoBUFOBJ_WaitFence
    gcoBUFOBJ_IsFenceEnabled
    gcoBUFOBJ_SetDirty
    gcoBUFOBJ_AlignIndexBufferWhenNeeded
    gcoBUFOBJ_FastLock
    gcoBUFOBJ_Dump

    gcfSTATISTICS_AddData
    gcfSTATISTICS_MarkFrameEnd
    gcfSTATISTICS_DisableDynamicEarlyZ
!ENDIF

!IF "$(VIVANTE_ENABLE_VX)" == "1"
    ; gcoVX
    gcoVX_Initialize
    gcoVX_BindImage
    gcoVX_BindKernel
    gcoVX_BindUniform
    gcoVX_InvokeKernel
    gcoVX_Commit
    gcoVX_AllocateMemory
    gcoVX_FreeMemory
    gcoVX_KernelConstruct
    gcoVX_LockKernel
    gcoVX_Replay
    gcoVX_LoadKernelShader
    gcoVX_InvokeKernelShader
    gcoVX_Flush
    gcoVX_AllocateMemoryEx
    gcoVX_FreeMemoryEx
!ENDIF
    gcGetUserDebugOption

<<
