/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - demos - wxc-pm
  File:     main.c

  Copyright 2005-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-11-15#$
  $Rev: 2414 $
  $Author: hatamoto_minoru $
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    This sample runs chance encounter communications over a wireless connection.
    It automatically connects to the wxc-pm demo in the area.
 *---------------------------------------------------------------------------*/

#include <nitro.h>

#include <nitro/wxc.h>
#include "user.h"
#include "common.h"
#include "dispfunc.h"


/*---------------------------------------------------------------------------*
    Constant Definitions
 *---------------------------------------------------------------------------*/
/* GGID used for testing */
#define SDK_MAKEGGID_SYSTEM(num)              (0x003FFF00 | (num))
#define GGID_ORG_1                            SDK_MAKEGGID_SYSTEM(0x53)
#define GGID_ORG_2                            SDK_MAKEGGID_SYSTEM(0x54)

/*---------------------------------------------------------------------------*
    Internal Function Definitions
 *---------------------------------------------------------------------------*/
static void ModeSelect(void);          // Parent/child select screen
static void ModeError(void);           // Error display screen
static void VBlankIntr(void);          // V-Blank interrupt handler

// General purpose subroutines
static void InitializeAllocateSystem(void);

static void Menu(void);

/*---------------------------------------------------------------------------*
    Internal Variable Definitions
 *---------------------------------------------------------------------------*/
static s32 gFrame;                     // Frame counter

/*---------------------------------------------------------------------------*
    External Variable Definitions
 *---------------------------------------------------------------------------*/
int passby_endflg;
int menu_flg;

BOOL passby_ggid[MENU_MAX];     // GGID that does chance encounter communications

u16 keyData;

BOOL sleepFlag;
/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Initialization and main loop.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    int i;
    
    // Various types of initialization
    OS_Init();
    OS_InitTick();

    OS_InitAlarm();
    
    FX_Init();

    {                                  /* Rendering settings initialization */
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
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();
    (void)GX_VBlankIntr(TRUE);

    // RTC initialization
    RTC_Init();

    /* Start display */
    GX_DispOn();
    GXS_DispOn();
    G2_SetBG0Offset(0, 0);
    G2S_SetBG0Offset(0, 0);

    /* Memory allocation initialization */
    InitializeAllocateSystem();

    // Debug string output
    OS_Printf("ARM9: WM simple demo started.");
    
    
    passby_endflg = 0;
    menu_flg = MENU_MIN;
    sleepFlag = TRUE;
    
    // Set all initial GGID values as capable of chance encounter
    for(i = 0; i < MENU_MAX; i++)
    {
        passby_ggid[i] = TRUE;
    }
    
    /* Main loop */
    while(1)
    {
        OS_WaitVBlankIntr();
        
        keyData = ReadKeySetTrigger(PAD_Read());
        
        /* Display clear */
        BgClear();
        
        // Chance encounter communications configuration screen
        Menu();

        // Transition to chance encounter communications
        if(menu_flg == 9)
        {
            int i;
            
            // Initialize only the selected GGID
            for(i = 0; i < MENU_MAX; i++)
            {
                if(passby_ggid[i])
                {
                    /* Initializes, registers, and runs chance encounter communications */
                    User_Init();
                    break;
                }
            }
            if(i >= MENU_MAX)
            {
                /* Starts an error message display */
                BgClear();
                BgPutString(5, 19, WHITE, "Set at least one GGID");
                passby_endflg = 4;
            }

            /* Main chance encounter communications loop */
            for (gFrame = 0; passby_endflg == 0; gFrame++)
            {
                OS_WaitVBlankIntr();

                keyData = ReadKeySetTrigger(PAD_Read());
                
                if(passby_endflg == 0)
                {
                    /* Display clear */
                    BgClear();
                }
        
                /* Distributes processes based on communication status */
                switch (WXC_GetStateCode())
                {
                case WXC_STATE_END:
                    ModeSelect();
                    break;
                case WXC_STATE_ENDING:
                    break;
                case WXC_STATE_ACTIVE:
                    if(WXC_IsParentMode() == TRUE)
                    {
                        BgPutString(8, 2, WHITE, "Now parent...");
                    }
                    else
                    {
                        BgPutString(8, 2, WHITE, "Now child...");
                    }
                    break;
                }
                
                BgPutString(5, 19, WHITE, "Push START to Reset");
                
                if (keyData & PAD_BUTTON_START)
                {
                    passby_endflg = 3;
                    /* Display clear */
                    BgClear();
                }
                // Waiting for the V-Blank
                OS_WaitVBlankIntr();
            }
            
            // End here if the WXC_RegisterCommonData function is being called
            if(passby_endflg != 4)
            {
                (void)WXC_End();
            }
            
            //Wait for end
            while( WXC_GetStateCode() != WXC_STATE_END )
            {
                ;
            }
            
            // Move to sleep or power off mode
            if(sleepFlag == TRUE)
            {
                PM_GoSleepMode(PM_TRIGGER_KEY, PM_PAD_LOGIC_AND, PAD_BUTTON_A);
            }
            else
            {
                GX_DispOff();
                GXS_DispOff();
                if(PM_GetLCDPower() == PM_LCD_POWER_ON)
                {
                    while(PM_SetLCDPower(PM_LCD_POWER_OFF) != TRUE)
                    {
                        OS_WaitVBlankIntr();
                    }
                }
            }
            
            // Empty read
            keyData = ReadKeySetTrigger(PAD_Read());
            
            // Power off mode exit loop
            while(sleepFlag == FALSE)
            //while(1)
            {
                keyData = ReadKeySetTrigger(PAD_Read());
                
                if (keyData & PAD_BUTTON_A)
                {
                    if(PM_GetLCDPower() == PM_LCD_POWER_OFF)
                    {
                        while(PM_SetLCDPower(PM_LCD_POWER_ON) != TRUE)
                        {
                            OS_WaitVBlankIntr();
                        }
                    }
                    GX_DispOn();
                    GXS_DispOn();
                    break;
                }
            }
            
            // Wait for A button
            while(1)
            {
                keyData = ReadKeySetTrigger(PAD_Read());
                
                BgPutString(5, 19, WHITE, "Push A to Reset    ");
                //BgPutString(5, 19, WHITE, "Push START to Reset");
                if (keyData & PAD_BUTTON_A)
                {
                    passby_endflg = 0;
                    menu_flg = MENU_MIN;
                    break;
                }
                OS_WaitVBlankIntr();
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         Menu

  Description:  Processing for the chance encounter configuration screen.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
 static void Menu(void)
{
    int i;
    enum
    { DISP_OX = 5, DISP_OY = 5 };
    
    /* Change cursor number */
    if (keyData & PAD_KEY_DOWN)
    {
        menu_flg++;
        if(menu_flg > MENU_MAX)
        {
            menu_flg = MENU_MIN;
        }
    }
    else if (keyData & PAD_KEY_UP)
    {
        menu_flg--;
        if(menu_flg < MENU_MIN)
        {
            menu_flg = MENU_MAX;
        }
    }
    else if (keyData & PAD_BUTTON_A)
    {
        menu_flg = 9;
    }

    /* Start display */
    BgPutString((s16)(DISP_OX + 3), (s16)(DISP_OY - 3), WHITE, "Main Menu");

    for(i = 0; i < MENU_MAX-1; i++)
    {
        BgPrintf((s16)(DISP_OX), (s16)(DISP_OY + i), WHITE,
                 "GGID No.%01d     %s", (i+1), (passby_ggid[i]) ? "[Yes]" : "[No ]");
    }
    BgPrintf((s16)(DISP_OX), (s16)(DISP_OY + i), WHITE,
                 "Sleep or PowerOff ?");
    BgPrintf((s16)(DISP_OX), (s16)(DISP_OY + i+1), WHITE,
                 "              %s", (sleepFlag) ? "[ Sleep  ]" : "[PowerOff]");
    /* Cursor display */
    BgPutChar((s16)(DISP_OX - 2), (s16)(DISP_OY + menu_flg-1), RED, '*');
    /* Wait for A button */
    BgPutString(1, DISP_OY + 12, WHITE, " Retrun Sleep or PowerOff Mode");
    BgPutString(1, DISP_OY + 13, WHITE, "               : Push A Button");
    BgPutString(1, DISP_OY + 15, WHITE, "    Test start : Push A Button");


    if (keyData & (PAD_KEY_RIGHT | PAD_KEY_LEFT))
    {
        if(menu_flg <= 2)
        {
            passby_ggid[menu_flg-1] ^= TRUE;
        }
        else
        {
            sleepFlag ^= TRUE;
        }
    }
}
 
/*---------------------------------------------------------------------------*
  Name:         ModeSelect

  Description:  Processing in parent/child selection screen.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ModeSelect(void)
{
    BgPutString(3, 1, WHITE, "Push A to start");

    if (keyData & PAD_BUTTON_A)
    {
        User_Init();
    }
}

/*---------------------------------------------------------------------------*
  Name:         ModeError

  Description:  Processing in error display screen.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ModeError(void)
{
    BgPutString(5, 0, WHITE, "======= ERROR! =======");
    BgPutString(5, 1, WHITE, " Fatal error occured.");
    BgPutString(5, 2, WHITE, "Please reboot program.");
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  V-Blank interrupt handler.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void)
{
    //---- OAM updating
    DC_FlushRange(oamBak, sizeof(oamBak));
    /* I/O register is accessed using DMA operation, so cache wait is not needed */
    // DC_WaitWriteBufferEmpty();
    MI_DmaCopy32(3, oamBak, (void *)HW_OAM, sizeof(oamBak));

    //---- Background screen update
    BgScrSetVblank();

    //(void)rand();

    //---- Interrupt check flag
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         InitializeAllocateSystem

  Description:  Initializes the memory allocation system in the main memory arena.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitializeAllocateSystem(void)
{
    void   *tempLo;
    OSHeapHandle hh;
    
    OS_Printf(" arena lo = %08x\n", OS_GetMainArenaLo());
    OS_Printf(" arena hi = %08x\n", OS_GetMainArenaHi());

    tempLo = OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1);
    OS_SetArenaLo(OS_ARENA_MAIN, tempLo);
    hh = OS_CreateHeap(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi());
    if (hh < 0)
    {
        OS_Panic("ARM9: Fail to create heap...\n");
    }
    hh = OS_SetCurrentHeap(OS_ARENA_MAIN, hh);
}


/*----------------------------------------------------------------------------*

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
