/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SPI - demos - pm-1
  File:     main.c

  Copyright 2003-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#include  <nitro.h>
#include  "font.h"
#include  "screen.h"

//---- For battery display
char   *myBatteryStr[] = { "HI", "LO", "--" };
#define MY_BATTERY_HI      0
#define MY_BATTERY_LO      1
#define MY_BATTERY_UNKNOWN 2

//---- For LCD switch (next status)
#define MY_LCD_KEEP        0           // No change
#define MY_LCD_OFF         1
#define MY_LCD_ON          2

static int myLCDNext = MY_LCD_KEEP;

//---- For power save
#define MY_POWER_SAVE_PERIOD  3000     // Milliseconds

//---- For battery check
#define MY_BATTERY_CHECK_INTERVAL 10   // Frame

static int myBatteryStatus = MY_BATTERY_UNKNOWN;
static u32 myBatteryCount;

//---- For RTC
static RTCTime myCurrentTime;
static int myResult;

//---- Alarm
static OSAlarm myAlarm;

//---- Message
#define  MY_MESSAGE_SIZE   4

static OSMessage myMessageBuffer[MY_MESSAGE_SIZE];
static OSMessageQueue myMessageQueue;

//---- V-Count
static u32 myVCount;

//---- Sleep reason
#define MY_SLEEP_BY_CLOSE_COVER    0
#define MY_SLEEP_BY_PUSH_START     1
#define MY_SLEEP_BY_PUSH_LEFT      2
static u32 mySleepReason;

static void myInit(void);
static void myVBlankIntr(void);
static void myAlarmHandler(void *arg);

void    myPreCallback(void *arg);
void    myPostCallback(void *arg);
PMSleepCallbackInfo myPreCallbackInfo;
PMSleepCallbackInfo myPostCallbackInfo;

#ifdef SDK_TWL
//---- exit callback
PMExitCallbackInfo info;
void exitCallback(void*arg);
void batteryLowCallback(void*arg);
#endif

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Main.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    u16     preButton;
    u16     button;
    u16     trigger;

    //---------------- Initialize
    myInit();
    RTC_Init();

#ifdef SDK_TWL
    if ( OS_IsRunOnTwl() )
    {
        PM_SetAutoExit( TRUE );
        OS_Printf("[ARM9] set exit callback\n");
        PM_SetExitCallbackInfo( &info, exitCallback, (void*)100 );
        PM_AppendPreExitCallback( &info );

        PM_SetBatteryLowCallback( batteryLowCallback, NULL );
    }
#endif

    OS_InitMessageQueue(&myMessageQueue, &myMessageBuffer[0], MY_MESSAGE_SIZE);
    OS_CreateAlarm(&myAlarm);

    preButton = PAD_Read();            // Dummy read for pressing 'A' on IPL

    //---- set SleepMode callbacks
    PM_SetSleepCallbackInfo(&myPreCallbackInfo, myPreCallback, (void *)&mySleepReason);
    PM_SetSleepCallbackInfo(&myPostCallbackInfo, myPostCallback, NULL);
    PM_AppendPreSleepCallback(&myPreCallbackInfo);
    PM_AppendPostSleepCallback(&myPostCallbackInfo);

    //---------------- Main loop
    while (TRUE)
    {
        button = PAD_Read();
        trigger = (u16)((button ^ preButton) & button);
        preButton = button;

        //---- V-Blank count
        myVCount = OS_GetVBlankCount();

        //---- Clear screen buffer
        ClearScreen();

        //---- Display key description
        PrintString(3, 2, 15, "START : sleep till push");
        PrintString(3, 3, 15, "        SELECT");
        PrintString(3, 4, 15, "A     : top    back light");
        PrintString(3, 5, 15, "B     : bottom back light");
#ifdef SDK_TWL
        if ( OS_IsRunOnTwl() )
        {
            PrintString(3, 6, 15, "X     : do hardware reset");
        }
#endif
        PrintString(3, 7, 15, "UP    : power save till push");
        PrintString(3, 8, 15, "        DOWN or pass 3 secs");
        PrintString(3, 9, 15, "LEFT  : sleep till next min");
        PrintString(3, 10, 15, "        by RTC or push RIGHT");
        PrintString(3, 12, 15, "CLOSE : sleep till open");
        PrintString(3, 13, 15, "L & R : power off");

        //---- Display time
        myResult = RTC_GetTime(&myCurrentTime);
        if (myResult == 0 /*No error */ )
        {
            PrintString(5, 20, 8, "%02d:%02d:%02d",
                        myCurrentTime.hour, myCurrentTime.minute, myCurrentTime.second);
        }

        //---- Display battery status
        if (myBatteryCount < myVCount || myBatteryStatus == MY_BATTERY_UNKNOWN)
        {
            PMBattery battery;
            u32 result = PM_GetBattery(&battery);

            if ( result == PM_RESULT_SUCCESS )
            {
                myBatteryStatus = (battery == PM_BATTERY_HIGH) ? MY_BATTERY_HI : MY_BATTERY_LO;
            }
            else
            {
                OS_Printf("battery level busy\n");
                myBatteryStatus = MY_BATTERY_UNKNOWN;
            }

            myBatteryCount = myVCount + MY_BATTERY_CHECK_INTERVAL;
        }
        {
            PMBatteryLevel level;
            if ( PM_GetBatteryLevel( &level ) == PM_RESULT_SUCCESS )
            {
                PrintString(3, 16, 15, "Battery status : %s val:%d", myBatteryStr[myBatteryStatus], level );
            }
            else
            {
                PrintString(3, 16, 15, "Battery status : %s val:-", myBatteryStr[myBatteryStatus] );
            }
        }

        //---- Display AC adapter
        {
            BOOL isConnected;

            if ( PM_GetACAdapter( &isConnected ) == PM_RESULT_SUCCESS )
            {
                PrintString(3, 17, 15, "AC Adapter     : %s", isConnected? "PLUGGED": "NOT PLUGGED" );
            }
            else
            {
                PrintString(3, 17, 15, "AC Adapter     : -" );
            }
        }

        //---- Display counter
        PrintString(18, 20, 4, "%08X", myVCount);

        //---- Display LED pattern
        {
            PMLEDPattern pattern;
            if ( PM_GetLEDPattern(&pattern) == PM_RESULT_SUCCESS )
            {
                PrintString(3, 18, 15, "LED Pattern    : %d", pattern);
            }
            else
            {
                PrintString(3, 18, 15, "LED Pattern    : -");
            }
        }

        //---- Push START to sleep
        if (trigger & PAD_BUTTON_START)
        {
            mySleepReason = MY_SLEEP_BY_PUSH_START;
            PM_GoSleepMode(PM_TRIGGER_KEY, PM_PAD_LOGIC_AND, PAD_BUTTON_SELECT);
        }

                //---- Push A to switch top backlight on/off
        if (trigger & PAD_BUTTON_A)
        {
            static int cnt = 0;
            switch (cnt % 2)
            {
            case 0:
                if ( PM_SetBackLight(PM_LCD_TOP, PM_BACKLIGHT_OFF) == PM_RESULT_SUCCESS )
                {
                    cnt++;
                }
                break;
            case 1:
                if ( PM_SetBackLight(PM_LCD_TOP, PM_BACKLIGHT_ON) == PM_RESULT_SUCCESS )
                {
                    cnt++;
                }
                break;
            }
        }

        //---- Push B to switch top backlight on/off
        if (trigger & PAD_BUTTON_B)
        {
            static int cnt = 0;
            switch (cnt % 2)
            {
            case 0:
                if ( PM_SetBackLight(PM_LCD_BOTTOM, PM_BACKLIGHT_OFF) == PM_RESULT_SUCCESS )
                {
                    cnt++;
                }
                break;
            case 1:
                if ( PM_SetBackLight(PM_LCD_BOTTOM, PM_BACKLIGHT_ON) == PM_RESULT_SUCCESS )
                {
                    cnt++;
                }
                break;
            }
        }

#ifdef SDK_TWL
        if ( OS_IsRunOnTwl() )
        {
            //---- Push X to do hardware reset
            if (trigger & PAD_BUTTON_X)
            {
                (void)OS_RebootSystem();
                // Do not call PM_ForceToResetHardware directly
            }

            //---- Push Y to change wireless LED
            if (trigger & PAD_BUTTON_Y)
            {
                static PMWirelessLEDStatus sw = PM_WIRELESS_LED_OFF;

                if ( PMi_SetWirelessLED(sw) == PM_RESULT_SUCCESS )
                {
                    sw = (sw == PM_WIRELESS_LED_OFF)? PM_WIRELESS_LED_ON: PM_WIRELESS_LED_OFF;
                }
            }
        }
#endif

        //---- Push UP for power save mode
        if (trigger & PAD_KEY_UP && PM_GetLCDPower() == PM_LCD_POWER_ON && myLCDNext == MY_LCD_KEEP)
        {
            //---- Set 3-second alarm
            OS_CancelAlarm(&myAlarm);
            OS_SetAlarm(&myAlarm, OS_MilliSecondsToTicks(MY_POWER_SAVE_PERIOD), myAlarmHandler,
                        NULL);

            //---- Turn LCD off
            myLCDNext = MY_LCD_OFF;
        }

        //---- Push LEFT to sleep till next minute
        if (trigger & PAD_KEY_LEFT)
        {
            RTCAlarmStatus st = RTC_ALARM_STATUS_ON;
            RTCAlarmParam p;

            p.week = RTC_WEEK_SUNDAY;  // Dummy
            p.hour = 1;                // Dummy
            p.minute = (myCurrentTime.minute == 59) ? 0 : (myCurrentTime.minute + 1);
            p.enable = RTC_ALARM_ENABLE_MINUTE;

            myResult = RTC_SetAlarmStatus(RTC_ALARM_CHAN_1, &st);
            myResult = RTC_SetAlarmParam(RTC_ALARM_CHAN_1, &p);

            mySleepReason = MY_SLEEP_BY_PUSH_LEFT;
            PM_GoSleepMode(PM_TRIGGER_KEY | PM_TRIGGER_RTC_ALARM, PM_PAD_LOGIC_AND, PAD_KEY_RIGHT);
        }

        //---- Push DOWN to return from power save mode
        if (trigger & PAD_KEY_DOWN && PM_GetLCDPower() == PM_LCD_POWER_OFF
            && myLCDNext == MY_LCD_KEEP)
        {
            //---- Turn LCD on
            myLCDNext = MY_LCD_ON;
            OS_CancelAlarm(&myAlarm);
        }

        //---- Close cover to sleep
        if (PAD_DetectFold() == TRUE)
        {
            myLCDNext = MY_LCD_KEEP;
            mySleepReason = MY_SLEEP_BY_CLOSE_COVER;
            PM_GoSleepMode(PM_TRIGGER_COVER_OPEN | PM_TRIGGER_CARD, 0, 0);
        }

        //---- Push L & R to power off
        if ((button & (PAD_BUTTON_L | PAD_BUTTON_R)) == (PAD_BUTTON_L | PAD_BUTTON_R))
        {
            (void)PM_ForceToPowerOff();
        }

        //---- Switch LCD power
        if (myLCDNext != MY_LCD_KEEP)
        {
            if (myLCDNext == MY_LCD_OFF)
            {
                OS_WaitVBlankIntr();
                GX_DispOff();
                GXS_DispOff();
                OS_WaitVBlankIntr();
                if (PM_SetLCDPower(PM_LCD_POWER_OFF))
                {
                    myLCDNext = MY_LCD_KEEP;
                }
            }
            else if (myLCDNext == MY_LCD_ON)
            {
                if (PM_SetLCDPower(PM_LCD_POWER_ON))
                {
                    OS_WaitVBlankIntr();
                    GX_DispOn();
                    GXS_DispOn();
                    OS_WaitVBlankIntr();
                    myLCDNext = MY_LCD_KEEP;
                }
            }
        }

        OS_WaitVBlankIntr();
    }
}

//----------------------------------------------------------------
//  myInit
//
void myInit(void)
{
    //---- Init
    OS_Init();
    OS_InitTick();
    OS_InitAlarm();
    FX_Init();
    GX_Init();
    GX_DispOff();
    GXS_DispOff();

    //---- Display init
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
    MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);
    (void)GX_DisableBankForLCDC();

    MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);
    MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);
    MI_CpuFillFast((void *)HW_DB_OAM, 192, HW_DB_OAM_SIZE);
    MI_CpuClearFast((void *)HW_DB_PLTT, HW_DB_PLTT_SIZE);

    //---- Setting 2D for top screen
    GX_SetBankForBG(GX_VRAM_BG_128_A);

    G2_SetBG0Control(GX_BG_SCRSIZE_TEXT_256x256,
                     GX_BG_COLORMODE_16,
                     GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, GX_BG_EXTPLTT_01);
    G2_SetBG0Priority(0);
    G2_BG0Mosaic(FALSE);
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D);
    GX_SetVisiblePlane(GX_PLANEMASK_BG0);

    GX_LoadBG0Char(d_CharData, 0, sizeof(d_CharData));
    GX_LoadBGPltt(d_PaletteData, 0, sizeof(d_PaletteData));



    //---- Setting 2D for bottom screen
    GX_SetBankForSubBG(GX_VRAM_SUB_BG_128_C);

    G2S_SetBG0Control(GX_BG_SCRSIZE_TEXT_256x256,
                      GX_BG_COLORMODE_16,
                      GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, GX_BG_EXTPLTT_01);
    G2S_SetBG0Priority(0);
    G2S_BG0Mosaic(FALSE);
    GXS_SetGraphicsMode(GX_BGMODE_0);
    GXS_SetVisiblePlane(GX_PLANEMASK_BG0);

    GXS_LoadBG0Char(d_CharData, 0, sizeof(d_CharData));
    GXS_LoadBGPltt(d_PaletteData, 0, sizeof(d_PaletteData));


    //---- Screen
    MI_CpuFillFast((void *)gScreen, 0, sizeof(gScreen));
    DC_FlushRange(gScreen, sizeof(gScreen));
    /* I/O register is accessed using DMA operation, so cache wait is not needed */
    // DC_WaitWriteBufferEmpty();
    GX_LoadBG0Scr(gScreen, 0, sizeof(gScreen));
    GXS_LoadBG0Scr(gScreen, 0, sizeof(gScreen));

    //---- Init interrupt
    OS_SetIrqFunction(OS_IE_V_BLANK, myVBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    //---- FileSytem init
    FS_Init(FS_DMA_NOT_USE);

    //---- Start displaying
    GX_DispOn();
    GXS_DispOn();
}

//----------------------------------------------------------------
//  myVBlankIntr
//             V-Blank interrupt handler
//
static void myVBlankIntr(void)
{
    //---- Upload pseudo screen to VRAM
    DC_FlushRange(gScreen, sizeof(gScreen));
    /* I/O register is accessed using DMA operation, so cache wait is not needed */
    // DC_WaitWriteBufferEmpty();
    GX_LoadBG0Scr(gScreen, 0, sizeof(gScreen));
    GXS_LoadBG0Scr(gScreen, 0, sizeof(gScreen));

    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

//----------------------------------------------------------------
//  myAlarmHandler
//             Alarm interrupt handler
//
void myAlarmHandler(void *arg)
{
#pragma unused(arg)
    if (PM_GetLCDPower() == PM_LCD_POWER_OFF && myLCDNext == MY_LCD_KEEP)
    {
        myLCDNext = MY_LCD_ON;
    }
}

//================================================================
//----------------------------------------------------------------
//  myPreCallback
//
static const char *myCallbackReasonStr[] = {
    "closing cover",
    "pushing START",
    "pushing LEFT"
};

void myPreCallback(void *arg)
{
    int     reason = *(int *)arg;
    OS_Printf("Go to sleep mode by %s\n", myCallbackReasonStr[reason]);
}

//----------------------------------------------------------------
//  myPostCallback
//
void myPostCallback(void *arg)
{
#pragma unused(arg)
    OS_Printf("Return from sleep mode.\n");
}

#ifdef SDK_TWL
//----------------------------------------------------------------
//  exitCallback
//
void exitCallback(void *arg)
{
#ifdef SDK_FINALROM
#pragma unused( arg )
#endif
    OS_Printf("exit callback. arg=%d factor=%d\n", arg, PM_GetExitFactor() );
}

//----------------------------------------------------------------
//  batteryLowCallback
//
void batteryLowCallback(void *arg)
{
#ifdef SDK_FINALROM
#pragma unused( arg )
#endif
    OS_Printf("battery low callback. arg=%d\n", arg);
}
#endif
