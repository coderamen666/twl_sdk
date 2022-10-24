/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - demos - alarm-1
  File:     main.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-04-01#$
  $Rev: 5205 $
  $Author: yada $
 *---------------------------------------------------------------------------*/
#include <nitro.h>

#define  ALARM_COUNT     (OS_MilliSecondsToTicks(1000))

static OSAlarm alarm;
static BOOL called = FALSE;
static int pushCounter = 0;

void    VBlankIntr(void);
void    alarmCallback(void *arg);

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    OS_Init();
    GX_Init();

    // ---- initialize tick system
    OS_InitTick();
    // ---- initialize alarm system
    OS_InitAlarm();

    //---- setup VBlank
    (void)OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();
    (void)GX_VBlankIntr(TRUE);

    OS_Printf("Push [A] to start alarm.\n");

    while (1)
    {
		u16 padData;

        OS_WaitVBlankIntr();
        padData = PAD_Read();

        if (padData & PAD_BUTTON_A && !called)
        {
            pushCounter++;
            called = TRUE;

            //---- setting alarm
            OS_CreateAlarm(&alarm);
            OS_SetAlarm(&alarm, ALARM_COUNT, &alarmCallback, (void *)pushCounter);

            OS_Printf("set alarm\n");
        }
    }
}


/*---------------------------------------------------------------------------*
  Name:         alarmCallback

  Description:  callback for alarm interrupt

  Arguments:    arg : user setting argument

  Returns:      None
 *---------------------------------------------------------------------------*/
void alarmCallback(void *arg)
{
#ifdef SDK_FINALROM
#pragma unused( arg )
#endif
    OS_Printf(">>> called alarm callback. arg=%d\n", arg);
    called = FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  VBlank interrupt handler

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void VBlankIntr(void)
{
    //---- check interrupt flag
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*====== End of main.c ======*/
