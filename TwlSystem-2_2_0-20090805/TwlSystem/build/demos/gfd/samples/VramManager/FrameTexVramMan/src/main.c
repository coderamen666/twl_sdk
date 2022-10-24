/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - gfd - samples - VramManager - FrameTexVramMan
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
// This sample uses the frame VRAM manager (texture image).
// Performs allocation tests with various patterns.
// 
#include <nitro.h>
#include <nnsys/gfd.h>

//   Variables used repeatedly inside the program
static NNSGfdFrmTexVramState     state_;
static NNSGfdTexKey mem;

//  Available area in each region
//  Use to check if the value of available VRAM obtained with the library get function is the same
static int vramFreeSize[5];


//  Check the size of the available area of the region
static int calcFreeSize(int regionNo) 
{
    //
    // Get the state
    //
    NNS_GfdGetFrmTexVramState(&state_);
    
    {
        // Get the available capacity from the state
        const int size 
            = (int)state_.address[regionNo * 2 + 1] - (int)state_.address[regionNo * 2 + 0];
            
        return size;
    }
}


//   Test Slot 0 with a variety of patterns
void testSlot0();

//   Test Slots 0 and 1 with a variety of patterns.
//   When slots 2 and 3 exist, test it in a fully used state.
void testSlot01(int numSlot);

//   Test Slots 0, 1, 2 using various patterns.
//   When Slot 3 exists, test it in a fully used state.
void testSlot012(int numSlot);

//   Test Slots 0, 1, 2, 3 using various patterns.
void testSlot0123();


//   When slots 2 and 3 exist, use them fully.
static void exhaustSlot23(int numSlot) {
    if(numSlot == 3) {
        mem = NNS_GfdAllocTexVram(256 * 512,FALSE,0);
    }
    if(numSlot == 4) {
        mem = NNS_GfdAllocTexVram(256 * 512,FALSE,0);
        mem = NNS_GfdAllocTexVram(256 * 512,FALSE,0);
    }
}

//   When Slot 3 exists, use it fully.
static void exhaustSlot3(int numSlot) {
    if(numSlot == 4) {
        mem = NNS_GfdAllocTexVram(256 * 512,FALSE,0);
    }
}


//------------------------------------------------------------------------------
//   Test Slot 0 with a variety of patterns
//------------------------------------------------------------------------------
static void testSlot0() {

    //   Deallocate the area
    NNS_GfdResetFrmTexVramState();

    vramFreeSize[0] = 0x20000;
    
    //  Allocate a standard texture area
    
    //  First, allocate a 4-color palette texture area of min. size (8x8)
    mem = NNS_GfdAllocTexVram(8 * 8 / 4,FALSE,0);
    vramFreeSize[0] -= 8 * 8 / 4;
    NNS_GFD_ASSERT(calcFreeSize(0) == vramFreeSize[0]);
    //   Take from higher address
    NNS_GFD_ASSERT(NNS_GfdGetTexKeyAddr(mem) == vramFreeSize[0]);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeySize(mem) == 8 * 8 / 4);
    NNS_GFD_ASSERT(NNS_GfdGetTexKey4x4Flag(mem) == FALSE);
    
    //  Allocate some areas
    mem = NNS_GfdAllocTexVram(16 * 16 / 2,FALSE,0);
    vramFreeSize[0] -= 16 * 16 / 2;
    NNS_GFD_ASSERT(calcFreeSize(0) == vramFreeSize[0]);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeyAddr(mem) == vramFreeSize[0]);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeySize(mem) == 16 * 16 / 2);
    NNS_GFD_ASSERT(NNS_GfdGetTexKey4x4Flag(mem) == FALSE);
    
    mem = NNS_GfdAllocTexVram(32 * 16,FALSE,0);
    vramFreeSize[0] -= 32 * 16;
    NNS_GFD_ASSERT(calcFreeSize(0) == vramFreeSize[0]);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeyAddr(mem) == vramFreeSize[0]);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeySize(mem) == 32 * 16);
    NNS_GFD_ASSERT(NNS_GfdGetTexKey4x4Flag(mem) == FALSE);
    
    mem = NNS_GfdAllocTexVram(64 * 128,FALSE,0);
    vramFreeSize[0] -= 64 * 128;
    NNS_GFD_ASSERT(calcFreeSize(0) == vramFreeSize[0]);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeyAddr(mem) == vramFreeSize[0]);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeySize(mem) == 64 * 128);
    NNS_GFD_ASSERT(NNS_GfdGetTexKey4x4Flag(mem) == FALSE);
    
    //
    // If you want to confirm failure of operation to secure memory, try running the commented out portion
    //    
    //  Try requesting more than the available area
    //mem = NNS_GfdAllocTexVram(2048 * 2048,FALSE,0);
    //NNS_GFD_ASSERT(mem == NNS_GFD_ALLOC_ERROR_TEXKEY);
    
    //  Save current state in state_
    NNS_GfdGetFrmTexVramState(&state_);
    //  Allocate area as needed
    mem = NNS_GfdAllocTexVram(64 * 128,FALSE,0);
    //   Set the manager to the state stored in state_. Check if it has returned.
    NNS_GfdSetFrmTexVramState(&state_);
    NNS_GFD_ASSERT(calcFreeSize(0) == vramFreeSize[0]);
    
    //   Deallocate the area
    NNS_GfdResetFrmTexVramState();
    vramFreeSize[0] = 0x20000;
    NNS_GFD_ASSERT(calcFreeSize(0) == vramFreeSize[0]);
    
    //   Allocate area again
    mem = NNS_GfdAllocTexVram(64 * 256,FALSE,0);
    vramFreeSize[0] -= 64 * 256;
    NNS_GFD_ASSERT(calcFreeSize(0) == vramFreeSize[0]);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeyAddr(mem) == vramFreeSize[0]);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeySize(mem) == 64 * 256);
    NNS_GFD_ASSERT(NNS_GfdGetTexKey4x4Flag(mem) == FALSE);
    
    {
        // Allocate the size zero region
        // Allocated by rounding up to the smallest size that can be expressed with the texture key
        mem = NNS_GfdAllocTexVram( 0,FALSE,0);
        NNS_GFD_ASSERT( NNS_GfdGetTexKeySize(mem) == NNS_GFD_TEXSIZE_MIN );
        NNS_GfdResetFrmTexVramState();
        
        // Allocate the size smaller than the smallest size
        mem = NNS_GfdAllocTexVram( 1,FALSE,0);
        NNS_GFD_ASSERT( NNS_GfdGetTexKeySize(mem) == NNS_GFD_TEXSIZE_MIN );
        NNS_GfdResetFrmTexVramState();
        
        // Allocate the exact size of the smallest size
        mem = NNS_GfdAllocTexVram( NNS_GFD_TEXSIZE_MIN,FALSE,0);
        NNS_GFD_ASSERT( NNS_GfdGetTexKeySize(mem) == NNS_GFD_TEXSIZE_MIN );
        NNS_GfdResetFrmTexVramState();
        
        // Allocate the size slightly larger than the smallest size
        mem = NNS_GfdAllocTexVram( NNS_GFD_TEXSIZE_MIN + 1,FALSE,0);
        NNS_GFD_ASSERT( NNS_GfdGetTexKeySize(mem) == NNS_GFD_TEXSIZE_MIN * 2 );
        NNS_GfdResetFrmTexVramState();
    }
}


//------------------------------------------------------------------------------
//   Test Slots 0, 1 using various patterns
//   If Slots 2, 3 exist, utilize them fully and test
//------------------------------------------------------------------------------
static void testSlot01(int numSlot) 
{
    
    //
    // The search order for available space will change based on the number of manager slots. Note that the following test will only pass if there are more than two manager slots.
    // 
    //
    NNS_GFD_ASSERT( numSlot > 2 );

    //   Deallocate the area
    NNS_GfdResetFrmTexVramState();
    //  When slots 2 and 3 exist, use them fully.
    exhaustSlot23(numSlot);
    
    vramFreeSize[0] = 0x20000;
    vramFreeSize[1] = 0x10000;
    vramFreeSize[2] = 0x10000;
    
    //  Allocate a standard texture area
    mem = NNS_GfdAllocTexVram(256 * 128,FALSE,0);
    vramFreeSize[0] -= 256 * 128;
    NNS_GFD_ASSERT(calcFreeSize(0) == vramFreeSize[0]);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeyAddr(mem) == vramFreeSize[0]);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeySize(mem) == 256 * 128);
    NNS_GFD_ASSERT(NNS_GfdGetTexKey4x4Flag(mem) == FALSE);
    
    //  Try allocating area that is not sufficient for Slot 0 available area.
    //  (No area can be allocated for Slot 0. Slot 1 is allocated only.)
    mem = NNS_GfdAllocTexVram(256 * (256 + 32),FALSE,0);  // Use more than half of Slot 0
    mem = NNS_GfdAllocTexVram(256 * 256,FALSE,0);  // Use all of Slot 1 Region 2
    vramFreeSize[0] -= 256 * (256 + 32);
    vramFreeSize[2] -= 256 * 256;
    NNS_GFD_ASSERT(calcFreeSize(0) == vramFreeSize[0]);
    NNS_GFD_ASSERT(calcFreeSize(2) == vramFreeSize[2]);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeyAddr(mem) == 0x40000 - 256 * 256);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeySize(mem) == 256 * 256);
    NNS_GFD_ASSERT(NNS_GfdGetTexKey4x4Flag(mem) == FALSE);
    
    //   Slot 1 Region 1 is allocated for normal texture use
    mem = NNS_GfdAllocTexVram(256 * 128,FALSE,0);
    vramFreeSize[1] -= 256 * 128;
    NNS_GFD_ASSERT(calcFreeSize(1) == vramFreeSize[1]);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeyAddr(mem) == 0x30000 - 256 * 128);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeySize(mem) == 256 * 128);
    NNS_GFD_ASSERT(NNS_GfdGetTexKey4x4Flag(mem) == FALSE);
    
    //   Allocate area for 4x4 compressed texture
    //   Region 1 has palette indices for normal texture and compressed texture.
    mem = NNS_GfdAllocTexVram(8 * 8 / 4,TRUE,0);
    vramFreeSize[0] -= 8 * 8 / 4;
    vramFreeSize[1] -= 8 * 8 / (4 * 4) * 2;
    NNS_GFD_ASSERT(calcFreeSize(0) == vramFreeSize[0]);
    NNS_GFD_ASSERT(calcFreeSize(1) == vramFreeSize[1]);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeyAddr(mem) == 0);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeySize(mem) == 8 * 8 / 4);
    NNS_GFD_ASSERT(NNS_GfdGetTexKey4x4Flag(mem) == TRUE);
    
    //
    // If you want to confirm failure of operation to secure memory, try running the commented out portion
    //    
    //  Try requesting more than the available area
    //mem = NNS_GfdAllocTexVram(2048 * 2048,FALSE,0);
    //NNS_GFD_ASSERT(mem == NNS_GFD_ALLOC_ERROR_TEXKEY);
    
    //   Deallocate the area
    NNS_GfdResetFrmTexVramState();
    //  When slots 2 and 3 exist, use them fully.
    exhaustSlot23(numSlot);
    vramFreeSize[0] = 0x20000;
    vramFreeSize[1] = 0x10000;
    vramFreeSize[2] = 0x10000;
    NNS_GFD_ASSERT(calcFreeSize(0) == vramFreeSize[0]);
    NNS_GFD_ASSERT(calcFreeSize(1) == vramFreeSize[1]);
    NNS_GFD_ASSERT(calcFreeSize(2) == vramFreeSize[2]);
    
    //
    // If you want to confirm failure of operation to secure memory, try running the commented out portion
    //    
    //   Allocate area to use Slots 0, 1 fully (Cannot allocate this)
    //mem = NNS_GfdAllocTexVram(512 * 512,FALSE,0);
    //NNS_GFD_ASSERT(mem == NNS_GFD_ALLOC_ERROR_TEXKEY);
    
    //   Deallocate the area
    NNS_GfdResetFrmTexVramState();
    //  When slots 2 and 3 exist, use them fully.
    exhaustSlot23(numSlot);
    vramFreeSize[0] = 0x20000;
    vramFreeSize[1] = 0x10000;
    vramFreeSize[2] = 0x10000;
    
    //   Allocate area for 4x4 compressed texture
    mem = NNS_GfdAllocTexVram(8 * 8 / 4,TRUE,0);
    vramFreeSize[0] -= 8 * 8 / 4;
    vramFreeSize[1] -= 8 * 8 / (4 * 4) * 2;
    NNS_GFD_ASSERT(calcFreeSize(0) == vramFreeSize[0]);
    NNS_GFD_ASSERT(calcFreeSize(1) == vramFreeSize[1]);
    //   Unlike a standard-format texture, it is allocated from the lower address
    NNS_GFD_ASSERT(NNS_GfdGetTexKeyAddr(mem) == 0);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeySize(mem) == 8 * 8 / 4);
    NNS_GFD_ASSERT(NNS_GfdGetTexKey4x4Flag(mem) == TRUE);
    
    //    Insert the allocation of standard-format texture area in between
    mem = NNS_GfdAllocTexVram(64 * 256,FALSE,0);
    vramFreeSize[0] -= 64 * 256;
    NNS_GFD_ASSERT(calcFreeSize(0) == vramFreeSize[0]);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeyAddr(mem) == 0x20000 - 64 * 256);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeySize(mem) == 64 * 256);
    NNS_GFD_ASSERT(NNS_GfdGetTexKey4x4Flag(mem) == FALSE);
    
    //    Allocate area for 4x4 compressed texture again
    mem = NNS_GfdAllocTexVram(16 * 128 / 4,TRUE,0);
    vramFreeSize[0] -= 16 * 128 / 4;
    vramFreeSize[1] -= 16 * 128 / (4 * 4) * 2;
    NNS_GFD_ASSERT(calcFreeSize(0) == vramFreeSize[0]);
    NNS_GFD_ASSERT(calcFreeSize(1) == vramFreeSize[1]);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeyAddr(mem) == 8 * 8 / 4);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeySize(mem) == 16 * 128 / 4);
    NNS_GFD_ASSERT(NNS_GfdGetTexKey4x4Flag(mem) == TRUE);
    
    //    Use all of Slot 0
    mem = NNS_GfdAllocTexVram(0x20000 - 8 * 8 / 4 - 64 * 256 - 16 * 128 / 4,FALSE,0);
    vramFreeSize[0] = 0;
    
    //    Use all of Slot 1 Region 2
    mem = NNS_GfdAllocTexVram(256 * 256,FALSE,0);
    vramFreeSize[2] -= 256 * 256;
    
    //   Allocate area in Slot 1 Region 1
    mem = NNS_GfdAllocTexVram(128 * 256,FALSE,0);
    vramFreeSize[1] -= 128 * 256;
    NNS_GFD_ASSERT(calcFreeSize(1) == vramFreeSize[1]);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeyAddr(mem) == 0x30000 - 128 * 256);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeySize(mem) == 128 * 256);
    NNS_GFD_ASSERT(NNS_GfdGetTexKey4x4Flag(mem) == FALSE);
    
    //
    // If you want to confirm failure of operation to secure memory, try running the commented out portion
    //    
    //   Should not be able to allocate because of conflict with compressed texture index table allocated from lower address
    //mem = NNS_GfdAllocTexVram(128 * 256,FALSE,0);
    //NNS_GFD_ASSERT(NNS_GfdGetTexKeySize(mem) == 0);
    
    //  Save current state in state_
    NNS_GfdGetFrmTexVramState(&state_);
    //  Allocate area as needed
    mem = NNS_GfdAllocTexVram(8 * 8,FALSE,0);
    //   Set the manager to the state stored in state_. Check if it has returned.
    NNS_GfdSetFrmTexVramState(&state_);
    NNS_GFD_ASSERT(calcFreeSize(0) == vramFreeSize[0]);
    NNS_GFD_ASSERT(calcFreeSize(1) == vramFreeSize[1]);
    NNS_GFD_ASSERT(calcFreeSize(2) == vramFreeSize[2]);
    
}


//------------------------------------------------------------------------------
//   Test Slots 0, 1, 2 using various patterns
//   When Slot 3 exists, test it in a fully used state.
//------------------------------------------------------------------------------
static void testSlot012(int numSlot) {

    //   Deallocate the area
    NNS_GfdResetFrmTexVramState();
    //  When Slot 3 exists, use it fully.
    exhaustSlot3(numSlot);
    
    //   After doing Slot 2-related testing, test Slots 0, 1 using Slot 2 fully.
    
    vramFreeSize[0] = 0x20000;
    vramFreeSize[1] = 0x10000;
    vramFreeSize[2] = 0x10000;
    vramFreeSize[3] = 0x20000;
    
    //  Allocate standard texture area for Slot 2
    mem = NNS_GfdAllocTexVram(256 * 128,FALSE,0);
    vramFreeSize[3] -= 256 * 128;
    NNS_GFD_ASSERT(calcFreeSize(3) == vramFreeSize[3]);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeyAddr(mem) == 0x40000 + vramFreeSize[3]);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeySize(mem) == 256 * 128);
    NNS_GFD_ASSERT(NNS_GfdGetTexKey4x4Flag(mem) == FALSE);
    mem = NNS_GfdAllocTexVram(256 * (256 + 64),FALSE,0);
    vramFreeSize[3] -= 256 * (256 + 64);
    NNS_GFD_ASSERT(calcFreeSize(3) == vramFreeSize[3]);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeyAddr(mem) == 0x40000 + vramFreeSize[3]);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeySize(mem) == 256 * (256 + 64));
    NNS_GFD_ASSERT(NNS_GfdGetTexKey4x4Flag(mem) == FALSE);
    
    //   Use all of Slot 0
    mem = NNS_GfdAllocTexVram(256 * 512,FALSE,0);
    vramFreeSize[0] -= 256 * 512;
    NNS_GFD_ASSERT(calcFreeSize(0) == vramFreeSize[0]);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeyAddr(mem) == vramFreeSize[0]);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeySize(mem) == 256 * 512);
    NNS_GFD_ASSERT(NNS_GfdGetTexKey4x4Flag(mem) == FALSE);
    
    //   Since slot 2 uses 256 * (128 + 256 + 64), an attempt to allocate a 256 * 248 standard texture format will result in taking space from slot 1's region 2.
    //   
    mem = NNS_GfdAllocTexVram(256 * 248,FALSE,0);
    vramFreeSize[2] -= 256 * 248;
    NNS_GFD_ASSERT(calcFreeSize(2) == vramFreeSize[2]);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeyAddr(mem) == 0x30000 + vramFreeSize[2]);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeySize(mem) == 256 * 248);
    NNS_GFD_ASSERT(NNS_GfdGetTexKey4x4Flag(mem) == FALSE);
    
    //  Slot 2 has 256*64 free, Slot 0 is fully used
    //  Slot 1 Region 2 has 256*8 free
    
    //  This can be allocated
    mem = NNS_GfdAllocTexVram(8 * 8 / 4,TRUE,0);
    vramFreeSize[2] -= 8 * 8 / (4 * 4) * 2;
    vramFreeSize[3] -= 8 * 8 / 4;
    NNS_GFD_ASSERT(calcFreeSize(2) == vramFreeSize[2]);
    NNS_GFD_ASSERT(calcFreeSize(3) == vramFreeSize[3]);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeyAddr(mem) == 0x40000);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeySize(mem) == 8 * 8 / 4);
    NNS_GFD_ASSERT(NNS_GfdGetTexKey4x4Flag(mem) == TRUE);
    
    //   Slot 1 Region 2 is not sufficient and allocation fails
    //
    // If you want to confirm failure of operation to secure memory, try running the commented out portion
    //    
    //mem = NNS_GfdAllocTexVram(256 * 64 / 4,TRUE,0);
    //NNS_GFD_ASSERT(mem == NNS_GFD_ALLOC_ERROR_TEXKEY);
    
    //   If you try to allocate 256*128 region for standard texture format, it will be taken from Slot 1 Region 1
    mem = NNS_GfdAllocTexVram(256 * 128,FALSE,0);
    vramFreeSize[1] -= 256 * 128;
    NNS_GFD_ASSERT(calcFreeSize(1) == vramFreeSize[1]);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeyAddr(mem) == 0x20000 + vramFreeSize[1]);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeySize(mem) == 256 * 128);
    NNS_GFD_ASSERT(NNS_GfdGetTexKey4x4Flag(mem) == FALSE);
    
    //  Use Slot 2 fully and then test Slots 0, 1
    testSlot01(numSlot);
}


//------------------------------------------------------------------------------
//   Test Slots 0, 1, 2, 3 using various patterns.
//------------------------------------------------------------------------------
static void testSlot0123() {
  
    //   After doing Slot 3-related tests, use Slot 3 fully and test Slots 0, 1, 2.
    
    vramFreeSize[0] = 0x20000;
    vramFreeSize[1] = 0x10000;
    vramFreeSize[2] = 0x10000;
    vramFreeSize[3] = 0x20000;
    vramFreeSize[4] = 0x20000;
    
    //  Allocate standard texture area for Slot 3
    mem = NNS_GfdAllocTexVram(256 * 128,FALSE,0);
    vramFreeSize[4] -= 256 * 128;
    NNS_GFD_ASSERT(calcFreeSize(4) == vramFreeSize[4]);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeyAddr(mem) == 0x60000 + vramFreeSize[4]);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeySize(mem) == 256 * 128);
    NNS_GFD_ASSERT(NNS_GfdGetTexKey4x4Flag(mem) == FALSE);
    mem = NNS_GfdAllocTexVram(256 * (256 + 64),FALSE,0);
    vramFreeSize[4] -= 256 * (256 + 64);
    NNS_GFD_ASSERT(calcFreeSize(4) == vramFreeSize[4]);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeyAddr(mem) == 0x60000 + vramFreeSize[4]);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeySize(mem) == 256 * (256 + 64));
    NNS_GFD_ASSERT(NNS_GfdGetTexKey4x4Flag(mem) == FALSE);
    
    //   Use all of Slots 2, 0
    mem = NNS_GfdAllocTexVram(256 * 512,FALSE,0);
    vramFreeSize[3] -= 256 * 512;
    NNS_GFD_ASSERT(calcFreeSize(3) == vramFreeSize[3]);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeyAddr(mem) == 0x40000 + vramFreeSize[3]);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeySize(mem) == 256 * 512);
    NNS_GFD_ASSERT(NNS_GfdGetTexKey4x4Flag(mem) == FALSE);
    
    // Manager status is output for debugging
    NNS_GfdDumpFrmTexVramManager();
    
    mem = NNS_GfdAllocTexVram(256 * 512,FALSE,0);
    vramFreeSize[0] -= 256 * 512;
    NNS_GFD_ASSERT(calcFreeSize(0) == vramFreeSize[0]);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeyAddr(mem) == vramFreeSize[0]);
    NNS_GFD_ASSERT(NNS_GfdGetTexKeySize(mem) == 256 * 512);
    NNS_GFD_ASSERT(NNS_GfdGetTexKey4x4Flag(mem) == FALSE);
    
    //
    // If you want to confirm failure of operation to secure memory, try running the commented out portion
    //    
    //   Even if there is available space in Slot 3, allocation of 4x4 compressed texture area will fail
    //mem = NNS_GfdAllocTexVram(8 * 8 / 4,TRUE,0);
    //NNS_GFD_ASSERT(mem == NNS_GFD_ALLOC_ERROR_TEXKEY);
    
    //  Test Slots 0, 1, 2 using Slot 3 fully.
    testSlot012(4);
}


/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  The Main function of the sample
                
  Arguments:    None.
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NitroMain()
{
    OS_Init();
        
    //  Allocate 4 VRAMs (for Slots 0, 1, 2, 3)
    {
        const char* vramName[1] = {"VRAM-A,B,C,D"};
        const GXVRamTex vramEnum[1] = {GX_VRAM_TEX_0123_ABCD};
        int i;
        
        OS_Printf( "----- Start tests for VRAM texture manager.\n -----" );
        
        //  Allocate four sets in VRAM-A to D and test
        OS_Printf("\nallocate slot 0,1,2,3\n");
        for(i = 0;i < 1;i ++) {
            OS_Printf("%s  checking\n",vramName[i]);
            GX_SetBankForTex(vramEnum[i]);
            
            //
            // Initializes manager.
            //
            NNS_GfdInitFrmTexVramManager(4,TRUE);
            
            //  Test slots 0, 1, 2, 3
            testSlot0123();
            
            NNS_GfdResetFrmTexVramState();
            (void)GX_DisableBankForTex();
            
            OS_Printf("%s  OK\n",vramName[i]);
        }
        
        OS_Printf( "----- All tests finish.\n -----" );
    }
    
    while(1) {}
}



