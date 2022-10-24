/*---------------------------------------------------------------------------*
  Project:  TwlSDK - PRC - include
  File:     prc/types.h

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_PRC_TYPES_H_
#define NITRO_PRC_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/misc.h>
#include <nitro/types.h>
#include <nitro/fx/fx.h>

/*===========================================================================*
  Constant Definitions
 *===========================================================================*/

//--- Value of indicator that shows the stylus was lifted
#define PRC_PEN_UP_MARKER_X (-1)
#define PRC_PEN_UP_MARKER_Y (-1)

//--- Code showing not recognized or not recognizable
#define PRC_CODE_UNKNOWN (-1)

//--- Value of kindMask that expresses all types
#define PRC_KIND_ALL      (0xffffffffU)

//--- Watchdog coordinate value that is sufficiently big/small
#define PRC_LARGE_ENOUGH_X (512)
#define PRC_SMALL_ENOUGH_X (-512)
#define PRC_LARGE_ENOUGH_Y (512)
#define PRC_SMALL_ENOUGH_Y (-512)

//--- Small value with fx32 value (avoid division by 0)
#define PRC_TINY_LENGTH (1)

/*===========================================================================*
  Type Definitions
 *===========================================================================*/

/*---------------------------------------------------------------------------*
  Two-Dimensional Coordinate
 *---------------------------------------------------------------------------*/
typedef struct PRCPoint
{
    s16     x;
    s16     y;
}
PRCPoint;

/*---------------------------------------------------------------------------*
  Bounding box
 *---------------------------------------------------------------------------*/
typedef struct PRCBoundingBox
{
    s16     x1, y1;                    // Upper-left coordinate of bounding box
    s16     x2, y2;                    // Lower-right coordinate of bounding box
}
PRCBoundingBox;

/*---------------------------------------------------------------------------*
  Combination of Strokes
  MUST always be terminated with PenUpMarker
 *---------------------------------------------------------------------------*/
typedef struct PRCStrokes
{
    PRCPoint *points;
    int     size;
    u32     capacity;
}
PRCStrokes;

/*---------------------------------------------------------------------------*
  Recognition Pattern
 *---------------------------------------------------------------------------*/
typedef struct PRCPrototypeEntry
{
    BOOL    enabled;
    u32     kind;
    u16     code;
    fx16    correction;
    void   *data;
    int     pointIndex;
    u16     pointCount;
    u16     strokeCount;
}
PRCPrototypeEntry;

/*---------------------------------------------------------------------------*
  Source Data for Sample DB
 *---------------------------------------------------------------------------*/
typedef struct PRCPrototypeList
{
    const PRCPrototypeEntry *entries;
    int     entrySize;
    const PRCPoint *pointArray;
    int     pointArraySize;

    union
    {
        int     normalizeSize;         // Normalize size of coordinate values in the sample DB
        int     regularizeSize;        // Old name
    };
}
PRCPrototypeList;

/*---------------------------------------------------------------------------*
  Resampling method
 *---------------------------------------------------------------------------*/
typedef enum PRCResampleMethod
{
    PRC_RESAMPLE_METHOD_NONE = 0,
    PRC_RESAMPLE_METHOD_DISTANCE,
    PRC_RESAMPLE_METHOD_ANGLE,
    PRC_RESAMPLE_METHOD_RECURSIVE,
    PRC_RESAMPLE_METHOD_USER = 256
}
PRCResampleMethod;


#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_PRC_TYPES_H_ */
#endif
