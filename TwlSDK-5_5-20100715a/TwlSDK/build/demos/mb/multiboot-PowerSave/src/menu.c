/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - multiboot-PowerSave
  File:     menu.c

  Copyright 2006-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/


#ifdef SDK_TWL
#include	<twl.h>
#else
#include	<nitro.h>
#endif


#include "common.h"
#include "dispfunc.h"


/*****************************************************************************/
/* Declaration */

static void MenuInit(void);
static void MenuUpdate(void);
static void MenuDraw(void);


/*****************************************************************************/
/* Constants */

enum
{
    MENU_POWERSAVE,
    MENU_NUM
};


/*****************************************************************************/
/* Variables */

static struct
{
    int     cursor;
    BOOL    loop;
}
menu_context;


/*****************************************************************************/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         MenuInit

  Description:  Initializes the state of the DS Single-Card Play initial settings menu.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MenuInit(void)
{
    menu_context.cursor = 0;
    menu_context.loop = TRUE;

    /* Initializes display settings */
    GX_DispOn();
    GXS_DispOn();
    G2_SetBG0Offset(0, 0);
    G2S_SetBG0Offset(0, 0);
}

/*---------------------------------------------------------------------------*
  Name:         MenuUpdate

  Description:  Updates the state of the DS Single-Card Play initial settings menu.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MenuUpdate(void)
{
    const u16 keyData = ReadKeySetTrigger();

    /* Change cursor number */
    if ((keyData & PAD_KEY_DOWN) != 0)
    {
        if (++menu_context.cursor >= MENU_NUM)
            menu_context.cursor -= MENU_NUM;
    }
    else if ((keyData & PAD_KEY_UP) != 0)
    {
        if (--menu_context.cursor < 0)
            menu_context.cursor += MENU_NUM;
    }
    else if ((keyData & (PAD_BUTTON_A | PAD_BUTTON_START)) != 0)
    {
        menu_context.loop = FALSE;
    }

    switch (menu_context.cursor)
    {
    case MENU_POWERSAVE:
        /* Configure power-save mode */
        if ((keyData & (PAD_KEY_RIGHT | PAD_KEY_LEFT)) != 0)
        {
            g_power_save_mode = !g_power_save_mode;
        }
        break;
    }
}

/*---------------------------------------------------------------------------*
  Name:         MenuDraw

  Description:  Draws the state of the DS Single-Card Play initial settings menu.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MenuDraw(void)
{
    enum
    { DISP_OX = 5, DISP_OY = 5 };
    BgClear();
    BgPutString(DISP_OX + 3, DISP_OY - 3, WHITE, "Multiboot Sample", 32);
    BgPrintf(DISP_OX + 0, DISP_OY + 0, WHITE,
             "Power Save       %s", g_power_save_mode ? "[TRUE]" : "[FALSE]");
    BgPutChar(DISP_OX - 2, DISP_OY + menu_context.cursor, RED, '*');
    BgPutString(DISP_OX, DISP_OY + 14, WHITE, " Press A or START Button", 32);
}

/*---------------------------------------------------------------------------*
  Name:         MenuMode

  Description:  Initial settings menu for the DS Single-Card parent device.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MenuMode(void)
{
    MenuInit();
    while (menu_context.loop)
    {
        MenuUpdate();
        MenuDraw();
        OS_WaitVBlankIntr();
    }
}
