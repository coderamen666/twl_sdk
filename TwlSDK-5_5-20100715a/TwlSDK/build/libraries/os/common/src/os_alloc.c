/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS
  File:     os_alloc.c

  Copyright 2003-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-19#$
  $Rev: 10786 $
  $Author: okajima_manabu $

 *---------------------------------------------------------------------------*/
#include <nitro/os.h>
#include <nitro/os/common/alloc.h>

/*---------------------------------------------------------------------------*
    Error messages of OSAlloc.c
 *---------------------------------------------------------------------------*/
#define OS_ERR_ALLOCFROMHEAP_NOHEAP         "OS_AllocFromHeap(): heap is not initialized."
#define OS_ERR_ALLOCFROMHEAP_INVSIZE        "OS_AllocFromHeap(): invalid size."
#define OS_ERR_ALLOCFROMHEAP_INVHEAP        "OS_AllocFromHeap(): invalid heap handle."
#define OS_ERR_ALLOCFROMHEAP_BROKENHEAP     "OS_AllocFromHeap(): heap is broken."
#define OS_ERR_ALLOCFIXED_NOHEAP            "OS_AllocFixed(): heap is not initialized."
#define OS_ERR_ALLOCFIXED_INVRANGE          "OS_AllocFixed(): invalid range."
#define OS_ERR_FREETOHEAP_NOHEAP            "OS_FreeToHeap(): heap is not initialized."
#define OS_ERR_FREETOHEAP_INVPTR            "OS_FreeToHeap(): invalid pointer."
#define OS_ERR_FREETOHEAP_INVHEAP           "OS_FreeToHeap(): invalid heap handle."
#define OS_ERR_SETCURRENTHEAP_NOHEAP        "OS_SetCurrentHeap(): heap is not initialized."
#define OS_ERR_SETCURRENTHEAP_INVHEAP       "OS_SetCurrentHeap(): invalid heap handle."
#define OS_ERR_INITALLOC_INVNUMHEAPS        "OS_InitAlloc(): invalid number of heaps."
#define OS_ERR_INITALLOC_INVRANGE           "OS_InitAlloc(): invalid range."
#define OS_ERR_INITALLOC_INSRANGE           "OS_InitAlloc(): too small range."
#define OS_ERR_CLEARALLOC_INVID             "OS_ClearAlloc(): invalid arena id."
#define OS_ERR_CREATEHEAP_NOHEAP            "OS_CreateHeap(): heap is not initialized."
#define OS_ERR_CREATEHEAP_INVRANGE          "OS_CreateHeap(): invalid range."
#define OS_ERR_CREATEHEAP_INSRANGE          "OS_CreateHeap(): too small range."
#define OS_ERR_DESTROYHEAP_NOHEAP           "OS_DestroyHeap(): heap is not initialized."
#define OS_ERR_DESTROYHEAP_INVHEAP          "OS_DestroyHeap(): invalid heap handle."
#define OS_ERR_ADDTOHEAP_NOHEAP             "OS_AddToHeap(): heap is not initialized."
#define OS_ERR_ADDTOHEAP_INVHEAP            "OS_AddToHeap(): invalid heap handle."
#define OS_ERR_ADDTOHEAP_INVRANGE           "OS_AddToHeap(): invalid range."
#define OS_ERR_ADDTOHEAP_INSRANGE           "OS_AddToHeap(): too small range."
#define OS_ERR_REFERENT_NOHEAP              "OS_ReferentSize(): heap is not initialized."
#define OS_ERR_REFERENT_INVPTR              "OS_ReferentSize(): invalid pointer."
#define OS_ERR_DUMPHEAP_NOHEAP              "OS_DumpHeap(): heap is not initialized."
#define OS_ERR_DUMPHEAP_INVHEAP             "OS_DumpHeap(): invalid heap handle."
#define OS_ERR_DUMPHEAP_BROKENHEAP          "OS_DumpHeap(): heap is broken."
#define OS_ERR_ALLOCFROMHEAP_INVID          "OS_AllocFromHeap(): illegal arena id."
#define OS_ERR_ALLOCFROMHEAP_NOINFO         "OS_AllocFromHeap(): heap not initialized."
#define OS_ERR_ALLOCFIXED_INVID             "OS_AllocFixed(): illegal arena id."
#define OS_ERR_ALLOCFIXED_NOINFO            "OS_AllocFixed(): heap not initialized."
#define OS_ERR_FREETOHEAP_INVID             "OS_FreeToHeap(): illegal arena id."
#define OS_ERR_FREETOHEAP_NOINFO            "OS_FreeToHeap(): heap not initialized."
#define OS_ERR_SETCURRENTHEAP_INVID         "OS_SetCurrentHeap(): illegal arena id."
#define OS_ERR_SETCURRENTHEAP_NOINFO        "OS_SetCurrentHeap(): heap not initialized."
#define OS_ERR_INITALLOC_INVID              "OS_SetInitAlloc(): illegal arena id."
#define OS_ERR_INITALLOC_INVINFO            "OS_SetInitAlloc(): heap already initialized."
#define OS_ERR_CREATEHEAP_INVID             "OS_CreateHeap(): illegal arena id."
#define OS_ERR_CREATEHEAP_NOINFO            "OS_CreateHeap(): heap not initialized."
#define OS_ERR_DESTROYHEAP_INVID            "OS_DestroyHeap(): illegal arena id."
#define OS_ERR_DESTROYHEAP_NOINFO           "OS_DestroyHeap(): heap not initialized."
#define OS_ERR_ADDTOHEAP_INVID              "OS_AddToHeap(): illegal arena id."
#define OS_ERR_ADDTOHEAP_NOINFO             "OS_AddToHeap(): heap not initialized."
#define OS_ERR_CHECKHEAP_INVID              "OS_CheckHeap(): illegal arena id."
#define OS_ERR_CHECKHEAP_NOINFO             "OS_CheckHeap(): heap not initialized."
#define OS_ERR_REFERENTSIZE_INVID           "OS_ReferentSize(): illegal arena id."
#define OS_ERR_REFERENTSIZE_NOINFO          "OS_ReferrentSize(): heap not initialized."
#define OS_ERR_DUMPHEAP_INVID               "OS_DumpHeap(): illegal arena id."
#define OS_ERR_DUMPHEAP_NOINFO              "OS_DumpHeap(): heap not initialized."
#define OS_ERR_VISITALLOCATED_INVID         "OS_VisitAllocated(): illegal arena id."
#define OS_ERR_VISITALLOCATED_NOINFO        "OS_VisitAllocated(): heap not initialized."


#define OFFSET(n, a)    (((u32) (n)) & ((a) - 1))
#define TRUNC(n, a)     (((u32) (n)) & ~((a) - 1))
#define ROUND(n, a)     (((u32) (n) + (a) - 1) & ~((a) - 1))

#define ALIGNMENT       32             // alignment in bytes
#define MINOBJSIZE      (HEADERSIZE + ALIGNMENT)        // smallest object
#define HEADERSIZE      ROUND(sizeof(Cell), ALIGNMENT)

//---- InRange():       True if a <= targ < b
#define InRange(targ, a, b)                                             \
    ((u32)(a) <= (u32)(targ) && (u32)(targ) < (u32)(b))

//---- RangeOverlap():  True if the ranges a and b overlap in any way.
#define RangeOverlap(aStart, aEnd, bStart, bEnd)                        \
    ((u32)(bStart) <= (u32)(aStart) && (u32)(aStart) < (u32)(bEnd) ||   \
     (u32)(bStart) < (u32)(aEnd) && (u32)(aEnd) <= (u32)(bEnd))

//---- RangeSubset():   True if range a is a subset of range b
//                  Assume (aStart < aEnd) and (bStart < bEnd)
#define RangeSubset(aStart, aEnd, bStart, bEnd)                         \
    ((u32)(bStart) <= (u32)(aStart) && (u32)(aEnd) <= (u32)(bEnd))

typedef struct Cell Cell;
typedef struct HeapDesc HeapDesc;

// Cell: header of object which resides HEADERSIZE bytes before payload.
//       doubly linked list are needed because of non-contiguous heaps
struct Cell
{
    Cell   *prev;
    Cell   *next;
    long    size;                      // size of object plus HEADERSIZE

#ifdef  SDK_DEBUG
    HeapDesc *hd;                      // from which the block is allocated
    // (NULL in free list).
    long    requested;                 // size of object to have been requested
#endif                                 // SDK_DEBUG
};

struct HeapDesc
{
    long    size;                      // if -1 then heap is free. Note OS_AllocFixed()
    // could make a heap empty.
    Cell   *free;                      // pointer to the first free cell
    Cell   *allocated;                 // pointer to the first used cell

#ifdef  SDK_DEBUG
    u32     paddingBytes;
    u32     headerBytes;
    u32     payloadBytes;
#endif                                 // SDK_DEBUG
};

#if defined(SDK_TWL) && !defined(SDK_TWLLTD)
static OSArenaId OSi_ExtraHeapArenaId = (OSArenaId)-1;
static OSHeapHandle OSi_ExtraHeapHandle = -1;
#endif

//----------------
//---- struct of Heap infomation
typedef struct
{
    // volatile because some functions use this as hidden macro parameter
    volatile OSHeapHandle currentHeap;
    int     numHeaps;
    void   *arenaStart;
    void   *arenaEnd;
    HeapDesc *heapArray;
}
OSHeapInfo;

void   *OSiHeapInfo[OS_ARENA_MAX] = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};                                     // because OS_ARENA_MAX = 9

//================================================================================
/*---------------------------------------------------------------------------*
  Name:         DLAddFront

  Description:  Inserts /cell/ into the head of the list pointed to by /list/

  Arguments:    list : pointer to the first cell in the list
                cell : pointer to a cell to be inserted

  Returns:      a pointer to the new first cell
 *---------------------------------------------------------------------------*/
static Cell *DLAddFront(Cell * list, Cell * cell)
{
    cell->next = list;
    cell->prev = NULL;
    if (list)
    {
        list->prev = cell;
    }
    return cell;
}

/*---------------------------------------------------------------------------*
  Name:         DLLookup

  Description:  Returns /cell/ if /cell/ is found in /list/.

  Arguments:    list : pointer to the first cell in the list
                cell : pointer to a cell to look for

  Returns:      /cell/ if /cell/ is in /list/. Otherwise, NULL.
 *---------------------------------------------------------------------------*/
static Cell *DLLookup(Cell * list, Cell * cell)
{
    for (; list; list = list->next)
    {
        if (list == cell)
        {
            return list;
        }
    }
    return NULL;
}

/*---------------------------------------------------------------------------*
  Name:         DLExtract

  Description:  Extracts /target/ from the list pointed to by /list/.
                If /target/ is at the head of the list, /list/ will be changed
                to reflect its removal.

  Arguments:    list : pointer to the first cell in the list
                cell : pointer to a cell to remove

  Returns:      a pointer to the new first cell
 *---------------------------------------------------------------------------*/
static Cell *DLExtract(Cell * list, Cell * cell)
{
    if (cell->next)
    {
        cell->next->prev = cell->prev;
    }

    if (cell->prev == NULL)
    {
        return cell->next;
    }
    else
    {
        cell->prev->next = cell->next;
        return list;
    }
}

/*---------------------------------------------------------------------------*
  Name:         DLInsert

  Description:  Inserts /cell/ into the list pointed to by /list/ in sorted
                order by address.  Also attempts to coalesce forward and
                backward blocks if possible.

  Arguments:    list : pointer to the first cell in the list
                cell : pointer to a cell to be inserted

  Returns:      a pointer to the new first cell
 *---------------------------------------------------------------------------*/
static Cell *DLInsert(Cell * list, Cell * cell)
{
    Cell   *prev;
    Cell   *next;

    for (next = list, prev = NULL; next; prev = next, next = next->next)
    {
        if (cell <= next)
        {
            break;
        }
    }

    cell->next = next;
    cell->prev = prev;
    if (next)
    {
        next->prev = cell;
        if ((char *)cell + cell->size == (char *)next)
        {
            //---- Coalesce forward
            cell->size += next->size;
            cell->next = next = next->next;
            if (next)
            {
                next->prev = cell;
            }
        }
    }
    if (prev)
    {
        prev->next = cell;
        if ((char *)prev + prev->size == (char *)cell)
        {
            //---- Coalesce back
            prev->size += cell->size;
            prev->next = next;
            if (next)
            {
                next->prev = prev;
            }
        }
        return list;
    }
    else
    {
        return cell;                   // cell becomes new head of list
    }
}

/*---------------------------------------------------------------------------*
  Name:         DLOverlap

  Description:  returns true if the range delimited by /start/ and /end/
                overlaps with any element of /list/.

  Arguments:    list  : pointer to the first cell in the list
                start : start of range
                end   : end of range

  Returns:      TRUE if /start/-/end/ overlaps with any element of /list/
 *---------------------------------------------------------------------------*/
static BOOL DLOverlap(Cell * list, void *start, void *end)
{
    Cell   *cell;

    for (cell = list; cell; cell = cell->next)
    {
        if (RangeOverlap(cell, (char *)cell + cell->size, start, end))
        {
            return TRUE;
        }
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         DLSize

  Description:  returns total number of bytes used by every element of /list/.

  Arguments:    list : pointer to the first cell in the list

  Returns:      total number of bytes used by every cell of /list/
 *---------------------------------------------------------------------------*/
static long DLSize(Cell * list)
{
    Cell   *cell;
    long    size = 0;

    for (cell = list; cell; cell = cell->next)
    {
        size += cell->size;
    }
    return size;
}

//================================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_AllocFromHeap

  Description:  Allocates /size/ bytes from /heap/. Some additional memory
                will also be consumed from /heap/.

  Arguments:    id   : arena ID
                heap : handle to a heap that was returned from OS_CreateHeap()
                size : size of object to be allocated

  Returns:      a null pointer or a pointer to the allocated space aligned
                with ALIGNMENT bytes boundaries
 *---------------------------------------------------------------------------*/
void   *OS_AllocFromHeap(OSArenaId id, OSHeapHandle heap, u32 size)
{
    OSHeapInfo *heapInfo;
    HeapDesc *hd;
    Cell   *cell;                      // candidate block
    Cell   *newCell;                   // ptr to leftover block
    long    leftoverSize;              // size of any leftover
    OSIntrMode enabled = OS_DisableInterrupts();

    //OS_TPrintf( "id=%d heap=%x size=%x\n",id, heap, size );

#ifdef  SDK_DEBUG
    long    requested = (long)size;
#endif // SDK_DEBUG

    //---- check arena id
    SDK_TASSERTMSG(id < OS_ARENA_MAX, OS_ERR_ALLOCFROMHEAP_INVID);
    SDK_TASSERTMSG(OSiHeapInfo[id], OS_ERR_ALLOCFROMHEAP_NOINFO);
    heapInfo = OSiHeapInfo[id];

    //---- check exist heap and size>0
    if (!heapInfo)
    {
        (void)OS_RestoreInterrupts(enabled);
        return NULL;
    }

    //---- heap<0  means current heap
    if (heap < 0)
    {
        heap = heapInfo->currentHeap;
    }

    SDK_TASSERTMSG(heapInfo->heapArray, OS_ERR_ALLOCFROMHEAP_NOHEAP);
    SDK_TASSERTMSG(0 < ((long)size), OS_ERR_ALLOCFROMHEAP_INVSIZE);
    SDK_TASSERTMSG(0 <= heap && heap < heapInfo->numHeaps, OS_ERR_ALLOCFROMHEAP_INVHEAP);
    SDK_TASSERTMSG(0 <= heapInfo->heapArray[heap].size, OS_ERR_ALLOCFROMHEAP_INVHEAP);

    hd = &heapInfo->heapArray[heap];

    // Enlarge size to smallest possible cell size
    size += HEADERSIZE;
    size = ROUND(size, ALIGNMENT);

    // Search for block large enough
    for (cell = hd->free; cell != NULL; cell = cell->next)
    {
        if ((long)size <= cell->size)
        {
            break;
        }
    }

    if (cell == NULL)
    {
#ifdef  SDK_DEBUG
        OS_TPrintf("OS_AllocFromHeap: Warning- failed to allocate %d bytes\n", size);
#endif // SDK_DEBUG
        (void)OS_RestoreInterrupts(enabled);
        return NULL;
    }

    SDK_TASSERTMSG(OFFSET(cell, ALIGNMENT) == 0, OS_ERR_ALLOCFROMHEAP_BROKENHEAP);
    SDK_TASSERTMSG(cell->hd == NULL, OS_ERR_ALLOCFROMHEAP_BROKENHEAP);

    leftoverSize = cell->size - (long)size;
    if (leftoverSize < MINOBJSIZE)
    {
        //---- Just extract this cell out since it's too small to split
        hd->free = DLExtract(hd->free, cell);
    }
    else
    {
        //---- cell is large enough to split into two pieces
        cell->size = (long)size;

        //---- Create a new cell
        newCell = (Cell *) ((char *)cell + size);
        newCell->size = leftoverSize;
#ifdef SDK_DEBUG
        newCell->hd = NULL;
#endif

        //---- Leave newCell in free, and take cell away
        newCell->prev = cell->prev;
        newCell->next = cell->next;

        if (newCell->next != NULL)
        {
            newCell->next->prev = newCell;
        }

        if (newCell->prev != NULL)
        {
            newCell->prev->next = newCell;
        }
        else
        {
            SDK_TASSERTMSG(hd->free == cell, OS_ERR_ALLOCFROMHEAP_BROKENHEAP);
            hd->free = newCell;
        }
    }

    //---- Add to allocated list
    hd->allocated = DLAddFront(hd->allocated, cell);

#ifdef  SDK_DEBUG
    cell->hd = hd;
    cell->requested = requested;
    hd->headerBytes += HEADERSIZE;
    hd->paddingBytes += cell->size - (HEADERSIZE + requested);
    hd->payloadBytes += requested;
#endif // SDK_DEBUG

    (void)OS_RestoreInterrupts(enabled);

    return (void *)((char *)cell + HEADERSIZE);
}

/*---------------------------------------------------------------------------*
  Name:         OS_AllocFixed

  Description:  Allocates the block of memory specified by /rstart/ and
                /rend/.  Will break up any heap.  Will not check for overlap
                with other fixed blocks.  May create a zero-length heap.

  Arguments:    id     : arena ID
                rstart : pointer to starting addr of block
                rend   : pointer to ending addr of block

  Returns:      a null pointer or a pointer to the allocated space aligned
                with ALIGNMENT bytes boundaries. /rstart/ and /rend/ might be
                adjusted to the boundaries of really allocated region.
 *---------------------------------------------------------------------------*/
void   *OS_AllocFixed(OSArenaId id, void **rstart, void **rend)
{
    OSHeapInfo *heapInfo;
    OSHeapHandle i;                    // heap iterator
    Cell   *cell;                      // object iterator
    Cell   *newCell;                   // for creating new objects if necessary
    HeapDesc *hd;
    void   *start = (void *)TRUNC(*rstart, ALIGNMENT);
    void   *end = (void *)ROUND(*rend, ALIGNMENT);
    OSIntrMode enabled = OS_DisableInterrupts();

    //---- check arena id
    SDK_TASSERTMSG(id < OS_ARENA_MAX, OS_ERR_ALLOCFIXED_INVID);
    SDK_TASSERTMSG(OSiHeapInfo[id], OS_ERR_ALLOCFIXED_NOINFO);
    heapInfo = OSiHeapInfo[id];

    SDK_TASSERTMSG(heapInfo->heapArray, OS_ERR_ALLOCFIXED_NOHEAP);
    SDK_TASSERTMSG(start < end, OS_ERR_ALLOCFIXED_INVRANGE);
    SDK_TASSERTMSG(RangeSubset(start, end, heapInfo->arenaStart, heapInfo->arenaEnd), OS_ERR_ALLOCFIXED_INVRANGE);

    //---- Check overlap with any allocated blocks.
    for (i = 0; i < heapInfo->numHeaps; i++)
    {
        hd = &heapInfo->heapArray[i];
        if (hd->size < 0)              // Is inactive?
        {
            continue;
        }

        if (DLOverlap(hd->allocated, start, end))
        {
#ifdef  SDK_DEBUG
            OS_TPrintf("OS_AllocFixed: Warning - failed to allocate from %p to %p\n", start, end);
#endif // SDK_DEBUG
            (void)OS_RestoreInterrupts(enabled);
            return NULL;
        }
    }

    /*
       ASSUME : if we get past all this, the fixed request will
       not overlap with any non-contiguous free memory.

       Iterate over heaps breaking up appropriate blocks.
       note that we cannot change the size of the heap by simply subtracting
       out the overlap area, since the heaps may be non-contiguous.
     */
    for (i = 0; i < heapInfo->numHeaps; i++)
    {
        //---- for each free obj in heap, find and break overlaps.
        hd = &heapInfo->heapArray[i];

        if (hd->size < 0)              // Is inactive?
        {
            continue;
        }

        for (cell = hd->free; cell; cell = cell->next)
        {
            void   *cellEnd = (char *)cell + cell->size;

            if ((char *)cellEnd <= (char *)start)
            {
                continue;
            }

            if ((char *)end <= (char *)cell)
            {
                break;                 // Since free is sorted in order of start addresses
            }

            if (InRange(cell, (char *)start - HEADERSIZE, end) && InRange((char *)cellEnd, start, (char *)end + MINOBJSIZE))
            {
                if ((char *)cell < (char *)start)
                {
                    start = (void *)cell;
                }
                if ((char *)end < (char *)cellEnd)
                {
                    end = (void *)cellEnd;
                }

                //---- cell is completely overlaped. Just extract this block
                hd->free = DLExtract(hd->free, cell);   // Note cell->next is intact. XXX
                hd->size -= cell->size; // Update stats
                continue;
            }

            if (InRange(cell, (char *)start - HEADERSIZE, end))
            {
                if ((char *)cell < (char *)start)
                {
                    start = (void *)cell;
                }

                // Start of object in middle of range. break off top.
                // Avoid DLExtract() and DLInsert() since we already know
                // exactly where the block should go
                SDK_ASSERT(MINOBJSIZE <= (char *)cellEnd - (char *)end);
                newCell = (Cell *) end;
                newCell->size = (char *)cellEnd - (char *)end;
#ifdef  SDK_DEBUG
                newCell->hd = NULL;
#endif // SDK_DEBUG
                newCell->next = cell->next;
                if (newCell->next)
                {
                    newCell->next->prev = newCell;
                }
                newCell->prev = cell->prev;
                if (newCell->prev)
                {
                    newCell->prev->next = newCell;
                }
                else
                {
                    hd->free = newCell; // new head
                }
                hd->size -= (char *)end - (char *)cell;
                break;
            }

            if (InRange((char *)cellEnd, start, (char *)end + MINOBJSIZE))
            {
                if ((char *)end < (char *)cellEnd)
                {
                    end = (void *)cellEnd;
                }

                //---- Nothing to change except size
                SDK_ASSERT(MINOBJSIZE <= (char *)start - (char *)cell);
                hd->size -= (char *)cellEnd - (char *)start;
                cell->size = (char *)start - (char *)cell;
                continue;
            }

            //---- Create a new cell after end of the fixed block.
            SDK_ASSERT(MINOBJSIZE <= (char *)cellEnd - (char *)end);
            newCell = (Cell *) end;
            newCell->size = (char *)cellEnd - (char *)end;
#ifdef  SDK_DEBUG
            newCell->hd = NULL;
#endif // SDK_DEBUG
            newCell->next = cell->next;
            if (newCell->next)
            {
                newCell->next->prev = newCell;
            }
            newCell->prev = cell;
            cell->next = newCell;      // cell is before newCell
            cell->size = (char *)start - (char *)cell;
            hd->size -= (char *)end - (char *)start;
            break;
        }
        SDK_ASSERT(0 <= hd->size);
    }

    SDK_ASSERT(OFFSET(start, ALIGNMENT) == 0);
    SDK_ASSERT(OFFSET(end, ALIGNMENT) == 0);
    SDK_ASSERT(start < end);
    *rstart = start;
    *rend = end;

    (void)OS_RestoreInterrupts(enabled);
    return *rstart;
}

/*---------------------------------------------------------------------------*
  Name:         OS_FreeToHeap

  Description:  Returns obj /ptr/ to /heap/.

  Arguments:    id       : arena ID
                heap     : handle to the heap that /ptr/ was allocated from
                ptr      : pointer to object previously returned from
                           OS_Alloc() or OS_AllocFromHeap().

  Returns:      None.
 *---------------------------------------------------------------------------*/
void OS_FreeToHeap(OSArenaId id, OSHeapHandle heap, void *ptr)
{
    OSHeapInfo *heapInfo;
    HeapDesc *hd;
    Cell   *cell;

    OSIntrMode enabled = OS_DisableInterrupts();

    //---- check arena id
    SDK_TASSERTMSG(id < OS_ARENA_MAX, OS_ERR_FREETOHEAP_INVID);
    SDK_TASSERTMSG(OSiHeapInfo[id], OS_ERR_FREETOHEAP_NOINFO);
    heapInfo = OSiHeapInfo[id];

    if (heap < 0)
    {
        heap = heapInfo->currentHeap;
    }

    SDK_TASSERTMSG(heapInfo->heapArray, OS_ERR_FREETOHEAP_NOHEAP);
#ifdef SDK_DEBUG
#if defined(SDK_TWL) && !defined(SDK_TWLLTD)
    if ( OSi_ExtraHeapArenaId != id || OSi_ExtraHeapHandle != heap )
    {
#endif
        SDK_TASSERTMSG(InRange(ptr, (char *)heapInfo->arenaStart + HEADERSIZE, (char *)heapInfo->arenaEnd), OS_ERR_FREETOHEAP_INVPTR);
#if defined(SDK_TWL) && !defined(SDK_TWLLTD)
    }
#endif
#endif
    SDK_TASSERTMSG(OFFSET(ptr, ALIGNMENT) == 0, OS_ERR_FREETOHEAP_INVPTR);
    SDK_TASSERTMSG(0 <= heapInfo->heapArray[heap].size, OS_ERR_FREETOHEAP_INVHEAP);

    cell = (Cell *) ((char *)ptr - HEADERSIZE);
    hd = &heapInfo->heapArray[heap];

    SDK_TASSERTMSG(cell->hd == hd, OS_ERR_FREETOHEAP_INVPTR);
    SDK_TASSERTMSG(DLLookup(hd->allocated, cell), OS_ERR_FREETOHEAP_INVPTR);

#ifdef  SDK_DEBUG
    cell->hd = NULL;
    hd->headerBytes -= HEADERSIZE;
    hd->paddingBytes -= cell->size - (HEADERSIZE + cell->requested);
    hd->payloadBytes -= cell->requested;
#endif // SDK_DEBUG

    //---- Extract from the allocated list
    hd->allocated = DLExtract(hd->allocated, cell);

    //---- Add in sorted order to free list (coalesced with next and prev)
    hd->free = DLInsert(hd->free, cell);

    (void)OS_RestoreInterrupts(enabled);
}

/*---------------------------------------------------------------------------*
  Name:         OS_FreeAllToHeap

  Description:  free all allocated block in the specified heap

  Arguments:    id       : arena ID
                heap     : handle to the heap

  Returns:      None.
 *---------------------------------------------------------------------------*/
void OS_FreeAllToHeap(OSArenaId id, OSHeapHandle heap)
{
    OSHeapInfo *heapInfo;
    HeapDesc *hd;
    Cell   *cell;

    OSIntrMode enabled = OS_DisableInterrupts();

    //---- check arena id
    SDK_TASSERTMSG(id < OS_ARENA_MAX, OS_ERR_FREETOHEAP_INVID);
    SDK_TASSERTMSG(OSiHeapInfo[id], OS_ERR_FREETOHEAP_NOINFO);
    heapInfo = OSiHeapInfo[id];

    if (heap < 0)
    {
        heap = heapInfo->currentHeap;
    }

    SDK_TASSERTMSG(heapInfo->heapArray, "heap not initialized");
    SDK_TASSERTMSG(0 <= heapInfo->heapArray[heap].size, "invalid heap handle");

    hd = &heapInfo->heapArray[heap];
    while ((cell = hd->allocated) != NULL)
    {
        SDK_ASSERT(cell->hd == hd);
        SDK_ASSERT(DLLookup(hd->allocated, cell));

#ifdef  SDK_DEBUG
        cell->hd = NULL;
        hd->headerBytes -= HEADERSIZE;
        hd->paddingBytes -= cell->size - (HEADERSIZE + cell->requested);
        hd->payloadBytes -= cell->requested;
#endif // SDK_DEBUG

        //---- Extract from the allocated list
        hd->allocated = DLExtract(hd->allocated, cell);

        //---- Add in sorted order to free list (coalesced with next and prev)
        hd->free = DLInsert(hd->free, cell);
    }

    (void)OS_RestoreInterrupts(enabled);
}

/*---------------------------------------------------------------------------*
  Name:         OS_SetCurrentHeap

  Description:  Sets OSi_CurrentHeap to /heap/.  All subsequent calls to
                OS_Alloc() will be performed on this heap until another
                call to OS_SetCurrentHeap().

  Arguments:    id   : arena ID
                heap : handle to a heap that was returned from OS_CreateHeap()

  Returns:      previous heap handle.
 *---------------------------------------------------------------------------*/
OSHeapHandle OS_SetCurrentHeap(OSArenaId id, OSHeapHandle heap)
{
    OSHeapInfo *heapInfo;
    OSHeapHandle prev;
    OSIntrMode enabled = OS_DisableInterrupts();

    //---- check arena id
    SDK_TASSERTMSG(id < OS_ARENA_MAX, OS_ERR_SETCURRENTHEAP_INVID);
    SDK_TASSERTMSG(OSiHeapInfo[id], OS_ERR_SETCURRENTHEAP_NOINFO);
    heapInfo = OSiHeapInfo[id];

    SDK_TASSERTMSG(heapInfo->heapArray, OS_ERR_SETCURRENTHEAP_NOHEAP);
    SDK_TASSERTMSG(0 <= heap && heap < heapInfo->numHeaps, OS_ERR_SETCURRENTHEAP_INVHEAP);
    SDK_TASSERTMSG(0 <= heapInfo->heapArray[heap].size, OS_ERR_SETCURRENTHEAP_INVHEAP);
    prev = heapInfo->currentHeap;
    heapInfo->currentHeap = heap;

    (void)OS_RestoreInterrupts(enabled);
    return prev;
}

/*---------------------------------------------------------------------------*
  Name:         OS_InitAlloc

  Description:  Initializes the arena in which all heaps will reside.
                Reserves some small amount of memory for array of heap
                descriptors.

  Arguments:    id         : arena ID
                arenaStart : beginning addr of arena
                arenaEnd   : ending addr of arena
                maxHeaps   : Maximum number of active heaps that will be
                           : used in lifetime of program

  Returns:      start of real arena, aligned with 32 bytes boundaries, after
                heap array has been allocated
 *---------------------------------------------------------------------------*/
void   *OS_InitAlloc(OSArenaId id, void *arenaStart, void *arenaEnd, int maxHeaps)
{
    OSHeapInfo *heapInfo;
    u32     arraySize;
    OSHeapHandle i;
    OSIntrMode enabled = OS_DisableInterrupts();

    //---- check arena id
    SDK_TASSERTMSG(id < OS_ARENA_MAX, OS_ERR_INITALLOC_INVID);
    SDK_TASSERTMSG(OSiHeapInfo[id] == NULL, OS_ERR_INITALLOC_INVINFO);

    SDK_TASSERTMSG(0 < maxHeaps, OS_ERR_INITALLOC_INVNUMHEAPS);
    SDK_TASSERTMSG((char *)arenaStart < (char *)arenaEnd, OS_ERR_INITALLOC_INVRANGE);
    SDK_TASSERTMSG(maxHeaps <= ((char *)arenaEnd - (char *)arenaStart) / sizeof(HeapDesc), OS_ERR_INITALLOC_INSRANGE);

    //---- save heapInfo
    heapInfo = arenaStart;
    OSiHeapInfo[id] = heapInfo;

    //---- Place HeapArray at head of the arena
    arraySize = sizeof(HeapDesc) * maxHeaps;
    heapInfo->heapArray = (void *)((u32)arenaStart + sizeof(OSHeapInfo));
    heapInfo->numHeaps = maxHeaps;

    for (i = 0; i < heapInfo->numHeaps; i++)
    {
        HeapDesc *hd = &heapInfo->heapArray[i];

        hd->size = -1;
        hd->free = hd->allocated = NULL;
#ifdef  SDK_DEBUG
        hd->paddingBytes = hd->headerBytes = hd->payloadBytes = 0;
#endif // SDK_DEBUG
    }

    //---- Set OSi_CurrentHeap to an invalid value
    heapInfo->currentHeap = -1;

    //---- Reset arenaStart to the nearest reasonable location
    arenaStart = (void *)((char *)heapInfo->heapArray + arraySize);
    arenaStart = (void *)ROUND(arenaStart, ALIGNMENT);

    heapInfo->arenaStart = arenaStart;
    heapInfo->arenaEnd = (void *)TRUNC(arenaEnd, ALIGNMENT);
    SDK_TASSERTMSG(MINOBJSIZE <= (char *)heapInfo->arenaEnd - (char *)heapInfo->arenaStart, OS_ERR_INITALLOC_INSRANGE);

    (void)OS_RestoreInterrupts(enabled);
    return heapInfo->arenaStart;
}

/*---------------------------------------------------------------------------*
  Name:         OS_ClearAlloc

  Description:  Clear heap pointer in system shared.
                After calling this, you can call OS_InitAlloc() again.

  Arguments:    id  :   arena ID

  Returns:      None
 *---------------------------------------------------------------------------*/
void OS_ClearAlloc(OSArenaId id)
{
    SDK_TASSERTMSG(id < OS_ARENA_MAX, OS_ERR_CLEARALLOC_INVID);
    OSiHeapInfo[id] = NULL;
}

/*---------------------------------------------------------------------------*
  Name:         OS_CreateHeap

  Description:  Reserves area of memory from /start/ to /end/ for use as a
                heap.  Initializes heap descriptor and free list.
                Will consume one entry in heap array.

  Arguments:    id    : arena ID
                start : starting addr of heap
                end   : ending addr of heap

  Returns:      If the function succeeds, it returns a new handle to heap
                for use in OS_AllocFromHeap(), OS_FreeToHeap(), etc.
                If the function fails, the return value is -1.
 *---------------------------------------------------------------------------*/
OSHeapHandle OS_CreateHeap(OSArenaId id, void *start, void *end)
{
    OSHeapInfo *heapInfo;
    OSHeapHandle heap;
    HeapDesc *hd;
    Cell   *cell;

    OSIntrMode enabled = OS_DisableInterrupts();

    //OS_TPrintf( "OS_CreateHeap  id=%d start=%x, end=%x\n", id, start, end );
    //---- check arena id
    SDK_TASSERTMSG(id < OS_ARENA_MAX, OS_ERR_CREATEHEAP_INVID);
    SDK_TASSERTMSG(OSiHeapInfo[id], OS_ERR_CREATEHEAP_NOINFO);
    heapInfo = OSiHeapInfo[id];

    SDK_TASSERTMSG(heapInfo->heapArray, OS_ERR_CREATEHEAP_NOHEAP);
    SDK_TASSERTMSG(start < end, OS_ERR_CREATEHEAP_INVRANGE);
    start = (void *)ROUND(start, ALIGNMENT);
    end = (void *)TRUNC(end, ALIGNMENT);
    SDK_TASSERTMSG(start < end, OS_ERR_CREATEHEAP_INVRANGE);
    SDK_TASSERTMSG(RangeSubset(start, end, heapInfo->arenaStart, heapInfo->arenaEnd), OS_ERR_CREATEHEAP_INVRANGE);
    SDK_TASSERTMSG(MINOBJSIZE <= (char *)end - (char *)start, OS_ERR_CREATEHEAP_INSRANGE);

#ifdef  SDK_DEBUG
    //---- Check that the range does not overlap with
    //     any other block in this or other heaps.
    for (heap = 0; heap < heapInfo->numHeaps; heap++)
    {
        if (heapInfo->heapArray[heap].size < 0)
        {
            continue;
        }
        SDK_TASSERTMSG(!DLOverlap(heapInfo->heapArray[heap].free, start, end), OS_ERR_CREATEHEAP_INVRANGE);
        SDK_TASSERTMSG(!DLOverlap(heapInfo->heapArray[heap].allocated, start, end), OS_ERR_CREATEHEAP_INVRANGE);
    }
#endif // SDK_DEBUG

    //---- Search for free descriptor
    for (heap = 0; heap < heapInfo->numHeaps; heap++)
    {
        hd = &heapInfo->heapArray[heap];
        if (hd->size < 0)
        {
            hd->size = (char *)end - (char *)start;

            cell = (Cell *) start;
            cell->prev = NULL;
            cell->next = NULL;
            cell->size = hd->size;
#ifdef  SDK_DEBUG
            cell->hd = NULL;
#endif // SDK_DEBUG

            hd->free = cell;
            hd->allocated = 0;
#ifdef  SDK_DEBUG
            hd->paddingBytes = hd->headerBytes = hd->payloadBytes = 0;
#endif // SDK_DEBUG

            (void)OS_RestoreInterrupts(enabled);
            return heap;
        }
    }

    //---- Could not find free descriptor
#ifdef  SDK_DEBUG
    OS_TPrintf("OS_CreateHeap: Warning - Failed to find free heap descriptor.");
#endif // SDK_DEBUG

    (void)OS_RestoreInterrupts(enabled);
    return -1;
}

/*---------------------------------------------------------------------------*
  Name:         OS_CreateExtraHeap

  Description:  Create extra heap using the parameter area of TWL ROM.
                So Cannot be available on TWL mode.
                Available only on NITRO of HYBRID.

  Arguments:    id    : arena ID

  Returns:      -1 ... Failed.
                0< ... Success. Value is heap handle.
 *---------------------------------------------------------------------------*/
#if defined(SDK_TWL) && !defined(SDK_TWLLTD)
OSHeapHandle OS_CreateExtraHeap(OSArenaId id)
{
    OSHeapInfo *heapInfo;
    OSHeapHandle heap;
    HeapDesc *hd;
    Cell   *cell;

    OSIntrMode enabled = OS_DisableInterrupts();
    SDK_TASSERTMSG(id < OS_ARENA_MAX, OS_ERR_CREATEHEAP_INVID);

    heapInfo = OSiHeapInfo[id];
    SDK_TASSERTMSG(heapInfo, OS_ERR_CREATEHEAP_NOINFO);
    SDK_TASSERTMSG(heapInfo->heapArray, OS_ERR_CREATEHEAP_NOHEAP);

    //---- available only on NITRO
    if ( ! OS_IsRunOnTwl() && OSi_ExtraHeapHandle < 0 )
    {
        //---- Search for free descriptor
        for (heap = 0; heap < heapInfo->numHeaps; heap++)
        {
            hd = &heapInfo->heapArray[heap];
            if (hd->size < 0)
            {
                hd->size = HW_MAIN_MEM_PARAMETER_BUF_SIZE;

                //---- initial free cell is from whole parameter buffer (NITRO does not use)
                cell = (Cell*)HW_MAIN_MEM_PARAMETER_BUF;
                cell->prev = NULL;
                cell->next = NULL;
                cell->size = HW_MAIN_MEM_PARAMETER_BUF_SIZE;
#ifdef SDK_DEBUG
                cell->hd = NULL;
#endif
                hd->free = cell;
                hd->allocated = 0;
#ifdef SDK_DEBUG
                hd->paddingBytes = hd->headerBytes = hd->payloadBytes = 0;
#endif
                OSi_ExtraHeapArenaId = id;
                OSi_ExtraHeapHandle = heap;

                (void) OS_RestoreInterrupts(enabled);
                return heap;
            }
        }
    }

    //---- Could not find free descriptor
    (void)OS_RestoreInterrupts(enabled);
    return -1;
}
#endif // defined(SDK_TWL) && !defined(SDK_TWLLTD)

/*---------------------------------------------------------------------------*
  Name:         OS_DestroyHeap

  Description:  Frees up the descriptor for the /heap/.  Subsequent
                allocation requests from this heap will fail unless another
                heap is created with the same handle.

  Arguments:    id   : arena ID
                heap : handle to a live heap, previously created with OS_CreateHeap().

  Returns:      None.
 *---------------------------------------------------------------------------*/
void OS_DestroyHeap(OSArenaId id, OSHeapHandle heap)
{
    OSHeapInfo *heapInfo;
    HeapDesc *hd;
#ifdef  SDK_DEBUG
    long    size;
#endif
    OSIntrMode enabled = OS_DisableInterrupts();

    //---- check arena id
    SDK_TASSERTMSG(id < OS_ARENA_MAX, OS_ERR_DESTROYHEAP_INVID);
    SDK_TASSERTMSG(OSiHeapInfo[id], OS_ERR_DESTROYHEAP_NOINFO);
    heapInfo = OSiHeapInfo[id];

    SDK_TASSERTMSG(heapInfo->heapArray, OS_ERR_DESTROYHEAP_NOHEAP);
    SDK_TASSERTMSG(0 <= heap && heap < heapInfo->numHeaps, OS_ERR_DESTROYHEAP_INVHEAP);
    SDK_TASSERTMSG(0 <= heapInfo->heapArray[heap].size, OS_ERR_DESTROYHEAP_INVHEAP);

    hd = &heapInfo->heapArray[heap];

#ifdef SDK_DEBUG
    //---- Check whether entire heap is empty
    size = DLSize(hd->free);
    if (hd->size != size)
    {
        OS_TPrintf("OS_DestroyHeap(%d): Warning - free list size %d, heap size %d\n", heap, size, hd->size);
    }
#endif // SDK_DEBUG

#if defined(SDK_TWL) && !defined(SDK_TWLLTD)
    //---- Check whether heap is the EXTRA HEAP
    if ( OSi_ExtraHeapArenaId == id && OSi_ExtraHeapHandle == heap )
    {
        OSi_ExtraHeapArenaId = (OSArenaId)-1;
        OSi_ExtraHeapHandle = -1;
    }
#endif // defined(SDK_TWL) && !defined(SDK_TWLLTD)

    hd->size = -1;
    hd->free = hd->allocated = NULL;   // add to dolphin src

#ifdef  SDK_DEBUG
    hd->paddingBytes = hd->headerBytes = hd->payloadBytes = 0;
    if (heapInfo->currentHeap == heap)
    {
        heapInfo->currentHeap = -1;
    }
#endif // SDK_DEBUG

    (void)OS_RestoreInterrupts(enabled);
}

/*---------------------------------------------------------------------------*
  Name:         OS_AddToHeap

  Description:  Adds an arbitrary block of memory to /heap/.  Used to free
                blocks previously allocated with OS_AllocFixed(), or to
                create non-contiguous heaps.

  Arguments:    id    : arena ID
                heap  : handle to live heap, previously created with OS_CreateHeap().
                start : starting addr of block to add to /heap/
                end   : ending addr of block to add to /heap/

  Returns:      None.
 *---------------------------------------------------------------------------*/
void OS_AddToHeap(OSArenaId id, OSHeapHandle heap, void *start, void *end)
{
    OSHeapInfo *heapInfo;
    HeapDesc *hd;
    Cell   *cell;
#ifdef  SDK_DEBUG
    OSHeapHandle i;
#endif // SDK_DEBUG
    OSIntrMode enabled = OS_DisableInterrupts();

    //---- check arena id
    SDK_TASSERTMSG(id < OS_ARENA_MAX, OS_ERR_ADDTOHEAP_INVID);
    SDK_TASSERTMSG(OSiHeapInfo[id], OS_ERR_ADDTOHEAP_NOINFO);
    heapInfo = OSiHeapInfo[id];

    SDK_TASSERTMSG(heapInfo->heapArray, OS_ERR_ADDTOHEAP_NOHEAP);
    SDK_TASSERTMSG(0 <= heap && heap < heapInfo->numHeaps, OS_ERR_ADDTOHEAP_INVHEAP);
    SDK_TASSERTMSG(0 <= heapInfo->heapArray[heap].size, OS_ERR_ADDTOHEAP_INVHEAP);

    hd = &heapInfo->heapArray[heap];

    SDK_TASSERTMSG(start < end, OS_ERR_ADDTOHEAP_INVRANGE);

    start = (void *)ROUND(start, ALIGNMENT);
    end = (void *)TRUNC(end, ALIGNMENT);
    SDK_TASSERTMSG(MINOBJSIZE <= (char *)end - (char *)start, OS_ERR_ADDTOHEAP_INSRANGE);

#ifdef SDK_DEBUG
#if defined(SDK_TWL) && !defined(SDK_TWLLTD)
    if ((u32)start != (u32)HW_MAIN_MEM_PARAMETER_BUF && (u32)end != (u32)(HW_MAIN_MEM_PARAMETER_BUF + HW_MAIN_MEM_PARAMETER_BUF_SIZE) )
    {
#endif
        SDK_TASSERTMSG(RangeSubset(start, end, heapInfo->arenaStart, heapInfo->arenaEnd), OS_ERR_ADDTOHEAP_INVRANGE);
#if defined(SDK_TWL) && !defined(SDK_TWLLTD)
    }
#endif
#endif

#ifdef  SDK_DEBUG
    //---- Check that the range does not already overlap with
    //     any other block in this or other heaps.
    for (i = 0; i < heapInfo->numHeaps; i++)
    {
        if (heapInfo->heapArray[i].size < 0)
        {
            continue;
        }
        SDK_TASSERTMSG(!DLOverlap(heapInfo->heapArray[i].free, start, end), OS_ERR_ADDTOHEAP_INVRANGE);
        SDK_TASSERTMSG(!DLOverlap(heapInfo->heapArray[i].allocated, start, end), OS_ERR_ADDTOHEAP_INVRANGE);
    }
#endif // SDK_DEBUG

    //---- Create a new cell
    cell = (Cell *) start;
    cell->size = (char *)end - (char *)start;
#ifdef  SDK_DEBUG
    cell->hd = NULL;
#endif // SDK_DEBUG

    //---- Insert new cell in free
    hd->size += cell->size;
    hd->free = DLInsert(hd->free, cell);

    (void)OS_RestoreInterrupts(enabled);
}

/*---------------------------------------------------------------------------*
  Name:         OS_AddExtraToHeap

  Description:  Adds extra area (top 16KB of main memory ) to /heap/.  Used to
                free blocks previously allocated with OS_AllocFixed(), or to
                create non-contiguous heaps.

  Arguments:    id    : arena ID
                heap  : handle to live heap, previously created with OS_CreateHeap().

  Returns:      None.
 *---------------------------------------------------------------------------*/
#if defined(SDK_TWL) && !defined(SDK_TWLLTD)
void OS_AddExtraAreaToHeap(OSArenaId id, OSHeapHandle heap)
{
    if ( ! OS_IsRunOnTwl() )
    {
        OS_AddToHeap(id, heap,
                     (void*)HW_MAIN_MEM_PARAMETER_BUF,
                     (void*)(HW_MAIN_MEM_PARAMETER_BUF + HW_MAIN_MEM_PARAMETER_BUF_SIZE ) );
        OSi_ExtraHeapArenaId = id;
        OSi_ExtraHeapHandle = heap;
    }
}
#endif

/*---------------------------------------------------------------------------*
  Name:         OS_CheckHeap

  Description:  Checks heap sanity for debugging

  Arguments:    id   : arena ID
                heap : handle to a live heap.

  Returns:      -1 if heap is not consistent. Otherwise, returns number
                of bytes available in free.
 *---------------------------------------------------------------------------*/

#ifndef  SDK_NO_MESSAGE
#ifdef SDK_ARM9
#define OSi_CHECK( exp )                                                 \
    do                                                                       \
{                                                                        \
    if ( !(exp) )                                                        \
    {                                                                    \
        OS_TPrintf( "OS_CheckHeap: Failed " #exp " in %d\n", __LINE__ );  \
        goto exit_OS_CheckHeap; /* goto is not beautiful, but less codes */ \
    }                                                                    \
} while (0)
#else /* SDK_ARM9 */
#define OSi_CHECK( exp )                                                 \
    do                                                                       \
{                                                                        \
    if ( !(exp) )                                                        \
    {                                                                    \
        OS_TPrintf( "OS_CheckHeap: Failed in %d\n", __LINE__ );  \
        goto exit_OS_CheckHeap; /* goto is not beautiful, but less codes */ \
    }                                                                    \
} while (0)
#endif
#else
#define OSi_CHECK( exp )                                                 \
    do                                                                       \
{                                                                        \
    if ( !(exp) )                                                        \
    {                                                                    \
        goto exit_OS_CheckHeap; /* goto is not beautiful, but less codes */ \
    }                                                                    \
} while (0)
#endif

s32 OS_CheckHeap(OSArenaId id, OSHeapHandle heap)
{
    OSHeapInfo *heapInfo;
    HeapDesc *hd;
    Cell   *cell;
    long    total = 0;
    long    free = 0;
    long    retValue = -1;
    OSIntrMode enabled = OS_DisableInterrupts();

    //---- check arena id
    SDK_TASSERTMSG(id < OS_ARENA_MAX, OS_ERR_CHECKHEAP_INVID);
    SDK_TASSERTMSG(OSiHeapInfo[id], OS_ERR_CHECKHEAP_NOINFO);
    heapInfo = OSiHeapInfo[id];

    //---- heap<0  means current heap
    if (heap == OS_CURRENT_HEAP_HANDLE)
    {
        heap = heapInfo->currentHeap;
    }
    SDK_ASSERT(heap >= 0);

    OSi_CHECK(heapInfo->heapArray);
    OSi_CHECK(0 <= heap && heap < heapInfo->numHeaps);

    hd = &heapInfo->heapArray[heap];
    OSi_CHECK(0 <= hd->size);

    OSi_CHECK(hd->allocated == NULL || hd->allocated->prev == NULL);
    for (cell = hd->allocated; cell; cell = cell->next)
    {
#ifdef SDK_DEBUG
#if defined(SDK_TWL) && !defined(SDK_TWLLTD)
        if ( OSi_ExtraHeapArenaId != id || OSi_ExtraHeapHandle != heap )
        {
#endif
            OSi_CHECK(InRange(cell, heapInfo->arenaStart, heapInfo->arenaEnd));
#if defined(SDK_TWL) && !defined(SDK_TWLLTD)
        }
#endif
#endif
        OSi_CHECK(OFFSET(cell, ALIGNMENT) == 0);
        OSi_CHECK(cell->next == NULL || cell->next->prev == cell);
        OSi_CHECK(MINOBJSIZE <= cell->size);
        OSi_CHECK(OFFSET(cell->size, ALIGNMENT) == 0);

        total += cell->size;
        OSi_CHECK(0 < total && total <= hd->size);

#ifdef  SDK_DEBUG
        OSi_CHECK(cell->hd == hd);
        OSi_CHECK(HEADERSIZE + cell->requested <= cell->size);
#endif // SDK_DEBUG
    }

    OSi_CHECK(hd->free == NULL || hd->free->prev == NULL);
    for (cell = hd->free; cell; cell = cell->next)
    {
#ifdef SDK_DEBUG
#if defined(SDK_TWL) && !defined(SDK_TWLLTD)
        if ( OSi_ExtraHeapArenaId != id || OSi_ExtraHeapHandle != heap )
        {
#endif
            OSi_CHECK(InRange(cell, heapInfo->arenaStart, heapInfo->arenaEnd));
#if defined(SDK_TWL) && !defined(SDK_TWLLTD)
        }
#endif
#endif
        OSi_CHECK(OFFSET(cell, ALIGNMENT) == 0);
        OSi_CHECK(cell->next == NULL || cell->next->prev == cell);
        OSi_CHECK(MINOBJSIZE <= cell->size);
        OSi_CHECK(OFFSET(cell->size, ALIGNMENT) == 0);
        OSi_CHECK(cell->next == NULL || (char *)cell + cell->size < (char *)cell->next);

        total += cell->size;
        free += cell->size - HEADERSIZE;
        OSi_CHECK(0 < total && total <= hd->size);

#ifdef  SDK_DEBUG
        OSi_CHECK(cell->hd == NULL);
#endif // SDK_DEBUG
    }

    OSi_CHECK(total == hd->size);
    retValue = free;

  exit_OS_CheckHeap:
    (void)OS_RestoreInterrupts(enabled);
    return retValue;
}

/*---------------------------------------------------------------------------*
  Name:         OS_ReferentSize

  Description:  Returns size of payload

  Arguments:    id  : arena ID
                ptr : pointer to object previously returned from
                      OS_Alloc() or OSAllocFromHeap().

  Returns:      size of payload
 *---------------------------------------------------------------------------*/
u32 OS_ReferentSize(OSArenaId id, void *ptr)
{
    OSHeapInfo *heapInfo;
    Cell   *cell;
    OSIntrMode enabled = OS_DisableInterrupts();

    //---- check arena id
    SDK_TASSERTMSG(id < OS_ARENA_MAX, OS_ERR_REFERENTSIZE_INVID);
    SDK_TASSERTMSG(OSiHeapInfo[id], OS_ERR_REFERENTSIZE_NOINFO);
    heapInfo = OSiHeapInfo[id];

    SDK_TASSERTMSG(heapInfo->heapArray, OS_ERR_REFERENT_NOHEAP);
#ifdef SDK_DEBUG
#if defined(SDK_TWL) && !defined(SDK_TWLLTD)
        if ( OSi_ExtraHeapArenaId != id )
        {
#endif
    SDK_TASSERTMSG(InRange(ptr, (char *)heapInfo->arenaStart + HEADERSIZE, (char *)heapInfo->arenaEnd), OS_ERR_REFERENT_INVPTR);
#if defined(SDK_TWL) && !defined(SDK_TWLLTD)
        }
#endif
#endif
    SDK_TASSERTMSG(OFFSET(ptr, ALIGNMENT) == 0, OS_ERR_REFERENT_INVPTR);

    cell = (Cell *) ((char *)ptr - HEADERSIZE);

    SDK_TASSERTMSG(cell->hd, OS_ERR_REFERENT_INVPTR);
    SDK_TASSERTMSG(((char *)cell->hd - (char *)heapInfo->heapArray) % sizeof(HeapDesc) == 0, OS_ERR_REFERENT_INVPTR);
    SDK_TASSERTMSG(heapInfo->heapArray <= cell->hd && cell->hd < &heapInfo->heapArray[heapInfo->numHeaps], OS_ERR_REFERENT_INVPTR);
    SDK_TASSERTMSG(0 <= cell->hd->size, OS_ERR_REFERENT_INVPTR);
    SDK_TASSERTMSG(DLLookup(cell->hd->allocated, cell), OS_ERR_REFERENT_INVPTR);

    (void)OS_RestoreInterrupts(enabled);
    return (u32)(cell->size - HEADERSIZE);
}

/*---------------------------------------------------------------------------*
  Name:         OS_DumpHeap

  Description:  Dumps statistics and elements of a heap

  Arguments:    id   :  arena ID
                heap :  handle to a heap.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void OS_DumpHeap(OSArenaId id, OSHeapHandle heap)
{
#ifndef SDK_FINALROM
    OSHeapInfo *heapInfo;
    HeapDesc *hd;
    Cell   *cell;
    OSIntrMode enabled = OS_DisableInterrupts();

    //---- check arena id
    SDK_TASSERTMSG(id < OS_ARENA_MAX, OS_ERR_DUMPHEAP_INVID);
    SDK_TASSERTMSG(OSiHeapInfo[id], OS_ERR_DUMPHEAP_NOINFO);
    heapInfo = OSiHeapInfo[id];

    SDK_TASSERTMSG(heapInfo && heapInfo->heapArray, OS_ERR_DUMPHEAP_NOHEAP);

    //---- heap<0  means current heap
    if (heap < 0)
    {
        heap = heapInfo->currentHeap;
    }
    SDK_TASSERTMSG(0 <= heap && heap < heapInfo->numHeaps, OS_ERR_DUMPHEAP_INVHEAP);

    hd = &heapInfo->heapArray[heap];
    if (hd->size < 0)
    {
        OS_TPrintf("----Inactive\n");
        return;
    }

    SDK_TASSERTMSG(0 <= OS_CheckHeap(id, heap), OS_ERR_DUMPHEAP_BROKENHEAP);

#ifdef SDK_ARM9
#ifdef  SDK_DEBUG
    OS_Printf("padding %d/(%f%%) header %d/(%f%%) payload %d/(%f%%)\n",
              hd->paddingBytes, 100.0 * hd->paddingBytes / hd->size,
              hd->headerBytes, 100.0 * hd->headerBytes / hd->size,
              hd->payloadBytes, 100.0 * hd->payloadBytes / hd->size);
#endif // SDK_DEBUG
#endif /* SDK_ARM9 */

    OS_TPrintf("  addr    size     end      prev     next\n");

    OS_TPrintf("----Allocated\n");
    SDK_TASSERTMSG(hd->allocated == NULL || hd->allocated->prev == NULL, OS_ERR_DUMPHEAP_BROKENHEAP);
    if ( ! hd->allocated )
    {
        OS_TPrintf("None.\n");
    }
    else
    {
        for (cell = hd->allocated; cell; cell = cell->next)
        {
            OS_TPrintf("%08x %6x %08x %08x %08x\n",
                      cell, cell->size, (char *)cell + cell->size, cell->prev, cell->next);
        }
    }

    OS_TPrintf("----Free\n");
    if ( ! hd->free )
    {
        OS_TPrintf("None.\n");
    }
    else
    {
        for (cell = hd->free; cell; cell = cell->next)
        {
            OS_TPrintf("%08x %6x %08x %08x %08x\n",
                      cell, cell->size, (char *)cell + cell->size, cell->prev, cell->next);
        }
    }

    (void)OS_RestoreInterrupts(enabled);
#else  // ifndef SDK_FINALROM

#pragma unused( id, heap )

#endif // ifndef SDK_FINALROM
}


/*---------------------------------------------------------------------------*
  Name:         OS_VisitAllocated

  Description:  Visits every element of every allocated block of memory,
                calling a routine on each one.

  Arguments:    id      : arena ID
                visitor : function to be called on each cell

  Returns:      None.
 *---------------------------------------------------------------------------*/
void OS_VisitAllocated(OSArenaId id, OSAllocVisitor visitor)
{
    OSHeapInfo *heapInfo;
    u32     heap;
    Cell   *cell;

    OSIntrMode enabled = OS_DisableInterrupts();

    //---- check arena id
    SDK_TASSERTMSG(id < OS_ARENA_MAX, OS_ERR_VISITALLOCATED_INVID);
    SDK_TASSERTMSG(OSiHeapInfo[id], OS_ERR_VISITALLOCATED_NOINFO);
    heapInfo = OSiHeapInfo[id];

    for (heap = 0; heap < heapInfo->numHeaps; heap++)
    {
        if (heapInfo->heapArray[heap].size >= 0)
        {
            for (cell = heapInfo->heapArray[heap].allocated; cell; cell = cell->next)
            {
                visitor((void *)((u8 *)cell + HEADERSIZE), (u32)(cell->size - HEADERSIZE));
            }
        }
    }

    (void)OS_RestoreInterrupts(enabled);
}

//================================================================================
//     Get information about heap
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         OSi_GetTotalAllocSize

  Description:  Get sum of allocated block size.
                Subroutine for OS_GetTotalAllocSize and OS_GetTotalOccupiedSize.

  Arguments:    id            : arena ID
                heap          : handle to a heap.
                isHeadInclude : whether if including block header.

  Returns:      sum of allocated block size
 *---------------------------------------------------------------------------*/
u32 OSi_GetTotalAllocSize(OSArenaId id, OSHeapHandle heap, BOOL isHeadInclude)
{
    OSHeapInfo *heapInfo;
    Cell   *cell;
    u32     sum = 0;
    OSIntrMode enabled = OS_DisableInterrupts();

    SDK_ASSERT(id < OS_ARENA_MAX);
    heapInfo = OSiHeapInfo[id];
    SDK_ASSERT(heapInfo);
    SDK_ASSERT(heap < heapInfo->numHeaps);

    //---- heap<0  means current heap
    if (heap < 0)
    {
        heap = heapInfo->currentHeap;
    }

    if (isHeadInclude)
    {
        for (cell = heapInfo->heapArray[heap].allocated; cell; cell = cell->next)
        {
            sum += (u32)(cell->size);
        }
    }
    else
    {
        for (cell = heapInfo->heapArray[heap].allocated; cell; cell = cell->next)
        {
            sum += (u32)(cell->size - HEADERSIZE);
        }
    }

    (void)OS_RestoreInterrupts(enabled);

    return sum;
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetTotalFreeSize

  Description:  Get sum of free block size,
                not includeing of block header.

  Arguments:    id   :  arena ID
                heap :  handle to a heap.

  Returns:      sum of free block size
 *---------------------------------------------------------------------------*/
u32 OS_GetTotalFreeSize(OSArenaId id, OSHeapHandle heap)
{
    OSHeapInfo *heapInfo;
    Cell   *cell;
    u32     sum = 0;
    OSIntrMode enabled = OS_DisableInterrupts();

    SDK_ASSERT(id < OS_ARENA_MAX);
    heapInfo = OSiHeapInfo[id];
    SDK_ASSERT(heapInfo);
    SDK_ASSERT(heap < heapInfo->numHeaps);

    //---- heap<0  means current heap
    if (heap < 0)
    {
        heap = heapInfo->currentHeap;
    }

    for (cell = heapInfo->heapArray[heap].free; cell; cell = cell->next)
    {
        sum += (u32)(cell->size - HEADERSIZE);
    }

    (void)OS_RestoreInterrupts(enabled);

    return sum;
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetMaxFreeSize

  Description:  Get maximun free block size

  Arguments:    id   :  arena ID
                heap :  handle to a heap.

  Returns:      maximum free block size.
 *---------------------------------------------------------------------------*/
u32 OS_GetMaxFreeSize(OSArenaId id, OSHeapHandle heap)
{
    OSHeapInfo *heapInfo;
    Cell   *cell;
    u32     candidate = 0;
    OSIntrMode enabled = OS_DisableInterrupts();

    SDK_ASSERT(id < OS_ARENA_MAX);
    heapInfo = OSiHeapInfo[id];
    SDK_ASSERT(heapInfo);
    SDK_ASSERT(heap < heapInfo->numHeaps);

    //---- heap<0  means current heap
    if (heap < 0)
    {
        heap = heapInfo->currentHeap;
    }

    for (cell = heapInfo->heapArray[heap].free; cell; cell = cell->next)
    {
        u32     size = (u32)(cell->size - HEADERSIZE);
        if (size > candidate)
        {
            candidate = size;
        }
    }

    (void)OS_RestoreInterrupts(enabled);

    return candidate;
}

//================================================================================
//     re-initialize heap.
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_ClearHeap

  Description:  re-initialize heap.

  Arguments:    id   :  arena ID
                heap :  handle to a heap.
                start:  starting addr of heap
                end  :  endinf addr of heap

  Returns:      None.
 *---------------------------------------------------------------------------*/
void OS_ClearHeap(OSArenaId id, OSHeapHandle heap, void *start, void *end)
{
    OSHeapInfo *heapInfo;
    HeapDesc *hd;
    Cell   *cell;

    OSIntrMode enabled = OS_DisableInterrupts();

    //---- check arena id
    SDK_TASSERTMSG(id < OS_ARENA_MAX, "invalid id");

    heapInfo = OSiHeapInfo[id];

    SDK_TASSERTMSG(heapInfo && heapInfo->heapArray, "heap not initialized");
    SDK_TASSERTMSG(start < end, "invalid range");
    start = (void *)ROUND(start, ALIGNMENT);
    end = (void *)TRUNC(end, ALIGNMENT);
    SDK_TASSERTMSG(start < end, "invalid range");
#ifdef SDK_DEBUG
#if defined(SDK_TWL) && !defined(SDK_TWLLTD)
    if ( OSi_ExtraHeapArenaId != id || OSi_ExtraHeapHandle != heap )
    {
#endif
        SDK_TASSERTMSG(RangeSubset(start, end, heapInfo->arenaStart, heapInfo->arenaEnd), "invalid range");
#if defined(SDK_TWL) && !defined(SDK_TWLLTD)
    }
#endif
#endif
    SDK_TASSERTMSG(MINOBJSIZE <= (char *)end - (char *)start, "too small range");

    //---- heap<0  means current heap
    if (heap < 0)
    {
        heap = heapInfo->currentHeap;
    }

    //---- Search for free descriptor
    hd = &heapInfo->heapArray[heap];
    hd->size = (char *)end - (char *)start;

    cell = (Cell *) start;
    cell->prev = NULL;
    cell->next = NULL;
    cell->size = hd->size;
#ifdef  SDK_DEBUG
    cell->hd = NULL;
#endif // SDK_DEBUG

    hd->free = cell;
    hd->allocated = 0;
#ifdef  SDK_DEBUG
    hd->paddingBytes = hd->headerBytes = hd->payloadBytes = 0;
#endif // SDK_DEBUG

    (void)OS_RestoreInterrupts(enabled);
}
/*---------------------------------------------------------------------------*
  Name:         OS_ClearExtraHeap

  Description:  re-initialize extra heap.

  Arguments:    id   :  arena ID
                heap :  handle to a heap.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#if defined(SDK_TWL) && !defined(SDK_TWLLTD)
void OS_ClearExtraHeap(OSArenaId id, OSHeapHandle heap)
{
    if ( ! OS_IsRunOnTwl() )
    {
        if ( OSi_ExtraHeapArenaId == id || OSi_ExtraHeapHandle == heap )
        {
            OS_ClearHeap(id, heap,
                         (void*)HW_MAIN_MEM_PARAMETER_BUF,
                         (void*)(HW_MAIN_MEM_PARAMETER_BUF + HW_MAIN_MEM_PARAMETER_BUF_SIZE ) );
        }
    }
}
#endif

//================================================================================
//     Check Alocated Memory
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_IsOnMainMemory

  Description:  check memory address to which a pointer points is on main RAM.

  Arguments:    ptr  : pointer

  Returns:      TRUE if ptr points is on main RAM.
 *---------------------------------------------------------------------------*/
BOOL OS_IsOnMainMemory(void* ptr)
{
#ifdef SDK_TWL
    if( OS_IsRunOnTwl() )
    {
        /* TWL */
        if( (u32)ptr >= HW_TWL_MAIN_MEM && (u32)ptr < HW_TWL_MAIN_MEM_END )
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
#endif //SDK_TWL
    {
        /* NITRO */
        if( (u32)ptr >= HW_MAIN_MEM && (u32)ptr < HW_MAIN_MEM_END )
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    return FALSE; /* Do Not Return HERE */
}

/*---------------------------------------------------------------------------*
  Name:         OS_IsOnExtendedMainMemory

  Description:  check memory address to which a pointer points is on extended main RAM.

  Arguments:    ptr  : pointer

  Returns:      TRUE if ptr points is on extended main RAM.
 *---------------------------------------------------------------------------*/
BOOL OS_IsOnExtendedMainMemory(void* ptr)
{
#ifdef SDK_TWL
    if( OS_IsRunOnTwl() )
    {
        /* TWL */
        if( (u32)ptr >= HW_TWL_MAIN_MEM_EX && (u32)ptr < HW_TWL_MAIN_MEM_EX_END )
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
#endif //SDK_TWL
    {
        /* NITRO */
        if( (u32)ptr >= HW_MAIN_MEM_END && (u32)ptr < HW_MAIN_MEM_EX_END )
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    return FALSE; /* Do Not Return HERE */
}




/*---------------------------------------------------------------------------*
  Name:         OS_IsOnWramB

  Description:  check memory address to which a pointer points is on WRAM B.

  Arguments:    ptr  : pointer

  Returns:      TRUE if ptr points is on WRAM B.
 *---------------------------------------------------------------------------*/
BOOL OS_IsOnWramB(void* ptr)
{
#ifdef SDK_TWL
    if( OS_IsRunOnTwl() )
    {
        /* TWL */
        if( (u32)ptr >= HW_WRAM_B && (u32)ptr < HW_WRAM_B_END )
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
#else
    #pragma unused(ptr)
#endif //SDK_TWL
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         OSi_IsOnWramC

  Description:  check memory address to which a pointer points is on WRAM C.

  Arguments:    ptr  : pointer

  Returns:      TRUE if ptr points is on WRAM C.
 *---------------------------------------------------------------------------*/
BOOL OS_IsOnWramC(void* ptr)
{
#ifdef SDK_TWL
    if( OS_IsRunOnTwl() )
    {
        /* TWL */
        if( (u32)ptr >= HW_WRAM_C && (u32)ptr < HW_WRAM_C_END )
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
#else
    #pragma unused(ptr)
#endif //SDK_TWL
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         OS_IsOnWram0

  Description:  check memory address to which a pointer points is on WRAM 0.

  Arguments:    ptr  : pointer

  Returns:      TRUE if ptr points is on WRAM 0.
 *---------------------------------------------------------------------------*/
BOOL OS_IsOnWram0(void* ptr)
{
#ifdef SDK_TWL
#ifdef SDK_ARM7
    if( OS_IsRunOnTwl() )
    {
#ifdef SDK_TWLLTD
        if( (u32)ptr >= HW_WRAM_0_LTD && (u32)ptr < HW_WRAM_0_LTD_END )
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
#else   
       // !SDK_TWLLTD
        if( (u32)ptr >= HW_WRAM_0_HYB && (u32)ptr < HW_WRAM_0_HYB_END )
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
#endif  //SDK_TWLLTD
    }
#else
 //!SDK_ARM7
    #pragma unused(ptr)
#endif  //SDK_ARM7
#else
      //!SDK_TWL
    {
        if( (u32)ptr >= HW_WRAM_0 && (u32)ptr < HW_WRAM_0_END )
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
#endif // SDK_ARM7

    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         OS_IsOnWram1

  Description:  check memory address to which a pointer points is on WRAM 1.

  Arguments:    ptr  : pointer

  Returns:      TRUE if ptr points is on WRAM 1.
 *---------------------------------------------------------------------------*/
BOOL OS_IsOnWram1(void* ptr)
{
#ifdef SDK_TWL
#ifdef SDK_ARM7
    if( OS_IsRunOnTwl() )
    {
#ifdef SDK_TWLLTD
        if( (u32)ptr >= HW_WRAM_1_LTD && (u32)ptr < HW_WRAM_1_LTD_END )
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
#else   
       // !SDK_TWLLTD
        if( (u32)ptr >= HW_WRAM_1_HYB && (u32)ptr < HW_WRAM_1_HYB_END )
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
#endif  //SDK_TWLLTD
    }
#else
  //SDK_TWL && !SDK_ARM7
    #pragma unused(ptr)
#endif  //SDK_ARM7
#else
      //!SDK_TWL
    {
        if( (u32)ptr >= HW_WRAM_1 && (u32)ptr < HW_WRAM_1_END )
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
#endif // SDK_ARM7

    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         OS_IsOnWram

  Description:  check memory address to which a pointer points is on WRAM.

  Arguments:    ptr  : pointer

  Returns:      TRUE if ptr points is on WRAM.
 *---------------------------------------------------------------------------*/
BOOL OS_IsOnWram(void* ptr)
{
    return ( OS_IsOnWramA(ptr) || OS_IsOnWramB(ptr) || OS_IsOnWramC(ptr) ||OS_IsOnWram0(ptr) ||OS_IsOnWram1(ptr) );
}


/*---------------------------------------------------------------------------*
  Name:         OS_IsOnVram

  Description:  check memory address to which a pointer points is on VRAM.

  Arguments:    ptr  : pointer

  Returns:      TRUE if ptr points is on VRAM.
 *---------------------------------------------------------------------------*/
BOOL OS_IsOnVram(void* ptr)
{
#ifdef SDK_ARM9
    if( (u32)ptr >= HW_PLTT && (u32)ptr < HW_DB_OAM_END )
    {
        return TRUE;
    }
#else
    #pragma unused(ptr)
#endif
    return FALSE;
}


/*---------------------------------------------------------------------------*
  Name:         OS_IsOnDtcm

  Description:  check memory address to which a pointer points is on DTCM.

  Arguments:    ptr  : pointer

  Returns:      TRUE if ptr points is on DTCM.
 *---------------------------------------------------------------------------*/
BOOL OS_IsOnDtcm(void* ptr)
{
#ifdef SDK_ARM9
    u32 dtcm;
    u32 dtcm_end;
    
    dtcm = OS_GetDTCMAddress();
    dtcm_end = dtcm + HW_DTCM_SIZE;
    
    if( (u32)ptr >= dtcm && (u32)ptr < dtcm_end )
    {
        return TRUE;
    }
#else
    #pragma unused(ptr)
#endif
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         OS_IsOnItcm

  Description:  check memory address to which a pointer points is on ITCM.

  Arguments:    ptr  : pointer

  Returns:      TRUE if ptr points is on ITCM.
 *---------------------------------------------------------------------------*/
BOOL OS_IsOnItcm(void* ptr)
{
#ifdef SDK_ARM9
    u32 itcm;
    u32 itcm_end;
    
    itcm = OS_GetITCMAddress();
    itcm_end = itcm + HW_ITCM_SIZE;
    
    if( (u32)ptr >= itcm && (u32)ptr < itcm_end )
    {
        return TRUE;
    }
#else
    #pragma unused(ptr)
#endif
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         OS_IsOnWramA

  Description:  check memory address to which a pointer points is on WRAM A.

  Arguments:    ptr  : pointer

  Returns:      TRUE if ptr points is on WRAM A.
 *---------------------------------------------------------------------------*/
BOOL OS_IsOnWramA(void* ptr)
{
#ifdef SDK_TWL
#ifdef SDK_ARM7
    if( OS_IsRunOnTwl() )
    {
#ifdef SDK_TWLLTD
        /* TWL LTD */
        if( (u32)ptr >= HW_WRAM_A_LTD && (u32)ptr < HW_WRAM_A_LTD_END )
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
#else
        /* TWL HYB */
        if( (u32)ptr >= HW_WRAM_A_HYB && (u32)ptr < HW_WRAM_A_HYB_END )
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
#endif  //SDK_TWLLTD
    }
#else   
   // SDK_TWL && !SDK_ARM7
    #pragma unused(ptr)
#endif //SDK_ARM7

#else
   // !SDK_TWL
    #pragma unused(ptr)
#endif //SDK_TWL
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         OS_IsOnArm7PrvWram

  Description:  check memory address to which a pointer points is on ARM7 Private WRAM.

  Arguments:    ptr  : pointer

  Returns:      TRUE if ptr points is on ARM7 Private WRAM.
 *---------------------------------------------------------------------------*/
BOOL OS_IsOnArm7PrvWram(void* ptr)
{
#ifdef SDK_ARM7
        if( (u32)ptr >= HW_PRV_WRAM && (u32)ptr < HW_PRV_WRAM_END )
        {
            return TRUE;
        }
#else
    #pragma unused(ptr)
#endif //SDK_ARM7
    return FALSE;
}

