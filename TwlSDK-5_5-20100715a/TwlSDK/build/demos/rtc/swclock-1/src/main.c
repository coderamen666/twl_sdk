/*---------------------------------------------------------------------------*
  Project:  TwlSDK - RTC - demos - swclock-1
  File:     main.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-11-02#$
  $Rev: 2048 $
  $Author: tokunaga_eiji $
 *---------------------------------------------------------------------------*/

#ifdef SDK_TWL
#include <twl.h>
#include <twl/rtc.h>
#else
#include <nitro.h>
#include <nitro/rtc.h>
#endif

#include    "font.h"


/*---------------------------------------------------------------------------*
    Constant Definitions
 *---------------------------------------------------------------------------*/
#define     KEY_REPEAT_START    25     // Number of frames until key repeat starts
#define     KEY_REPEAT_SPAN     10     // Number of frames between key repeats


/*---------------------------------------------------------------------------*
    Character string constant definitions
 *---------------------------------------------------------------------------*/
// Day of the week
const char *StrWeek[7] = {
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday"
};

/*---------------------------------------------------------------------------*
    Structure Definitions
 *---------------------------------------------------------------------------*/
// Key input data
typedef struct KeyInformation
{
    u16     cnt;                       // Unprocessed input value
    u16     trg;                       // Push trigger input
    u16     up;                        // Release trigger input
    u16     rep;                       // Press and hold repeat input

}
KeyInformation;

/*---------------------------------------------------------------------------*
    Internal Function Definitions
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void);
static void AlarmIntrCallback(void);

static void KeyRead(KeyInformation * pKey);
static void ClearString(void);
static void PrintString(s16 x, s16 y, u8 palette, char *text, ...);
static void ColorString(s16 x, s16 y, s16 length, u8 palette);


/*---------------------------------------------------------------------------*
    Internal Variable Definitions
 *---------------------------------------------------------------------------*/
static u16 gScreen[32 * 32];           // Virtual screen
static KeyInformation gKey;            // Key input

static RTCDate   gRtcDate;             // Date obtained from the RTC
static RTCTime   gRtcTime;             // Time obtained from the RTC
static RTCDate   gSWClockDate;         // Date obtained from the software clock
static RTCTimeEx gSWClockTimeEx;       // Time obtained from the software clock

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Initialization and main loop.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    // Various types of initialization
    OS_Init();
    FX_Init();
    GX_Init();
    GX_DispOff();
    GXS_DispOff();

    // Initializes display settings
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
    MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);
    (void)GX_DisableBankForLCDC();
    MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);
    MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);
    MI_CpuFillFast((void *)HW_DB_OAM, 192, HW_DB_OAM_SIZE);
    MI_CpuClearFast((void *)HW_DB_PLTT, HW_DB_PLTT_SIZE);

    // 2D display settings for text string display
    GX_SetBankForBG(GX_VRAM_BG_128_A);
    G2_SetBG0Control(GX_BG_SCRSIZE_TEXT_256x256, GX_BG_COLORMODE_16, GX_BG_SCRBASE_0xf800,      // SCR base block 31
                     GX_BG_CHARBASE_0x00000,    // CHR base block 0
                     GX_BG_EXTPLTT_01);
    G2_SetBG0Priority(0);
    G2_BG0Mosaic(FALSE);
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D);
    GX_SetVisiblePlane(GX_PLANEMASK_BG0);
    GX_LoadBG0Char(d_CharData, 0, sizeof(d_CharData));
    GX_LoadBGPltt(d_PaletteData, 0, sizeof(d_PaletteData));
    MI_CpuFillFast((void *)gScreen, 0, sizeof(gScreen));
    DC_FlushRange(gScreen, sizeof(gScreen));
    /* I/O register is accessed using DMA operation, so cache wait is not needed */
    // DC_WaitWriteBufferEmpty();
    GX_LoadBG0Scr(gScreen, 0, sizeof(gScreen));

    // Interrupt settings
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();


    //****************************************************************
    // RTC initialization
    RTC_Init();
    // Initialize tick
    OS_InitTick();
    // Initialize software clock
    (void)RTC_InitSWClock();
    //****************************************************************

    // LCD display start
    GX_DispOn();
    GXS_DispOn();

    // Debug string output
    OS_Printf("ARM9: SWClock demo started.\n");

    // Empty call for getting key input data (strategy for pressing A button in the IPL)
    KeyRead(&gKey);

    // Main loop
    while (TRUE)
    {
        // Get key input data
        KeyRead(&gKey);

        // Clear the screen
        ClearString();

        //****************************************************************
        // Read date and time from the RTC
        (void)RTC_GetDateTime(&gRtcDate, &gRtcTime);
        // Read date and time from the software clock
        (void)RTC_GetDateTimeExFromSWClock(&gSWClockDate, &gSWClockTimeEx);
        //****************************************************************

        // Display date obtained from the RTC
        PrintString(1, 3, 0xf, "RTCDate:");
        PrintString(10, 3, 0xf, "%04d/%02d/%02d/%s",
                    gRtcDate.year + 2000, gRtcDate.month, gRtcDate.day, StrWeek[gRtcDate.week]);
        // Display date obtained from the software clock
        PrintString(1, 5, 0xf, "SWCDate:");
        PrintString(10, 5, 0xf, "%04d/%02d/%02d/%s",
                    gSWClockDate.year + 2000, gSWClockDate.month, gSWClockDate.day, StrWeek[gSWClockDate.week]);

        // Display time obtained from the RTC
        PrintString(1, 8, 0xf, "RTCTime:");
        PrintString(10, 8, 0xf, "%02d:%02d:%02d", gRtcTime.hour, gRtcTime.minute, gRtcTime.second);
        // Display time obtained from the software clock
        PrintString(1, 10, 0xf, "SWCTime:");        
        PrintString(10, 10, 0xf, "%02d:%02d:%02d:%03d", gSWClockTimeEx.hour, gSWClockTimeEx.minute,
                    gSWClockTimeEx.second, gSWClockTimeEx.millisecond);

        // Display the total number of ticks that have occurred since midnight on January 1st, 2000 (this value was obtained from the software clock)
        PrintString(1, 13, 0xf, "SWCTick:");
        PrintString(10, 13, 0xf, "%llu", RTC_GetSWClockTick() );        
        
        // Display explanation of button operations
        PrintString(1, 19, 0x3, "START > Synclonize SWC with RTC.");
        PrintString(1, 20, 0x3, "A     > Go to sleep.");
        PrintString(1, 21, 0x3, "B     > Wake up from sleep.");


        // Button input operation
        // START button
        if ((gKey.trg) & PAD_BUTTON_START)
        {
            OS_TPrintf("Synchronizing SWC with RTC.\n");
            (void)RTC_SyncSWClock();
        }

        // A button
        if (gKey.trg & PAD_BUTTON_A)
        {
            OS_TPrintf("Going to sleep... Press B button to wake up.\n");
            PM_GoSleepMode( PM_TRIGGER_KEY, PM_PAD_LOGIC_AND, PAD_BUTTON_B );
        }

        // Waiting for the V-Blank
        OS_WaitVBlankIntr();
    }
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  V-Blank interrupt vector.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void)
{
    // Reflect virtual screen in VRAM
    DC_FlushRange(gScreen, sizeof(gScreen));
    /* I/O register is accessed using DMA operation, so cache wait is not needed */
    // DC_WaitWriteBufferEmpty();
    GX_LoadBG0Scr(gScreen, 0, sizeof(gScreen));

    // Sets the IRQ check flag
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         KeyRead

  Description:  Edits key input data.
                Detects press trigger, release trigger, and press-and-hold repeat.

  Arguments:    pKey: Structure that holds key input data to be edited

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void KeyRead(KeyInformation * pKey)
{
    static u16 repeat_count[12];
    int     i;
    u16     r;

    r = PAD_Read();
    pKey->trg = 0x0000;
    pKey->up = 0x0000;
    pKey->rep = 0x0000;

    for (i = 0; i < 12; i++)
    {
        if (r & (0x0001 << i))
        {
            if (!(pKey->cnt & (0x0001 << i)))
            {
                pKey->trg |= (0x0001 << i);     // Press trigger
                repeat_count[i] = 1;
            }
            else
            {
                if (repeat_count[i] > KEY_REPEAT_START)
                {
                    pKey->rep |= (0x0001 << i); // Press-and-hold repeat
                    repeat_count[i] = KEY_REPEAT_START - KEY_REPEAT_SPAN;
                }
                else
                {
                    repeat_count[i]++;
                }
            }
        }
        else
        {
            if (pKey->cnt & (0x0001 << i))
            {
                pKey->up |= (0x0001 << i);      // Release trigger
            }
        }
    }
    pKey->cnt = r;                     // Unprocessed key input
}

/*---------------------------------------------------------------------------*
  Name:         ClearString

  Description:  Clears the virtual screen.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ClearString(void)
{
    MI_CpuClearFast((void *)gScreen, sizeof(gScreen));
}

/*---------------------------------------------------------------------------*
  Name:         PrintString

  Description:  Positions the text string on the virtual screen. The string can be up to 32 chars.

  Arguments:    x: X-coordinate where character string starts (x 8 dots)
                y: Y-coordinate where character string starts (x 8 dots)
                palette: Specify text color by palette number
                text: Text string to position. NULL terminated.
                ...: Virtual argument

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PrintString(s16 x, s16 y, u8 palette, char *text, ...)
{
    va_list vlist;
    char    temp[32 + 2];
    s32     i;

    va_start(vlist, text);
    (void)vsnprintf(temp, 33, text, vlist);
    va_end(vlist);

    *(u16 *)(&temp[32]) = 0x0000;
    for (i = 0;; i++)
    {
        if (temp[i] == 0x00)
        {
            break;
        }
        gScreen[((y * 32) + x + i) % (32 * 32)] = (u16)((palette << 12) | temp[i]);
    }
}

/*---------------------------------------------------------------------------*
  Name:         ColorString

  Description:  Changes the color of character strings printed on the virtual screen.

  Arguments:    x: X-coordinate (x 8 dots ) from which to start color change
                y: Y-coordinate (x 8 dots ) from which to start color change
                length: Number of characters to continue the color change for
                palette: Specify text color by palette number

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ColorString(s16 x, s16 y, s16 length, u8 palette)
{
    s32     i;
    u16     temp;
    s32     index;

    if (length < 0)
        return;

    for (i = 0; i < length; i++)
    {
        index = ((y * 32) + x + i) % (32 * 32);
        temp = gScreen[index];
        temp &= 0x0fff;
        temp |= (palette << 12);
        gScreen[index] = temp;
    }
}

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
