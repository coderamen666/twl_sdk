/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - snd
  File:     heap.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
#include <nnsys/snd/heap.h>

#include <nnsys/misc.h>
#include <nnsys/fnd.h>

/******************************************************************************
	Macro Definitions
 ******************************************************************************/

#define HEAP_ALIGN 32

#define ROUNDUP( value, align ) ( ( (u32)(value) + ( (align) - 1 ) ) & ~( (align) - 1 ) )

/******************************************************************************
	Structure Definitions
 ******************************************************************************/

typedef struct NNSSndHeap
{
    NNSFndHeapHandle handle;
    NNSFndList sectionList;
} NNSSndHeap;

typedef struct NNSSndHeapBlock
{
    NNSFndLink link;
    u32 size;
    NNSSndHeapDisposeCallback callback;
    u32 data1;
    u32 data2;
    u8 padding[ 0x20 - ( ( sizeof( NNSFndLink ) + sizeof( NNSSndHeapDisposeCallback ) + sizeof( u32 ) * 3 ) & 0x1f ) ];
    u32 buffer[ 0 ];
} NNSSndHeapBlock; // NOTE: must be 32-byte aligned

typedef struct NNSSndHeapSection
{
    NNSFndList blockList;
    NNSFndLink link;
} NNSSndHeapSection;

/******************************************************************************
	static function declarations
 ******************************************************************************/

static void InitHeapSection( NNSSndHeapSection* section );
static BOOL InitHeap( NNSSndHeap* heap, NNSFndHeapHandle handle );
static BOOL NewSection( NNSSndHeap* heap );
static void EraseSync( void );

/******************************************************************************
	Public Functions
 ******************************************************************************/

/*---------------------------------------------------------------------------*
  Name:         NNS_SndHeapCreate

  Description:  Creates the heap.

  Arguments:    startAddress - starting address
                size     - memory size

  Returns:      the heap handle
 *---------------------------------------------------------------------------*/
NNSSndHeapHandle NNS_SndHeapCreate( void* startAddress, u32 size )
{
    NNSSndHeap* heap;
    void* endAddress;
    NNSFndHeapHandle handle;
    
    NNS_NULL_ASSERT( startAddress );
    
    endAddress   = (u8*)startAddress + size;
    startAddress = (void*)ROUNDUP( startAddress, 4 ); // NNSSndHeap align
    
    if ( startAddress > endAddress ) return NNS_SND_HEAP_INVALID_HANDLE;
    
    size = (u32)( (u8*)endAddress - (u8*)startAddress );
    if ( size < sizeof( NNSSndHeap ) ) {
        return NNS_SND_HEAP_INVALID_HANDLE;
    }
    
    size -= sizeof( NNSSndHeap );
    
    heap = (NNSSndHeap*)startAddress;
    startAddress = heap + 1;
    
    handle = NNS_FndCreateFrmHeap( startAddress, size );
    if ( handle == NNS_FND_HEAP_INVALID_HANDLE ) {
        return NULL;
    }
    
    if ( ! InitHeap( heap, handle ) ) {
        NNS_FndDestroyFrmHeap( handle );
        return NNS_SND_HEAP_INVALID_HANDLE;
    }
    
    return heap;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndHeapDestroy

  Description:  Destroys the heap.

  Arguments:    heap - sound heap

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndHeapDestroy( NNSSndHeapHandle heap )
{
    NNS_ASSERT( heap != NNS_SND_HEAP_INVALID_HANDLE );
    
    NNS_SndHeapClear( heap );
    
    NNS_FndDestroyFrmHeap( heap->handle );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndHeapClear

  Description:  Restores the state at the time of heap creation.

  Arguments:    heap - sound heap

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndHeapClear( NNSSndHeapHandle heap )
{
    NNSSndHeapSection* section=NULL;
    void* object;
    BOOL result;
    BOOL doCallback = FALSE;
    
    NNS_ASSERT( heap != NNS_SND_HEAP_INVALID_HANDLE );
    
    // discard section
    while ( ( section = (NNSSndHeapSection*)NNS_FndGetPrevListObject( & heap->sectionList, NULL ) ) != NULL )
    {
        // call the callback
        object = NULL;
        while ( ( object = NNS_FndGetPrevListObject( & section->blockList, object ) ) != NULL )
        {
            NNSSndHeapBlock* block = (NNSSndHeapBlock*)object;
            if ( block->callback != NULL ) {
                block->callback( block->buffer, block->size, block->data1, block->data2 );
                doCallback = TRUE;
            }
        }
        
        // delete from the section list
        NNS_FndRemoveListObject( & heap->sectionList, section );
    }
    
    // clear the heap
    NNS_FndFreeToFrmHeap( heap->handle, NNS_FND_FRMHEAP_FREE_ALL );
    
    // synchronizes discontinuation of sound data usage
    if ( doCallback ) EraseSync();
    
    // create the base section
    result = NewSection( heap );
    NNS_ASSERTMSG( result, "NNS_SndHeapClear(): NewSection is Failed");
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndHeapAlloc

  Description:  Allocates memory from the heap.

  Arguments:    heap - sound heap
                size     - memory size
                callback - callback function to invoke when memory is deleted
                data1    - callback data 1
                data2    - callback data 2

  Returns:      pointer for the allocated memory
 *---------------------------------------------------------------------------*/
void* NNS_SndHeapAlloc( NNSSndHeapHandle heap, u32 size, NNSSndHeapDisposeCallback callback, u32 data1, u32 data2 )
{
    NNSSndHeapSection* section;
    NNSSndHeapBlock* block;
    
    NNS_ASSERT( heap != NNS_SND_HEAP_INVALID_HANDLE );
    
    block = (NNSSndHeapBlock*)NNS_FndAllocFromFrmHeapEx(
        heap->handle, sizeof( NNSSndHeapBlock ) + ROUNDUP( size, HEAP_ALIGN ), HEAP_ALIGN );
    if ( block == NULL ) return NULL;
    
    section = (NNSSndHeapSection*)NNS_FndGetPrevListObject( & heap->sectionList, NULL );
    
    block->size = size;
    block->callback = callback;
    block->data1 = data1;
    block->data2 = data2;
    NNS_FndAppendListObject( & section->blockList, block );
    
    NNS_ASSERTMSG( ( (u32)( block->buffer ) & 0x1f ) == 0, "NNS_SndHeapAlloc: Internal Error" );
    
    return block->buffer;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndHeapSaveState

  Description:  Saves the heap state.

  Arguments:    heap - sound heap

  Returns:      Returns the saved hierarchy level.
                Returns -1 if failed.
 *---------------------------------------------------------------------------*/
int NNS_SndHeapSaveState( NNSSndHeapHandle heap )
{
    BOOL result;
    
    NNS_ASSERT( heap != NNS_SND_HEAP_INVALID_HANDLE );
    
    if ( ! NNS_FndRecordStateForFrmHeap( heap->handle, heap->sectionList.numObjects ) ) {
        return -1;
    }
    
    if ( ! NewSection( heap ) ) {
        result = NNS_FndFreeByStateToFrmHeap( heap->handle, 0 );
        NNS_ASSERTMSG( result, "NNS_SndHeapSaveState(): NNS_FndFreeByStateToFrmHeap is Failed");
        return -1;
    }
    
    return heap->sectionList.numObjects - 1;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndHeapLoadState

  Description:  Returns the heap state.

  Arguments:    heap - sound heap
                level - hierarchy level

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_SndHeapLoadState( NNSSndHeapHandle heap, int level )
{
    NNSSndHeapSection* section;
    void* object = NULL;
    BOOL result;
    BOOL doCallback = FALSE;
    
    NNS_ASSERT( heap != NNS_SND_HEAP_INVALID_HANDLE );
    NNS_MINMAX_ASSERT( level, 0, heap->sectionList.numObjects-1 );
    
    if ( level == 0 ) {
        NNS_SndHeapClear( heap );
        return;
    }
    while( level < heap->sectionList.numObjects )
    {
        // get latest section
        section = (NNSSndHeapSection*)NNS_FndGetPrevListObject( & heap->sectionList, NULL );
        
        // call dispose callback
        while ( ( object = NNS_FndGetPrevListObject( & section->blockList, object ) ) != NULL )
        {
            NNSSndHeapBlock* block = (NNSSndHeapBlock*)object;
            if ( block->callback != NULL ) {
                block->callback( block->buffer, block->size, block->data1, block->data2 );
                doCallback = TRUE;
            }
        }
        
        // delete from the section list
        NNS_FndRemoveListObject( & heap->sectionList, section );
    }
    
    // restore the heap state
    result = NNS_FndFreeByStateToFrmHeap( heap->handle, (u32)level );
    NNS_ASSERTMSG( result, "NNS_SndHeapLoadState(): NNS_FndFreeByStateToFrmHeap is Failed");   

    // synchronizes discontinuation of sound data usage
    if ( doCallback ) EraseSync();
    
    // resave
    result =  NNS_FndRecordStateForFrmHeap( heap->handle, heap->sectionList.numObjects );
    NNS_ASSERTMSG( result, "NNS_SndHeapLoadState(): NNS_FndRecordStateForFrmHeap is Failed");
    
    // create section
    result = NewSection( heap );
    NNS_ASSERTMSG( result, "NNS_SndHeapLoadState(): NewSection is Failed");
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndHeapGetCurrentLevel

  Description:  Gets the current hierarchy level of the heap.

  Arguments:    heap - sound heap

  Returns:      current hierarchy level
 *---------------------------------------------------------------------------*/
int NNS_SndHeapGetCurrentLevel( NNSSndHeapHandle heap )
{
    NNS_ASSERT( heap != NNS_SND_HEAP_INVALID_HANDLE );
    
    return heap->sectionList.numObjects - 1;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndHeapGetSize

  Description:  Gets the capacity of the heap.

  Arguments:    heap - sound heap

  Returns:      heap capacity
 *---------------------------------------------------------------------------*/
u32 NNS_SndHeapGetSize( NNSSndHeapHandle heap )
{
    NNS_ASSERT( heap != NNS_SND_HEAP_INVALID_HANDLE );
    
    return
        (u32)( (u8*)NNS_FndGetHeapEndAddress( heap->handle ) -
               (u8*)NNS_FndGetHeapStartAddress( heap->handle ) )
        ;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndHeapGetFreeSize

  Description:  Gets the amount of free space in the heap.

  Arguments:    heap - sound heap

  Returns:      amount of free space
 *---------------------------------------------------------------------------*/
u32 NNS_SndHeapGetFreeSize( NNSSndHeapHandle heap )
{
    u32 size;
    
    NNS_ASSERT( heap != NNS_SND_HEAP_INVALID_HANDLE );
    
    size = NNS_FndGetAllocatableSizeForFrmHeapEx( heap->handle, HEAP_ALIGN );
    
    if ( size < sizeof( NNSSndHeapBlock ) ) return 0;
    size -= sizeof( NNSSndHeapBlock );
    
    size &= ~0x1f;
    
    return size;
}

/******************************************************************************
	Static Functions
 ******************************************************************************/

/*---------------------------------------------------------------------------*
  Name:         InitHeapSection

  Description:  Initializes the NNSSndHeapSection structure.

  Arguments:    section - heap section

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitHeapSection( NNSSndHeapSection* section )
{
    NNS_FND_INIT_LIST( & section->blockList, NNSSndHeapBlock, link );    
}

/*---------------------------------------------------------------------------*
  Name:         InitHeap

  Description:  Initializes the NNSSndHeap structure.

  Arguments:    heap - sound heap
                handle - frame heap handle

  Returns:      Whether it was successful or not.
 *---------------------------------------------------------------------------*/
static BOOL InitHeap( NNSSndHeap* heap, NNSFndHeapHandle handle )
{
    NNS_FND_INIT_LIST( & heap->sectionList, NNSSndHeapSection, link );
    heap->handle = handle;
    
    // create the base section
    if ( ! NewSection( heap ) ) {
        return FALSE;
    }
    
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         NewSection

  Description:  Creates a new section.

  Arguments:    heap - sound heap

  Returns:      Whether it was successful or not.
 *---------------------------------------------------------------------------*/
static BOOL NewSection( NNSSndHeap* heap )
{
    NNSSndHeapSection* section;
    
    // new NNSSndHeapSection
    section = (NNSSndHeapSection*)NNS_FndAllocFromFrmHeap( heap->handle, sizeof( NNSSndHeapSection ) );
    if ( section == NULL ) {
        return FALSE;
    }
    InitHeapSection( section );
    
    NNS_FndAppendListObject( & heap->sectionList, section );
    
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         EraseSync

  Description:  Performs synchronization to stop the use of data completely when the heap is deleted.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void EraseSync( void )
{
    u32 commandTag;
    
    commandTag = SND_GetCurrentCommandTag();
    (void)SND_FlushCommand( SND_COMMAND_BLOCK );
    SND_WaitForCommandProc( commandTag );
}

/*====== End of heap.c ======*/

