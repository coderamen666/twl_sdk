/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - demos - timer-1
  File:     main.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-04-01#$
  $Rev: 5208 $
  $Author: yada $
 *---------------------------------------------------------------------------*/
#include <nitro.h>
#include "data.h"

#define MY_COUNT1   (33514000/1024/10)
#define MY_SCALE1   OS_TIMER_PRESCALER_1024

#define MY_COUNT2   (33514000/256)
#define MY_SCALE2   OS_TIMER_PRESCALER_256

void    MyInit(void);
void    DisplayCounter( int objNo, int y, u32 count );
void    VBlankIntr(void);
void    SetObj(int objNo, int x, int y, int charNo, int paletteNo);

void    SetTimer_1(void);
void    TimerIntr_1(void);

void    SetTimer_2(void);
void    TimerIntr_2(void);

GXOamAttr oamBak[128];
volatile u32 timerCnt1 = 0;
volatile u32 timerCnt2 = 0;
BOOL pauseFlag = FALSE;

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void NitroMain()
{
	MyInit();

    //---- set TIMER
    SetTimer_1();
    SetTimer_2();

    while (1)
    {
		static u16 preButton = 0;
        u16  button = PAD_Read();
		u16 trigger = (u16)((button ^ preButton) & button);
		preButton = button;

        //---- wait VBlank
        OS_WaitVBlankIntr();

		//---- display counter
		DisplayCounter(0, 100, timerCnt1);
		DisplayCounter(8, 140, timerCnt2);

        //---- button A to enable/disable TIMER
        if (trigger & PAD_BUTTON_A)
        {
			if ( pauseFlag )
			{
				(void)OS_EnableIrqMask(OS_IE_TIMER0);
				(void)OS_EnableIrqMask(OS_IE_TIMER2);
				pauseFlag = FALSE;
			}
			else
			{
				(void)OS_DisableIrqMask(OS_IE_TIMER0);
				(void)OS_DisableIrqMask(OS_IE_TIMER2);
				pauseFlag = TRUE;
			}
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         MyInit

  Description:  initialize (about graphics mainly)

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void MyInit(void)
{
    OS_Init();
    GX_Init();

    //---- set power on
    GX_SetPower(GX_POWER_ALL);

    //----  enable VBlank interrupt
    (void)OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();

    //---- setting VBlank
    (void)GX_VBlankIntr(TRUE);

    //---- clae VRAM
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
    MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);
    (void)GX_DisableBankForLCDC();

    //---- clear OAM and palette
    MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);
    MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);

    //---- allocate bank A to OBJ
    GX_SetBankForOBJ(GX_VRAM_OBJ_128_A);

    //---- set to graphics display mode
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D);

    //---- set OBJ be visible
    GX_SetVisiblePlane(GX_PLANEMASK_OBJ);

    //---- 32KByte OBJ, 2D map mode
    GX_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_2D);

    //---- load character and palette data
    MI_DmaCopy32(3, sampleCharData, (void *)HW_OBJ_VRAM, sizeof(sampleCharData));
    MI_DmaCopy32(3, samplePlttData, (void *)HW_OBJ_PLTT, sizeof(samplePlttData));

    //---- drive unused OBJs out of screen
    MI_DmaFill32(3, oamBak, 0xc0, sizeof(oamBak));

    //---- start displaying
    OS_WaitVBlankIntr();
    GX_DispOn();
}

/*---------------------------------------------------------------------------*
  Name:         DisplayCounter

  Description:  display counter

  Arguments:    objNo :
                y     :
                count :

  Returns:      None
 *---------------------------------------------------------------------------*/
inline int valToChar(int val)
{
    return (int)((val < 10) ? '0' + val : 'A' + val - 10);
}

void DisplayCounter( int objNo, int y, u32 count )
{
	SetObj(objNo+0,  50, y, valToChar((int)((count>>28)&0xf)), 2);
	SetObj(objNo+1,  60, y, valToChar((int)((count>>24)&0xf)), 2);
	SetObj(objNo+2,  70, y, valToChar((int)((count>>20)&0xf)), 2);
	SetObj(objNo+3,  80, y, valToChar((int)((count>>16)&0xf)), 2);
	SetObj(objNo+4,  90, y, valToChar((int)((count>>12)&0xf)), 2);
	SetObj(objNo+5, 100, y, valToChar((int)((count>> 8)&0xf)), 2);
	SetObj(objNo+6, 110, y, valToChar((int)((count>> 4)&0xf)), 2);
	SetObj(objNo+7, 120, y, valToChar((int)((count    )&0xf)), 2);
}

/*---------------------------------------------------------------------------*
  Name:         SetObj

  Description:  set a object

  Arguments:    objNo     : object No.
                x         : x
                y         : y
                charNo    : character No.
                paletteNo : palette No.

  Returns:      None
 *---------------------------------------------------------------------------*/
void SetObj(int objNo, int x, int y, int charNo, int paletteNo)
{
    G2_SetOBJAttr((GXOamAttr *)&oamBak[objNo],
                  x,
                  y,
                  0,
                  GX_OAM_MODE_NORMAL,
                  FALSE,
                  GX_OAM_EFFECT_NONE, GX_OAM_SHAPE_8x8, GX_OAM_COLOR_16, charNo, paletteNo, 0);
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  VBlank interrupt handler

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void VBlankIntr(void)
{
    //---- renew OAM
    DC_FlushRange(oamBak, sizeof(oamBak));
    MI_DmaCopy32(3, oamBak, (void *)HW_OAM, sizeof(oamBak));

    //---- interrupt flag
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

//================================================================================
//  TIMER 0 as 16bit timer
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         SetTimer_1

  Description:  Set timer

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void SetTimer_1(void)
{
    //---- set interrupt handler
    OS_SetIrqFunction(OS_IE_TIMER0, TimerIntr_1);

    //---- enable TIMER 0
    (void)OS_EnableIrqMask(OS_IE_TIMER0);

    //---- start TIMER 0
    OS_StartTimer(OS_TIMER_0, MY_COUNT1, MY_SCALE1);
}

/*---------------------------------------------------------------------------*
  Name:         TimerIntr_1

  Description:  timer interrupt handler

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void TimerIntr_1(void)
{
    timerCnt1 = timerCnt1 + 1;

    //---- interrupt flag
    OS_SetIrqCheckFlag(OS_IE_TIMER0);

    //---- stop TIMER
    OS_StopTimer(OS_TIMER_0);

    //---- re-set TIMER
    SetTimer_1();
}

//================================================================================
//  TIMER 1 and 2 as 32bit timer
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         SetTimer

  Description:  Set timer

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void SetTimer_2(void)
{
    //---- set interrupt handler
    OS_SetIrqFunction(OS_IE_TIMER2, TimerIntr_2);

    //---- enable TIMER 1 and 2
    (void)OS_EnableIrqMask(OS_IE_TIMER1 | OS_IE_TIMER2);

    //---- start TIMER 1 and 2
    OS_StartTimer32(OS_TIMER32_12, MY_COUNT2, MY_SCALE2);
}

/*---------------------------------------------------------------------------*
  Name:         TimerIntr

  Description:  timer interrupt handler

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void TimerIntr_2(void)
{
    timerCnt2 = timerCnt2 + 1;

    //---- interrupt flag
    OS_SetIrqCheckFlag(OS_IE_TIMER2);

    //---- stop TIMER
    OS_StopTimer32(OS_TIMER32_12);

    //---- re-set TIMER
    SetTimer_2();
}


/*====== End of main.c ======*/
