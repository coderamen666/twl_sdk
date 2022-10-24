/*---------------------------------------------------------------------------*
  Project:  TwlSDK - PRC - 
  File:     prc_algo_common.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-19#$
  $Rev: 10786 $
  $Author: okajima_manabu $
 *---------------------------------------------------------------------------*/

#include <nitro.h>
#include <nitro/prc/resample.h>

/*===========================================================================*
  Prototype Declarations
 *===========================================================================*/


/*===========================================================================*
  Variable Definitions
 *===========================================================================*/

/*===========================================================================*
  Functions
 *===========================================================================*/

static BOOL
PRCi_TerminateStrokes(u16 *selectedPoints,
                      int *pSelectedPointNum, int maxPointCount, const PRCStrokes *strokes)
{
    int     selectedPointNum = *pSelectedPointNum;
    const PRCPoint *inputPoints;

    inputPoints = strokes->points;
    if (selectedPointNum < 2)
    {
        // If length of dot series is 0 or 1, ignore
        *pSelectedPointNum = 0;
        return FALSE;
    }
    if (!PRC_IsPenUpMarker(&inputPoints[selectedPoints[selectedPointNum - 1]]))
    {
        // Doesn't finish with Pen Up Marker
        if (!PRC_IsPenUpMarker(&inputPoints[selectedPoints[selectedPointNum - 2]]))
        {
            // Next to last is not Pen Up Marker either
            if (selectedPointNum < maxPointCount)
            {
                // If there is room at the end, append Pen Up Marker
                selectedPoints[selectedPointNum] = (u16)-1;
                selectedPointNum++;
            }
            else
            {
                // If no space at the end
                if (selectedPointNum >= 3
                    && !PRC_IsPenUpMarker(&inputPoints[selectedPoints[selectedPointNum - 3]]))
                {
                    // If the third from last is also not a Pen Up Marker, overwrite the last one with Pen Up
                    selectedPoints[selectedPointNum - 1] = (u16)-1;
                }
                else
                {
                    // Otherwise delete two
                    selectedPointNum -= 2;
                }
            }
        }
        else
        {
            // If the next to last is a Pen Up Marker, reduce by one without any pause to think
            selectedPointNum--;
        }
    }

    *pSelectedPointNum = selectedPointNum;
    return TRUE;
}


/*---------------------------------------------------------------------------*
  Name:         PRC_ResampleStrokes_None

  Description:  Does not resample.

  Arguments:    selectedPoints, pSelectedPointNum: Pointers which return results
                maxPointCount: Upper limit of number of input points (includes pen up marker)
                maxStrokeCount: Upper limit of stroke count
                strokes: Raw input coordinate values before sorting
                threshold: Resampling threshold
                buffer: Work area ([sizeof(int)*maxPointCount] required)

  Returns:      TRUE if resampling succeeded.
 *---------------------------------------------------------------------------*/
BOOL
PRC_ResampleStrokes_None(u16 *selectedPoints,
                         int *pSelectedPointNum,
                         int maxPointCount,
                         int maxStrokeCount, const PRCStrokes *strokes, int threshold, void *buffer)
{
    // Use the raw dot sequence data without resampling
    u16     iPoint;
    int     size = strokes->size;

    (void)maxStrokeCount;
    (void)threshold;
    (void)buffer;

    if (size > maxPointCount)
    {
        size = maxPointCount;
    }
    if (size < 2)
    {
        // If length of dot series is 0 or 1, ignore
        *pSelectedPointNum = 0;
    }
    else
    {
        // Select point automatically
        for (iPoint = 0; iPoint < size; iPoint++)
        {
            selectedPoints[iPoint] = iPoint;
        }
        *pSelectedPointNum = iPoint;

        if (!PRC_IsPenUpMarker(&strokes->points[size - 1]))
        {
            // Not terminated with Pen Up Marker
            (void)PRCi_TerminateStrokes(selectedPoints, pSelectedPointNum, maxPointCount, strokes);
        }
    }

    return (*pSelectedPointNum > 0);
}

#define PRCi_ABS(x) (((x)>=0)?(x):-(x))

/*---------------------------------------------------------------------------*
  Name:         PRC_ResampleStrokes_Distance

  Description:  Resamples based on city block distance.

  Arguments:    selectedPoints, pSelectedPointNum: Pointers which return results
                maxPointCount: Upper limit of number of input points (includes pen up marker)
                maxStrokeCount: Upper limit of stroke count
                strokes: Raw input coordinate values before sorting
                threshold: Resampling threshold
                buffer: Work area ([sizeof(int)*maxPointCount] required)

  Returns:      TRUE if resampling succeeded.
 *---------------------------------------------------------------------------*/
BOOL
PRC_ResampleStrokes_Distance(u16 *selectedPoints,
                             int *pSelectedPointNum,
                             int maxPointCount,
                             int maxStrokeCount,
                             const PRCStrokes *strokes, int threshold, void *buffer)
{
    int     selectedPointNum;
    int     strokeCount;
    int     iPoint;
    int     size;
    PRCPoint prevPoint;
    PRCPoint *point;
    BOOL    newFlag;
    int     length;

    SDK_ASSERT(maxPointCount > 0);
    SDK_ASSERT(maxStrokeCount > 0);

    (void)buffer;

    selectedPointNum = 0;
    strokeCount = 0;

    size = strokes->size;
    point = strokes->points;

    newFlag = TRUE;
    for (iPoint = 0; iPoint < size && selectedPointNum < maxPointCount; iPoint++, point++)
    {
        if (!PRC_IsPenUpMarker(point))
        {
            if (newFlag)
            {
                // The starting point is always selected
                selectedPoints[selectedPointNum] = (u16)iPoint;
                selectedPointNum++;
                length = 0;
                newFlag = FALSE;
            }
            else
            {
                length += PRCi_ABS(point->x - prevPoint.x) + PRCi_ABS(point->y - prevPoint.y);
                if (length >= threshold)
                {
                    selectedPoints[selectedPointNum] = (u16)iPoint;
                    selectedPointNum++;
                    length = 0;
                }
            }
            prevPoint = *point;
        }
        else
        {
            if (newFlag)
            {
                // Ignore several Pen Up Markers in a row
                continue;
            }
            else
            {
                if (selectedPoints[selectedPointNum - 1] != iPoint - 1) // When execution comes here, [selectedPointNum>0] is always true 
                {
                    // End point is always selected
                    selectedPoints[selectedPointNum] = (u16)(iPoint - 1);
                    selectedPointNum++;
                    if (selectedPointNum >= maxPointCount)
                    {
                        break;
                    }
                }
                selectedPoints[selectedPointNum] = (u16)iPoint;
                selectedPointNum++;
                newFlag = TRUE;

                strokeCount++;
                if (strokeCount >= maxStrokeCount)
                {
                    // Max number of strokes exceeded
                    iPoint++;          // For the processing after the loop. // At the present time, has no meaning because newFlag == TRUE
                    break;
                }
            }
        }
    }

    *pSelectedPointNum = selectedPointNum;

    if (!newFlag)
    {
        // Not terminated with Pen Up Marker

        // First, confirm that end point is selected
        if (selectedPointNum > 0 && selectedPoints[selectedPointNum - 1] != iPoint - 1
            && selectedPointNum < maxPointCount)
        {
            // End point is always selected
            selectedPoints[*pSelectedPointNum] = (u16)(iPoint - 1);
            (*pSelectedPointNum)++;
        }

        (void)PRCi_TerminateStrokes(selectedPoints, pSelectedPointNum, maxPointCount, strokes);
    }

    return (*pSelectedPointNum > 0);
}

/*---------------------------------------------------------------------------*
  Name:         PRC_ResampleStrokes_Angle

  Description:  Resamples, based on the angle difference.

  Arguments:    selectedPoints, pSelectedPointNum: Pointers which return results
                maxPointCount: Upper limit of number of input points (includes pen up marker)
                maxStrokeCount: Upper limit of stroke count
                strokes: Raw input coordinate values before sorting
                threshold: Resampling threshold
                buffer: Work area ([sizeof(int)*maxPointCount] required)

  Returns:      TRUE if resampling succeeded.
 *---------------------------------------------------------------------------*/
BOOL
PRC_ResampleStrokes_Angle(u16 *selectedPoints,
                          int *pSelectedPointNum,
                          int maxPointCount,
                          int maxStrokeCount,
                          const PRCStrokes *strokes, int threshold, void *buffer)
{
#define PRC_RESAMPLE_ANGLE_LENGTH_THRESHOLD 6   // A valid angle cannot be obtained unless one moves about 6 city block lengths away
    int     selectedPointNum;
    int     strokeCount;
    int     iPoint;
    int     size;
    PRCPoint *point;
    BOOL    newFlag;
    u16     prevAngle;
    PRCPoint prevPoint;
    BOOL    firstFlag;

    SDK_ASSERT(maxPointCount > 0);
    SDK_ASSERT(maxStrokeCount > 0);

    (void)buffer;

    selectedPointNum = 0;
    strokeCount = 0;

    size = strokes->size;
    point = strokes->points;

    newFlag = TRUE;
    for (iPoint = 0; iPoint < size && selectedPointNum < maxPointCount; iPoint++, point++)
    {
        if (!PRC_IsPenUpMarker(point))
        {
            if (newFlag)
            {
                // The starting point is always selected
                selectedPoints[selectedPointNum] = (u16)iPoint;
                selectedPointNum++;
                prevPoint = *point;
                if (iPoint + 1 < size)
                {
                    prevAngle =
                        FX_Atan2Idx(((point + 1)->y - point->y) << FX32_SHIFT,
                                    ((point + 1)->x - point->x) << FX32_SHIFT);
                    newFlag = FALSE;
                    firstFlag = TRUE;
                }
            }
            else
            {
                int     length;
                length = PRCi_ABS(point->x - prevPoint.x) + PRCi_ABS(point->y - prevPoint.y);
                if (length >= PRC_RESAMPLE_ANGLE_LENGTH_THRESHOLD)
                {
                    if (firstFlag)
                    {
                        // Watch the angle formed with the next point until the second point is selected.
                        // Otherwise, the direction of the initial segment will be lost.
                        if (iPoint + 1 < size && !PRC_IsPenUpMarker(point + 1))
                            // If [point+1] is the Pen Up Marker, then it is always selected as the endpoint and the following conditions are not needed, but...
                            // 
                        {
                            u16     currAngle, nextAngle;
                            nextAngle =
                                FX_Atan2Idx(((point + 1)->y - point->y) << FX32_SHIFT,
                                            ((point + 1)->x - point->x) << FX32_SHIFT);
                            if (PRCi_ABS((s16)(prevAngle - nextAngle)) >= threshold)
                            {
                                currAngle =
                                    FX_Atan2Idx((point->y - prevPoint.y) << FX32_SHIFT,
                                                (point->x - prevPoint.x) << FX32_SHIFT);
                                selectedPoints[selectedPointNum] = (u16)iPoint;
                                selectedPointNum++;
                                prevAngle = currAngle;
                                firstFlag = FALSE;
                            }
                        }
                    }
                    else
                    {
                        u16     currAngle;
                        currAngle =
                            FX_Atan2Idx((point->y - prevPoint.y) << FX32_SHIFT,
                                        (point->x - prevPoint.x) << FX32_SHIFT);
                        if (PRCi_ABS((s16)(prevAngle - currAngle)) >= threshold)
                        {
                            selectedPoints[selectedPointNum] = (u16)iPoint;
                            selectedPointNum++;
                            prevAngle = currAngle;
                        }
                    }
                    prevPoint = *point;
                }
            }
        }
        else
        {
            if (newFlag)
            {
                // Ignore several Pen Up Markers in a row
                continue;
            }
            else
            {
                if (selectedPoints[selectedPointNum - 1] != iPoint - 1) // Always selectedPointNum>0 when comes to here
                {
                    // End point is always selected
                    selectedPoints[selectedPointNum] = (u16)(iPoint - 1);
                    selectedPointNum++;
                    if (selectedPointNum >= maxPointCount)
                    {
                        break;
                    }
                }
                selectedPoints[selectedPointNum] = (u16)iPoint;
                selectedPointNum++;
                newFlag = TRUE;

                strokeCount++;
                if (strokeCount >= maxStrokeCount)
                {
                    // Max number of strokes exceeded
                    iPoint++;          // For the processing after the loop. // At the present time, has no meaning because newFlag == TRUE
                    break;
                }
            }
        }
    }

    *pSelectedPointNum = selectedPointNum;

    if (!newFlag)
    {
        // Not terminated with Pen Up Marker

        // First, confirm that end point is selected
        if (selectedPointNum > 0 && selectedPoints[selectedPointNum - 1] != iPoint - 1
            && selectedPointNum < maxPointCount)
        {
            // End point is always selected
            selectedPoints[*pSelectedPointNum] = (u16)(iPoint - 1);
            (*pSelectedPointNum)++;
        }

        (void)PRCi_TerminateStrokes(selectedPoints, pSelectedPointNum, maxPointCount, strokes);
    }

    return (*pSelectedPointNum > 0);
}

/*---------------------------------------------------------------------------*
  Name:         PRC_ResampleStrokes_Recursive

  Description:  Resamples with recursive method.

  Arguments:    selectedPoints, pSelectedPointNum: Pointers which return results
                maxPointCount: Upper limit of number of input points (includes pen up marker)
                maxStrokeCount: Upper limit of stroke count
                strokes: Raw input coordinate values before sorting
                threshold: Resampling threshold
                buffer: Work area ([sizeof(int)*maxPointCount] required)

  Returns:      TRUE if resampling succeeded.
 *---------------------------------------------------------------------------*/
BOOL
PRC_ResampleStrokes_Recursive(u16 *selectedPoints,
                              int *pSelectedPointNum,
                              int maxPointCount,
                              int maxStrokeCount,
                              const PRCStrokes *strokes, int threshold, void *buffer)
{
    u16     beginIndex, endIndex;
    int     stackSize;
    int     stackTop, stackTail;
    int     strokeCount;
    int     selectedPointNum;
    int     size;
    PRCPoint *inputPoints;
    u16    *stackP1;
    u16    *stackP2;
    int     squaredThreshold;

    stackP1 = (u16 *)buffer;
    stackP2 = (u16 *)buffer + maxPointCount;

    squaredThreshold = threshold * threshold;

    beginIndex = 0;
    endIndex = 0;
    strokeCount = 0;
    selectedPointNum = 0;

    inputPoints = strokes->points;
    size = strokes->size;

    while (1)
    {
        if (selectedPointNum + 3 > maxPointCount || strokeCount > maxStrokeCount)
        {
            // No space to store the next stroke
            // To store a single 'stroke' you must have at least three available spaces: starting point, end point, and PenUpMarker.
            // 
            break;
        }

        // Skip over PenUpMarker
        while (endIndex < size && PRC_IsPenUpMarker(&inputPoints[endIndex]))
        {
            endIndex++;
        }

        beginIndex = endIndex;
        if (beginIndex >= size)
        {
            // Quit
            break;
        }

        // Search for next PenUpMarker
        while (endIndex < size && !PRC_IsPenUpMarker(&inputPoints[endIndex]))
        {
            endIndex++;
        }
        if (endIndex < size)
        {
            selectedPoints[selectedPointNum] = endIndex;
            selectedPointNum++;        // Select required PenUpMarker
        }
        else
        {
            selectedPoints[selectedPointNum] = (u16)-1;
            selectedPointNum++;        // -1 is specially reserved to indicate the final PenUpMarker
        }

        SDK_ASSERT(endIndex > 0);
        selectedPoints[selectedPointNum] = beginIndex;
        selectedPointNum++;
        selectedPoints[selectedPointNum] = (u16)(endIndex - 1);
        selectedPointNum++;

        strokeCount++;                 // strokeCount is only counted for the sake of limiting it with maxStrokeCount

        if (selectedPointNum >= maxPointCount)
        {
            // Number of vertices is at the limit
            break;
        }

        if (endIndex - beginIndex <= 2)
            continue;

        // Using the stack, recursively extract characteristic points
        stackP1[0] = beginIndex;
        stackP2[0] = (u16)(endIndex - 1);
        stackSize = 1;
        stackTop = 0;
        stackTail = 1;
        while (stackSize > 0)
        {
            u16     p1, p2;
            int     x1, y1, x2, y2, xDir, yDir, offs;
            int     lastX, lastY;
            int     i;
            int     maxDist;
            u16     maxP;

            p1 = stackP1[stackTop];
            p2 = stackP2[stackTop];
            stackTop++;
            if (stackTop >= maxPointCount)
            {
                stackTop = 0;
            }
            stackSize--;               // pop

            if (p2 - p1 <= 1)
                continue;

            x1 = inputPoints[p1].x;    // Starting point
            y1 = inputPoints[p1].y;
            x2 = inputPoints[p2].x;    // End point
            y2 = inputPoints[p2].y;
            xDir = x2 - x1;            // Direction vector
            yDir = y2 - y1;
            offs = -(x1 * y2 - x2 * y1);        // Items bundled together in order to reduce the amount of calculation.

            maxDist = -1;
            maxP = (u16)-1;
            lastX = -1;
            lastY = -1;
            for (i = p1 + 1; i < p2; i++)
            {
                int     dist;
                int     x, y;
                x = inputPoints[i].x;
                y = inputPoints[i].y;

                if (lastX == x && lastY == y)
                    continue;
                lastX = x;
                lastY = y;

                // Calculate the distance between the straight line and the point
                // The actual value is the original distance multiplied by the distance from the start point to end point
                dist = x * yDir - y * xDir + offs;
                if (dist < 0)
                {
                    dist = -dist;
                }

                if (maxDist < dist)
                {
                    maxP = (u16)i;
                    maxDist = dist;
                }
            }

            // Because [maxDist == 0] in cases where the start coordinates and end coordinates exactly match, it would be preferable if the 'dist' calculation separately found the Euclidean distance between (x, y) and the start point, but because we don't want to introduce special processing for rare cases, try coding so that [p1+1] is always used when [maxDist==0] and [maxP==p1+1]. (Note that [xDir*xDir+yDir*yDir == 0] at such times.)
            // 
            // 
            // 
            if (maxDist * maxDist >= (xDir * xDir + yDir * yDir) * squaredThreshold)
            {
                // Adopt points located at or outside the threshold as characteristic points
                // Note that maxDist was the original distance multiplied by the distance from the start point to end point.
                selectedPoints[selectedPointNum] = maxP;
                selectedPointNum++;
                stackP1[stackTail] = maxP;
                stackP2[stackTail] = p2;
                stackTail++;
                if (stackTail >= maxPointCount)
                {
                    stackTail = 0;
                }
                stackSize++;           // push
                stackP1[stackTail] = p1;
                stackP2[stackTail] = maxP;
                stackTail++;
                if (stackTail >= maxPointCount)
                {
                    stackTail = 0;
                }
                stackSize++;           // push
                SDK_ASSERT(stackSize <= maxPointCount);
                if (selectedPointNum >= maxPointCount)
                {
                    // Number of vertices is at the limit
                    break;
                }
            }
        }
    }

    *pSelectedPointNum = selectedPointNum;

//{OSTick start, end; start = OS_GetTick();
    // Sort in ascending order before returning
    // To Do: Examine to what degree it is sped up by quick sort
    {
        int     i, j;
        for (i = 0; i < selectedPointNum - 1; i++)
        {
            for (j = i + 1; j < selectedPointNum; j++)
            {
                if (selectedPoints[i] > selectedPoints[j])
                {
                    u16     tmp;
                    tmp = selectedPoints[i];
                    selectedPoints[i] = selectedPoints[j];
                    selectedPoints[j] = tmp;
                }
            }
        }
    }
//end = OS_GetTick(); OS_TPrintf("// sort in resample: %lldmicro s selectedPointNum=%d\n", OS_TicksToMicroSeconds(end-start), selectedPointNum); }
    return (*pSelectedPointNum > 0);
}























/*===========================================================================*
  Static Functions
 *===========================================================================*/


/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
