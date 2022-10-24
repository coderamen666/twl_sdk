/*---------------------------------------------------------------------------*
  Project:  TWL-System - include - nnsys - fnd
  File:     heapcommon.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

#ifndef NNS_FND_HEAPCOMMON_H_
#define NNS_FND_HEAPCOMMON_H_

#include <nitro/types.h>
#include <nnsys/fnd/list.h>

#ifdef __cplusplus
extern "C" {
#endif


/* ========================================================================
    Macro Constants
   ======================================================================== */

// Invalid heap handle
#define NNS_FND_HEAP_INVALID_HANDLE     NULL

// Default alignment size when memory is allocated from heap
#define NNS_FND_HEAP_DEFAULT_ALIGNMENT    4

// Signature of expanded heap
#define NNSI_EXPHEAP_SIGNATURE    ('EXPH')

// Signature of frame heap
#define NNSI_FRMHEAP_SIGNATURE    ('FRMH')

// Signature of unit heap
#define NNSI_UNTHEAP_SIGNATURE    ('UNTH')


/* ------------------------------------------------------------------------
    Fill-related
   ------------------------------------------------------------------------ */

// Zero out memory when memory is allocated.
#define NNS_FND_HEAP_OPT_0_CLEAR        (1 <<0)

// Memory fill when heap is created, memory is allocated, or memory is freed.
#define NNS_FND_HEAP_OPT_DEBUG_FILL     (1 <<1)


/* ------------------------------------------------------------------------
    Heap check related
   ------------------------------------------------------------------------ */

//  If this bit stands, output error
#define NNS_FND_HEAP_ERROR_PRINT        (1 <<0)


/* ========================================================================
    enum constant
   ======================================================================== */

enum {
    NNS_FND_HEAP_FILL_NOUSE,    // When debug fill is not used
    NNS_FND_HEAP_FILL_ALLOC,    // When debug fill is allocated
    NNS_FND_HEAP_FILL_FREE,     // When debug fill is freed

    NNS_FND_HEAP_FILL_MAX
};


/* =======================================================================
    Type Definitions
   ======================================================================== */

typedef struct NNSiFndHeapHead NNSiFndHeapHead;

// Header common among heaps
struct NNSiFndHeapHead
{
    u32             signature;

    NNSFndLink      link;
    NNSFndList      childList;

    void*           heapStart;      // Heap start address
    void*           heapEnd;        // Heap end (+1) address

    u32             attribute;      // Attribute
                                    // [8:Option flag]
};

typedef NNSiFndHeapHead* NNSFndHeapHandle;   // Type to represent heap handle


/* ========================================================================
    Macro Functions
   ======================================================================== */

/*---------------------------------------------------------------------------*
  Name:         NNS_FndGetHeapStartAddress

  Description:  Gets start address of memory area used by heap

  Arguments:    heap:  Heap handle

  Returns:      Return start address of memory area used by heap
 *---------------------------------------------------------------------------*/
#define             NNS_FndGetHeapStartAddress(heap) \
                        ((void*)(heap))

/*---------------------------------------------------------------------------*
  Name:         NNS_FndGetHeapEndAddress

  Description:  Gets end address +1 of memory area used by heap

  Arguments:    heap:  Heap handle

  Returns:      Return end address +1 of memory area used by heap
 *---------------------------------------------------------------------------*/
#define             NNS_FndGetHeapEndAddress(heap) \
                        (((NNSiFndHeapHead*)(heap))->heapEnd)


/* =======================================================================
    Function Prototypes
   ======================================================================== */

NNSFndHeapHandle    NNS_FndFindContainHeap(
                        const void* memBlock);

#if defined(NNS_FINALROM)
    #define             NNS_FndDumpHeap(heap)				((void)0)
#else
	void                NNS_FndDumpHeap(
	                        NNSFndHeapHandle heap);
#endif

#if defined(NNS_FINALROM)
    #define             NNS_FndSetFillValForHeap(type, val) (0)
#else
    u32                 NNS_FndSetFillValForHeap(
                            int     type,
                            u32     val);
#endif

#if defined(NNS_FINALROM)
    #define             NNS_FndGetFillValForHeap(type)      (0)
#else
    u32                 NNS_FndGetFillValForHeap(
                            int     type);
#endif


#ifdef __cplusplus
} /* extern "C" */
#endif

/* NNS_FND_HEAPCOMMON_H_ */
#endif
