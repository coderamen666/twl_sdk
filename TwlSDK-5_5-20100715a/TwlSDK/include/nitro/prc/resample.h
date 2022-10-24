/*---------------------------------------------------------------------------*
  Project:  TwlSDK - PRC - include
  File:     prc_resample.h

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

#ifndef NITRO_PRC_RESAMPLE_H_
#define NITRO_PRC_RESAMPLE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/misc.h>
#include <nitro/types.h>
#include <nitro/prc/common.h>
#include <nitro/prc/types.h>

/*===========================================================================*
  Constant Definitions
 *===========================================================================*/

/*===========================================================================*
  Type Definitions
 *===========================================================================*/



/*---------------------------------------------------------------------------*
  Name:         PRC_ResampleStrokes_None

  Description:  Does not resample.

  Arguments:    selectedPoints, pSelectedPointNum: Pointers that return the results.
                maxPointCount   Upper limit of input points (includes stylus up marker)
                maxStrokeCount      Upper limit of stroke count
                strokes             Raw input coordinate value before forming
                threshold           Resample threshold
                buffer              Work area (sizeof(int)*maxPointCount required)

  Returns:      TRUE if resampling succeeded.
 *---------------------------------------------------------------------------*/
BOOL
 
 PRC_ResampleStrokes_None(u16 *selectedPoints,
                          int *pSelectedPointNum,
                          int maxPointCount,
                          int maxStrokeCount,
                          const PRCStrokes *strokes, int threshold, void *buffer);

/*---------------------------------------------------------------------------*
  Name:         PRC_ResampleStrokes_Distance

  Description:  Resamples based on city block distance.

  Arguments:    selectedPoints, pSelectedPointNum: Pointers that return the results.
                maxPointCount   Upper limit of input points (includes stylus up marker)
                maxStrokeCount      Upper limit of stroke count
                strokes             Raw input coordinate value before forming
                threshold           Resample threshold
                buffer              Work area (sizeof(int)*maxPointCount required)

  Returns:      TRUE if resampling succeeded.
 *---------------------------------------------------------------------------*/
BOOL
 
 PRC_ResampleStrokes_Distance(u16 *selectedPoints,
                              int *pSelectedPointNum,
                              int maxPointCount,
                              int maxStrokeCount,
                              const PRCStrokes *strokes, int threshold, void *buffer);

/*---------------------------------------------------------------------------*
  Name:         PRC_ResampleStrokes_Angle

  Description:  Resamples based on angle.

  Arguments:    selectedPoints, pSelectedPointNum: Pointers that return the results.
                maxPointCount   Upper limit of input points (includes stylus up marker)
                maxStrokeCount      Upper limit of stroke count
                strokes             Raw input coordinate value before forming
                threshold           Resample threshold
                buffer              Work area (sizeof(int)*maxPointCount required)

  Returns:      TRUE if resampling succeeded.
 *---------------------------------------------------------------------------*/
BOOL
 
 PRC_ResampleStrokes_Angle(u16 *selectedPoints,
                           int *pSelectedPointNum,
                           int maxPointCount,
                           int maxStrokeCount,
                           const PRCStrokes *strokes, int threshold, void *buffer);

/*---------------------------------------------------------------------------*
  Name:         PRC_ResampleStrokes_Recursive

  Description:  Resamples with recursive method.

  Arguments:    selectedPoints, pSelectedPointNum: Pointers that return the results.
                maxPointCount   Upper limit of input points (includes stylus up marker)
                maxStrokeCount      Upper limit of stroke count
                strokes             Raw input coordinate value before forming
                threshold           Resample threshold
                buffer              Work area (sizeof(int)*maxPointCount required)

  Returns:      TRUE if resampling succeeded.
 *---------------------------------------------------------------------------*/
BOOL
 
 PRC_ResampleStrokes_Recursive(u16 *selectedPoints,
                               int *pSelectedPointNum,
                               int maxPointCount,
                               int maxStrokeCount,
                               const PRCStrokes *strokes, int threshold, void *buffer);


/*===========================================================================*
  Inline Functions
 *===========================================================================*/

#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_PRC_RESAMPLE_H_ */
#endif
