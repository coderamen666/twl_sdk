/*---------------------------------------------------------------------------*
  Project:  TwlSDK - demos - mi - wramManager-1
  File:     main.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-12-09#$
  $Rev: 9590 $
  $Author: yada $
 *---------------------------------------------------------------------------*/
#include <twl.h>

void VBlankIntr(void);
void DisplayWramStatus(void);

/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
	OS_Init();

	OS_Printf("*** start wramManager-1 demo\n");

	//---- interrupt setting
	OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
	(void)OS_EnableIrqMask(OS_IE_V_BLANK);
	(void)OS_EnableIrq();
	(void)GX_VBlankIntr(TRUE);

	//---- set free shared WRAM allocated to ARM9
	(void)MI_FreeWram_B( MI_WRAM_ARM9 );
	(void)MI_CancelWram_B( MI_WRAM_ARM9 );
	(void)MI_FreeWram_C( MI_WRAM_ARM9 );
	(void)MI_CancelWram_C( MI_WRAM_ARM9 );

	//---- display each WRAM area
	OS_Printf("WRAM-A (%x-%x)\n",    MI_GetWramMapStart_A(),  MI_GetWramMapEnd_A() );
	OS_Printf("WRAM-B (%x-%x)\n",    MI_GetWramMapStart_B(),  MI_GetWramMapEnd_B() );
	OS_Printf("WRAM-C (%x-%x)\n\n",  MI_GetWramMapStart_C(),  MI_GetWramMapEnd_C() );
	DisplayWramStatus();

	while(1)
	{
		{
			static u16 preButton = 0;
			u16 button = PAD_Read();
			u16 trigger;

			trigger = (u16)( (preButton ^ button) & button );
			preButton = button;

			if ( trigger & PAD_BUTTON_A )
			{
				static int i=0;

				OS_Printf("*** (%d) ", i);
				switch(i)
				{
					case 0:
						{
							u32 addr = MI_AllocWram_B( MI_WRAM_SIZE_64KB, MI_WRAM_ARM9 );
							OS_Printf("alloc for ARM9 from WRAM_B: addr=%x\n", addr);
						}
						break;
					case 1:
						{
							u32 addr = MI_AllocWram_B( MI_WRAM_SIZE_64KB, MI_WRAM_ARM7 );
							OS_Printf("alloc for ARM7 from WRAM_B: addr=%x\n", addr);
						}
						break;
					case 2:
						{
							u32 addr = MI_AllocWram_B( MI_WRAM_SIZE_64KB, MI_WRAM_DSP );
							OS_Printf("alloc for DSP from WRAM_B: addr=%x\n", addr);
						}
						break;
					case 3:
						{
							int x = MI_FreeWram_B( MI_WRAM_ARM7 );
							OS_Printf("free %d blocks all ARM7 slot from WRAM_B\n", x );
						}
						break;
					case 4:
						{
							u32 addr = MI_AllocWram_B( MI_WRAM_SIZE_64KB, MI_WRAM_ARM9 );
							OS_Printf("alloc for ARM9 from WRAM_B: addr=%x\n", addr);
						}
						break;
					case 5:
						{
							int x = MI_FreeWram_B( MI_WRAM_ARM9 );
							OS_Printf("free %d blocks all ARM9 slot from WRAM_B\n", x );
						}
						break;
					case 6:
						{
							u32 addr = MI_AllocWramSlot_B( 1, MI_WRAM_SIZE_64KB, MI_WRAM_ARM9 );
							OS_Printf("alloc for ARM9 from WRAM_B: addr=%x (64KB from slot 1)\n", addr );
						}
						break;
					case 7:
						{
							int x = MI_SwitchWramSlot_B( 2, MI_WRAM_SIZE_64KB, MI_WRAM_ARM9, MI_WRAM_DSP );
							OS_Printf("switch %d blocks in WRAM_B from ARM9 to DSP (64KB from slot 2)\n", x );
						}
						break;
				}
				i++;

				DisplayWramStatus();
				if ( i>7 )
				{
					OS_Printf("*** End of demo\n");
					OS_Terminate();
				}
			}
		}
	}
}

/*---------------------------------------------------------------------------*
  Name:         DisplayWramStatus

  Description:  display status of wram manager

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static char* getMasterStr( MIWramProc m );
static char* getEnableStr( MIWramEnable en );
static char* getBoolStr( BOOL b );

void DisplayWramStatus(void)
{
	int n;

	//---- WRAM-A
	OS_Printf("WRAM-A:reserve");
	for( n=0; n<MI_WRAM_A_MAX_NUM; n++ )
	{
		OS_Printf("[%s]", getMasterStr( MI_GetWramReservation_A(n) ) );
	}
	OS_Printf("\nWRAM-A:alloc  ");
	for( n=0; n<MI_WRAM_A_MAX_NUM; n++ )
	{
		if ( MI_GetWramBankEnable_A(n) )
		{
			OS_Printf("[%s]", getMasterStr(MI_GetWramBankMaster_A(n)) );
		}
		else
		{
			OS_Printf("[----]");
		}
	}

	//---- WRAM-B
	OS_Printf("\nWRAM-B:reserve");
	for( n=0; n<MI_WRAM_B_MAX_NUM; n++ )
	{
		OS_Printf("[%s]", getMasterStr( MI_GetWramReservation_B(n) ) );
	}
	OS_Printf("\nWRAM-B:alloc  ");
	for( n=0; n<MI_WRAM_B_MAX_NUM; n++ )
	{
		if ( MI_GetWramBankEnable_B(n) )
		{
			OS_Printf("[%s]", getMasterStr(MI_GetWramBankMaster_B(n)) );
		}
		else
		{
			OS_Printf("[----]");
		}
	}

	//---- WRAM-C
	OS_Printf("\nWRAM-C:reserve");
	for( n=0; n<MI_WRAM_C_MAX_NUM; n++ )
	{
		OS_Printf("[%s]", getMasterStr( MI_GetWramReservation_C(n) ) );
	}
	OS_Printf("\nWRAM-C:alloc  ");
	for( n=0; n<MI_WRAM_C_MAX_NUM; n++ )
	{
		if ( MI_GetWramBankEnable_C(n) )
		{
			OS_Printf("[%s]", getMasterStr(MI_GetWramBankMaster_C(n)) );
		}
		else
		{
			OS_Printf("[----]");
		}
	}
	OS_Printf("\n\n");

}

static char* getMasterStr( MIWramProc m )
{
	static char strARM9[] = "ARM9";
	static char strARM7[] = "ARM7";
	static char strDSP[]  = "DSP ";
	static char strNA[]   = "----";

	switch( m )
	{
		case MI_WRAM_ARM9: return strARM9;
		case MI_WRAM_ARM7: return strARM7;
		case MI_WRAM_DSP:  return strDSP;
	}
	return strNA;
}

static char* getEnableStr( MIWramEnable en )
{
	static char strEnable[]  = "o";
	static char strDisable[] = "x";
	static char strNA[]      = "-";

	switch( en )
	{
		case MI_WRAM_ENABLE:   return strEnable;
		case MI_WRAM_DISABLE:  return strDisable;
	}
	return strNA;
}


static char* getBoolStr( BOOL b )
{
	static char strTrue[]  = "o";
	static char strFalse[] = "x";

	return b? strTrue: strFalse;
}



/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  interrupt handler of V-Blank

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void VBlankIntr(void)
{
	OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}
