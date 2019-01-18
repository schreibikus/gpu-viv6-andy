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


/**
 * \file gc_dfb_raster.h
 */

#ifndef __gc_dfb_raster_h_
#define __gc_dfb_raster_h_

#include <gc_hal_base.h>

#include "gc_dfb.h"


gceSTATUS
_FillRectangles( GalDriverData *vdrv,
                 GalDeviceData *vdev );

gceSTATUS
_Line( GalDriverData *vdrv,
       GalDeviceData *vdev );

gceSTATUS
_Blit( GalDriverData *vdrv,
       GalDeviceData *vdev );

gceSTATUS
_FlushPendingPrimitives( GalDriverData *vdrv,
                         GalDeviceData *vdev );

bool
galFillRectangle( void         *drv,
                  void         *dev,
                  DFBRectangle *rect );

bool
galFillTriangle( void        *drv,
                 void        *dev,
                 DFBTriangle *rect );

#if (DIRECTFB_MAJOR_VERSION >= 1) && (DIRECTFB_MINOR_VERSION >= 6)
bool
galFillTrapezoid( void         *drv,
                  void         *dev,
                  DFBTrapezoid *trap );

bool
galBatchFill( void                  *drv,
              void                  *dev,
              const DFBRectangle    *rects,
              unsigned int           num,
              unsigned int          *done_num );
#endif

bool
galDrawRectangle( void            *drv,
                  void            *dev,
                  DFBRectangle    *rect );

bool
galDrawLine( void      *drv,
             void      *dev,
             DFBRegion *line );

bool
galBlit( void         *drv,
         void         *dev,
         DFBRectangle *rect,
         int           dx,
         int           dy );

bool
galBlit2( void         *drv,
          void         *dev,
          DFBRectangle *rect,
          int           dx,
          int           dy,
          int           sx2,
          int           sy2 );

bool
galStretchBlit( void         *drv,
                void         *dev,
                DFBRectangle *srect,
                DFBRectangle *drect );

gceSTATUS
galStretchBlitDstNoAlpha( void    *drv,
                          void    *dev,
                          gcsRECT *srects,
                          int      src_num,
                          gcsRECT *drects,
                          int      dst_num );
#endif /* __gc_dfb_raster_h_ */
