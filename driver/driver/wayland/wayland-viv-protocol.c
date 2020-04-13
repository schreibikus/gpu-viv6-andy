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


/* Generated by wayland-scanner 1.11.0 */

#include <stdlib.h>
#include <stdint.h>
#include "wayland-util.h"

extern const struct wl_interface wl_buffer_interface;

static const struct wl_interface *types[] = {
    &wl_buffer_interface,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    &wl_buffer_interface,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

static const struct wl_message wl_viv_requests[] = {
    { "create_buffer", "nuuuiiuiuuiuh", types + 0 },
    { "enable_tile_status", "ouuuuu", types + 13 },
};

WL_EXPORT const struct wl_interface wl_viv_interface = {
    "wl_viv", 1,
    2, wl_viv_requests,
    0, NULL,
};

