/*---------------------------------------------------------------------------*
  Project:  TwlSDK - PAD - demos - padRead
  File:     main.c

  Copyright 2007-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04 #$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif

#include "DEMO.h"


/*---------------------------------------------------------------------------*
  Name:         TwlMain / NitroMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplayBitmap();
    DEMOHookConsole();

    DEMOStartDisplay();
    DEMOSetBitmapGroundColor(DEMO_RGB_CLEAR);

    for (;;)
    {
        /* *INDENT-OFF* */
        typedef struct PadAssignment
        {
            int         posX;
            int         posY;
            int         bit;
            const char *name;
        }
        PadAssignment;
        static const PadAssignment assignment_table[] =
        {
            { 200,  90, PAD_BUTTON_A,         "A", },
            { 180,  95, PAD_BUTTON_B,         "B", },
            { 110,  95, PAD_BUTTON_SELECT,    "S", },
            { 130,  95, PAD_BUTTON_START,     "S", },
            {  70,  90, PAD_KEY_RIGHT,        "R", },
            {  50,  90, PAD_KEY_LEFT,         "L", },
            {  60,  80, PAD_KEY_UP,           "U", },
            {  60, 100, PAD_KEY_DOWN,         "D", },
            { 180,  50, PAD_BUTTON_R,         "R", },
            {  60,  50, PAD_BUTTON_L,         "L", },
            { 200,  75, PAD_BUTTON_X,         "X", },
            { 180,  80, PAD_BUTTON_Y,         "Y", },
            { 200, 120, PAD_BUTTON_DEBUG,     "D", },
        };
        /* *INDENT-ON* */
        static const u32 assignment_table_max = sizeof(assignment_table) / sizeof(*assignment_table);
        int     i;
        u16     bits;

        DEMOFillRect(0, 0, GX_LCD_SIZE_X, GX_LCD_SIZE_Y, GX_RGBA( 0, 0, 0, 1));
        bits = PAD_Read();
        for (i = 0; i < assignment_table_max; ++i)
        {
            const PadAssignment *p = &assignment_table[i];
            DEMOSetBitmapTextColor((GXRgb)(((bits & p->bit) != 0) ? GX_RGBA(31, 31, 0, 1) : GX_RGBA(16, 16, 16, 1)));
            DEMODrawText(p->posX, p->posY, "%s", p->name);
        }
        DEMOSetBitmapTextColor(GX_RGBA(31, 31, 31, 1));
        DEMODrawText(10, 0, "PAD_DetectFold:%s", PAD_DetectFold() ? "true" : "false");
        DEMO_DrawFlip();

        OS_WaitVBlankIntr();
    }
}

/*====== End of main.c ======*/
