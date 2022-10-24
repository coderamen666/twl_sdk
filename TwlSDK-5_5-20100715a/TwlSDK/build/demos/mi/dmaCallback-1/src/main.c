/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MI - demos - dmaCallback-1
  File:     main.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-05-22#$
  $Rev: 6123 $
  $Author: yada $
 *---------------------------------------------------------------------------*/
#include <nitro.h>
#include "data.h"

//---- DMA number
#define DMA_NO  3

GXOamAttr oamBak[128];
volatile u32 dmaCnt = 0;

void	myInit(void);
void	DisplayCounter( int objNo, int y, u32 count );
void	myVBlankIntr(void);
void	SetObj(int objNo, int x, int y, int charNo, int paletteNo);

//---- DMA callback
void    myDmaCallback(void *arg);

//================================================================================
/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
OSAlarm alarm;
void NitroMain()
{
	myInit();

    while (1)
    {
        //---- wait vblank
        OS_WaitVBlankIntr();

        //---- display counter
		DisplayCounter(0, 100, dmaCnt);
    }
}

/*---------------------------------------------------------------------------*
  Name:         myInit

  Description:  initialize

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void myInit(void)
{
    OS_Init();
    GX_Init();

    //---- set power on
    GX_SetPower(GX_POWER_ALL);

    //----  enable VBlank interrupt
    (void)OS_SetIrqFunction(OS_IE_V_BLANK, myVBlankIntr);
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
  Name:         myVBlankIntr

  Description:  vblank interrupt handler

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void myVBlankIntr(void)
{
    DC_FlushRange(oamBak, sizeof(oamBak));

	//---- transfer OAM data
    MI_DmaCopy32Async(DMA_NO, oamBak, (void *)HW_OAM, sizeof(oamBak), myDmaCallback, (void *)0x1);

    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         myVBlankIntr

  Description:  dma callback

  Arguments:    arg :

  Returns:      None
 *---------------------------------------------------------------------------*/
void myDmaCallback(void *arg)
{
    dmaCnt = dmaCnt + (u32)arg;
}

/*====== End of main.c ======*/
