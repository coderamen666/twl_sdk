/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - multiboot
  File:     main.c

  Copyright 2006-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/


#include <nitro.h>
#include <nitro/wm.h>
#include <nitro/mb.h>


#include "common.h"
#include "dispfunc.h"

/*
 * For BeaconPeriod randomization (rand)
 */
#include <stdlib.h>

/*
 * A simple example of a multiboot parent
 *
 * Because the MB library samples use the multiboot functionality, multiple development units with the same communications environment (wired or wireless) are required
 * 
 * The mb_child_NITRO.srl and mb_child_TWL.srl sample programs in the $TwlSDK/bin/ARM9-TS/Rom/ directory provide the same features as a multiboot child for final retail units. Load these binaries on other hardware as with any sample program, and run them together.
 * 
 * 
 * 
 * 
 */


/*---------------------------------------------------------------------------*/
/* Functions */

static void Menu(void);

/* V-Blank interrupt process */
static void VBlankIntr(void)
{
    //---- OAM updating
    DC_FlushRange(oamBak, sizeof(oamBak));
    /* I/O register is accessed using DMA operation, so cache wait is not needed */
    // DC_WaitWriteBufferEmpty();
    MI_DmaCopy32(3, oamBak, (void *)HW_OAM, sizeof(oamBak));

    //---- BG screen update
    BgScrSetVblank();

    (void)rand();

    //---- Interrupt check flag
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/* Main */
void NitroMain()
{
    /* OS initialization */
    OS_Init();
    OS_InitTick();
    OS_InitAlarm();
    FX_Init();

    /* GX initialization */
    GX_Init();
    GX_DispOff();
    GXS_DispOff();

    {                                  /* Memory allocation initialization */
        OSHeapHandle heapHandle;       // Heap handle
        enum
        { MAIN_HEAP_SIZE = 0x80000 };
        void   *heapStart, *nstart;
        nstart = OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 16);
        OS_SetMainArenaLo(nstart);
        heapStart = OS_AllocFromMainArenaLo((u32)MAIN_HEAP_SIZE, 32);
        heapHandle =
            OS_CreateHeap(OS_ARENA_MAIN, heapStart, (void *)((u32)heapStart + MAIN_HEAP_SIZE));
        if (heapHandle < 0)
        {
            OS_Panic("ARM9: Fail to create heap...\n");
        }
        heapHandle = OS_SetCurrentHeap(OS_ARENA_MAIN, heapHandle);
    }
    
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    /* FS initialization */
    FS_Init(FS_DMA_NOT_USE);
    {
        u32     need_size = FS_GetTableSize();
        void   *p_table = OS_Alloc(need_size);
        SDK_ASSERT(p_table != NULL);
        (void)FS_LoadTable(p_table, need_size);
    }

    {                                  /* Initialize drawing setting */
        GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
        MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);
        (void)GX_DisableBankForLCDC();
        MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);
        MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);
        MI_CpuFillFast((void *)HW_DB_OAM, 192, HW_DB_OAM_SIZE);
        MI_CpuClearFast((void *)HW_DB_PLTT, HW_DB_PLTT_SIZE);
        BgInitForPrintStr();
        GX_SetBankForOBJ(GX_VRAM_OBJ_128_B);
        GX_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_2D);
        GX_SetVisiblePlane(GX_PLANEMASK_BG0 | GX_PLANEMASK_OBJ);
        GXS_SetVisiblePlane(GX_PLANEMASK_BG0 | GX_PLANEMASK_OBJ);
        GX_LoadOBJ(sampleCharData, 0, sizeof(sampleCharData));
        GX_LoadOBJPltt(samplePlttData, 0, sizeof(samplePlttData));
        MI_DmaFill32(3, oamBak, 0xc0, sizeof(oamBak));
    }

    /* V-Blank interrupt configuration */
    (void)OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrqMask(OS_IE_FIFO_RECV);

    (void)GX_VBlankIntr(TRUE);

    // RTC initialization
    RTC_Init();

    /* Start display */
    GX_DispOn();
    GXS_DispOn();
    G2_SetBG0Offset(0, 0);
    G2S_SetBG0Offset(0, 0);

    /* Initialization of the pointer to the file buffer */
    InitFileBuffer();

    /* Main loop */
    prog_state = STATE_MENU;
    for (;;)
    {
        OS_WaitVBlankIntr();

        switch (prog_state)
        {
        case STATE_MENU:
            Menu();
            break;
        case STATE_MB_PARENTINIT:
            ParentInit();
            break;
        case STATE_MB_PARENT:
            ParentMain();
            break;
        default:
            break;
        }
    }

}

/* 
 * -------------------------------------------------------------
 * Menu processing functions
 * -------------------------------------------------------------
 */

enum
{
    MENU_SENDSIZE,
    MENU_MAX_CHILDREN,
    MENU_AUTO_ACCEPT,
    MENU_AUTO_SENDFILE,
    MENU_AUTO_BOOTREQUEST,
    MENU_NUM
};

/* Send size table */
u16     sendsize_table[] = {
    256, 384, 510,
};

/* Work for the menu */
typedef struct
{
    u8      item_no;
    u8      size_index;
    u8      channel;
    u8      auto_accept_flag;
    u8      auto_send_flag;
    u8      auto_boot_flag;
}
MenuData;

/* Menu */
static void Menu(void)
{
    const u16 keyData = ReadKeySetTrigger(PAD_Read());
    static MenuData menu;
    enum
    { DISP_OX = 5, DISP_OY = 5 };

    /* Change cursor number */
    if (keyData & PAD_KEY_DOWN)
    {
        (void)RotateU8(&menu.item_no, 0, MENU_NUM - 1, 1);
    }
    else if (keyData & PAD_KEY_UP)
    {
        (void)RotateU8(&menu.item_no, 0, MENU_NUM - 1, -1);
    }
    else if (keyData & (PAD_BUTTON_A | PAD_BUTTON_START))
    {
        prog_state = STATE_MB_PARENTINIT;
    }

    /* Start display */
    BgClear();
    BgPutString((s16)(DISP_OX + 3), (s16)(DISP_OY - 3), WHITE, "Multiboot Sample");

    BgPrintf((s16)(DISP_OX), (s16)(DISP_OY), WHITE,
             "Send Size(Bytes)    [%3d]", GetParentSendSize());
    BgPrintf((s16)(DISP_OX), (s16)(DISP_OY + 1), WHITE, "Children Entry Max  [%2d]", GetMaxEntry());
    BgPrintf((s16)(DISP_OX), (s16)(DISP_OY + 2), WHITE,
             "Auto Accept Entries %s", (menu.auto_accept_flag) ? "[Yes]" : "[No ]");
    BgPrintf((s16)(DISP_OX), (s16)(DISP_OY + 3), WHITE,
             "Auto Send Start     %s", (menu.auto_send_flag) ? "[Yes]" : "[No ]");
    BgPrintf((s16)(DISP_OX), (s16)(DISP_OY + 4), WHITE,
             "Auto Boot           %s", (menu.auto_boot_flag) ? "[Yes]" : "[No ]");
    /* Cursor display */
    BgPutChar((s16)(DISP_OX - 2), (s16)(DISP_OY + menu.item_no), RED, '*');
    /* Wait for A button */
    BgPutString(DISP_OX, DISP_OY + 14, WHITE, " Push A or START Button");

    switch (menu.item_no)
    {
        /* MP send size configuration */
    case MENU_SENDSIZE:
        if (keyData & PAD_KEY_RIGHT)
        {
            (void)RotateU8(&menu.size_index, 0, sizeof(sendsize_table) / sizeof(u16) - 1, 1);
        }
        else if (keyData & PAD_KEY_LEFT)
        {
            (void)RotateU8(&menu.size_index, 0, sizeof(sendsize_table) / sizeof(u16) - 1, -1);
        }
        SetParentSendSize(sendsize_table[menu.size_index]);
        break;

        /* Set the maximum number of child connections */
    case MENU_MAX_CHILDREN:
        {
            u8      entry = GetMaxEntry();
            if (keyData & PAD_KEY_RIGHT)
            {
                (void)RotateU8(&entry, 1, MB_MAX_CHILD, 1);
            }
            else if (keyData & PAD_KEY_LEFT)
            {
                (void)RotateU8(&entry, 1, MB_MAX_CHILD, -1);
            }
            SetMaxEntry(entry);
        }
        break;

        /* AUTO ACCEPT configuration */
    case MENU_AUTO_ACCEPT:
        if (keyData & (PAD_KEY_RIGHT | PAD_KEY_LEFT))
        {
            menu.auto_accept_flag ^= 1;
        }
        break;

        /* AUTO SEND configuration */
    case MENU_AUTO_SENDFILE:
        if (keyData & (PAD_KEY_RIGHT | PAD_KEY_LEFT))
        {
            menu.auto_send_flag ^= 1;
        }
        break;

        /* AUTO BOOT configuration */
    case MENU_AUTO_BOOTREQUEST:
        if (keyData & (PAD_KEY_RIGHT | PAD_KEY_LEFT))
        {
            menu.auto_boot_flag ^= 1;
        }
        break;
    default:
        break;
    }

    SetAutoOperation(menu.auto_accept_flag, menu.auto_send_flag, menu.auto_boot_flag);

}
