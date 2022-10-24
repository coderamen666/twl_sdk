/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - demos - simple-1
  File:     main.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-12-08#$
  $Rev: 9555 $
  $Author: kitase_hirotake $
 *---------------------------------------------------------------------------*/
#include <nitro.h>

#define COUNT OS_MilliSecondsToTicks( 1000 )

OSAlarm alarm;
int alarmCount = 0;

void alarmCallback(void* arg);
void VBlankIntr(void);


/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
	OS_Init();
	OS_InitTick();
	OS_InitAlarm();

	//---- interrupt setting
	OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
	(void)OS_EnableIrqMask(OS_IE_V_BLANK);
	(void)OS_EnableIrq();
	(void)GX_VBlankIntr(TRUE);

	OS_Printf("*** start simple-1 demo\n");

	OS_SetPeriodicAlarm( &alarm, COUNT, COUNT, alarmCallback, NULL );

	while( alarmCount <= 10 )
	{
		static u16 prevButton = 0;
		u16 button = PAD_Read();
		u16 trigger = (u16)((button ^ prevButton) & button);
		prevButton = button;

        SVC_WaitVBlankIntr();

		if ( trigger & PAD_BUTTON_A )
		{
			OS_Printf("pushed A.\n");
		}
	}

	OS_Printf("***End of demo\n");
	OS_Terminate();
}


/*---------------------------------------------------------------------------*
  Name:         alarmCallback

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void alarmCallback(void* arg)
{
#pragma unused(arg)
	OS_Printf( "alarm callback (%d)\n", alarmCount );
	alarmCount ++;
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void VBlankIntr(void)
{
	OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

