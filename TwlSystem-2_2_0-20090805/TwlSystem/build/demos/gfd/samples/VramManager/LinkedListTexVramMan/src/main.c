/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - gfd - samples - VramManager - LinkedListTexVramMan
  File:     main.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
//
// Tests the operations of the Linked List VRAM manager's internal module.
// The internal module is shared by the Texture manager and the Palette manager.
// 
//
#include <nitro.h>

#include <nnsys/gfd.h>
#include "gfd_demolib.h"

//
// Manager initialization parameters
//
#define SIZE_VRAMMAN        0x80000     // Size of managed texture region
#define SIZE_VRAMMAN_4X4    0x21000     // Size of managed texture region (for 4x4 texture)
#define NUM_VRAMMAN_MEMBLK  20          // Number of managed blocks (the maximum value for the divided free region)
#define SIZE_PLTTMAN        0x18000     // Size of managed palette region

//
// Size of region to use for storage test
//
#define ALLOCATE_SIZE               0x4000
#define PLTT_ALLOC_SIZE_4           0x0008
#define PLTT_ALLOC_SIZE_16          0x0020
#define PLTT_ALLOC_SIZE_X           0x0023
#define PLTT_ALLOC_SIZE_4PLTTMAX    0x10000


#define VRAM_SLOT_SIZE    0x20000


#define TEX4X4_ENABLE           TRUE
#define TEX4X4_DISABLE          FALSE
#define PLTT4_TRUE              TRUE
#define PLTT4_FALSE             FALSE

/*---------------------------------------------------------------------------*
  Name:         TexAllocateTest_

  Description:  Initialization test of Texture manager
                Initialization is tested out under various conditions.
                If initialization does not proceed correctly, the internal manager assert should fail.
                
                
  Arguments:    None.
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
static void TexManInitTest_()
{
    const u32 szWork = NNS_GfdGetLnkTexVramManagerWorkSize( NUM_VRAMMAN_MEMBLK );
    void* pMgrWork   = GfDDemo_Allock( szWork );
    const u32 myMgrSize = 0x8000;
    u32 szNrm;
    
    NNS_GFD_NULL_ASSERT( pMgrWork );
    
    //
    // Initializes manager.
    //
    // 1 slot or less for normal use
    {
        szNrm = myMgrSize;
        // 0 slot for 4x4
        NNS_GfdInitLnkTexVramManager( szNrm,
                                      0,
                                      pMgrWork, szWork, TRUE );
    }
    
    // 1to 2 slots for normal use
    {
        szNrm = myMgrSize + VRAM_SLOT_SIZE;
        // 0 slot for 4x4
        NNS_GfdInitLnkTexVramManager( szNrm,
                                      0,
                                      pMgrWork, szWork, TRUE );
        
        // 1 slot or less for 4x4
        NNS_GfdInitLnkTexVramManager( szNrm,
                                      myMgrSize,
                                      pMgrWork, szWork, TRUE );
    }
    
    // 2 to 3 slots for normal use
    {
        szNrm  = myMgrSize + VRAM_SLOT_SIZE * 2;
        // 0 slot for 4x4
        NNS_GfdInitLnkTexVramManager( szNrm,
                                      0,
                                      pMgrWork, szWork, TRUE );
        // 1 slot or less for 4x4
        NNS_GfdInitLnkTexVramManager( szNrm,
                                      myMgrSize,
                                      pMgrWork, szWork, TRUE );
        // 2 slot or less for 4x4
        NNS_GfdInitLnkTexVramManager( szNrm,
                                      myMgrSize + VRAM_SLOT_SIZE,
                                      pMgrWork, szWork, TRUE );
    }        
    
    // 3 to 4 slots for normal use
    {
        szNrm  = myMgrSize + VRAM_SLOT_SIZE * 3;
        // 0 slot for 4x4
        NNS_GfdInitLnkTexVramManager( szNrm,
                                      0,
                                      pMgrWork, szWork, TRUE );
        // 1 slot or less for 4x4
        NNS_GfdInitLnkTexVramManager( szNrm,
                                      myMgrSize,
                                      pMgrWork, szWork, TRUE );
        // 2 slot or less for 4x4
        NNS_GfdInitLnkTexVramManager( szNrm,
                                      myMgrSize + VRAM_SLOT_SIZE,
                                      pMgrWork, szWork, TRUE );
    }
    
    // Examine boundary value parameter for other unchecked cases
    {
        
        NNS_GfdInitLnkTexVramManager( VRAM_SLOT_SIZE * 4,
                                      0,
                                      pMgrWork, szWork, TRUE );
        
        NNS_GfdInitLnkTexVramManager( VRAM_SLOT_SIZE * 4,
                                      VRAM_SLOT_SIZE * 2,
                                      pMgrWork, szWork, TRUE );
        
        NNS_GfdInitLnkTexVramManager( VRAM_SLOT_SIZE * 3,
                                      VRAM_SLOT_SIZE * 2,
                                      pMgrWork, szWork, TRUE );
    }
    
    GfDDemo_Free( pMgrWork );
}

/*---------------------------------------------------------------------------*
  Name:         TexAllocateTest_

  Description:  Tests allocation of Texture manager
                
  Arguments:    None.
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
static void TexAllocateTest_()
{
    const u32 szWork = NNS_GfdGetLnkTexVramManagerWorkSize( NUM_VRAMMAN_MEMBLK );
    void* pMgrWork   = GfDDemo_Allock( szWork );
    
    NNS_GFD_NULL_ASSERT( pMgrWork );
    
    //
    // Initializes manager.
    //
    NNS_GfdInitLnkTexVramManager( SIZE_VRAMMAN,
                                  SIZE_VRAMMAN_4X4,
                                  pMgrWork,
                                  szWork,
                                  TRUE );
                                  
    
    
    {
        NNSGfdTexKey texKey1, texKey2, texKey3;
    
        // Successfully allocated a region larger than the slot size.
        texKey1 = NNS_GfdAllocTexVram( VRAM_SLOT_SIZE + 0x100, TEX4X4_DISABLE, 0 );
        NNS_GFD_ASSERT( texKey1 != NNS_GFD_ALLOC_ERROR_TEXKEY );
        NNS_GFD_ASSERT( NNS_GfdFreeTexVram( texKey1 ) == 0 );
        
        
        // Allocate
        texKey1 = NNS_GfdAllocTexVram( ALLOCATE_SIZE, TEX4X4_ENABLE, 0 );
        texKey2 = NNS_GfdAllocTexVram( ALLOCATE_SIZE, TEX4X4_ENABLE, 0 );
        texKey3 = NNS_GfdAllocTexVram( ALLOCATE_SIZE, TEX4X4_ENABLE, 0 );
        
        
        //OS_Printf("addr = %x, size = %x\n", NNS_GfdGetTexKeyAddr(texKey1), NNS_GfdGetTexKeySize(texKey1) );
        //OS_Printf("addr = %x, size = %x\n", NNS_GfdGetTexKeyAddr(texKey2), NNS_GfdGetTexKeySize(texKey2) );
        //OS_Printf("addr = %x, size = %x\n", NNS_GfdGetTexKeyAddr(texKey3), NNS_GfdGetTexKeySize(texKey3) );
        
        
        NNS_GFD_ASSERT( NNS_GfdGetTexKeyAddr(texKey1) == ALLOCATE_SIZE * 0 &&  
                     NNS_GfdGetTexKeySize(texKey1) == ALLOCATE_SIZE );
        
        NNS_GFD_ASSERT( NNS_GfdGetTexKeyAddr(texKey2) == ALLOCATE_SIZE * 1 &&  
                     NNS_GfdGetTexKeySize(texKey2) == ALLOCATE_SIZE );
                     
        NNS_GFD_ASSERT( NNS_GfdGetTexKeyAddr(texKey3) == ALLOCATE_SIZE * 2 &&  
                     NNS_GfdGetTexKeySize(texKey3) == ALLOCATE_SIZE );
        
        // "Free" is successful
        NNS_GFD_ASSERT( NNS_GfdFreeTexVram( texKey2 ) == 0 );
        
        // Allocate again in same size
        texKey2 = NNS_GfdAllocTexVram( ALLOCATE_SIZE, TEX4X4_ENABLE, 0 );
        // The region just freed is allocated
        NNS_GFD_ASSERT( NNS_GfdGetTexKeyAddr(texKey2) == ALLOCATE_SIZE * 1 &&  
                     NNS_GfdGetTexKeySize(texKey2) == ALLOCATE_SIZE );
    
        // "Free" is successful
        NNS_GFD_ASSERT( NNS_GfdFreeTexVram( texKey1 ) == 0 );
        NNS_GFD_ASSERT( NNS_GfdFreeTexVram( texKey2 ) == 0 );
        NNS_GFD_ASSERT( NNS_GfdFreeTexVram( texKey3 ) == 0 );
        
        
        texKey1 = NNS_GfdAllocTexVram( ALLOCATE_SIZE * 2 , TEX4X4_ENABLE, 0 );
        NNS_GFD_ASSERT( NNS_GfdGetTexKeyAddr(texKey1) == ALLOCATE_SIZE * 0 &&  
                     NNS_GfdGetTexKeySize(texKey1) == ALLOCATE_SIZE * 2  );
        NNS_GFD_ASSERT( NNS_GfdFreeTexVram( texKey1 ) == 0 );
        
        
        // Make full use of all VRAM, fail on allocation
        // NNS_GFD_MAX_NUM_TEX_VRAM_SLOT       4
        // NNS_GFD_MAX_NUM_TEX4x4_VRAM_SLOT    2
        {
            // 4X4
            texKey1 = NNS_GfdAllocTexVram( 0x20000, TEX4X4_ENABLE, 0 );
            NNS_GFD_ASSERT( texKey1 != NNS_GFD_ALLOC_ERROR_TEXKEY );
            texKey2 = NNS_GfdAllocTexVram( 0x01000, TEX4X4_ENABLE, 0 );
            NNS_GFD_ASSERT( texKey2 != NNS_GFD_ALLOC_ERROR_TEXKEY );
            
            //
            // If you want to confirm failure of operation to secure memory, try running the commented out portion
            //
            // Make full use of all. Should fail.
            //texKey3 = NNS_GfdAllocTexVram( 0x1, TEX4X4_ENABLE, 0 );
            //NNS_GFD_ASSERT( texKey3 == NNS_GFD_ALLOC_ERROR_TEXKEY );
            
            // Normal
            texKey3 = NNS_GfdAllocTexVram( 0x20000, TEX4X4_DISABLE, 0 );
            NNS_GFD_ASSERT( texKey3 != NNS_GFD_ALLOC_ERROR_TEXKEY );
            
            
            // Internal status is output for debugging
            NNS_GfdDumpLnkTexVramManager();
        
            
            // Deallocate
            NNS_GFD_ASSERT( NNS_GfdFreeTexVram( texKey1 ) == 0 );
            NNS_GFD_ASSERT( NNS_GfdFreeTexVram( texKey2 ) == 0 );
            NNS_GFD_ASSERT( NNS_GfdFreeTexVram( texKey3 ) == 0 );
            
        
        }
        
        // Check the round up process of small size
        {
            NNSGfdTexKey    key;
            key = NNS_GfdAllocTexVram( 0, TEX4X4_ENABLE, 0 );
            NNS_GFD_ASSERT( NNS_GfdGetTexKeySize(key) == NNS_GFD_TEXSIZE_MIN  );
            NNS_GFD_ASSERT( NNS_GfdFreeTexVram( key ) == 0 );
            
            key = NNS_GfdAllocTexVram( 1, TEX4X4_ENABLE, 0 );
            NNS_GFD_ASSERT( NNS_GfdGetTexKeySize(key) == NNS_GFD_TEXSIZE_MIN  );
            NNS_GFD_ASSERT( NNS_GfdFreeTexVram( key ) == 0 );
            
            key = NNS_GfdAllocTexVram( NNS_GFD_TEXSIZE_MIN, TEX4X4_ENABLE, 0 );
            NNS_GFD_ASSERT( NNS_GfdGetTexKeySize(key) == NNS_GFD_TEXSIZE_MIN  );
            NNS_GFD_ASSERT( NNS_GfdFreeTexVram( key ) == 0 );
            
            key = NNS_GfdAllocTexVram( NNS_GFD_TEXSIZE_MIN + 1, TEX4X4_ENABLE, 0 );
            NNS_GFD_ASSERT( NNS_GfdGetTexKeySize(key) == NNS_GFD_TEXSIZE_MIN * 2 );
            NNS_GFD_ASSERT( NNS_GfdFreeTexVram( key ) == 0 );
        }
    }
    GfDDemo_Free( pMgrWork );
    
}

/*---------------------------------------------------------------------------*
  Name:         IsValidKey_

  Description:  Determines if palette key is key containing intended information.
                
  Arguments:    addr            Intended address
                size            Intended size
                
  Returns:      TRUE if palette key contains intended information.
  
 *---------------------------------------------------------------------------*/
static BOOL IsValidKey_( NNSGfdPlttKey key, u32 addr, u32 size )
{
    const u32 kaddr = NNS_GfdGetPlttKeyAddr( key );
    const u32 ksize = NNS_GfdGetPlttKeySize( key );
    
    return (BOOL)( kaddr == addr  &&  ksize == size );
}

/*---------------------------------------------------------------------------*
  Name:         PlttAllocateTest_

  Description:  Tests allocation of palette manager
                
  Arguments:    None.
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
static void PlttAllocateTest_()
{
    NNSGfdPlttKey     key1, key2, key3;
    
    const u32 szWork 
        = NNS_GfdGetLnkPlttVramManagerWorkSize( NUM_VRAMMAN_MEMBLK );
    void* pMgrWork   = GfDDemo_Allock( szWork );
    
    //
    // Initializes manager.
    //
    NNS_GfdInitLnkPlttVramManager( SIZE_PLTTMAN, 
                                   pMgrWork,
                                   szWork,
                                   TRUE );
    // Allocate, deallocate
    {
        key1 = NNS_GfdAllocPlttVram( PLTT_ALLOC_SIZE_16, PLTT4_FALSE, TRUE );
        NNS_GFD_ASSERT( IsValidKey_( key1, 0, PLTT_ALLOC_SIZE_16 ) );
        NNS_GFD_ASSERT( NNS_GfdFreePlttVram( key1 ) == 0 );
    }
    
    
    // Allocate - Deallocate - Reallocate ( [*][*][*] => [*][ ][*] => [*][*][*] )
    {
        key1 = NNS_GfdAllocPlttVram( PLTT_ALLOC_SIZE_16, PLTT4_FALSE, TRUE );
        key2 = NNS_GfdAllocPlttVram( PLTT_ALLOC_SIZE_16, PLTT4_FALSE, TRUE );
        key3 = NNS_GfdAllocPlttVram( PLTT_ALLOC_SIZE_16, PLTT4_FALSE, TRUE );
        
        NNS_GFD_ASSERT( IsValidKey_( key1,                  0    , PLTT_ALLOC_SIZE_16 ) );
        NNS_GFD_ASSERT( IsValidKey_( key2, PLTT_ALLOC_SIZE_16 * 1, PLTT_ALLOC_SIZE_16 ) );
        NNS_GFD_ASSERT( IsValidKey_( key3, PLTT_ALLOC_SIZE_16 * 2, PLTT_ALLOC_SIZE_16 ) );
        
        
        NNS_GFD_ASSERT( NNS_GfdFreePlttVram( key2 ) == 0 );
        key2 = NNS_GfdAllocPlttVram( PLTT_ALLOC_SIZE_16, PLTT4_FALSE, TRUE );
        NNS_GFD_ASSERT( IsValidKey_( key2, PLTT_ALLOC_SIZE_16 * 1, PLTT_ALLOC_SIZE_16 ) );
        
        
        NNS_GFD_ASSERT( NNS_GfdFreePlttVram( key1 ) == 0 );
        NNS_GFD_ASSERT( NNS_GfdFreePlttVram( key2 ) == 0 );
        NNS_GFD_ASSERT( NNS_GfdFreePlttVram( key3 ) == 0 );
    }
    
    // Try to fail by allocating the region more than 0x10000 with 4-color palette
    // Try to allocate the maximum and fail the allocation.
    {
        // Allocate 4-color palette in maximum size that can be referenced
        key1 = NNS_GfdAllocPlttVram( PLTT_ALLOC_SIZE_4PLTTMAX, PLTT4_TRUE, TRUE );
        NNS_GFD_ASSERT( IsValidKey_( key1, 0, PLTT_ALLOC_SIZE_4PLTTMAX ) );
        
        //
        // If you want to confirm failure of operation to secure memory, try running the commented out portion
        //
        // Should not be able to allocate any more 4-color palette
        // key2 = NNS_GfdAllocPlttVram( 0x7000, PLTT4_TRUE, TRUE );
        // NNS_GFD_ASSERT( key2 == NNS_GFD_ALLOC_ERROR_PLTTKEY );
        
        // However, should still be able to allocate normal palette
        key2 = NNS_GfdAllocPlttVram( 0x7000, PLTT4_FALSE, TRUE );
        NNS_GFD_ASSERT( IsValidKey_( key2, PLTT_ALLOC_SIZE_4PLTTMAX, 0x7000 ) );
        NNS_GFD_ASSERT( key2 != NNS_GFD_ALLOC_ERROR_PLTTKEY );
        
        //
        // If you want to confirm failure of operation to secure memory, try running the commented out portion
        //
        // This time, failure occurs due to lack of available space
        // key3 = NNS_GfdAllocPlttVram( 0x7000, PLTT4_FALSE, TRUE );
        // NNS_GFD_ASSERT( key3 == NNS_GFD_ALLOC_ERROR_PLTTKEY );
        
        // Internal status is output for debugging
        NNS_GfdDumpLnkPlttVramManager();
            
        NNS_GFD_ASSERT( NNS_GfdFreePlttVram( key1 ) == 0 );
        NNS_GFD_ASSERT( NNS_GfdFreePlttVram( key2 ) == 0 );
    }
    
    
    
    // Is 4-color palette aligned correctly?
    //
    {
        // Allocate one 4-color palette
        // => Allocate 16-color palette (For alignment, an 8-byte free block is generated)
        // => Allocate 4-color palette (This should make use of 8-byte free block)
        key1 = NNS_GfdAllocPlttVram( PLTT_ALLOC_SIZE_4, PLTT4_TRUE, TRUE );
        key2 = NNS_GfdAllocPlttVram( PLTT_ALLOC_SIZE_16, PLTT4_FALSE, TRUE );
        key3 = NNS_GfdAllocPlttVram( PLTT_ALLOC_SIZE_4, PLTT4_TRUE, TRUE );
        
        NNS_GFD_ASSERT( IsValidKey_( key1, 0, PLTT_ALLOC_SIZE_4 ) );
        // With padding, should not be addr == PLTT_ALLOC_SIZE_4
        NNS_GFD_ASSERT( IsValidKey_( key2, 0x10, PLTT_ALLOC_SIZE_16 ) );
        // The free block generated during padding should be used.
        NNS_GFD_ASSERT( IsValidKey_( key3, PLTT_ALLOC_SIZE_4, PLTT_ALLOC_SIZE_4 ) );
        
        NNS_GFD_ASSERT( NNS_GfdFreePlttVram( key1 ) == 0 );
        NNS_GFD_ASSERT( NNS_GfdFreePlttVram( key2 ) == 0 );
        NNS_GFD_ASSERT( NNS_GfdFreePlttVram( key3 ) == 0 );
        
        
        // Allocate two 4-color palettes
        // => Allocate 16-color palette (Aligned, so padding can be avoided!)
        // => Allocate 4-color palette (The allocated address should be different!)
        key1 = NNS_GfdAllocPlttVram( PLTT_ALLOC_SIZE_4 * 2, PLTT4_TRUE, TRUE );
        key2 = NNS_GfdAllocPlttVram( PLTT_ALLOC_SIZE_16, PLTT4_FALSE, TRUE );
        
        NNS_GFD_ASSERT( IsValidKey_( key1, 0, PLTT_ALLOC_SIZE_4 * 2) );
        // Padding should not be performed.
        NNS_GFD_ASSERT( IsValidKey_( key2, PLTT_ALLOC_SIZE_4 * 2, PLTT_ALLOC_SIZE_16 ) );
        
        NNS_GFD_ASSERT( NNS_GfdFreePlttVram( key1 ) == 0 );
        NNS_GFD_ASSERT( NNS_GfdFreePlttVram( key2 ) == 0 );
    }
    
    // Check the round up process of small size
    {
        NNSGfdPlttKey    key;
        key = NNS_GfdAllocPlttVram( 0, PLTT4_TRUE, TRUE );
        NNS_GFD_ASSERT( NNS_GfdGetPlttKeySize(key) == NNS_GFD_PLTTSIZE_MIN  );
        NNS_GFD_ASSERT( NNS_GfdFreePlttVram( key ) == 0 );
        
        key = NNS_GfdAllocPlttVram( 1, PLTT4_TRUE, TRUE );
        NNS_GFD_ASSERT( NNS_GfdGetPlttKeySize(key) == NNS_GFD_PLTTSIZE_MIN  );
        NNS_GFD_ASSERT( NNS_GfdFreePlttVram( key ) == 0 );
        
        key = NNS_GfdAllocPlttVram( NNS_GFD_PLTTSIZE_MIN, PLTT4_TRUE, TRUE );
        NNS_GFD_ASSERT( NNS_GfdGetPlttKeySize(key) == NNS_GFD_PLTTSIZE_MIN  );
        NNS_GFD_ASSERT( NNS_GfdFreePlttVram( key ) == 0 );
        
        key = NNS_GfdAllocPlttVram( NNS_GFD_PLTTSIZE_MIN+1, PLTT4_TRUE, TRUE );
        NNS_GFD_ASSERT( NNS_GfdGetPlttKeySize(key) == NNS_GFD_PLTTSIZE_MIN * 2 );
        NNS_GFD_ASSERT( NNS_GfdFreePlttVram( key ) == 0 );
    }
    
    GfDDemo_Free( pMgrWork );
}

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  The Main function of the sample
                
  Arguments:    None.
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NitroMain()
{
    GFDDemo_CommonInit();
    
    //
    // Test of basic function group
    //
    {
        OS_Printf( "----- Start tests for VRAM texture manager.\n -----" );
            TexManInitTest_();
            TexAllocateTest_();
            PlttAllocateTest_();
            
        OS_Printf( "----- All tests finish.\n -----" );
    }
    
    //
    // endless loop
    //
    while(TRUE){}
}

