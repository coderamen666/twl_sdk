/*---------------------------------------------------------------------------*
  Project:  TwlSDK - EL - demos - simple-2
  File:     main.c

  Copyright 2007-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-09-24#$
  $Rev: 11063 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <twl/el.h>

#include "DEMO.h"
#include "dllA.h"
#include "dllB.h"

/*---------------------------------------------------------------------------*
    static variables
 *---------------------------------------------------------------------------*/
static u32* lib_bufA;
static u32* lib_bufB;

typedef void (*global_func_p)( void);
global_func_p   global_funcA;
global_func_p   global_funcB;

/*---------------------------------------------------------------------------*
  Name:         MY_Alloc

  Description:  The user's memory allocation function passed to the EL library.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void *MY_Alloc(size_t size)
{
    void* heap;
    heap = OS_Alloc( size);
    if( heap == NULL) { OS_TPanic( "OS_Alloc failed.\n");}
    return( heap);
}

/*---------------------------------------------------------------------------*
  Name:         MY_Free

  Description:  The user's memory deallocation function passed to the EL library.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MY_Free(void *ptr)
{
    OS_Free( ptr);
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  V-Blank interrupt handler

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void)
{
    //---- check interrupt flag
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  Main

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TwlMain(void)
{
    ELDlld my_dlldA, my_dlldB;

    /* OS initialization */
    OS_EnableMainExArena();
    OS_Init();
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();
    (void)GX_VBlankIntr(TRUE);

    // When in NITRO mode, stopped by Panic
    DEMOCheckRunOnTWL();

    {
        void   *tempLo;
        OSHeapHandle hh;

        // Based on the premise that OS_Init has been already called
        tempLo = OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1);
        OS_SetArenaLo(OS_ARENA_MAIN, tempLo);
        hh = OS_CreateHeap(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi());
        if (hh < 0)
        {
            OS_Panic("ARM9: Fail to create heap...\n");
        }
        hh = OS_SetCurrentHeap(OS_ARENA_MAIN, hh);
    }


    OS_TPrintf("\n");
    OS_TPrintf("===================================\n");
    OS_TPrintf("EL Library test\n");
    OS_TPrintf("===================================\n");

    /* Start testing the EL library */
    {
        FS_Init( FS_DMA_NOT_USE );
        
        if( EL_Init( MY_Alloc, MY_Free) < 0)
        {
            OS_TPanic( "EL_Init failed.\n");
        }
        else
        {
            OS_TPrintf( "EL_Init success.\n");
        }

        // Dynamic modules are in the ROM
        lib_bufA = MY_Alloc(8192);
        if(lib_bufA == 0)
            OS_Panic("failed alloc\n");
        my_dlldA = EL_LinkFileEx( "rom:/data/dllA.a", lib_bufA, 8192);
        if(my_dlldA == 0)
            OS_Panic("failed EL_LinkFileEx(dllA.a)\n");
        
        OS_TPrintf( "dll loaded 0x%x - 0x%x\n", (u32)lib_bufA, (u32)lib_bufA + EL_GetLibSize( my_dlldA));

        OS_TPrintf( "object resolved flag = %d\n", EL_IsResolved( my_dlldA));

        // Next, link dllB
#ifndef SDK_FINALROM // Because this demo uses the main memory expanded arena, it will not work in FINALROM mode
        lib_bufB = (u32*)OS_AllocFromMainExArenaLo(8192, 32);
#else
        lib_bufB = MY_Alloc(8192);
#endif // SDK_FINALROM
        my_dlldB = EL_LinkFileEx( "rom:/data/dllB.a", lib_bufB, 8192);
        if(my_dlldB == 0)
            OS_Panic("failed EL_LinkFileEx(dllB.a)\n");
        
        OS_TPrintf( "dll loaded 0x%x - 0x%x\n", (u32)lib_bufB, (u32)lib_bufB + EL_GetLibSize( my_dlldB));

        OS_TPrintf( "object resolved flag = %d\n", EL_IsResolved( my_dlldB));

        /* Export the static-side symbols */
        EL_AddStaticSym();

        /* Resolve the DLL symbols */
        (void)EL_ResolveAll();

        OS_TPrintf( "object resolved flag = %d\n", EL_IsResolved( my_dlldA));
        OS_TPrintf( "object resolved flag = %d\n", EL_IsResolved( my_dlldB));
        if(!EL_IsResolved(my_dlldA))
            OS_Panic("failed EL_IsResolved(my_dlldA)\n");
        if(!EL_IsResolved(my_dlldB))
            OS_Panic("failed EL_IsResolved(my_dlldB)\n");

        OS_TPrintf( "LINK : dynamic\n");
        global_funcA = (global_func_p)EL_GetGlobalAdr( my_dlldA, "global_func_A\0");
        OS_TPrintf( "global_func_A : 0x%x\n", global_funcA);
        global_funcB = (global_func_p)EL_GetGlobalAdr( my_dlldB, "global_func_B\0");
        OS_TPrintf( "global_func_B : 0x%x\n", global_funcB);
        if(global_funcA == 0)
            OS_Panic("failed EL_GetGlobalAdr(my_dlldA)\n");
        if(global_funcB == 0)
            OS_Panic("failed EL_GetGlobalAdr(my_dlldB)\n");
        
        OS_TPrintf( "----- global_func_A execution -----\n");
        (*global_funcA)();
        OS_TPrintf( "----- global_func_B execution -----\n");
        (*global_funcB)();
        
        OS_TPrintf( "----- dll execution end -----\n");

        /* Unlink */
        OS_TPrintf("EL_Unlink ... dllA\n");
        (void)EL_Unlink( my_dlldA);
        OS_TPrintf( "EL_Unlink success.\n");
        
        OS_TPrintf( "object resolved flag = %d\n", EL_IsResolved( my_dlldA));
        OS_TPrintf( "object resolved flag = %d\n", EL_IsResolved( my_dlldB));
        
    }

    OS_TPrintf("\n");
    OS_TPrintf("===================================\n");
    OS_TPrintf("Finish\n");
    OS_TPrintf("===================================\n");
    OS_Terminate();
}

/*====== End of main.c ======*/
