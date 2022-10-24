/*---------------------------------------------------------------------------*
  Project:  TwlSDK - PRC - include
  File:     prc/common.h

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

#ifndef NITRO_PRC_COMMON_H_
#define NITRO_PRC_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/misc.h>
#include <nitro/types.h>

#include <nitro/prc/types.h>
#include <nitro/fx/fx_cp.h>
#include <nitro/fx/fx_trig.h>

/*===========================================================================*
  Constant Definitions
 *===========================================================================*/

/*===========================================================================*
  Type Definitions
 *===========================================================================*/


/*===========================================================================*
  Function Declarations
 *===========================================================================*/

/*---------------------------------------------------------------------------*
  Name:         PRCi_Init

  Description:  Initializes pattern recognition API.
                Called first from initialization routines in each algorithm.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    PRCi_Init(void);

/*---------------------------------------------------------------------------*
  Name:         PRC_GetEntryStrokes

  Description:  Gets dot sequence data from sample DB and sample DB entry.

  Arguments:    strokes:        Obtained dot sequence data
                                Overwriting is not permitted.
                prototypeList:  sample pattern list
                entry:      sample DB entry

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
 
 PRC_GetEntryStrokes(PRCStrokes *strokes,
                     const PRCPrototypeList *prototypeList, const PRCPrototypeEntry *entry);

/*---------------------------------------------------------------------------*
  Name:         PRC_CopyStrokes

  Description:  Deep copies dot sequence data.

  Arguments:    srcStrokes      The PRC_Strokes structure to copy from
                dstStrokes:      The copy destination PRC_Strokes structure.

  Returns:      If copy worked, TRUE.
 *---------------------------------------------------------------------------*/
BOOL    PRC_CopyStrokes(const PRCStrokes *srcStrokes, PRCStrokes *dstStrokes);

/*===========================================================================*
  Inline Functions
 *===========================================================================*/

/*---------------------------------------------------------------------------*
  Name:         PRC_GetEntryCode

  Description:  Returns the code number corresponding to the sample DB entry.

  Arguments:    entry:      sample DB entry

  Returns:      code number
 *---------------------------------------------------------------------------*/
static inline int PRC_GetEntryCode(const PRCPrototypeEntry *entry)
{
    return (entry != NULL) ? entry->code : -1;
}

/*---------------------------------------------------------------------------*
  Name:         PRC_GetEntryData

  Description:  Returns user data associated with the sample DB entry.

  Arguments:    entry:      sample DB entry

  Returns:      User data
 *---------------------------------------------------------------------------*/
static inline void *PRC_GetEntryData(const PRCPrototypeEntry *entry)
{
    return (entry != NULL) ? entry->data : NULL;
}

/*---------------------------------------------------------------------------*
  Name:         PRC_InitStrokes

  Description:  Initializes PRCStrokes structure.

  Arguments:    strokes:     Initialization target.
                points:      Pointer to PRCPoint array which becomes actual memory region.
                size:        Size of array

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void PRC_InitStrokes(PRCStrokes *strokes, PRCPoint *points, u32 size)
{
    SDK_ASSERT(strokes && points);
    strokes->points = points;
    strokes->capacity = size;
    strokes->size = 0;
}

/*---------------------------------------------------------------------------*
  Name:         PRC_AppendPoint

  Description:  Adds dots to PRCStrokes.

  Arguments:    strokes:     target.
                x, y:        Points to add.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void PRC_AppendPoint(PRCStrokes *strokes, int x, int y)
{
    SDK_ASSERT(strokes);
    SDK_ASSERT(strokes->size < strokes->capacity);
    strokes->points[strokes->size].x = (s16)x;
    strokes->points[strokes->size].y = (s16)y;
    (strokes->size)++;
}

/*---------------------------------------------------------------------------*
  Name:         PRC_AppendPenUpMarker

  Description:  Adds marker that means stylus up to PRCStrokes.

  Arguments:    strokes:     target.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void PRC_AppendPenUpMarker(PRCStrokes *strokes)
{
    PRC_AppendPoint(strokes, PRC_PEN_UP_MARKER_X, PRC_PEN_UP_MARKER_Y);
}

/*---------------------------------------------------------------------------*
  Name:         PRC_Clear

  Description:  Empties PRCStrokes.

  Arguments:    strokes:     target.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void PRC_Clear(PRCStrokes *strokes)
{
    SDK_ASSERT(strokes);
    strokes->size = 0;
}

/*---------------------------------------------------------------------------*
  Name:         PRC_IsFull

  Description:  Determines if PRCStrokes is full.

  Arguments:    strokes:     target.

  Returns:      If full, TRUE.
 *---------------------------------------------------------------------------*/
static inline BOOL PRC_IsFull(const PRCStrokes *strokes)
{
    SDK_ASSERT(strokes);
    return strokes->size >= strokes->capacity;
}

/*---------------------------------------------------------------------------*
  Name:         PRC_IsEmpty

  Description:  Determines if PRCStrokes is empty or not.

  Arguments:    strokes:     target.

  Returns:      If empty, TRUE.
 *---------------------------------------------------------------------------*/
static inline BOOL PRC_IsEmpty(const PRCStrokes *strokes)
{
    SDK_ASSERT(strokes);
    return strokes->size == 0;
}

/*---------------------------------------------------------------------------*
  Name:         PRC_IsPenUpMarker

  Description:  Determines if marker is one that means stylus up.

  Arguments:    point:       Target to check.

  Returns:      If it is a stylus up marker, TRUE.
 *---------------------------------------------------------------------------*/
static inline int PRC_IsPenUpMarker(const PRCPoint *point)
{
    SDK_ASSERT(point);
    // If point->x == PRC_PEN_UP_MARKER_X, then point->y==PRC_PEN_UP_MARKER_Y
    SDK_ASSERT((point->x != PRC_PEN_UP_MARKER_X) || (point->y == PRC_PEN_UP_MARKER_Y));
    return (point->x == PRC_PEN_UP_MARKER_X);
}

/*---------------------------------------------------------------------------*
  Name:         PRCi_CalcDistance

  Description:  Calculates Euclidean distance.

  Arguments:    p1, p2:      Targets to check.

  Returns:      Euclidean distance (fx32).
 *---------------------------------------------------------------------------*/
static inline fx32 PRCi_CalcDistance(const PRCPoint *p1, const PRCPoint *p2)
{
    SDK_ASSERT(p1 && p2);
    return (FX_Sqrt
            (((p1->x - p2->x) * (p1->x - p2->x) +
              (p1->y - p2->y) * (p1->y - p2->y)) << FX32_SHIFT));
}

/*---------------------------------------------------------------------------*
  Name:         PRCi_CalcAngle

  Description:  Calculates an angle.

  Arguments:    p1, p2:      Targets to check.

  Returns:      Angle in (2pi/65536) radian units (u16).
 *---------------------------------------------------------------------------*/
static inline u16 PRCi_CalcAngle(const PRCPoint *from, const PRCPoint *to)
{
    SDK_ASSERT(from && to);
    return ((u16)(FX_Atan2Idx(to->y - from->y, to->x - from->x)));
}


#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_PRC_COMMON_H_ */
#endif
