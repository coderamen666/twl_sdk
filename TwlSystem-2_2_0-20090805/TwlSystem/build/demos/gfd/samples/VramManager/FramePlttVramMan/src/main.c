/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - gfd - samples - VramManager - FramePlttVramMan
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
// This sample uses the frame VRAM manager (texture palette).
// It allocates regions using a variety of patterns.
// 
#include <nitro.h>
#include <nnsys/gfd.h>


//   Variables used repeatedly inside the program
static NNSGfdFrmPlttVramState           state_;
static NNSGfdPlttKey                    mem;
static int                              vramLowerPtr,vramHigherPtr;

//  Match the VRAM higher and lower pointers with the manager pointers.
static BOOL checkVramPtr() {
     NNS_GfdGetFrmPlttVramState(&state_);
	return ((state_.address[0] == vramLowerPtr) && (state_.address[1] == vramHigherPtr));
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
    
    {
        const char* vramName[6] 
            = {"VRAM-F","VRAM-G","VRAM-F,G","VRAM-E","VRAM-E,F","VRAM-E,F,G"};
        const GXVRamTexPltt vramEnum[6] 
            = {GX_VRAM_TEXPLTT_0_F,GX_VRAM_TEXPLTT_0_G,GX_VRAM_TEXPLTT_01_FG,
               GX_VRAM_TEXPLTT_0123_E,GX_VRAM_TEXPLTT_01234_EF,GX_VRAM_TEXPLTT_012345_EFG};
        const int vramSize[6] 
            = {16 * 1024,16 * 1024,(16 + 16) * 1024,64 * 1024,(64 + 16) * 1024,(64 + 16 + 16) * 1024};
       
       
        //  Allocate all VRAM combination patterns to texture palettes and test them
        int i;
        
        OS_Printf( "----- Start tests for VRAM palette manager.\n -----" );
        
        
        for(i = 0;i < 6;i ++) 
        {
            //  Initial pointer settings
            vramLowerPtr = 0;
            vramHigherPtr = vramSize[i];
            
            OS_Printf("%s  checking\n",vramName[i]);
            GX_SetBankForTexPltt(vramEnum[i]);
            
            //
            // Initializes manager.
            //
            NNS_GfdInitFrmPlttVramManager( (u32)vramSize[i], TRUE );
            
            //   Allocate area not for 4-color palette from the lower address
            mem = NNS_GfdAllocPlttVram(0x100,FALSE,TRUE);
            vramLowerPtr += 0x100;
            NNS_GFD_ASSERT(checkVramPtr());
            NNS_GFD_ASSERT(NNS_GfdGetPlttKeyAddr(mem) == vramLowerPtr - 0x100);
            NNS_GFD_ASSERT(NNS_GfdGetPlttKeySize(mem) == 0x100);
            
            //  Allocate area not for 4-color palette from the higher address
            mem = NNS_GfdAllocPlttVram(0x100,FALSE,FALSE);
            vramHigherPtr -= 0x100;
            NNS_GFD_ASSERT(checkVramPtr());
            NNS_GFD_ASSERT(NNS_GfdGetPlttKeyAddr(mem) == vramHigherPtr);
            NNS_GFD_ASSERT(NNS_GfdGetPlttKeySize(mem) == 0x100);
            
            //  Allocate area not for 4-color palette from the lower address.
            mem = NNS_GfdAllocPlttVram(0x8,TRUE,TRUE);
            vramLowerPtr += 0x8;
            NNS_GFD_ASSERT(checkVramPtr());
            NNS_GFD_ASSERT(NNS_GfdGetPlttKeyAddr(mem) == vramLowerPtr - 0x8);
            NNS_GFD_ASSERT(NNS_GfdGetPlttKeySize(mem) == 0x8);
            //   Only 4-color palettes are 0x8 aligned, so check if they are correct
            mem = NNS_GfdAllocPlttVram(0x10,TRUE,TRUE);
            vramLowerPtr += 0x10;
            NNS_GFD_ASSERT(checkVramPtr());
            NNS_GFD_ASSERT(NNS_GfdGetPlttKeyAddr(mem) == vramLowerPtr - 0x10);
            NNS_GFD_ASSERT(NNS_GfdGetPlttKeySize(mem) == 0x10);
            mem = NNS_GfdAllocPlttVram(0x8,TRUE,TRUE);
            vramLowerPtr += 0x8;
            NNS_GFD_ASSERT(checkVramPtr());
            NNS_GFD_ASSERT(NNS_GfdGetPlttKeyAddr(mem) == vramLowerPtr - 0x8);
            NNS_GFD_ASSERT(NNS_GfdGetPlttKeySize(mem) == 0x8);
            mem = NNS_GfdAllocPlttVram(0x8,TRUE,TRUE);
            vramLowerPtr += 0x8;
            NNS_GFD_ASSERT(checkVramPtr());
            NNS_GFD_ASSERT(NNS_GfdGetPlttKeyAddr(mem) == vramLowerPtr - 0x8);
            NNS_GFD_ASSERT(NNS_GfdGetPlttKeySize(mem) == 0x8);
            
            // Manager status is output for debugging
            NNS_GfdDumpFrmPlttVramManager();
            
            //  Address is 0x****8, so allocate a texture area not for 4-color palette here
            //  Allocate from current address+0x8
            mem = NNS_GfdAllocPlttVram(0x10,FALSE,TRUE);
            vramLowerPtr += 0x10 + 0x8;
            NNS_GFD_ASSERT(checkVramPtr());
            NNS_GFD_ASSERT(NNS_GfdGetPlttKeyAddr(mem) == vramLowerPtr - 0x10);
            NNS_GFD_ASSERT(NNS_GfdGetPlttKeySize(mem) == 0x10);
            
            //  When VRAM exceeding 0x10000 in size has been assigned and all regions of that size or higher have not been allocated, four-color palette texture regions cannot be allocated from the top.
            //  
            //  
            if(vramSize[i] > 0x10000) {
                //
                // If you want to confirm failure of operation to secure memory, try running the commented out portion
                //
                //   Cannot allocate this
                //mem = NNS_GfdAllocPlttVram(0x8,TRUE,FALSE);
                //NNS_GFD_ASSERT(mem == NNS_GFD_ALLOC_ERROR_PLTTKEY);
                
                //   Allocate the entire area over 0x10000
                mem = NNS_GfdAllocPlttVram((u32)(vramSize[i] - 0x10000),FALSE,FALSE);
                vramHigherPtr -= vramSize[i] - 0x10000;
                NNS_GFD_ASSERT(checkVramPtr());
                NNS_GFD_ASSERT(NNS_GfdGetPlttKeyAddr(mem) == vramHigherPtr);
                NNS_GFD_ASSERT(NNS_GfdGetPlttKeySize(mem) == vramSize[i] - 0x10000);
            }
            
            //   Even if unable to allocate in the above if statement, it can be allocated here
            mem = NNS_GfdAllocPlttVram(0x8,TRUE,FALSE);
            vramHigherPtr -= 0x8;
            NNS_GFD_ASSERT(checkVramPtr());
            NNS_GFD_ASSERT(NNS_GfdGetPlttKeyAddr(mem) == vramHigherPtr);
            NNS_GFD_ASSERT(NNS_GfdGetPlttKeySize(mem) == 0x8);
            
            //   Same as with lower address
            //   Only 4-color palettes are 0x8 aligned, so check if they are correct
            mem = NNS_GfdAllocPlttVram(0x10,TRUE,FALSE);
            vramHigherPtr -= 0x10;
            NNS_GFD_ASSERT(checkVramPtr());
            NNS_GFD_ASSERT(NNS_GfdGetPlttKeyAddr(mem) == vramHigherPtr);
            NNS_GFD_ASSERT(NNS_GfdGetPlttKeySize(mem) == 0x10);
            mem = NNS_GfdAllocPlttVram(0x8,TRUE,FALSE);
            vramHigherPtr -= 0x8;
            NNS_GFD_ASSERT(checkVramPtr());
            NNS_GFD_ASSERT(NNS_GfdGetPlttKeyAddr(mem) == vramHigherPtr);
            NNS_GFD_ASSERT(NNS_GfdGetPlttKeySize(mem) == 0x8);
            mem = NNS_GfdAllocPlttVram(0x8,TRUE,FALSE);
            vramHigherPtr -= 0x8;
            NNS_GFD_ASSERT(checkVramPtr());
            NNS_GFD_ASSERT(NNS_GfdGetPlttKeyAddr(mem) == vramHigherPtr);
            NNS_GFD_ASSERT(NNS_GfdGetPlttKeySize(mem) == 0x8);
            //  Address is 0x****8, so allocate a texture area not for 4-color palette here
            //  Allocate from current address-0x8
            mem = NNS_GfdAllocPlttVram(0x10,FALSE,FALSE);
            vramHigherPtr -= 0x10 + 0x8;
            NNS_GFD_ASSERT(checkVramPtr());
            NNS_GFD_ASSERT(NNS_GfdGetPlttKeyAddr(mem) == vramHigherPtr);
            NNS_GFD_ASSERT(NNS_GfdGetPlttKeySize(mem) == 0x10);
            
            //
            // If you want to confirm failure of operation to secure memory, try running the commented out portion
            //
            //  An area larger than available capacity is requested, and the request is denied
            //mem = NNS_GfdAllocPlttVram(vramSize[i],TRUE,FALSE);
            //NNS_GFD_ASSERT(mem == NNS_GFD_ALLOC_ERROR_PLTTKEY);
            //mem = NNS_GfdAllocPlttVram(vramSize[i],FALSE,FALSE);
            //NNS_GFD_ASSERT(mem == NNS_GFD_ALLOC_ERROR_PLTTKEY);
            //mem = NNS_GfdAllocPlttVram(vramSize[i],TRUE,TRUE);
            //NNS_GFD_ASSERT(mem == NNS_GFD_ALLOC_ERROR_PLTTKEY);
            //mem = NNS_GfdAllocPlttVram(vramSize[i],FALSE,TRUE);
            //NNS_GFD_ASSERT(mem == NNS_GFD_ALLOC_ERROR_PLTTKEY);
            
            //  Save current state in state_
            NNS_GfdGetFrmPlttVramState(&state_);
            
            //  Allocate proper area for higher and lower sides
            mem = NNS_GfdAllocPlttVram(0x10,FALSE,FALSE);
            mem = NNS_GfdAllocPlttVram(0x18,TRUE,FALSE);
            mem = NNS_GfdAllocPlttVram(0x10,FALSE,TRUE);
            mem = NNS_GfdAllocPlttVram(0x18,TRUE,TRUE);
            
            //  Return the state recorded in state_ to the manager
            NNS_GfdSetFrmPlttVramState(&state_);
            NNS_GFD_ASSERT(checkVramPtr());
            
            
            {
                // Size zero
                mem = NNS_GfdAllocPlttVram(0x0,TRUE,FALSE);
                NNS_GFD_ASSERT( NNS_GfdGetPlttKeySize(mem) == NNS_GFD_PLTTSIZE_MIN );
                NNS_GfdSetFrmPlttVramState(&state_);
                // Smaller than NNS_GFD_PLTTSIZE_MIN
                mem = NNS_GfdAllocPlttVram(0x1,TRUE,FALSE);
                NNS_GFD_ASSERT( NNS_GfdGetPlttKeySize(mem) == NNS_GFD_PLTTSIZE_MIN );
                NNS_GfdSetFrmPlttVramState(&state_);
                // Exactly NNS_GFD_PLTTSIZE_MIN
                mem = NNS_GfdAllocPlttVram(NNS_GFD_PLTTSIZE_MIN,TRUE,FALSE);
                NNS_GFD_ASSERT( NNS_GfdGetPlttKeySize(mem) == NNS_GFD_PLTTSIZE_MIN );
                NNS_GfdSetFrmPlttVramState(&state_);
                // Slightly larger than NNS_GFD_PLTTSIZE_MIN
                mem = NNS_GfdAllocPlttVram(NNS_GFD_PLTTSIZE_MIN+1,TRUE,FALSE);
                NNS_GFD_ASSERT( NNS_GfdGetPlttKeySize(mem) == NNS_GFD_PLTTSIZE_MIN * 2);
                NNS_GfdSetFrmPlttVramState(&state_);
            }
            
            (void)GX_DisableBankForTex();
            
            OS_Printf("%s  OK\n",vramName[i]);
        }
        OS_Printf( "----- All tests finish.\n -----" );
    }
    
    while(1) {}
}

