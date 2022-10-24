/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - demos - alarm-2
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
#define  ALARM_COUNT_P1  (OS_MilliSecondsToTicks(400))
#define  ALARM_COUNT_P2  (OS_MilliSecondsToTicks(500))

#define  TEST_DATA       0x12345678
#define  TEST_DATA2      0x9abcdef0

static OSAlarm alarm;
static OSAlarm alarm2;

static BOOL called = FALSE;
static BOOL alarm2_sw = FALSE;

static int pushCounter = 0;
static int pushCounter2 = 0;

void    alarmCallback(void *arg);
void    alarmCallback2(void *arg);
void    VBlankIntr(void);

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void NitroMain()
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

    OS_Printf("Push [A]  to start alarm.\n");
    OS_Printf("Push [UP] to start periodic alarm. ([DOWN] to stop)\n");

    while (1)
    {
		u16 padData;
		u16 trigger;
		static u16 prePadData = 0;

        OS_WaitVBlankIntr();
        padData = PAD_Read();
		trigger = (u16)((padData ^ prePadData) & padData);
		prePadData = padData;

        //---- push A to start alarm
        if (trigger & PAD_BUTTON_A && !called)
        {
            pushCounter++;
            called = TRUE;

            //---- setting alarm
            OS_CreateAlarm(&alarm);
            OS_SetAlarm(&alarm, ALARM_COUNT, &alarmCallback, (void *)pushCounter);

            OS_Printf("set alarm\n");
        }


        //---- push UP to start periodic alarm
        if (trigger & PAD_KEY_UP && !alarm2_sw)
        {
            pushCounter2++;
            alarm2_sw = TRUE;

            //---- setting periodic alarm
            OS_CreateAlarm(&alarm2);
            OS_SetPeriodicAlarm(&alarm2, OS_GetTick() + ALARM_COUNT_P1, ALARM_COUNT_P2,
                                &alarmCallback2, (void *)(1000 + pushCounter2));

            OS_Printf("set periodic alarm\n");
        }

        //---- push DOWN to stop periodic alarm
        if (trigger & PAD_KEY_DOWN && alarm2_sw)
        {
            alarm2_sw = FALSE;

            OS_CancelAlarm(&alarm2);
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
  Name:         alarmCallback2

  Description:  callback for alarm interrupt, for periodic alarm

  Arguments:    arg : user setting argument

  Returns:      None
 *---------------------------------------------------------------------------*/
void alarmCallback2(void *arg)
{
#ifdef SDK_FINALROM
#pragma unused( arg )
#endif
    OS_Printf(">>> called periodic alarm callback. arg=%d  TICK=%llx\n", arg, OS_GetTick());
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
