/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_vk_framebuffer_h__
#define __gc_vk_framebuffer_h__


#define __VK_MAX_RENDER_TARGETS 4
#define __VK_MAX_RENDER_TARGETS_INTERNAL 8

typedef struct __vkAttachmentDescRec
{
    VkFormat format;
    uint32_t sampleCount;
    VkImageLayout initialLayout;
    VkImageLayout finalLayout;
    VkBool32 usedInRenderPass;
    VkBool32 loadClear;
    VkBool32 ignoreStore;
    VkBool32 stencil_loadClear;
    VkBool32 stencil_ignoreStore;
    const __vkFormatInfo * formatInfo;
} __vkAttachmentDesc;


typedef struct __vkRenderSubPassInfoRec
{
    uint32_t color_attachment_index[__VK_MAX_RENDER_TARGETS];
    VkImageLayout color_attachment_imageLayout[__VK_MAX_RENDER_TARGETS];
    uint32_t resolve_attachment_index[__VK_MAX_RENDER_TARGETS];
    VkImageLayout resolve_attachment_imageLayout[__VK_MAX_RENDER_TARGETS];
    uint32_t colorCount;

    uint32_t input_attachment_index[__VK_MAX_RENDER_TARGETS];
    VkImageLayout input_attachment_imageLayout[__VK_MAX_RENDER_TARGETS];
    uint32_t inputCount;

    uint32_t dsAttachIndex;
    VkImageLayout dsImageLayout;

}__vkRenderSubPassInfo;

typedef struct __vkRenderPassRec
{
    __vkObject obj; /* Must be the first field */

    uint32_t attachmentCount;
    __vkAttachmentDesc *attachments;

    uint32_t subPassInfoCount;
    __vkRenderSubPassInfo *subPassInfo;

    uint32_t dependencyCount;
    VkSubpassDependency *pDependencies;

    struct __vkFramebufferRec *fbDefault;
} __vkRenderPass;

typedef struct __vkFramebufferRec
{
    __vkObject obj; /* Must be the first field */

    __vkRenderPass *renderPass;

    uint32_t attachmentCount;
    __vkImageView **imageViews;

    uint32_t width;
    uint32_t height;
    uint32_t layers;

} __vkFramebuffer;

#endif /* __gc_vk_framebuffer_h__ */


