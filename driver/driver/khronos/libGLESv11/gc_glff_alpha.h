/****************************************************************************
*
*    Copyright (c) 2005 - 2015 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_glff_alpha_h_
#define __gc_glff_alpha_h_

#ifdef __cplusplus
extern "C" {
#endif

gceSTATUS glfSetDefaultAlphaStates(
    glsCONTEXT_PTR Context
    );

gceSTATUS glfFlushAlphaStates(
    glsCONTEXT_PTR Context
    );

GLenum glfEnableAlphaBlend(
    glsCONTEXT_PTR Context,
    GLboolean Enable
    );

GLenum glfEnableAlphaTest(
    glsCONTEXT_PTR Context,
    GLboolean Enable
    );

GLboolean glfQueryAlphaState(
    glsCONTEXT_PTR Context,
    GLenum Name,
    GLvoid* Value,
    gleTYPE Type
    );

#ifdef __cplusplus
}
#endif

#endif /* __gc_glff_alpha_h_ */
