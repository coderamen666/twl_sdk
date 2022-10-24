/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MI - include
  File:     allocator.h

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-11-04#$
  $Rev: 9197 $
  $Author: yosizaki $
 *---------------------------------------------------------------------------*/
#ifndef	NITRO_MI_ALLOCATOR_H_
#define	NITRO_MI_ALLOCATOR_H_


#include <nitro/misc.h>
#include <nitro/types.h>
#include <nitro/platform.h>


#ifdef __cplusplus
extern  "C"
{
#endif


/*---------------------------------------------------------------------------*/
/* Declarations */

/* Memory Allocator Function Prototypes */
typedef void* (*MIAllocatorAllocFunction)(void *userdata, u32 length, u32 alignment);
typedef void  (*MIAllocatorFreeFunction)(void *userdata, void *buffer);

/* Memory Allocator Structures */
typedef struct MIAllocator
{
    void                       *userdata;
    MIAllocatorAllocFunction    Alloc;
    MIAllocatorFreeFunction     Free;
}
MIAllocator;


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         MI_InitAllocator

  Description:  Initializes the allocator.

  Arguments:    allocator        The MIAllocator structure to be initialized.
                userdata         Arbitrary user-defined argument.
                alloc            Pointer to the memory allocation function
                free             Pointer to the memory deallocation function

  Returns:      None.
 *---------------------------------------------------------------------------*/
PLATFORM_ATTRIBUTE_INLINE
void MI_InitAllocator(MIAllocator *allocator, void *userdata,
                      MIAllocatorAllocFunction alloc,
                      MIAllocatorFreeFunction free)
{
    allocator->userdata = userdata;
    allocator->Alloc = alloc;
    allocator->Free = free;
}

/*---------------------------------------------------------------------------*
  Name:         MI_CallAlloc

  Description:  Allocates memory from the allocator.

  Arguments:    allocator        The initialized MIAllocator structure.
                length           The size to be allocated.
                alignment        The required byte-alignment (must be a power of 2).

  Returns:      Either the allocated memory or NULL.
 *---------------------------------------------------------------------------*/
PLATFORM_ATTRIBUTE_INLINE
void* MI_CallAlloc(MIAllocator *allocator, u32 length, u32 alignment)
{
    return allocator->Alloc(allocator->userdata, length, alignment);
}

/*---------------------------------------------------------------------------*
  Name:         MI_CallFree

  Description:  Deallocates memory back to the allocator.

  Arguments:    allocator        The initialized MIAllocator structure.
                buffer           The memory to deallocate.

  Returns:      None.
 *---------------------------------------------------------------------------*/
PLATFORM_ATTRIBUTE_INLINE
void MI_CallFree(MIAllocator *allocator, void *buffer)
{
    allocator->Free(allocator->userdata, buffer);
}


/*---------------------------------------------------------------------------*/


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* NITRO_MI_ALLOCATOR_H_ */
