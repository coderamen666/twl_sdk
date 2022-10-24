/*---------------------------------------------------------------------------*
  Project:  TwlSDK - RTC - demos - convert-1
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
  Demo to confirm operation of RTC conversion function
 *---------------------------------------------------------------------------*/

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif
#include    <string.h>


static void VBlankIntr(void);
static void DisplayInit(void);
static void FillScreen(u16 col);
static BOOL ConvTest(void);
static void FormatDateTime(char *str, const RTCDate *date, const RTCTime *time);

/*---------------------------------------------------------------------------*
    Variable Definitions
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    Function Definitions
 *---------------------------------------------------------------------------*/

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

    DisplayInit();

    if (ConvTest())
    {
        // Success
        OS_TPrintf("------ Test Succeeded ------\n");
        FillScreen(GX_RGB(0, 31, 0));
    }
    else
    {
        // Failed
        OS_TPrintf("****** Test Failed ******\n");
        FillScreen(GX_RGB(31, 0, 0));
    }

    // Main loop
    while (TRUE)
    {
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
    // Sets the IRQ check flag
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         DisplayInit

  Description:  Graphics initialization.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DisplayInit(void)
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

    GX_SetGraphicsMode(GX_DISPMODE_VRAM_C,      // VRAM mode
                       (GXBGMode)0,    // Dummy
                       (GXBG0As)0);    // Dummy

    GX_SetVisiblePlane(GX_PLANEMASK_OBJ);       // Make OBJs visible
    GX_SetOBJVRamModeBmp(GX_OBJVRAMMODE_BMP_1D_128K);   // 2D mapping OBJ

    OS_WaitVBlankIntr();               // Waiting for the end of the V-Blank interrupt
    GX_DispOn();

}


/*---------------------------------------------------------------------------*
  Name:         FillScreen

  Description:  Fills the screen.

  Arguments:    col: FillColor

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void FillScreen(u16 col)
{
    MI_CpuFill16((void *)HW_LCDC_VRAM_C, col, 256 * 192 * 2);
}

/*---------------------------------------------------------------------------*
  Name:         ConvTest

  Description:  RTC conversion function test routine.

  Arguments:    None.

  Returns:      TRUE if test succeeds.
 *---------------------------------------------------------------------------*/
static char *sWeekName[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
#define PrintResultEq( a, b, f ) \
    { OS_TPrintf( ((a) == (b)) ? "[--OK--] " : "[**NG**] " ); (f) = (f) && ((a) == (b)); }
#define PrintResultStringEq( a, b, f ) \
    { OS_TPrintf( (strcmp((a), (b)) == 0) ? "[--OK--] " : "[**NG**] " ); (f) = (f) && (strcmp((a), (b)) == 0); }

static BOOL ConvTest(void)
{
    int     i;
    BOOL    flag = TRUE;

    {
        s64     a[] = {
            0, 3155759999LL, -1, 3155760000LL,
            5097600, 5184000, 154335600,
        };
        char   *result_str[] = {
            "2000/01/01(Sat) 00:00:00", "2099/12/31(Thu) 23:59:59",
            "2000/01/01(Sat) 00:00:00", "2099/12/31(Thu) 23:59:59",
            "2000/02/29(Tue) 00:00:00", "2000/03/01(Wed) 00:00:00",
            "2004/11/21(Sun) 07:00:00",
        };
        s64     result_sec[] = {
            0, 3155759999LL, 0, 3155759999LL,
            5097600, 5184000, 154335600,
        };
        for (i = 0; i < sizeof(a) / sizeof(a[0]); i++)
        {
            RTCDate date;
            RTCTime time;
            char    datestr[64];
            s64     result;

            RTC_ConvertSecondToDateTime(&date, &time, a[i]);
            FormatDateTime(datestr, &date, &time);
            PrintResultStringEq(datestr, result_str[i], flag);
            OS_TPrintf("RTC_ConvertSecondToDateTime(%lld) = %s\n", a[i], datestr);
            result = RTC_ConvertDateTimeToSecond(&date, &time);
            PrintResultEq(result, result_sec[i], flag);
            OS_TPrintf("RTC_ConvertDateTimeToSecond(%s) = %lld\n", datestr, result);
        }
    }


    return flag;
}

static void FormatDateTime(char *str, const RTCDate *date, const RTCTime *time)
{
    (void)OS_SPrintf(str, "%04d/%02d/%02d(%s) %02d:%02d:%02d",
                     date->year + 2000, date->month, date->day, sWeekName[date->week],
                     time->hour, time->minute, time->second);
}

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
