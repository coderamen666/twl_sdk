/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - demos - valarm-1
  File:     main.c

  Copyright 2008 Nintendo.  All rights reserved.

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

void    v_alarmCallback1(void *arg);
void    v_alarmCallback2(void *arg);

void    VBlankIntr(void);

static OSVAlarm v_alarm1;
static OSVAlarm v_alarm2;
static OSVAlarm v_alarm3;
static OSVAlarm pvAlarm1;
static OSVAlarm pvAlarm2;


static int num_valarms = 0;
static BOOL pvAlarm1_sw = FALSE;
static BOOL pvAlarm2_sw = FALSE;

static u16 keyData;
static u16 preData;
static u16 trigger;

//================================================================================
/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void NitroMain()
{
    OS_Init();
#ifdef SDK_ARM9
    GX_Init();
#endif

    //---- init tick system
    OS_InitTick();

    //---- init alarm system (but OS_InitVAlarm called in OS_Init, so not necessary)
    OS_InitAlarm();
    //OS_InitVAlarm(); /*called in OS_Init()*/

    //---- setting vblank
    (void)OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();
    (void)GX_VBlankIntr(TRUE);

    //---- Dummy READ workaround for pushing 'A' at IPL
    preData = PAD_Read();

    OS_Printf("Push [A]    to start 3 valarms.\n");
    OS_Printf("Push [UP]   to start periodic valarm1. ([DOWN]  to stop)\n");
    OS_Printf("Push [LEFT] to start periodic valarm2. ([RIGHT] to stop)\n");

    //---- setting vblank
    while (1)
    {
        OS_WaitVBlankIntr();

        keyData = PAD_Read();
        trigger = (u16)(keyData & (keyData ^ preData));
        preData = keyData;

        //---- PUSH A to start valarm
        if (trigger & PAD_BUTTON_A && num_valarms == 0)
        {
            num_valarms = 3;

            OS_Printf("set 3 alarms");
            //---- setting alarm
            OS_CreateVAlarm(&v_alarm1);
            OS_SetVAlarm(&v_alarm1, 100, OS_VALARM_DELAY_MAX, &v_alarmCallback1, (void *)1);

            OS_CreateVAlarm(&v_alarm2);
            OS_SetVAlarm(&v_alarm2, 150, OS_VALARM_DELAY_MAX, &v_alarmCallback1, (void *)2);

            OS_CreateVAlarm(&v_alarm3);
            OS_SetVAlarm(&v_alarm3, 250, 50, &v_alarmCallback1, (void *)3);

            OS_Printf("set valarms\n");
        }

        //---- PUSH UP to start periodic valarm1
        if (trigger & PAD_KEY_UP && !pvAlarm1_sw)
        {
            pvAlarm1_sw = TRUE;

            //---- setting periodic valarm
            OS_CreateVAlarm(&pvAlarm1);
            OS_SetPeriodicVAlarm(&pvAlarm1, 120, 10, &v_alarmCallback2, (void *)10);

            OS_Printf("set periodic valarm 1\n");
        }

        //---- PUSH DOWN to stop periodic valarm1
        if (trigger & PAD_KEY_DOWN && pvAlarm1_sw)
        {
            pvAlarm1_sw = FALSE;
            OS_CancelVAlarm(&pvAlarm1);

            OS_Printf("stop periodic valarm 1\n");
        }


        //---- PUSH LEFT to start periodic valarm2
        if (trigger & PAD_KEY_LEFT && !pvAlarm2_sw)
        {
            pvAlarm2_sw = TRUE;

            //---- setting periodic valarm
            OS_CreateVAlarm(&pvAlarm2);
            OS_SetPeriodicVAlarm(&pvAlarm2, 121, 50, &v_alarmCallback2, (void *)20);

            OS_Printf("set periodic valarm 2\n");
        }

        //---- PUSH RIGHT to stop periodic valarm2
        if (trigger & PAD_KEY_RIGHT && pvAlarm2_sw)
        {
            pvAlarm2_sw = FALSE;
            OS_CancelVAlarm(&pvAlarm2);

            OS_Printf("stop periodic valarm 2\n");
        }
    }
}


//----------------------------------------------------------------
//  V-Alarm callback
//
void v_alarmCallback1(void *arg)
{
#ifdef SDK_FINALROM
#pragma unused( arg )
#endif
    OS_Printf("[%x]>>> VLINE=%d\n", arg, GX_GetVCount());
    num_valarms--;

#ifdef	TEST
    OS_SpinWait(66000000 / 60 * 30 / 263);
#endif
}

//----------------------------------------------------------------
//  V-Alarm callback
//
void v_alarmCallback2(void *arg)
{
#ifdef SDK_FINALROM
#pragma unused( arg )
#endif
    OS_Printf("[%d] VLINE=%d, TICK=%llx\n", arg, GX_GetVCount(), OS_GetTick());
}


//----------------------------------------------------------------
//  VBlank interrupt handler
void VBlankIntr(void)
{
#ifdef	TEST
    s32     before = GX_GetVCount();
#endif

    //---- check interrupt flag
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);

#ifdef	TEST
    if (PAD_Read() & PAD_BUTTON_X)
    {
        OS_SpinWait(66000000 / 60 * 80 / 263);
    }
    OS_Printf("VB VLINE=%d-%d\n", before, GX_GetVCount());
#endif
}

/*====== End of main.c ======*/
