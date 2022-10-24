/*---------------------------------------------------------------------------*
  Project:  TwlSDK - demos - math - qsort
  File:     main.c

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include    <nitro.h>

/*---------------------------------------------------------------------------*
    Prototype definitions
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void);
static void KeyInit(void);
static void KeyRead(void);


// Key States
static struct
{
    u16     con;
    u16     trg;
}
keys;

#define DATA_NUM    1024

/*---------------------------------------------------------------------------*
    Static variable definitions
 *---------------------------------------------------------------------------*/
static u32 gDataArray[DATA_NUM];
static u8 *gSortBuf;


/*---------------------------------------------------------------------------*
  Name:         KeyInit

  Description:  Initialize Pad Keys.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void KeyInit(void)
{

    keys.trg = 0;
    keys.con = 0;

}

/*---------------------------------------------------------------------------*
  Name:         KeyRead

  Description:  Read Pad Keys.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void KeyRead(void)
{
    u16     r = PAD_Read();

    keys.trg = (u16)(~keys.con & r);
    keys.con = r;
}


/*---------------------------------------------------------------------------*
  Name:         InitializeAllocateSystem

  Description:  Initializes the memory allocation system within the main memory arena.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitializeAllocateSystem(void)
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



/*---------------------------------------------------------------------------*
  Name:         DisplayInit

  Description:  Graphics Initialization

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DisplayInit()
{

    GX_Init();
    FX_Init();

    GX_DispOff();
    GXS_DispOff();

    GX_SetDispSelect(GX_DISP_SELECT_SUB_MAIN);

    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);         // To generate V-Blank interrupt request
    (void)OS_EnableIrq();


    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
    MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);

    MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);   // Clear OAM
    MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);     // Clear the standard palette

    MI_CpuFillFast((void *)HW_DB_OAM, 192, HW_DB_OAM_SIZE);     // Clear OAM
    MI_CpuClearFast((void *)HW_DB_PLTT, HW_DB_PLTT_SIZE);       // Clear the standard palette
    MI_DmaFill32(3, (void *)HW_LCDC_VRAM_C, 0x7FFF7FFF, 256 * 192 * sizeof(u16));


    GX_SetBankForOBJ(GX_VRAM_OBJ_256_AB);       // Set VRAM-A,B for OBJ

    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS,    // 2D / 3D Mode
                       GX_BGMODE_0,    // BGMODE 0
                       GX_BG0_AS_2D);  // Set BG0 as 2D

    GX_SetVisiblePlane(GX_PLANEMASK_OBJ);       // Make OBJs visible
    GX_SetOBJVRamModeBmp(GX_OBJVRAMMODE_BMP_1D_128K);   // 2D mapping OBJ

    OS_WaitVBlankIntr();               // Waiting for the end of the V-Blank interrupt
    GX_DispOn();

}







/* User data comparison function */
static s32 compare(void *elem1, void *elem2)
{
    // To ensure there is no overflow into the sign bit, promote to s64 and then compare.
    s64     diff = (s64)*(u32 *)elem1 - (s64)*(u32 *)elem2;
    // Normalize only the large-small ordering to {+1,0,-1}, and return it.
    return (diff > 0) ? +1 : ((diff < 0) ? -1 : 0);
}

/* Function for debug output of array data */
static void PrintArray(u32 *array, u32 size)
{
#pragma unused( array )
    u32     i;

    for (i = 0; i < size; i++)
    {
        if ((i & 0x7) == 0)
        {
            OS_TPrintf("\n");
        }
        OS_TPrintf("%08lx,", array[i]);
    }
    OS_TPrintf("\n");
}


/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Initialization and main loop

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    MATHRandContext32 context;

    // Initialization
    KeyInit();
    OS_Init();
    TP_Init();
    OS_InitTick();

    // Memory allocation
    InitializeAllocateSystem();

    DisplayInit();

    // Empty call for getting key input data (strategy for pressing A Button in the IPL)
    KeyRead();
    // Bury data with random numbers
    MATH_InitRand32(&context, 0);

    OS_TPrintf("Press A Button to start MATH_QSort() test\n");

    while (TRUE)
    {
        static u32 cnt = 0;
        static OSTick sum = 0;

        KeyRead();

        if ((keys.trg & PAD_BUTTON_A) != 0)
        {
            u32     i;

            {
                for (i = 0; i < DATA_NUM; i++)
                {
                    gDataArray[i] = MATH_Rand32(&context, 0);
                }
            }
            {
                OSTick  before, after;

                // Quick sort
                gSortBuf = (u8 *)OS_Alloc(MATH_QSortStackSize(DATA_NUM));

                before = OS_GetTick();
                MATH_QSort(gDataArray, DATA_NUM, sizeof(u32), compare, NULL);
                after = OS_GetTick();
                OS_Free(gSortBuf);

                // * Verify it is correctly sorted in ascending order
                for (i = 0; i < DATA_NUM; i++)
                {
                    u32 lower = gDataArray[MATH_MAX(i - 1, 0)];
                    u32 upper = gDataArray[i];
                    if (lower > upper)
                    {
                        OS_TPanic("result array is not sorted correctly!");
                    }
                }

                // Print and display sort results
                PrintArray(gDataArray, DATA_NUM);
                cnt++;
                sum += OS_TicksToMicroSeconds(after - before);

                OS_TPrintf("time = %lld us (avg %lld us)\n", OS_TicksToMicroSeconds(after - before),
                           sum / cnt);
                OS_TPrintf("Press A Button to start MATH_QSort() test again\n");
            }
        }
        // Wait for V-Blank interrupt
        OS_WaitVBlankIntr();
    }
}


/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  V-Blank function

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void)
{

    // Set IRQ check flag
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}



/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
