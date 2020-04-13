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


/*
 * $QNXLicenseC:
 * Copyright 2008, QNX Software Systems. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable
 * license fees to QNX Software Systems before you may reproduce,
 * modify or distribute this software, or any work that includes
 * all or part of this software.   Free development licenses are
 * available for evaluation and non-commercial purposes.  For more
 * information visit http://licensing.qnx.com or email
 * licensing@qnx.com.
 *
 * This file may contain contributions from others.  Please review
 * this entire file for other proprietary rights or license notices,
 * as well as the QNX Development Suite License Guide at
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */

#ifndef __CONFIG_IMX6X_H__
#define __CONFIG_IMX6X_H__

#define gcdDEVICE_BASE_ADDRESS  (0x10000000)

#define gcd3D_IRQ               (41)
#define gcd3D_REG_BASE          (0x00130000)
#define gcd3D_REG_SIZE          (16 << 10)

#define gcd2D_IRQ               (42)
#define gcd2D_REG_BASE          (0x00134000)
#define gcd2D_REG_SIZE          (16 << 10)

#define gcdVG_IRQ               (43)
#define gcdVG_REG_BASE          (0x02204000)
#define gcdVG_REG_SIZE          (16 << 10)

#define gcdPOWER_MANAGEMENT     (1)
#define gcdGPUPROFILER          (0)

#define gcdMMU_PhysicalMemoryBase   (0)
#define gcdMMU_PhysicalMemorySize   (0)

#endif
