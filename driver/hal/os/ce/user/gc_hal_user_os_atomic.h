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


#ifndef __gc_hal_user_os_atomic_h_
#define __gc_hal_user_os_atomic_h_

struct gcsATOM
{
    /* Counter. */
    gctINT32 counter;
};

#define gcmATOM_INITIALIZER \
    { \
        0 \
    };

#endif
