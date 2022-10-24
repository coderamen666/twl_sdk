/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include
  File:     qsort.h

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev:$
  $Author:$
 *---------------------------------------------------------------------------*/

#ifndef NITRO_MATH_QSORT_H_
#define NITRO_MATH_QSORT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/misc.h>
#include <nitro/types.h>
#include <nitro/math/math.h>

/* Function for comparing values. */
typedef s32 (*MATHCompareFunc) (void *elem1, void *elem2);

/* Maximum necessary stack size. */
/*---------------------------------------------------------------------------*
  Name:         MATH_QSortStackSize
  
  Description:  Calculates the necessary work buffer size for when executing MATH_QSort.
  
  Arguments:    num:      Number of data items to sort
                
  Returns:      Necessary buffer size.
 *---------------------------------------------------------------------------*/
static inline u32 MATH_QSortStackSize(u32 num)
{
    int     tmp = MATH_ILog2(num);

    if (tmp <= 0)
    {
        return sizeof(u32);
    }
    else
    {
        return (u32)((tmp + 1) * sizeof(u32) * 2);
    }
}


/*---------------------------------------------------------------------------*
  Name:         MATH_QSort
  
  Description:  Performs a quicksort without using recursion.
                The buffer area used for sorting must be passed, and the required buffer size can be obtained with MATH_QSORT_STACK_SIZE( num ).
                
  
  Arguments:    head:     Pointer to the data to sort
                num:      Number of data items to sort
                width:    Data size of one portion of data to sort
                comp:     Pointer to comparison function
                stackBuf:  Buffer to use for sorting
                
  Returns:      None.
 *---------------------------------------------------------------------------*/
void    MATH_QSort(void *head, u32 num, u32 width, MATHCompareFunc comp, void *stackBuf);


#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_MATH_QSORT_H_ */
#endif
