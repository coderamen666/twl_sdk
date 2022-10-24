/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MATH - demos
  File:     main.c

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
  Demo using linear congruential method to get random numbers
 *---------------------------------------------------------------------------*/

#include    <nitro.h>



static void VBlankIntr(void);
static void KeyRead(void);
static void DisplayInit();
static void PutDot(u16 x, u16 y, u16 col);

/*---------------------------------------------------------------------------*
    Variable Definitions
 *---------------------------------------------------------------------------*/
static struct
{
    u16     trig;
    u16     press;
}
gKey;                                  // Key input



/*---------------------------------------------------------------------------*
    Function Definitions
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Initialization and main loop.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    MATHRandContext16 rnd;
    u16     init_rnd = 0;

    // Various types of initialization
    OS_Init();

    DisplayInit();

    // Empty call for getting key input data (strategy for pressing A Button in the IPL)
    KeyRead();

    OS_TPrintf("press any button\n");

    // Main loop
    while (TRUE)
    {
        static u16 col = 0;
        static u16 *pVram = (u16 *)HW_LCDC_VRAM_C;
        static u16 *pbVram = (u16 *)(HW_LCDC_VRAM_C + 0x18000 - 2);

        // Waiting for the V-Blank
        OS_WaitVBlankIntr();

        // Get key input data
        KeyRead();

        if (!init_rnd)
        {
            if (gKey.trig)
            {
                // Initialize random number, use the value of V-Blank counter as the base
                MATH_InitRand16(&rnd, OS_GetVBlankCount());
                init_rnd = 1;
            }
            continue;
        }

        {
            u16     x, y;
            u32     i;

            for (i = 0; i < 0x400; i++)
            {
                // Get random number
                x = (u16)MATH_Rand16(&rnd, HW_LCD_WIDTH);
                y = (u16)MATH_Rand16(&rnd, HW_LCD_HEIGHT);

                PutDot(x, y, col);
            }
        }

        // All white/black
        while (*pVram == col)
        {
            pVram++;
            if (pVram > pbVram)
            {
                pVram = (u16 *)HW_LCDC_VRAM_C;
                pbVram = (u16 *)(HW_LCDC_VRAM_C + 0x18000 - 2);
                col ^= 0x7FFF;
            }
        }
        while (*pbVram == col)
        {
            pbVram--;
            if (pVram > pbVram)
            {
                pVram = (u16 *)HW_LCDC_VRAM_C;
                pbVram = (u16 *)(HW_LCDC_VRAM_C + 0x18000 - 2);
                col ^= 0x7FFF;
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  V-Blank interrupt vector.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void)
{
    // Sets the IRQ check flag
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         KeyRead

  Description:  Edits key input data.
                Detects press trigger, release trigger, and press-and-hold repeat.

  Arguments:    pKey:   Structure that holds key input data to be edited

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void KeyRead(void)
{
    u16     readData = PAD_Read();
    gKey.trig = (u16)(readData & (readData ^ gKey.press));
    gKey.press = readData;
}


/*---------------------------------------------------------------------------*
  Name:         DisplayInit

  Description:  Graphics Initialization

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DisplayInit()
{

    GX_Init();
    FX_Init();

    GX_DispOff();
    GXS_DispOff();

    GX_SetDispSelect(GX_DISP_SELECT_SUB_MAIN);

    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);         // To generate V-Blank interrupt request
    (void)OS_EnableIrq();


    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
    MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);

    MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);   // Clear OAM
    MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);     // Clear the standard palette

    MI_CpuFillFast((void *)HW_DB_OAM, 192, HW_DB_OAM_SIZE);     // Clear OAM
    MI_CpuClearFast((void *)HW_DB_PLTT, HW_DB_PLTT_SIZE);       // Clear the standard palette
    MI_DmaFill32(3, (void *)HW_LCDC_VRAM_C, 0x7FFF7FFF, 256 * 192 * sizeof(u16));


    GX_SetBankForOBJ(GX_VRAM_OBJ_256_AB);       // Set VRAM-A,B for OBJ

    GX_SetGraphicsMode(GX_DISPMODE_VRAM_C,      // VRAM mode
                       (GXBGMode)0,    // Dummy
                       (GXBG0As)0);    // Dummy

    GX_SetVisiblePlane(GX_PLANEMASK_OBJ);       // Make OBJs visible
    GX_SetOBJVRamModeBmp(GX_OBJVRAMMODE_BMP_1D_128K);   // 2D mapping OBJ

    OS_WaitVBlankIntr();               // Waiting for the end of the V-Blank interrupt
    GX_DispOn();

}


/*---------------------------------------------------------------------------*
  Name:         PutDot

  Description:  Draw a Dot in VRAM for LCDC.

  Arguments:    x: position X
                y: position Y
                col: DotColor

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void PutDot(u16 x, u16 y, u16 col)
{
    if (x >= HW_LCD_WIDTH || y >= HW_LCD_HEIGHT)
    {
        return;
    }
    *(u16 *)(HW_LCDC_VRAM_C + y * 256 * 2 + x * 2) = col;
}


/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
