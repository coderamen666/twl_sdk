/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - demos - cpuSpeed-1
  File:     main.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-12-09#$
  $Rev: 9589 $
  $Author: yada $
 *---------------------------------------------------------------------------*/
#include <twl.h>

volatile int vCount = 0;
int fCount = 0;

void VBlankIntr(void);


/*---------------------------------------------------------------------------*
  Name:         TwlMain / NitroMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void TwlMain(void)
{
	SCFGCpuSpeed speed = SCFG_GetCpuSpeed();

	OS_Init();

	//---- interrupt setting
	OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
	(void)OS_EnableIrqMask(OS_IE_V_BLANK);
	(void)OS_EnableIrq();
	(void)GX_VBlankIntr(TRUE);

	OS_Printf("*** start cpuSpeed-1 demo\n");

	while( fCount <= 30 )
	{
		int count;
		int preVCount;

		count = preVCount = vCount = 0;
		while( vCount <= 60 )
		{
			if ( preVCount != vCount )
			{
				static u16 prevButton = 0;
				u16 button = PAD_Read();
				u16 trigger = (u16)((button ^ prevButton) & button);
				prevButton = button;

				if ( trigger & PAD_BUTTON_A )
				{
					speed = (speed == SCFG_CPU_SPEED_1X)? SCFG_CPU_SPEED_2X: SCFG_CPU_SPEED_1X;
					SCFG_SetCpuSpeed( speed );
					OS_Printf("speed: %s\n", (speed == SCFG_CPU_SPEED_1X)? "1X": "2X" );
				}
				preVCount = vCount;
			}

			count ++;
		}

		OS_Printf( "count= %d\n", count );

		fCount ++;
	}

	OS_Printf("***End of demo\n");
	OS_Terminate();
}


/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void VBlankIntr(void)
{
	vCount ++;

	OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

