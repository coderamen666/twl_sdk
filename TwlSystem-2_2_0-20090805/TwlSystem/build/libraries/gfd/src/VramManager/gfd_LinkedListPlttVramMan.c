/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - gfd - src - VramManager
  File:     gfd_LinkedListPlttVramMan.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
#include <nnsys/gfd/gfd_common.h>
#include <nnsys/gfd/VramManager/gfd_LinkedListPlttVramMan.h>
#include "gfdi_LinkedListVramMan_Common.h"

#define GFD_SLOT_SIZE        0x18000

#define NNS_GFD_BARPLTT_FREE_ERROR_INVALID_SIZE 1

//
// Manager
//
typedef struct NNS_GfdLnkPlttVramManager
{
    NNSiGfdLnkVramMan       mgr;
    NNSiGfdLnkVramBlock*    pBlockPoolList;
    
    //
    // member to use when reset
    //
    u32                     szByte;
    NNSiGfdLnkVramBlock*    pWorkHead;
    u32                     szByteManagementWork;

}NNS_GfdLnkPlttVramManager;

static NNS_GfdLnkPlttVramManager         mgr_;


/*---------------------------------------------------------------------------*
  Name:         NNS_GfdDumpLnkPlttVramManager

  Description:  Manager internal status is output for debugging.

  Arguments:    None.            
  Returns:      None.

 *---------------------------------------------------------------------------*/
void NNS_GfdDumpLnkPlttVramManager()
{
    OS_Printf("=== NNS_Gfd LnkPlttVramManager Dump ===\n");
    OS_Printf("   address:        size    \n");   // header line
    OS_Printf("=======================================\n");
    //
    // Display the entire free list of ordinary textures and compute the total usage amount.
    //
    OS_Printf("------ Free Blocks                -----\n");   
    
    NNSi_GfdDumpLnkVramManFreeListInfo( mgr_.mgr.pFreeList, mgr_.szByte );
    
    OS_Printf("=======================================\n");   
}

/*---------------------------------------------------------------------------*
  Name:         NNS_GfdDumpLnkPlttVramManagerEx

  Description:  
                Specifies a debug output function and the manager's internal status is output for debugging.

  Arguments:    pFunc               debug output processing function
                pUserData           data that is passed to the debug output processing function as an argument
                
  Returns:      None.

 *---------------------------------------------------------------------------*/

void NNS_GfdDumpLnkPlttVramManagerEx( 
    NNSGfdLnkDumpCallBack   pFunc, 
    void*                   pUserData )
{
    NNS_GFD_NULL_ASSERT( pFunc );
    NNSi_GfdDumpLnkVramManFreeListInfoEx( mgr_.mgr.pFreeList, pFunc, pUserData );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_GfdGetLnkPlttVramManagerWorkSize

  Description:  Gets the byte size of the memory required by VRAM manager for management information.
                Use this function's return value as a manager initialization parameter to intialize the manager region. 
                


                
                
  Arguments:    numMemBlk   number of blocks to be allocated; becomes the maximum amount of segmentation for managing empty regions 

                
  Returns:      byte size required for managing regions

 *---------------------------------------------------------------------------*/
u32 NNS_GfdGetLnkPlttVramManagerWorkSize( u32 numMemBlk )
{
    return numMemBlk * sizeof( NNSiGfdLnkVramBlock );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_GfdInitLnkPlttVramManager

  Description:  Initializes the frame palette VRAM manager.
                The frame palette VRAM manager is initialized to manage a region whose size runs from the start of palette RAM to the size specified in szByte. 
                
                
                
  Arguments:    szByte                  byte size of the region to manage (maximum: 0x18000)
                pManagementWork         pointer to the memory region used as management information 
                szByteManagementWork    size of the management information region 
                useAsDefault            whether to use the linked list palette VRAM manager as a current manager 

                
  Returns:      None.

 *---------------------------------------------------------------------------*/
void NNS_GfdInitLnkPlttVramManager
( 
    u32     szByte,
    void*   pManagementWork, 
    u32     szByteManagementWork,
    BOOL    useAsDefault
)
{
    NNS_GFD_ASSERT( szByte <= GFD_SLOT_SIZE );
    NNS_GFD_NULL_ASSERT( pManagementWork );
    NNS_GFD_ASSERT( szByteManagementWork != 0 );
    
    {
        //
        mgr_.szByte                 = szByte;
        mgr_.pWorkHead              = pManagementWork;
        mgr_.szByteManagementWork   = szByteManagementWork;
        
        NNS_GfdResetLnkPlttVramState();
        
        //
        // use as a default allocator
        //
        if( useAsDefault )
        {
            NNS_GfdDefaultFuncAllocPlttVram = NNS_GfdAllocLnkPlttVram;
            NNS_GfdDefaultFuncFreePlttVram  = NNS_GfdFreeLnkPlttVram;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_GfdAllocLnkPlttVram

  Description:  Allocates memory for palette in the linked list palette VRAM manager.
                
  Arguments:    szByte:     byte size of the region you want to allocate 
                b4Pltt:     whether a 4-color palette; TRUE if it is. 
                opt:        option; no longer used 
                
  Returns:      Returns a palette key that shows the allocated VRAM region.
                If allocation fails, returns NNS_GFD_ALLOC_ERROR_PLTTKEY, the key that indicates the error.
                

 *---------------------------------------------------------------------------*/
NNSGfdPlttKey    NNS_GfdAllocLnkPlttVram( u32 szByte, BOOL b4Pltt, u32 opt )
{
#pragma unused(opt)
    u32     addr;
    BOOL    result;
    
    {
        //
        // If a size is allocated that is too small to be displayed with a palette key, this rounds the size up.
        //
        szByte = NNSi_GfdGetPlttKeyRoundupSize( szByte );
        //
        // If a size is allocated that is too large for the palette key to display, this returns an error key.
        //
        if( szByte >= NNS_GFD_PLTTSIZE_MAX )
        {
            NNS_GFD_WARNING("Allocation size is too big. : NNS_GfdAllocLnkPlttVram()");
            return NNS_GFD_ALLOC_ERROR_PLTTKEY;
        }
        
        NNS_GFD_MINMAX_ASSERT( szByte, NNS_GFD_PLTTSIZE_MIN, NNS_GFD_PLTTSIZE_MAX );
    }
    
    //
    // Sets the alignment in the palette format.
    //
    if( b4Pltt )
    {
        result = NNSi_GfdAllocLnkVramAligned( &mgr_.mgr, 
                                              &mgr_.pBlockPoolList, 
                                              &addr, 
                                              szByte, 
                                              0x08 );
        // Check for a region that cannot be referenced.
        if( addr + szByte > NNS_GFD_4PLTT_MAX_ADDR )
        {
            // NG
            if( !NNSi_GfdFreeLnkVram( &mgr_.mgr, 
                                      &mgr_.pBlockPoolList, 
                                      addr, 
                                      szByte ) )
            {
                // Displays warning
            }
                                 
            return NNS_GFD_ALLOC_ERROR_PLTTKEY;
        }
    }else{
        result = NNSi_GfdAllocLnkVramAligned( &mgr_.mgr, 
                                              &mgr_.pBlockPoolList, 
                                              &addr, 
                                              szByte, 
                                              0x10 );
    }
    
    if( result )
    {
        return NNS_GfdMakePlttKey( addr, szByte );    
    }else{
        NNS_GFD_WARNING("failure in Vram Allocation. : NNS_GfdAllocLnkPlttVram()");
        return NNS_GFD_ALLOC_ERROR_PLTTKEY;
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_GfdFreeLnkPlttVram

  Description:  Deallocates the allocated memory for the palette in the linked list palette VRAM manager.
                
                
  Arguments:    plttKey          :    palette key representing the VRAM region to be deallocated
                
  Returns:      Returns whether the deallocation succeeded.
                If the deallocation was successful, 0 is returned.
                Returns 1 when a palette key indicating an invalid size, such as zero, is deallocated.
                

 *---------------------------------------------------------------------------*/
int             NNS_GfdFreeLnkPlttVram( NNSGfdPlttKey plttKey )
{
    
    const u32   addr     = NNS_GfdGetPlttKeyAddr( plttKey );
    const u32   szByte   = NNS_GfdGetPlttKeySize( plttKey );
    
    const BOOL  result   = NNSi_GfdFreeLnkVram( &mgr_.mgr, 
                                                &mgr_.pBlockPoolList, 
                                                addr, 
                                                szByte );
    
    if( result ) 
    {
        return 0;
    }else{
        return NNS_GFD_BARPLTT_FREE_ERROR_INVALID_SIZE;
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_GfdResetLnkPlttVramState

  Description:  Resets the manager to the initial state.
                
  Arguments:    None.
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void            NNS_GfdResetLnkPlttVramState( void )
{
    
    //
    // initialize the shared management block
    //
    mgr_.pBlockPoolList 
        = NNSi_GfdInitLnkVramBlockPool( 
            (NNSiGfdLnkVramBlock*)mgr_.pWorkHead, 
            mgr_.szByteManagementWork / sizeof( NNSiGfdLnkVramBlock ) );
    
    //
    // initialize free list
    //
    {
        BOOL    result;
        NNSi_GfdInitLnkVramMan( &mgr_.mgr );
        
        result = NNSi_GfdAddNewFreeBlock( &mgr_.mgr, 
                                          &mgr_.pBlockPoolList,
                                          0,
                                          mgr_.szByte );
        NNS_GFD_ASSERT( result );        
    }
    
    //
    // merge the free list
    // 
    NNSi_GfdMergeAllFreeBlocks( &mgr_.mgr, &mgr_.pBlockPoolList );
    
}

