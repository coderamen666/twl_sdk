/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - gfd - src - VramManager
  File:     gfdi_LinkedListVramMan_Common.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
#include <nnsys/gfd/gfd_common.h>
#include "gfdi_LinkedListVramMan_Common.h"



//------------------------------------------------------------------------------
// Gets the region from the block.
static NNS_GFD_INLINE void 
GetRegionOfMemBlock_
(
    NNSiGfdLnkMemRegion*            pRegion,
    const NNSiGfdLnkVramBlock*      pBlk
)
{
    NNS_GFD_NULL_ASSERT( pBlk );
    NNS_GFD_NON_ZERO_ASSERT( pBlk->szByte );
    
    pRegion->start = pBlk->addr;
    pRegion->end   = pBlk->addr + pBlk->szByte;
    
    NNS_GFD_ASSERT( pRegion->end > pRegion->start );
}

//------------------------------------------------------------------------------
// Initializes the block from the region.
static NNS_GFD_INLINE void InitBlockFromRegion_
( 
    NNSiGfdLnkVramBlock*        pBlk, 
    const NNSiGfdLnkMemRegion*  pRegion
)
{
    NNS_GFD_NULL_ASSERT( pBlk );
    NNS_GFD_ASSERT( pRegion->end > pRegion->start );
    
    pBlk->addr      = pRegion->start;
    pBlk->szByte    = (u32)(pRegion->end - pRegion->start);
    pBlk->pBlkPrev  = NULL;
    pBlk->pBlkNext  = NULL;
}

//------------------------------------------------------------------------------
// Initializes the block from the parameter types.
static NNS_GFD_INLINE void InitBlockFromPrams_
( 
    NNSiGfdLnkVramBlock*    pBlk, 
    u32                     addr, 
    u32                     szByte 
)
{   
    NNS_GFD_NULL_ASSERT( pBlk );
    NNS_GFD_NON_ZERO_ASSERT( szByte );
    
    pBlk->addr      = addr;
    pBlk->szByte    = szByte;
    pBlk->pBlkPrev  = NULL;
    pBlk->pBlkNext  = NULL;
}

//------------------------------------------------------------------------------
// Inserts elements in beginning of list.
static NNS_GFD_INLINE void InsertBlock_
(
    NNSiGfdLnkVramBlock**   pListHead,
    NNSiGfdLnkVramBlock*    prev
)
{
    NNS_GFD_NULL_ASSERT( prev );
    
    if( (*pListHead) != NULL )
    {
        (*pListHead)->pBlkPrev    = prev;
    }
    
    prev->pBlkNext          = *pListHead;
    prev->pBlkPrev          = NULL;
    *pListHead = prev;
    
}

//------------------------------------------------------------------------------
// Removes elements from list.
static NNS_GFD_INLINE void RemoveBlock_
(
    NNSiGfdLnkVramBlock**  pListHead,
    NNSiGfdLnkVramBlock*   pBlk
)
{
    NNS_GFD_NULL_ASSERT( pBlk );
    {
    
        NNSiGfdLnkVramBlock *const pPrev = pBlk->pBlkPrev;
        NNSiGfdLnkVramBlock *const pNext = pBlk->pBlkNext;

        // previous reference link
        if ( pPrev )
        {
            pPrev->pBlkNext = pNext;
        }else{
            *pListHead = pNext;
        }
        

        // next reference link
        if ( pNext )
        {
            pNext->pBlkPrev = pPrev;
        }
    }
}

//------------------------------------------------------------------------------
// Gets new block.
static NNS_GFD_INLINE NNSiGfdLnkVramBlock* 
GetNewBlock_( NNSiGfdLnkVramBlock**   ppBlockPoolList )
{
    NNS_GFD_NULL_ASSERT( ppBlockPoolList );
    {
        // remove from the front of the list
        NNSiGfdLnkVramBlock*    pNew = *ppBlockPoolList;
        if( pNew )
        {
            *ppBlockPoolList = pNew->pBlkNext;
        }
        
        return pNew;
    }
}   


//------------------------------------------------------------------------------
// Gets the end address of the block.
static NNS_GFD_INLINE u32 GetBlockEndAddr_( NNSiGfdLnkVramBlock* pBlk )
{
    NNS_GFD_NULL_ASSERT( pBlk );
    
    return (u32)(pBlk->addr + pBlk->szByte);
}

//------------------------------------------------------------------------------
// The debug output function that is used in NNSi_GfdDumpLnkVramManFreeListInfo().
static void DefaultDumpCallBack_( 
    u32                             addr, 
    u32                             szByte, 
    void*                           pUserData )
{

#ifdef SDK_FINALROM
    #pragma unused(addr)
#endif // SDK_FINALROM

    if( szByte != 0 )
    {
        u32*                        pszFreeTotal    = (u32*)pUserData;
           
        OS_Printf("0x%08x:  0x%08x    \n", addr, szByte );   
        (*pszFreeTotal) += szByte;
    }
}

//------------------------------------------------------------------------------
// Scan free blocks and merge regions.
// Returns a Boolean value that indicates whether a merge was performed.
static BOOL TryToMergeBlockRegion_( 
    NNSiGfdLnkVramMan*      pMan, 
    NNSiGfdLnkVramBlock**   ppBlockPoolList,
    NNSiGfdLnkMemRegion*    pRegion )
{
    NNS_GFD_NULL_ASSERT( pMan );
    NNS_GFD_NULL_ASSERT( ppBlockPoolList );
    NNS_GFD_NULL_ASSERT( pRegion );
            
    {
        // search for free area next to the specified one
        NNSiGfdLnkVramBlock*        pCursor         = pMan->pFreeList;
        NNSiGfdLnkVramBlock*        pNext           = NULL;
        BOOL                        bMerged         = FALSE;

        // the free list elements
        while( pCursor )
        {
            pNext = pCursor->pBlkNext;
              
            // Is the block adjacent to the end?
            if( pCursor->addr == pRegion->end )   
            {
                // combine the available regions
                pRegion->end = GetBlockEndAddr_( pCursor );
                // Removes from the list and returns to pool.
                RemoveBlock_( &pMan->pFreeList, pCursor );
                InsertBlock_( ppBlockPoolList, pCursor );
                bMerged |= TRUE;
            }
                            
            // Is it a block that is next to the beginning?
            if( GetBlockEndAddr_( pCursor ) == pRegion->start )
            {
                // combine the available regions
                pRegion->start  = pCursor->addr;
                // Removes from the list and returns to pool.
                RemoveBlock_( &pMan->pFreeList, pCursor );
                InsertBlock_( ppBlockPoolList, pCursor );
                bMerged |= TRUE;
            }
            
            pCursor = pNext;
        }

        return bMerged;
    }
}


/*---------------------------------------------------------------------------*
  Name:         NNSi_GfdDumpLnkVramManFreeListInfo

  Description:  Outputs free block information for debugging.
                
  Arguments:    pFreeListHead: head of free block information list
                szReserved: size of allocated area
               
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNSi_GfdDumpLnkVramManFreeListInfo
( 
    const NNSiGfdLnkVramBlock*      pFreeListHead,
    u32                             szReserved 
)
{
    
    u32                         szFreeTotal = 0; 
    const NNSiGfdLnkVramBlock*  pBlk        = pFreeListHead;
    
    // display all free list information
    NNSi_GfdDumpLnkVramManFreeListInfoEx( pBlk, DefaultDumpCallBack_, &szFreeTotal );
    
    // If there is no free list, display dummy line
    if( szFreeTotal == 0 )
    {
        OS_Printf("0x--------:  0x--------    \n");
    }
    
    // display utilization
    {
        const u32 szUsedTotal = (szReserved - szFreeTotal);
        OS_Printf("    %08d / %08d bytes (%6.2f%%) used \n", 
            szUsedTotal, szReserved, 100.f *  szUsedTotal / szReserved );   
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNSi_GfdDumpLnkVramManFreeListInfoEx

  Description:  
                Specifies debug output function and outputs free block information for debugging.
                
  Arguments:    pFreeListHead: head of free block information list
                pFunc: debug output function
                pUserData: Debug output data passed to the debug output function as an argument.
                                         
               
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNSi_GfdDumpLnkVramManFreeListInfoEx( 
    const NNSiGfdLnkVramBlock*      pFreeListHead, 
    NNSGfdLnkDumpCallBack           pFunc, 
    void*                           pUserData )
{
    // display all free list information
    const NNSiGfdLnkVramBlock*  pBlk        = pFreeListHead;
    
    NNS_GFD_NULL_ASSERT( pFunc );
    
    while( pBlk )
    {
        (*pFunc)( pBlk->addr, pBlk->szByte, pUserData );
        pBlk          = pBlk->pBlkNext;
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNSi_GfdInitLnkVramMan

  Description:  This function initializes NNSiGfdLnkVramMan.
                
  Arguments:    pMgr: VRAM Manager
                
               
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void 
NNSi_GfdInitLnkVramMan( NNSiGfdLnkVramMan* pMgr )
{
    NNS_GFD_NULL_ASSERT( pMgr );
    pMgr->pFreeList = NULL;
}


/*---------------------------------------------------------------------------*
  Name:         NNSi_GfdInitLnkVramBlockPool

  Description:  Initializes the managed block pool.
                number of management information elements == number of memory segments that can be managed
                
  Arguments:    pArrayHead: beginning of management information array
                lengthOfArray: number of management information elements
               
  Returns:      beginning of list
  
 *---------------------------------------------------------------------------*/
NNSiGfdLnkVramBlock* 
NNSi_GfdInitLnkVramBlockPool
( 
    NNSiGfdLnkVramBlock*    pArrayHead, 
    u32                     lengthOfArray 
)
{
    NNS_GFD_NULL_ASSERT( pArrayHead );
    NNS_GFD_NON_ZERO_ASSERT( lengthOfArray );
    
    //
    // connected by list
    //
    {
        int i;
        for( i = 0; i < lengthOfArray - 1; i++ )
        {
            pArrayHead[i].pBlkNext      = &pArrayHead[i+1];
            pArrayHead[i+1].pBlkPrev    = &pArrayHead[i];
        }
        
        pArrayHead[0].pBlkPrev                  = NULL;  
        pArrayHead[lengthOfArray - 1].pBlkNext  = NULL;    
    }
    
    // returns the beginning of list
    return &pArrayHead[0];
}

/*---------------------------------------------------------------------------*
  Name:         NNSi_GfdInitLnkVramMan

  Description:  This function adds the blocks of newly freed areas to the manager.
                
                
  Arguments:    pMan: Manager
                ppBlockPoolList: common block management information
                baseAddr: management area base address
                szByte: memory size to allocate
               
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
BOOL NNSi_GfdAddNewFreeBlock
(
    NNSiGfdLnkVramMan*      pMan, 
    NNSiGfdLnkVramBlock**   ppBlockPoolList,
    u32                     baseAddr,
    u32                     szByte
)
{
    NNS_GFD_NULL_ASSERT( pMan );
    NNS_GFD_NULL_ASSERT( ppBlockPoolList );
    NNS_GFD_NON_ZERO_ASSERT( szByte );
    
    // Creates free blocks.
    {
        NNSiGfdLnkVramBlock*        pNew  = GetNewBlock_( ppBlockPoolList );
        if( pNew )
        {
            InitBlockFromPrams_( pNew, baseAddr, szByte );
            InsertBlock_( &pMan->pFreeList, pNew );
            
            return TRUE;
        }else{
            return FALSE;
        }
    }
}



/*---------------------------------------------------------------------------*
  Name:         NNSi_GfdAllocLnkVram

  Description:  This function allocates memory.
                
                Take note that if the management blocks are insufficient, even with enough remaining memory, the allocation may fail. 
                
                
  Arguments:    pMan: Manager
                ppBlockPoolList: common block management information
                pRetAddr: pointer to allocation address
                szByte: memory size to allocate
                
  Returns:      memory allocation success or failure
  
 *---------------------------------------------------------------------------*/
BOOL
NNSi_GfdAllocLnkVram
( 
    NNSiGfdLnkVramMan*      pMan, 
    NNSiGfdLnkVramBlock**   ppBlockPoolList,
    u32*                    pRetAddr,
    u32                     szByte
)
{
    return NNSi_GfdAllocLnkVramAligned( pMan, ppBlockPoolList, pRetAddr, szByte, 0 );
}

//------------------------------------------------------------------------------
// memory allocation with alignment that can be specified
// Open areas that occurred during alignment are newly registered in the free list.
// Take note that if a management information block does not exist for the open area, the memory allocation will fail.
BOOL
NNSi_GfdAllocLnkVramAligned
( 
    NNSiGfdLnkVramMan*      pMan, 
    NNSiGfdLnkVramBlock**   ppBlockPoolList,
    u32*                    pRetAddr,
    u32                     szByte,
    u32                     alignment
)
{
    NNS_GFD_NULL_ASSERT( pMan );
    NNS_GFD_NULL_ASSERT( pRetAddr );
    
    NNS_GFD_NON_ZERO_ASSERT( szByte );
    {
        //
        // Searches for blocks from the free list that meet the requirements.
        //
        u32     alignedAddr;
        u32     szNeeded;
        u32     difference;
        
        NNSiGfdLnkVramBlock* pBlkFound  = NULL;
        NNSiGfdLnkVramBlock* pBlk       = pMan->pFreeList;
        
        
        while( pBlk )
        {
            //
            // If necessary, calculates the address rounded up to the align boundary.
            //
            if( alignment > 1 )
            {
                alignedAddr = (u32)(  (pBlk->addr + (alignment - 1)) & ~(alignment - 1) );
                // Increased by rounded up portion only, which is the actual size needed.
                difference  = ( alignedAddr - pBlk->addr );
                szNeeded    = szByte + difference;
            }else{
                alignedAddr = pBlk->addr;
                difference  = 0;
                szNeeded    = szByte;
            }
            
            
            // Does the size satisfy the request?
            if( pBlk->szByte >= szNeeded )
            {
                pBlkFound = pBlk;
                break;
            }
            pBlk = pBlk->pBlkNext;
        }
        
        //
        // If a block in requirements was found...
        //
        if ( pBlkFound ) 
        {
            // The area that was moved as alignment is registered as a free block.
            if( difference > 0 )
            {    
                NNSiGfdLnkVramBlock*        pNewFreeBlk = GetNewBlock_( ppBlockPoolList );
                if( pNewFreeBlk )
                {
                    // registration
                    InitBlockFromPrams_( pNewFreeBlk, pBlkFound->addr, difference );
                    InsertBlock_( &pMan->pFreeList, pNewFreeBlk );
                }else{
                    // allocation failed
                    goto NG_CASE;
                }
            }
            
            // Update information for found free block.
            {
                // Subtracts used area.
                pBlkFound->szByte   -= szNeeded;
                // Allocates memory from front of block.
                pBlkFound->addr     += szNeeded; 
                
                // perfect size
                if( pBlkFound->szByte == 0 )
                {
                    // removes from free list
                    RemoveBlock_( &pMan->pFreeList, pBlkFound );
                    InsertBlock_( ppBlockPoolList, pBlkFound );
                }
            }
                    
            *pRetAddr = alignedAddr;
            return TRUE;
        }
        
NG_CASE:            
        //
        // Could not find a block that meets the requirements.
        //
        *pRetAddr = 0;
        return FALSE;
        
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNSi_GfdMergeAllFreeBlocks

  Description:  Scans and merges free blocks.
  
 *---------------------------------------------------------------------------*/
void NNSi_GfdMergeAllFreeBlocks( 
    NNSiGfdLnkVramMan*      pMan, 
    NNSiGfdLnkVramBlock**   ppBlockPoolList
)
{
    NNSiGfdLnkMemRegion         region;
    
    // the entire free list...
    NNSiGfdLnkVramBlock*        pCursor         = pMan->pFreeList;
    while( pCursor )
    {
        region.start    = pCursor->addr;
        region.end      = pCursor->addr + pCursor->szByte;
        
        // Check whether blocks can be merged.
        // if merged...
        if( TryToMergeBlockRegion_( pMan, ppBlockPoolList, &region ) )
        {
            //
            // Set the new size after an attempted merge.
            //
            pCursor->addr    = region.start;
            pCursor->szByte  = region.end - region.start;       
            
            // Rescan from the start of the list.
            pCursor = pMan->pFreeList;
        }else{
            pCursor = pCursor->pBlkNext;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNSi_GfdFreeLnkVram

  Description:  This function deallocates memory.
                Take note that if the management blocks are insufficient the deallocation may fail.
                (Failure occurs with generation of new free blocks.)
                
                
  Arguments:    pMan: Manager
                ppBlockPoolList: common block management information
                addr: deallocation address
                szByte: deallocation memory size
               
  Returns:      memory deallocation success or failure
  
 *---------------------------------------------------------------------------*/
BOOL NNSi_GfdFreeLnkVram
( 
    NNSiGfdLnkVramMan*      pMan, 
    NNSiGfdLnkVramBlock**   ppBlockPoolList,
    u32                     addr,
    u32                     szByte
)
{
    NNS_GFD_NULL_ASSERT( pMan );
    NNS_GFD_NON_ZERO_ASSERT( szByte );
    
    {
        
        //------------------------------------------------------------------------------
        // Incorporates an empty region into a free memory block.
        //      If it is adjacent to a free block, the free block is expanded.
        //      When neither adjacent to a free block nor having enough size for a free block, used as an alignment value for used blocks adjacent to the end.
        //      
        //      If there is no used block adjacent to the end of the empty region, the function fails.
        {
            NNSiGfdLnkMemRegion     region;
        
            region.start    = addr;
            region.end      = addr + szByte;
        
            NNS_GFD_NULL_ASSERT( pMan );
            NNS_GFD_NULL_ASSERT( ppBlockPoolList );
            {
                (void)TryToMergeBlockRegion_( pMan, ppBlockPoolList, &region );
                                
                //
                // Registers new free blocks.
                //
                {
                    NNSiGfdLnkVramBlock*        pNewFreeBlk = GetNewBlock_( ppBlockPoolList );
                    if( pNewFreeBlk == NULL )
                    {
                        // The data for the management area is insufficient.
                        // failed to deallocate
                        return FALSE;
                    }else{
                    
                        InitBlockFromRegion_( pNewFreeBlk, &region );
                        InsertBlock_( &pMan->pFreeList, pNewFreeBlk );
                    }
                }
                
                return TRUE;
            }
        }
    }
}
