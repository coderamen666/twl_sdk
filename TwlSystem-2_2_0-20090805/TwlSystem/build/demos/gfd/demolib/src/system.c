/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - gfd - demolib
  File:     system.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
#include "gfd_demolib/system.h"


#define SYSTEM_HEAP_SIZE        64*1024

#define GFDDEMO_ROUND_UP(value, alignment) \
    (((u32)(value) + (alignment-1)) & ~(alignment-1))

#define GFDDEMO_ROUND_DOWN(value, alignment) \
    ((u32)(value) & ~(alignment-1))

static NNSFndHeapHandle gfdDemoAppHeap;     // Application heap (Use expanded heap)


/*---------------------------------------------------------------------------*
    Initialization-related.
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
  Name:         VBlankIntr_

  Description:  V-Blank callback routine.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
VBlankIntr_(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         GFDDemo_CommonInit

  Description:  Initializes the OS, graphics engine, heap, and file system.
                

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void GFDDemo_CommonInit(void)
{
    OS_Init();
    FX_Init();
    GX_SetPower(GX_POWER_ALL);
    GX_Init();
    OS_InitTick();

    GX_DispOff();
    GXS_DispOff();

    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr_ );

    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrqMask(OS_IE_FIFO_RECV);
    (void)OS_EnableIrq();

    (void)GX_VBlankIntr(TRUE);         // To generate V-Blank interrupt request
    
    {
        void* sysHeapMemory = OS_AllocFromMainArenaLo(SYSTEM_HEAP_SIZE, 16);
        u32   arenaLow      = GFDDEMO_ROUND_UP  (OS_GetMainArenaLo(), 16);
        u32   arenaHigh     = GFDDEMO_ROUND_DOWN(OS_GetMainArenaHi(), 16);
        u32   appHeapSize   = arenaHigh - arenaLow;
        void* appHeapMemory = OS_AllocFromMainArenaLo(appHeapSize, 16);
        gfdDemoAppHeap      = NNS_FndCreateExpHeap(appHeapMemory, appHeapSize);
    }
}


void* GfDDemo_Allock( u32 size )
{
    return NNS_FndAllocFromExpHeapEx( gfdDemoAppHeap, size, 16);
}

void GfDDemo_Free( void* pBlk )
{
    NNS_FndFreeToExpHeap( gfdDemoAppHeap, pBlk );
}

