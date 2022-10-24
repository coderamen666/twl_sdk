/*---------------------------------------------------------------------------*
  Project:  NitroCrypto - build - demos
  File:     main.c

  Copyright 2005-2008 Nintendo. All rights reserved.

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
  This demo confirms that the RC4 algorithm for the CRYPT library works and measures its speed of operation.
  Measures speed
 *---------------------------------------------------------------------------*/

#include <nitro.h>
#include <nitro/crypto.h>



static void VBlankIntr(void);
static void DisplayInit(void);
static void FillScreen(u16 col);
static BOOL CompareRC4Speed(void);

/*---------------------------------------------------------------------------*
    Variable Definitions
 *---------------------------------------------------------------------------*/

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
    // Various types of initialization
    OS_Init();
    OS_InitTick();

    DisplayInit();

    if (CompareRC4Speed())
	{
        // Success
        OS_TPrintf("------ Test Succeeded ------\n");
        FillScreen(GX_RGB(0, 31, 0));
	}
	else
	{
        // Failed
        OS_TPrintf("****** Test Failed ******\n");
        FillScreen(GX_RGB(31, 0, 0));
	}

    // Main loop
    while (TRUE)
    {
        // Waiting for the V-Blank
        OS_WaitVBlankIntr();
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
  Name:         DisplayInit

  Description:  Graphics initialization.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DisplayInit(void)
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

    OS_WaitVBlankIntr();              // Waiting for the end of the V-Blank interrupt
    GX_DispOn();

}


/*---------------------------------------------------------------------------*
  Name:         FillScreen

  Description:  Fills the screen.

  Arguments:    col: FillColor.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void FillScreen(u16 col)
{
    MI_CpuFill16((void *)HW_LCDC_VRAM_C, col, 256 * 192 * 2);
}

/*---------------------------------------------------------------------------*
  Name:         CompareRC4Speed

  Description:  RC4 function test routine.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#define DATA_SIZE (1024*1024)
static BOOL CompareRC4Speed(void)
{
    static u8 data[DATA_SIZE] ATTRIBUTE_ALIGN(32);
    static u8 orig[DATA_SIZE] ATTRIBUTE_ALIGN(32);

    u8 key[] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
        0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
    };

    int i;
    MATHRandContext32 rand;
    OSTick start, end;

    OS_TPrintf("Initialize Array...\n");
    // Initializes data
    MATH_InitRand32(&rand, 0);
    for (i = 0; i < DATA_SIZE; i++)
    {
        data[i] = orig[i] = (u8)MATH_Rand32(&rand, 0x100);
    }

    OS_TPrintf("Start...\n");

    OS_TPrintf("RC4 Encoding:\t\t");
    start = OS_GetTick();
    CRYPTO_RC4(key, sizeof(key), data, sizeof(data));
    end   = OS_GetTick();
    OS_Printf("%lld us/MB\n", OS_TicksToMicroSeconds(end - start));

    OS_TPrintf("RC4 Decoding:\t\t");
    start = OS_GetTick();
    CRYPTO_RC4(key, sizeof(key), data, sizeof(data));
    end   = OS_GetTick();
    OS_Printf("%lld us/MB\n", OS_TicksToMicroSeconds(end - start));

    for (i = 0; i < DATA_SIZE; i++)
    {
        if ( data[i] != orig[i] )
        {
            //OS_Panic("Decoding Failed!");
			return FALSE;
        }
    }

    OS_TPrintf("RC4Fast Encoding:\t");
    start = OS_GetTick();
    CRYPTO_RC4Fast(key, sizeof(key), data, sizeof(data));
    end   = OS_GetTick();
    OS_Printf("%lld us/MB\n", OS_TicksToMicroSeconds(end - start));

    OS_TPrintf("RC4Fast Decoding:\t");
    start = OS_GetTick();
    CRYPTO_RC4Fast(key, sizeof(key), data, sizeof(data));
    end   = OS_GetTick();
    OS_Printf("%lld us/MB\n", OS_TicksToMicroSeconds(end - start));

    for (i = 0; i < DATA_SIZE; i++)
    {
        if ( data[i] != orig[i] )
        {
            //OS_Panic("Decoding Failed!");
			return FALSE;
        }
    }

    OS_TPrintf("Done.\n");

    return TRUE;
}

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
