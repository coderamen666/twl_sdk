/*---------------------------------------------------------------------------*
  Project:  TwlSDK - demos - spi - tp-1
  File:     main.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-08#$
  $Rev: 10700 $
  $Author: yosizaki $
 *---------------------------------------------------------------------------*/

#include    <nitro.h>
#include    <nitro/spi/ARM9/tp.h>
#include    "data.h"


/*---------------------------------------------------------------------------*
    Prototype definitions
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void);


/*---------------------------------------------------------------------------*
    Static variable definitions
 *---------------------------------------------------------------------------*/
static GXOamAttr gOam[128];


/*---------------------------------------------------------------------------*
  Name:         SetPoint16x16

  Description:  Displays a 16x16 OBJ at indicated point.

  Arguments:    x: Position X
                y: Position Y

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void SetPoint16x16(u16 pos_x, u16 pos_y)
{
    G2_SetOBJAttr(&gOam[0],            // OAM number
                  pos_x - 8,           // X position
                  pos_y - 8,           // Y position
                  0,                   // Priority
                  GX_OAM_MODE_BITMAPOBJ,        // Bitmap mode
                  FALSE,               // Mosaic off
                  GX_OAM_EFFECT_NONE,  // Affine off
                  GX_OAM_SHAPE_16x16,  // 16x16 size
                  GX_OAM_COLOR_16,     // 16 color
                  1,                   // Character
                  15,                  // Alpha
                  0);
}


/*---------------------------------------------------------------------------*
  Name:         DisplayInit

  Description:  Graphics initialization.

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
    (void)OS_EnableIrq();
    (void)GX_VBlankIntr(TRUE);         // To generate V-Blank interrupt request

    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
    MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);

    (void)GX_DisableBankForLCDC();

    MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);   // Clear OAM
    MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);     // Clear the standard palette

    MI_CpuFillFast((void *)HW_DB_OAM, 192, HW_DB_OAM_SIZE);     // Clear OAM
    MI_CpuClearFast((void *)HW_DB_PLTT, HW_DB_PLTT_SIZE);       // Clear the standard palette

    GX_SetBankForOBJ(GX_VRAM_OBJ_256_AB);       // Set VRAM-A, B for OBJ

    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS,    // 2D / 3D Mode
                       GX_BGMODE_0,    // BGMODE 0
                       GX_BG0_AS_2D);  // Set BG0 as 2D

    GX_SetVisiblePlane(GX_PLANEMASK_OBJ);       // Make OBJ visible
    GX_SetOBJVRamModeBmp(GX_OBJVRAMMODE_BMP_1D_128K);   // 1D mapping OBJ

    /* Load character bitmap data */
    GX_LoadOBJ((const void *)IMAGE_DATA, 0, IMAGE_DATA_SIZE);   // Transfer OBJ bitmap data to VRAM

    OS_WaitVBlankIntr();               // Waiting for the end of the V-Blank interrupt
    GX_DispOn();

}



/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Initialization and main loop.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    TPData  raw_point;
    TPData  disp_point;
    TPCalibrateParam calibrate;

    // Initialization
    OS_Init();
    TP_Init();

    // Get CalibrateParameter from FlashMemory
    if (!TP_GetUserInfo(&calibrate))
    {
        OS_Panic("FATAL ERROR: can't read UserOwnerInfo\n");
    }
    else
    {
        OS_Printf("Get Calibration Parameter from NVRAM\n");
    }

    TP_SetCalibrateParam(&calibrate);

    DisplayInit();

    // Parameters can be changed, but it is also acceptable to keep the default settings
#if 0
    // Send parameter of revision noise
    if (TP_RequestSetStability(3, 15) != 0)
    {
        OS_Panic("SetStability request err!\n");
    }
#endif

    while (TRUE)
    {
        // Draw Marker by calibrated point
        while (TP_RequestRawSampling(&raw_point) != 0)
        {
        };
        TP_GetCalibratedPoint(&disp_point, &raw_point);

        if (disp_point.touch)
        {
            SetPoint16x16(disp_point.x, disp_point.y);

            switch (disp_point.validity)
            {
            case TP_VALIDITY_VALID:
                OS_Printf("( %d, %d ) -> ( %d, %d )\n", raw_point.x, raw_point.y, disp_point.x,
                          disp_point.y);
                break;
            case TP_VALIDITY_INVALID_X:
                OS_Printf("( *%d, %d ) -> ( *%d, %d )\n", raw_point.x, raw_point.y, disp_point.x,
                          disp_point.y);
                break;
            case TP_VALIDITY_INVALID_Y:
                OS_Printf("( %d, *%d ) -> ( %d, *%d )\n", raw_point.x, raw_point.y, disp_point.x,
                          disp_point.y);
                break;
            case TP_VALIDITY_INVALID_XY:
                OS_Printf("( *%d, *%d ) -> ( *%d, *%d )\n", raw_point.x, raw_point.y, disp_point.x,
                          disp_point.y);
                break;
            }
        }

        /* Flush cache of OAM buffers to main memory */
        DC_FlushRange(gOam, sizeof(gOam));
        /* I/O register is accessed using DMA operation, so cache wait is not needed */
        // DC_WaitWriteBufferEmpty();
        GX_LoadOAM(gOam, 0, sizeof(gOam));
        MI_DmaFill32(3, gOam, 192, sizeof(gOam));       // Clear OAM buffer

        // Wait for V-Blank interrupt
        OS_WaitVBlankIntr();

    }
}


/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  V-Blank function.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void)
{

    // Set IRQ check flag
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}


/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
