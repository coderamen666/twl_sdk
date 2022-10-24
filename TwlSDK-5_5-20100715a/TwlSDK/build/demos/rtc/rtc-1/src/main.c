/*---------------------------------------------------------------------------*
  Project:  TwlSDK - RTC - demos - rtc-1
  File:     main.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
    NOTICE: This sample includes functionality that confirms the operation of the RTC alarm. As of 2004/10/06, it may not work properly with versions of the IS-NITRO-DEBUGGER software older than 1.11.
            
            
            Specifically, alarms are not triggered in situations where they should be triggered.
            This is due to a conflict between the Nintendo DS system and the IS-NITRO-EMULATOR hardware; this problem does not occur on the production prototype of the machine.
            
            
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

// Alarm status
const char *StrAlarmStatus[2] = {
    "OFF",
    "ON"
};

// Alarm enable flag
const char *StrAlarmEnable[8] = {
    "- - -",                           // No enabled flag
    "W - -",                           // Week setting enabled
    "- H -",                           //    Hour setting enabled
    "W H -",                           // Week, hour setting enabled
    "- - M",                           //       Minute setting enabled
    "W - M",                           // Week, minute setting enabled
    "- H M",                           //    Hour, minute setting enabled
    "W H M"                            // Week, hour, minute setting enabled
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

// Edit mode
typedef enum EditMode
{
    EDIT_MODE_NONE = 0,                // No edit mode
    EDIT_MODE_DATE_TIME,               // Date and time edit mode
    EDIT_MODE_ALARM_1,                 // Alarm 1 status and setting value edit mode
    EDIT_MODE_ALARM_2,                 // Alarm 2 status and setting value edit mode
    EDIT_MODE_MAX
}
EditMode;


/*---------------------------------------------------------------------------*
    Internal Function Definitions
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void);
static void AlarmIntrCallback(void);

static void IncValue(void);
static void DecValue(void);
static void IncSelect(void);
static void DecSelect(void);

static void KeyRead(KeyInformation * pKey);
static void ClearString(void);
static void PrintString(s16 x, s16 y, u8 palette, char *text, ...);
static void ColorString(s16 x, s16 y, s16 length, u8 palette);


/*---------------------------------------------------------------------------*
    Internal Variable Definitions
 *---------------------------------------------------------------------------*/
static u16 gScreen[32 * 32];           // Virtual screen
static KeyInformation gKey;            // Key input

static RTCDate gRtcDate;               // Date
static RTCTime gRtcTime;               // Time
static RTCAlarmStatus gRtcAlarmStatus1; // Alarm 1 status
static RTCAlarmParam gRtcAlarmParam1;  // Alarm 1 setting value
static RTCAlarmStatus gRtcAlarmStatus2; // Alarm 2 status
static RTCAlarmParam gRtcAlarmParam2;  // Alarm 2 setting value

static EditMode gEditMode;             // Edit mode
static u32 gSelect;                    // Selected place index
static s32 gMosaic;                    // Mosaic display counter


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
    // Set up jump destination vector for when the time alarm is generated
    RTC_SetAlarmInterrupt(AlarmIntrCallback);
    //****************************************************************

    // Initialize internal variables
    gEditMode = EDIT_MODE_NONE;
    gSelect = 0;
    gMosaic = 0;

    // LCD display start
    GX_DispOn();
    GXS_DispOn();

    // Debug string output
    OS_Printf("ARM9: RTC demo started.\n");

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
        // Read current status from RTC
        if (gEditMode != EDIT_MODE_DATE_TIME)
        {
            // Read date and time
            (void)RTC_GetDateTime(&gRtcDate, &gRtcTime);
        }
        if (gEditMode != EDIT_MODE_ALARM_1)
        {
            // Read alarm 1 status
            (void)RTC_GetAlarmStatus(RTC_ALARM_CHAN_1, &gRtcAlarmStatus1);
            if (gRtcAlarmStatus1 == RTC_ALARM_STATUS_ON)
            {
                // Read setting value if alarm 1 is ON
                (void)RTC_GetAlarmParam(RTC_ALARM_CHAN_1, &gRtcAlarmParam1);
            }
        }
        if (gEditMode != EDIT_MODE_ALARM_2)
        {
            // Read alarm 2 status
            (void)RTC_GetAlarmStatus(RTC_ALARM_CHAN_2, &gRtcAlarmStatus2);
            if (gRtcAlarmStatus2 == RTC_ALARM_STATUS_ON)
            {
                // Read setting value if alarm 2 is ON
                (void)RTC_GetAlarmParam(RTC_ALARM_CHAN_2, &gRtcAlarmParam2);
            }
        }
        //****************************************************************

        // Display date and time
        PrintString(7, 3, 0xf, "%04d/%02d/%02d/%s",
                    gRtcDate.year + 2000, gRtcDate.month, gRtcDate.day, StrWeek[gRtcDate.week]);
        PrintString(9, 5, 0xf, " %02d:%02d:%02d", gRtcTime.hour, gRtcTime.minute, gRtcTime.second);
        // Change color of selected location if in date and time edit mode.
        if (gEditMode == EDIT_MODE_DATE_TIME)
        {
            switch (gSelect)
            {
            case 0:
                ColorString(7, 3, 4, 0x1);
                break;
            case 1:
                ColorString(12, 3, 2, 0x1);
                break;
            case 2:
                ColorString(15, 3, 2, 0x1);
                break;
            case 3:
                ColorString(18, 3, 9, 0x1);
                break;
            case 4:
                ColorString(10, 5, 2, 0x1);
                break;
            case 5:
                ColorString(13, 5, 2, 0x1);
                break;
            case 6:
                ColorString(16, 5, 2, 0x1);
                break;
            }
        }

        // Display alarm 1 status and setting value
        PrintString(3, 8, 0x4, "ALARM-1 %s", StrAlarmStatus[gRtcAlarmStatus1]);
        PrintString(4, 10, 0xf, "% 9s  %02d:%02d  %s",
                    StrWeek[gRtcAlarmParam1.week],
                    gRtcAlarmParam1.hour,
                    gRtcAlarmParam1.minute, StrAlarmEnable[gRtcAlarmParam1.enable]);
        // Change color of selected location if in alarm 1 edit mode
        if (gEditMode == EDIT_MODE_ALARM_1)
        {
            switch (gSelect)
            {
            case 0:
                ColorString(11, 8, 6, 0x1);
                break;
            case 1:
                ColorString(4, 10, 9, 0x1);
                break;
            case 2:
                ColorString(15, 10, 2, 0x1);
                break;
            case 3:
                ColorString(18, 10, 2, 0x1);
                break;
            case 4:
                ColorString(22, 10, 5, 0x1);
                break;
            }
        }

        // Display alarm 2 status and setting value
        PrintString(3, 12, 0x4, "ALARM-2 %s", StrAlarmStatus[gRtcAlarmStatus2]);
        PrintString(4, 14, 0xf, "% 9s  %02d:%02d  %s",
                    StrWeek[gRtcAlarmParam2.week],
                    gRtcAlarmParam2.hour,
                    gRtcAlarmParam2.minute, StrAlarmEnable[gRtcAlarmParam2.enable]);
        // Change color of selected location if in alarm 2 edit mode
        if (gEditMode == EDIT_MODE_ALARM_2)
        {
            switch (gSelect)
            {
            case 0:
                ColorString(11, 12, 6, 0x1);
                break;
            case 1:
                ColorString(4, 14, 9, 0x1);
                break;
            case 2:
                ColorString(15, 14, 2, 0x1);
                break;
            case 3:
                ColorString(18, 14, 2, 0x1);
                break;
            case 4:
                ColorString(22, 14, 5, 0x1);
                break;
            }
        }

        // Display explanation of button operations
        if (gEditMode == EDIT_MODE_NONE)
        {
            PrintString(1, 19, 0x3, "DOWN  > Change Date & Time.");
            PrintString(1, 20, 0x3, "LEFT  > Change Alarm1 Setting.");
            PrintString(1, 21, 0x3, "RIGHT > Change Alarm2 Setting.");
        }
        else
        {
            PrintString(1, 18, 0x3, "UP    > Increment value.");
            PrintString(1, 19, 0x3, "DOWN  > Decrement value.");
            PrintString(1, 20, 0x3, "LEFT  > Move selector left.");
            PrintString(1, 21, 0x3, "RIGHT > Move selector right.");
            PrintString(1, 22, 0x3, "A     > Commit editing.");
            PrintString(1, 23, 0x3, "B     > Cancel editing.");
        }

        // Button input operation
        if (gEditMode == EDIT_MODE_NONE)
        {
            // Down-arrow key
            if ((gKey.trg | gKey.rep) & PAD_KEY_DOWN)
            {
                // Change to date and time edit mode
                gSelect = 0;
                gEditMode = EDIT_MODE_DATE_TIME;
            }
            // Left-arrow key
            if ((gKey.trg | gKey.rep) & PAD_KEY_LEFT)
            {
                // Change to alarm 1 status and setting value edit mode
                gSelect = 0;
                gEditMode = EDIT_MODE_ALARM_1;
            }
            // Right-arrow key
            if ((gKey.trg | gKey.rep) & PAD_KEY_RIGHT)
            {
                // Change to alarm 2 status and setting value edit mode
                gSelect = 0;
                gEditMode = EDIT_MODE_ALARM_2;
            }
        }
        else                           // Edit mode common key operation
        {
            // Up-arrow key
            if ((gKey.trg | gKey.rep) & PAD_KEY_UP)
            {
                IncValue();
            }
            // Down-arrow key
            if ((gKey.trg | gKey.rep) & PAD_KEY_DOWN)
            {
                DecValue();
            }
            // Left-arrow key
            if ((gKey.trg | gKey.rep) & PAD_KEY_LEFT)
            {
                DecSelect();
            }
            // Right-arrow key
            if ((gKey.trg | gKey.rep) & PAD_KEY_RIGHT)
            {
                IncSelect();
            }
            // A button
            if ((gKey.trg | gKey.rep) & PAD_BUTTON_A)
            {
                //****************************************************************
                // Write changed variable in RTC
                switch (gEditMode)
                {
                case EDIT_MODE_DATE_TIME:
                    // Write date and time
                    (void)RTC_SetDateTime(&gRtcDate, &gRtcTime);
                    break;
                case EDIT_MODE_ALARM_1:
                    // Write alarm 1 status
                    (void)RTC_SetAlarmStatus(RTC_ALARM_CHAN_1, &gRtcAlarmStatus1);
                    if (gRtcAlarmStatus1 == RTC_ALARM_STATUS_ON)
                    {
                        // Write setting value if alarm 1 is ON
                        (void)RTC_SetAlarmParam(RTC_ALARM_CHAN_1, &gRtcAlarmParam1);
                    }
                    break;
                case EDIT_MODE_ALARM_2:
                    // Write alarm 2 status
                    (void)RTC_SetAlarmStatus(RTC_ALARM_CHAN_2, &gRtcAlarmStatus2);
                    if (gRtcAlarmStatus2 == RTC_ALARM_STATUS_ON)
                    {
                        // Write setting value if alarm 2 is ON
                        (void)RTC_SetAlarmParam(RTC_ALARM_CHAN_2, &gRtcAlarmParam2);
                    }
                    break;
                }
                //****************************************************************

                // Change to no edit mode
                gEditMode = EDIT_MODE_NONE;
            }
            // B button
            if ((gKey.trg | gKey.rep) & PAD_BUTTON_B)
            {
                // Change to no edit mode
                gEditMode = EDIT_MODE_NONE;
            }
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

    // Display mosaic according to mosaic display counter
    if (gMosaic)
    {
        int     w;
        int     i = 5;

        w = gMosaic / 4 % (i * 2);
        w = w - (w - ((w - i) * 2)) * (w / (i + 1));
        gMosaic--;

        G2_BG0Mosaic(TRUE);
        G2_SetBGMosaicSize(w, w);
    }
    else
    {
        G2_BG0Mosaic(FALSE);
    }

    // Sets the IRQ check flag
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         AlarmIntrCallback

  Description:  Interrupt vector when RTC alarm occurs

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void AlarmIntrCallback(void)
{
    // Debug string output
    OS_Printf("ARM9: Alarm interrupt on %04d/%02d/%02d (%s) %02d:%02d:%02d .\n",
              gRtcDate.year + 2000,
              gRtcDate.month,
              gRtcDate.day,
              StrWeek[gRtcDate.week], gRtcTime.hour, gRtcTime.minute, gRtcTime.second);
    // Start mosaic animation
    gMosaic = 300;
}

/*---------------------------------------------------------------------------*
  Name:         IncValue

  Description:  Add 1 to selected value using edit mode.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void IncValue(void)
{
    switch (gEditMode)
    {

        // Date and time edit mode
    case EDIT_MODE_DATE_TIME:
        {
            RTCDate *d = &gRtcDate;
            RTCTime *t = &gRtcTime;

            switch (gSelect)
            {
            case 0:
                d->year = (d->year + 1) % 100;
                break;
            case 1:
                d->month = (d->month % 12) + 1;
                break;
            case 2:
                d->day = (d->day % 31) + 1;
                break;
            case 3:
                d->week = (RTCWeek)((d->week + 1) % RTC_WEEK_MAX);
                break;
            case 4:
                t->hour = (t->hour + 1) % 24;
                break;
            case 5:
                t->minute = (t->minute + 1) % 60;
                break;
            case 6:
                t->second = (t->second + 1) % 60;
                break;
            }
        }
        break;

        // Alarm 1 or 2 edit mode
    case EDIT_MODE_ALARM_1:
    case EDIT_MODE_ALARM_2:
        {
            RTCAlarmStatus *s;
            RTCAlarmParam *p;

            s = (gEditMode == EDIT_MODE_ALARM_1 ? &gRtcAlarmStatus1 : &gRtcAlarmStatus2);
            p = (gEditMode == EDIT_MODE_ALARM_1 ? &gRtcAlarmParam1 : &gRtcAlarmParam2);

            switch (gSelect)
            {
            case 0:
                *s = (RTCAlarmStatus)((*s + 1) % RTC_ALARM_STATUS_MAX);
                break;
            case 1:
                p->week = (RTCWeek)((p->week + 1) % RTC_WEEK_MAX);
                break;
            case 2:
                p->hour = (p->hour + 1) % 24;
                break;
            case 3:
                p->minute = (p->minute + 1) % 60;
                break;
            case 4:
                p->enable = (p->enable + 1) % (RTC_ALARM_ENABLE_ALL + 1);
                break;
            }
        }
        break;

    }
}

/*---------------------------------------------------------------------------*
  Name:         DecValue

  Description:  Subtract 1 from selected value using edit mode.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DecValue(void)
{
    switch (gEditMode)
    {

        // Date and time edit mode
    case EDIT_MODE_DATE_TIME:
        {
            RTCDate *d = &gRtcDate;
            RTCTime *t = &gRtcTime;

            switch (gSelect)
            {
            case 0:
                d->year = (d->year + 99) % 100;
                break;
            case 1:
                d->month = ((d->month + 10) % 12) + 1;
                break;
            case 2:
                d->day = ((d->day + 29) % 31) + 1;
                break;
            case 3:
                d->week = (RTCWeek)((d->week + (RTC_WEEK_MAX - 1)) % RTC_WEEK_MAX);
                break;
            case 4:
                t->hour = (t->hour + 23) % 24;
                break;
            case 5:
                t->minute = (t->minute + 59) % 60;
                break;
            case 6:
                t->second = (t->second + 59) % 60;
                break;
            }
        }
        break;

        // Alarm 1 or 2 edit mode
    case EDIT_MODE_ALARM_1:
    case EDIT_MODE_ALARM_2:
        {
            RTCAlarmStatus *s;
            RTCAlarmParam *p;

            s = (gEditMode == EDIT_MODE_ALARM_1 ? &gRtcAlarmStatus1 : &gRtcAlarmStatus2);
            p = (gEditMode == EDIT_MODE_ALARM_1 ? &gRtcAlarmParam1 : &gRtcAlarmParam2);

            switch (gSelect)
            {
            case 0:
                *s = (RTCAlarmStatus)((*s + (RTC_ALARM_STATUS_MAX - 1)) % RTC_ALARM_STATUS_MAX);
                break;
            case 1:
                p->week = (RTCWeek)((p->week + (RTC_WEEK_MAX - 1)) % RTC_WEEK_MAX);
                break;
            case 2:
                p->hour = (p->hour + 23) % 24;
                break;
            case 3:
                p->minute = (p->minute + 59) % 60;
                break;
            case 4:
                p->enable = (p->enable + RTC_ALARM_ENABLE_ALL) % (RTC_ALARM_ENABLE_ALL + 1);
                break;
            }
        }
        break;

    }
}

/*---------------------------------------------------------------------------*
  Name:         IncSelect

  Description:  Add 1 to selection index using edit mode.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void IncSelect(void)
{
    switch (gEditMode)
    {

        // Date and time edit mode
    case EDIT_MODE_DATE_TIME:
        gSelect = (gSelect + 1) % 7;
        break;

        // Alarm 1 or 2 edit mode
    case EDIT_MODE_ALARM_1:
    case EDIT_MODE_ALARM_2:
        gSelect = (gSelect + 1) % 5;
        break;

    }
}

/*---------------------------------------------------------------------------*
  Name:         DecSelect

  Description:  Subtract 1 from selection index by edit mode.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DecSelect(void)
{
    switch (gEditMode)
    {

        // Date and time edit mode
    case EDIT_MODE_DATE_TIME:
        gSelect = (gSelect + 6) % 7;
        break;

        // Alarm 1 or 2 edit mode
    case EDIT_MODE_ALARM_1:
    case EDIT_MODE_ALARM_2:
        gSelect = (gSelect + 4) % 5;
        break;

    }
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

  Arguments:    x: X-oordinate where character string starts (x 8 dots)
                y: Y-coordinate where character string starts (x 8 dots)
                palette: Specify text color by palette number
                text: Text string to position. NULL terminated
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
