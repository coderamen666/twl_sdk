/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - fnd
  File:     heapcommon.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
#include <nitro.h>
#include <nnsys/misc.h>
#include <nnsys/fnd/heapcommon.h>
#include <nnsys/fnd/expheap.h>
#include <nnsys/fnd/frameheap.h>
#include <nnsys/fnd/unitheap.h>
#include <nnsys/fnd/config.h>
#include "heapcommoni.h"


/* ========================================================================
    static variables
   ======================================================================== */

/* ------------------------------------------------------------------------
    list-related
   ------------------------------------------------------------------------ */

static NNSFndList sRootList;                // root heap list
static BOOL sRootListInitialized = FALSE;   // if sRootList is initialized, true


/* ------------------------------------------------------------------------
    fill-related
   ------------------------------------------------------------------------ */

#if ! defined(NNS_FINALROM)

    static u32 sFillVals[NNS_FND_HEAP_FILL_MAX] =
    {
        0xC3C3C3C3, // value to fill with when creating heap
        0xF3F3F3F3, // value to fill with when allocating memory block
        0xD3D3D3D3, // value to fill with when deallocating memory block
    };

// #if ! defined(NNS_FINALROM)
#endif


/* ========================================================================
    static functions
   ======================================================================== */

/* ------------------------------------------------------------------------
    list-related
   ------------------------------------------------------------------------ */

/*---------------------------------------------------------------------------*
  Name:         FindContainHeap

  Description:  Recursively searches a list for the heap that contains the specified memory block.
                

  Arguments:    pList:     Pointer to list
                memBlock:  Pointer to memory block

  Returns:      Returns a pointer to the heap that allocated the specifed memory block if that heap is found.
                
                Returns NULL if it is not found.
 *---------------------------------------------------------------------------*/
static NNSiFndHeapHead*
FindContainHeap(
    NNSFndList* pList,
    const void* memBlock
)
{
    NNSiFndHeapHead* pHeapHd = NULL;
    while (NULL != (pHeapHd = NNS_FndGetNextListObject(pList, pHeapHd)))
    {
        if ( NNSiGetUIntPtr(pHeapHd->heapStart) <= NNSiGetUIntPtr(memBlock)
         &&  NNSiGetUIntPtr(memBlock) < NNSiGetUIntPtr(pHeapHd->heapEnd)
        )
        {
            NNSiFndHeapHead* pChildHeapHd = FindContainHeap(&pHeapHd->childList, memBlock);
            if(pChildHeapHd)
            {
                return pChildHeapHd;
            }

            return pHeapHd;
        }
    }

    return NULL;
}

/*---------------------------------------------------------------------------*
  Name:         FindListContainHeap

  Description:  Searches the parent heap that contains the heap, and returns a pointer to the parent heap list.
                

  Arguments:    pHeapHd:  Pointer to the header of search target heap

  Returns:      Returns a pointer to the parent heap's child list if the parent heap that includes the specified heap is found.
                
                If the parent heap is not found, a pointer to the root list is returned.
 *---------------------------------------------------------------------------*/
static NNSFndList*
FindListContainHeap(NNSiFndHeapHead* pHeapHd)
{
    NNSFndList* pList = &sRootList;

    NNSiFndHeapHead* pContainHeap = FindContainHeap(&sRootList, pHeapHd);

    if(pContainHeap)
    {
        pList = &pContainHeap->childList;
    }

    return pList;
}

#if 1
    static NNS_FND_INLINE void
    DumpHeapList() {}
#else
    static void
    DumpHeapList()
    {
        NNSiFndHeapHead* pHeapHd = NULL;
        int count = 0;

        OS_Printf("Dump Heap List\n");
        while (NULL != (pHeapHd = NNS_FndGetNextListObject(&sRootList, pHeapHd)))
        {
            count++;
            OS_Printf("[%d] -> %p %08X\n", count, pHeapHd, pHeapHd->signature);
        }
    }
#endif

/* ========================================================================
    external functions (non-public)
   ======================================================================== */

/*---------------------------------------------------------------------------*
  Name:         NNSi_FndInitHeapHead

  Description:  Initializes the heap header.

  Arguments:    pHeapHd:    Pointer to the heap header
                signature:  Signature
                heapStart:  Start address of heap memory
                heapEnd:    End address +1 of heap memory
                optFlag:    Heap option

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
NNSi_FndInitHeapHead(
    NNSiFndHeapHead*    pHeapHd,
    u32                 signature,
    void*               heapStart,
    void*               heapEnd,
    u16                 optFlag
)
{
    pHeapHd->signature = signature;

    pHeapHd->heapStart = heapStart;
    pHeapHd->heapEnd   = heapEnd;

    pHeapHd->attribute = 0;
    SetOptForHeap(pHeapHd, optFlag);

    FillNoUseMemory(
        pHeapHd,
        heapStart,
        GetOffsetFromPtr(heapStart, heapEnd));

    NNS_FND_INIT_LIST(&pHeapHd->childList, NNSiFndHeapHead, link);

    // heap list operation
    if(! sRootListInitialized)
    {
        NNS_FND_INIT_LIST(&sRootList, NNSiFndHeapHead, link);
        sRootListInitialized = TRUE;
    }

    NNS_FndAppendListObject(FindListContainHeap(pHeapHd), pHeapHd);
    DumpHeapList();

}

/*---------------------------------------------------------------------------*
  Name:         NNSi_FndFinalizeHeap

  Description:  Performs common heap cleanup.

  Arguments:    pHeapHd:  Pointer to the heap header

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
NNSi_FndFinalizeHeap(NNSiFndHeapHead* pHeapHd)
{
    // heap list operation
    NNS_FndRemoveListObject(FindListContainHeap(pHeapHd), pHeapHd);
    DumpHeapList();
}


/*---------------------------------------------------------------------------*
  Name:         NNSi_FndDumpHeapHead

  Description:  Displays heap header information.

  Arguments:    pHeapHd:  Pointer to the heap header

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
NNSi_FndDumpHeapHead(NNSiFndHeapHead* pHeapHd)
{
    OS_Printf("[NNS Foundation ");

    switch(pHeapHd->signature)
    {
    case NNSI_EXPHEAP_SIGNATURE: OS_Printf("Exp");   break;
    case NNSI_FRMHEAP_SIGNATURE: OS_Printf("Frame"); break;
    case NNSI_UNTHEAP_SIGNATURE: OS_Printf("Unit");  break;
    default:
        NNS_ASSERT(FALSE);
    }

    OS_Printf(" Heap]\n");

    OS_Printf("    whole [%p - %p)\n", pHeapHd, pHeapHd->heapEnd);
}


/* ========================================================================
    External Functions (public)
   ======================================================================== */

/*---------------------------------------------------------------------------*
  Name:         NNS_FndFindContainHeap

  Description:  Searches for the heap containing the memory block.

  Arguments:    memBlock:  Memory block to be searched for

  Returns:      Returns a handle to the heap that includes the specifeed memory block if that heap is found.
                
                If it is not found, NNS_FND_HEAP_INVALID_HANDLE is returned.
 *---------------------------------------------------------------------------*/
NNSFndHeapHandle
NNS_FndFindContainHeap(const void* memBlock)
{
    return FindContainHeap(&sRootList, memBlock);
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndDumpHeap

  Description:  Displays the information in the heap.
                This function is used for debugging.

  Arguments:    heap:    Handle for the frame heap.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#if ! defined(NNS_FINALROM)

    void
    NNS_FndDumpHeap(NNSFndHeapHandle heap)
    {
        NNSiFndHeapHead* pHeapHd = heap;
        switch(pHeapHd->signature)
        {
        case NNSI_EXPHEAP_SIGNATURE: NNSi_FndDumpExpHeap(heap);  break;
        case NNSI_FRMHEAP_SIGNATURE: NNSi_FndDumpFrmHeap(heap);  break;
        case NNSI_UNTHEAP_SIGNATURE: NNSi_FndDumpUnitHeap(heap); break;
        default:
            OS_Printf("[NNS Foundation] dump heap : unknown heap. - %p\n", heap);
        }
    }

// #if ! defined(NNS_FINALROM)
#endif

/*---------------------------------------------------------------------------*
  Name:         NNS_FndSetFillValForHeap

  Description:  Sets the value to be set to memory when a heap is created and when memory blocks are allocated and deallocated.
                
                This function is for use in debugging.
                Always returns 0 in the final ROM version library (FINALROM).

  Arguments:    type:  Type of value to get
                val:   Value to set

  Returns:      Returns the previous value that was set in memory when the memory block was allocated.
 *---------------------------------------------------------------------------*/

#if ! defined(NNS_FINALROM)

    u32
    NNS_FndSetFillValForHeap(
        int     type,
        u32     val
    )
    {
        NNS_ASSERT(type < NNS_FND_HEAP_FILL_MAX);

        {
            u32 oldVal = sFillVals[type];
            sFillVals[type] = val;
            return oldVal;
        }
    }

// #if ! defined(NNS_FINALROM)
#endif


/*---------------------------------------------------------------------------*
  Name:         NNS_FndGetFillValForHeap

  Description:  Gets the value to be set to memory when a heap is created and when memory blocks are allocated and deallocated.
                
                This function is for use in debugging.
                Always returns 0 in the final ROM version library (FINALROM).

  Arguments:    type:  Type of value to get

  Returns:      Returns the value set in the memory of the type specified.
 *---------------------------------------------------------------------------*/

#if ! defined(NNS_FINALROM)

    u32
    NNS_FndGetFillValForHeap(int type)
    {
        NNS_ASSERT(type < NNS_FND_HEAP_FILL_MAX);

        return sFillVals[type];
    }

// #if ! defined(NNS_FINALROM)
#endif
