/*---------------------------------------------------------------------------*
  Project:  TWL-System - include - nnsys - fnd
  File:     frameheap.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

#ifndef NNS_FND_FRAMEHEAP_H_
#define NNS_FND_FRAMEHEAP_H_

#include <nnsys/fnd/heapcommon.h>

#ifdef __cplusplus
extern "C" {
#endif


/* =======================================================================
    Constant Definitions
   ======================================================================== */

#define NNS_FND_FRMHEAP_FREE_HEAD (1 <<0)
#define NNS_FND_FRMHEAP_FREE_TAIL (1 <<1)
#define NNS_FND_FRMHEAP_FREE_ALL  (NNS_FND_FRMHEAP_FREE_HEAD | NNS_FND_FRMHEAP_FREE_TAIL)


/* =======================================================================
    Type Definitions
   ======================================================================== */

typedef struct NNSiFndFrmHeapState NNSiFndFrmHeapState;

// Structure for state storage
struct NNSiFndFrmHeapState
{
	u32						tagName;        // Tag name
	void*					headAllocator;  // Head location of frame heap
	void*					tailAllocator;  // Tail location of frame heap
	NNSiFndFrmHeapState*    pPrevState;     // Pointer for last state storage
};

typedef struct NNSiFndFrmHeapHead NNSiFndFrmHeapHead;

// Header information for frame heap
struct NNSiFndFrmHeapHead
{
	void*				    headAllocator;  // Pointer for head memory allocation
	void*				    tailAllocator;  // Pointer for tail memory allocation

	NNSiFndFrmHeapState*	pState;         // State storage parameter
};


/* =======================================================================
    Macro Functions
   ======================================================================== */

/*---------------------------------------------------------------------------*
  Name:         NNS_FndCreateFrmHeap

  Description:  Creates a frame heap.

  Arguments:    startAddress: Start address of heap area
                size:         Size of heap area

  Returns:      Returns the handle for the created frame heap if the function succeeds.
                If function fails, NNS_FND_INVALID_HEAP_HANDLE is returned.
 *---------------------------------------------------------------------------*/
#define             NNS_FndCreateFrmHeap(startAddress, size) \
                        NNS_FndCreateFrmHeapEx(startAddress, size, 0)

/*---------------------------------------------------------------------------*
  Name:         NNS_FndAllocFromFrmHeap

  Description:  Allocates a memory block from the frame heap.
                Alignment of the memory block is 4-byte fixed.

  Arguments:    heap:   Handle for the frame heap.
                size:   Size of the memory block to be allocated (in bytes)

  Returns:      Returns a pointer to the allocated memory block when that allocation succeeded.
                
                If the operation fails, NULL is returned.
 *---------------------------------------------------------------------------*/
#define             NNS_FndAllocFromFrmHeap(heap, size) \
                        NNS_FndAllocFromFrmHeapEx(heap, size, NNS_FND_HEAP_DEFAULT_ALIGNMENT)

/*---------------------------------------------------------------------------*
  Name:         NNS_FndGetAllocatableSizeForFrmHeap

  Description:  Gets the maximum allocatable size in the frame heap.
                Alignment of the memory block is 4-byte fixed.

  Arguments:    heap:      Handle for the frame heap.

  Returns:      Returns the maximum allocatable size in the frame heap (in bytes).
 *---------------------------------------------------------------------------*/
#define             NNS_FndGetAllocatableSizeForFrmHeap(heap) \
                        NNS_FndGetAllocatableSizeForFrmHeapEx(heap, NNS_FND_HEAP_DEFAULT_ALIGNMENT)


/* =======================================================================
    Function Prototypes
   ======================================================================== */

void*               NNSi_FndGetFreeStartForFrmHeap(
                        NNSFndHeapHandle    heap);

void*               NNSi_FndGetFreeEndForFrmHeap(
                        NNSFndHeapHandle    heap);

#if ! defined(NNS_FINALROM)

    void                NNSi_FndDumpFrmHeap(
                            NNSFndHeapHandle    heap);

// #if ! defined(NNS_FINALROM)
#endif

NNSFndHeapHandle    NNS_FndCreateFrmHeapEx(
                        void*   startAddress,
                        u32     size,
                        u16     optFlag);

void                NNS_FndDestroyFrmHeap(
                        NNSFndHeapHandle    heap);

void*               NNS_FndAllocFromFrmHeapEx(
                        NNSFndHeapHandle    heap,
                        u32                 size,
                        int                 alignment);

void                NNS_FndFreeToFrmHeap(
                        NNSFndHeapHandle    heap,
                        int                 mode);

u32                 NNS_FndGetAllocatableSizeForFrmHeapEx(
                        NNSFndHeapHandle    heap,
                        int                 alignment);

BOOL                NNS_FndRecordStateForFrmHeap(
                        NNSFndHeapHandle    heap,
                        u32                 tagName);

BOOL                NNS_FndFreeByStateToFrmHeap(
                        NNSFndHeapHandle    heap,
                        u32                 tagName);

u32                 NNS_FndAdjustFrmHeap(
                        NNSFndHeapHandle    heap);

u32                 NNS_FndResizeForMBlockFrmHeap(
                        NNSFndHeapHandle    heap,
                        void*               memBlock,
                        u32                 newSize);


#ifdef __cplusplus
} /* extern "C" */
#endif

/* NNS_FND_FRAMEHEAP_H_ */
#endif
