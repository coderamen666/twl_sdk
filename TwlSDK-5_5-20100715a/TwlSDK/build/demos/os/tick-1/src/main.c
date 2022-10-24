/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - demos - tick-1
  File:     main.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-04-01#$
  $Rev: 5205 $
  $Author: yada $
 *---------------------------------------------------------------------------*/
#include <nitro.h>
#include "data.h"

static GXOamAttr oamBak[128];

void    VBlankIntr(void);
void    ObjSet(int objNo, int x, int y, int charNo, int paletteNo);

//---- Return 0-9, A-F code for OBJ display
inline int ObjChar(u32 cnt, int shift)
{
    u32     d = (cnt >> shift) & 0xf;
    return (int)((d < 10) ? '0' + d : 'A' + d - 10);
}

//================================================================================
/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Main.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain()
{
    //================ Initialization
    OS_Init();
    OS_InitTick();                     // Initialize system clock

    GX_Init();

    //================ Settings
    //---- All Power ON
    GX_SetPower(GX_POWER_ALL);

    //----  Enable V-Blank interrupt
    (void)OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();

    //---- V-Blank occurrence settings
    (void)GX_VBlankIntr(TRUE);

    //---- Clear VRAM
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
    MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);
    (void)GX_DisableBankForLCDC();

    //---- OAM and palette clear
    MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);
    MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);

    //---- Set bank A for OBJ
    GX_SetBankForOBJ(GX_VRAM_OBJ_128_A);

    //---- Set to graphics display mode
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D);

    //---- Set only OBJ display ON
    GX_SetVisiblePlane(GX_PLANEMASK_OBJ);

    //---- Used with 32-KB OBJ in 2D mapping mode
    GX_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_2D);

    //---- Data load
    MI_DmaCopy32(3, sampleCharData, (void *)HW_OBJ_VRAM, sizeof(sampleCharData));
    MI_DmaCopy32(3, samplePlttData, (void *)HW_OBJ_PLTT, sizeof(samplePlttData));

    GX_DispOn();


    //================ Main loop
    while (1)
    {
        u16     keyData;
        u32     hi, low;
        OSTick  timerCnt;

        //---- Wait for V-Blank interrupt completion
        OS_WaitVBlankIntr();

        //---- Move hidden OBJ off screen
        MI_DmaFill32(3, oamBak, 0xc0, sizeof(oamBak));

        //---- Load pad data
        keyData = PAD_Read();

        //---- Get system clock value
        timerCnt = OS_GetTick();

        //---- Display system clock value
        hi = (u32)(timerCnt >> 32);
        low = (u32)(timerCnt & 0xffffffff);

        ObjSet(0, 50, 100, ObjChar(hi, 28), 2);
        ObjSet(1, 60, 100, ObjChar(hi, 24), 2);
        ObjSet(2, 70, 100, ObjChar(hi, 20), 2);
        ObjSet(3, 80, 100, ObjChar(hi, 16), 2);
        ObjSet(4, 90, 100, ObjChar(hi, 12), 2);
        ObjSet(5, 100, 100, ObjChar(hi, 8), 2);
        ObjSet(6, 110, 100, ObjChar(hi, 4), 2);
        ObjSet(7, 120, 100, ObjChar(hi, 0), 2);

        ObjSet(8, 140, 100, ObjChar(low, 28), 2);
        ObjSet(9, 150, 100, ObjChar(low, 24), 2);
        ObjSet(10, 160, 100, ObjChar(low, 20), 2);
        ObjSet(11, 170, 100, ObjChar(low, 16), 2);
        ObjSet(12, 180, 100, ObjChar(low, 12), 2);
        ObjSet(13, 190, 100, ObjChar(low, 8), 2);
        ObjSet(14, 200, 100, ObjChar(low, 4), 2);
        ObjSet(15, 210, 100, ObjChar(low, 0), 2);

        //---- Pressing the A button advances the system clock a little
        if (keyData & PAD_BUTTON_A)
        {
            OS_SetTick(OS_GetTick() + (u64)0x20000000000ULL);
        }
    }
}

//--------------------------------------------------------------------------------
//  Set OBJ
//
void ObjSet(int objNo, int x, int y, int charNo, int paletteNo)
{
    G2_SetOBJAttr((GXOamAttr *)&oamBak[objNo],
                  x,
                  y,
                  0,
                  GX_OAM_MODE_NORMAL,
                  FALSE,
                  GX_OAM_EFFECT_NONE, GX_OAM_SHAPE_8x8, GX_OAM_COLOR_16, charNo, paletteNo, 0);
}


//--------------------------------------------------------------------------------
//    V-Blank interrupt process
//
void VBlankIntr(void)
{
    //---- OAM updating
    DC_FlushRange(oamBak, sizeof(oamBak));
    /* I/O register is accessed using DMA operation, so cache wait is not needed */
    // DC_WaitWriteBufferEmpty();
    MI_DmaCopy32(3, oamBak, (void *)HW_OAM, sizeof(oamBak));

    //---- Interrupt check flag
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*====== End of main.c ======*/
