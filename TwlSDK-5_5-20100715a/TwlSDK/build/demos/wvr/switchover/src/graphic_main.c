/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WVR - demos - switchover
  File:     graphic_main.c

  Copyright 2005-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-08-31#$
  $Rev: 11029 $
  $Author: tominaga_masafumi $
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------
 * This demo dynamically switches WVR library use on and off.
 * Buttons are used to switch the dual-screen 3D display mode and the wireless communications mode.
 *
 * This demo is simply a combination of the following two samples.
 * For details on operation in the individual modes, see the related source code.
 *    /build/demos/wvr/simple
 *    /build/demos/gx/UnitTours/Sub_Double3D
 *---------------------------------------------------------------------------*/


#include <nitro.h>
#include    "common.h"


/*****************************************************************************/
/* Constants */

/* Cube vertex coordinates */
static const s16 gCubeGeometry[3 * 8] = {
    FX16_ONE, FX16_ONE, FX16_ONE,
    FX16_ONE, FX16_ONE, -FX16_ONE,
    FX16_ONE, -FX16_ONE, FX16_ONE,
    FX16_ONE, -FX16_ONE, -FX16_ONE,
    -FX16_ONE, FX16_ONE, FX16_ONE,
    -FX16_ONE, FX16_ONE, -FX16_ONE,
    -FX16_ONE, -FX16_ONE, FX16_ONE,
    -FX16_ONE, -FX16_ONE, -FX16_ONE
};

/* Cube vertex color */
static const GXRgb gCubeColor[8] = {
    GX_RGB(31, 31, 31),
    GX_RGB(31, 31, 0),
    GX_RGB(31, 0, 31),
    GX_RGB(31, 0, 0),
    GX_RGB(0, 31, 31),
    GX_RGB(0, 31, 0),
    GX_RGB(0, 0, 31),
    GX_RGB(0, 0, 0)
};


/*****************************************************************************/
/* Variables */

/* OAM information used to display the captured screen as a bitmap OBJ */
static GXOamAttr sOamBak[128];


/*****************************************************************************/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         GetPadTrigger

  Description:  Updates key input information and returns the latest press trigger bit.
                Detects press trigger, release trigger, and press-and-hold repeat.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static int GetPadTrigger(void)
{
    static u16 pad_bak = 0;
    const u16 pad_cur = PAD_Read();
    const int trig = (u16)(~pad_bak & pad_cur);
    return (pad_bak = pad_cur), trig;
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  V-Blank interrupt handler (set by DEMO library).

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         Color

  Description:  Sets specified vertex color of cube.

  Arguments:    idx: Vertex index

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void Color(int idx)
{
    G3_Color(gCubeColor[idx]);
}

/*---------------------------------------------------------------------------*
  Name:         Vtx

  Description:  Sets specified vertex coordinate of cube.

  Arguments:    idx: Vertex index

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void Vtx(int idx)
{
    G3_Vtx(gCubeGeometry[idx * 3], gCubeGeometry[idx * 3 + 1], gCubeGeometry[idx * 3 + 2]);
}

/*---------------------------------------------------------------------------*
  Name:         ColVtxQuad

  Description:  Creates a rectangle by specifying coordinates for the vertices and color in series.

  Arguments:    idx0: Index of Vertex 0
                idx1: Index of Vertex 1
                idx2: Index of Vertex 2
                idx3: Index of Vertex 3
                bOwnIndexColor: If TRUE, use the colors of each index.
                                If FALSE, use color of idx0 for all.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ColVtxQuad(int idx0, int idx1, int idx2, int idx3, BOOL bOwnIndexColor)
{
    if (bOwnIndexColor)
    {
        Color(idx0);
        Vtx(idx0);
        Color(idx1);
        Vtx(idx1);
        Color(idx2);
        Vtx(idx2);
        Color(idx3);
        Vtx(idx3);
    }
    else
    {
        Color(idx0);
        Vtx(idx0);
        Vtx(idx1);
        Vtx(idx2);
        Vtx(idx3);
    }
}

/*---------------------------------------------------------------------------*
  Name:         drawCube

  Description:  Draws cube on left side of screen.

  Arguments:    Rotate: Angle of rotation
                bIsRight: If right, TRUE; if left, FALSE

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void drawCube(u16 Rotate, BOOL bIsRight)
{
    G3_PushMtx();
    {
        const int pos = bIsRight ? +3 : -3;

        /* Parallel movement and rotational movement */
        G3_Translate(pos << (FX32_SHIFT - 1), 0, 0);
        {
            const fx16 s = FX_SinIdx(Rotate);
            const fx16 c = FX_CosIdx(Rotate);
            G3_RotX(s, c);
            G3_RotY(s, c);
            G3_RotZ(s, c);
        }

        /*
         * Specify the material and polygon properties with the following settings.
         *   Diffuse reflection color = GX_RGB(31, 31, 31) (not used as vertex color)
         *   Ambient reflection color = GX_RGB(16, 16, 16)
         *   Specular reflection color = GX_RGB(16, 16, 16) (not using a table)
         *   Emission color = GX_RGB(0, 0, 0)
         *   Light = All disabled
         *   Mode = Modulation
         *   Culling = Hide back surface
         *   Polygon ID = 0
         *   Opacity = 31
         *   Other = None
         */
        G3_MaterialColorDiffAmb(GX_RGB(31, 31, 31), GX_RGB(16, 16, 16), FALSE);
        G3_MaterialColorSpecEmi(GX_RGB(16, 16, 16), GX_RGB(0, 0, 0), FALSE);
        G3_PolygonAttr(GX_LIGHTMASK_NONE, GX_POLYGONMODE_MODULATE, GX_CULL_BACK,
                       0, 31, GX_POLYGON_ATTR_MISC_NONE);

        /* Render the cube */
        G3_Begin(GX_BEGIN_QUADS);
        {
            ColVtxQuad(2, 0, 4, 6, bIsRight);
            ColVtxQuad(7, 5, 1, 3, bIsRight);
            ColVtxQuad(6, 4, 5, 7, bIsRight);
            ColVtxQuad(3, 1, 0, 2, bIsRight);
            ColVtxQuad(5, 4, 0, 1, bIsRight);
            ColVtxQuad(6, 7, 3, 2, bIsRight);
        }
        G3_End();

    }
    G3_PopMtx(1);
}

/*---------------------------------------------------------------------------*
  Name:         setupFrame

  Description:  Initializes settings for frame drawing.

  Arguments:    bIsRight: If right, TRUE; if left, FALSE

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void setupFrame(BOOL bIsRight)
{
    /*
     * Alternate use of the 3D engine for each picture frame on two screens.
     * When the 3D engine cannot be used in the current frame, use captured results from the previous frame.
     * VRAM-C captures the left cube and VRAM-D captures the right cube. Each VRAM can be used for only BGs or OBJs, however, so VRAM-C is displayed with a direct bitmap and VRAM-D is displayed with a texture OBJ.
     * 
     * 
     */

    /*
     * Initialize the LCDs with the following settings.
     *   Main BG mode: 0
     *     BG-0: 3D (Priority 0)
     *     BG-1: -
     *     BG-2: -
     *     BG-3: -
     *     OBJ: -
     */
    (void)GX_ResetBankForSubOBJ();
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_3D);
    GX_SetVisiblePlane(GX_PLANEMASK_BG0);
    G2_SetBG0Priority(0);
    GXS_SetGraphicsMode(GX_BGMODE_5);

    if (bIsRight)
    {
        /*
         *   Show the main LCD above and the sub LCD below.
         *   Sub BG mode: 5
         *     BG-0: -
         *     BG-1: -
         *     BG-2: Direct bitmap
         *     BG-3: -
         *     OBJ: -
         *   Capture mode: (256, 192), 3D only 100%
         *   VRAM-A: -
         *   VRAM-B: -
         *   VRAM-C: Sub OBJ
         *   VRAM-D: LCDC
         */
        (void)GX_DisableBankForLCDC();
        (void)GX_DisableBankForSubOBJ();
        GX_SetBankForSubBG(GX_VRAM_SUB_BG_128_C);
        GX_SetBankForLCDC(GX_VRAM_LCDC_D);
        GX_SetCapture(GX_CAPTURE_SIZE_256x192,
                      GX_CAPTURE_MODE_A, GX_CAPTURE_SRCA_3D, (GXCaptureSrcB)0,
                      GX_CAPTURE_DEST_VRAM_D_0x00000, 16, 0);
        GXS_SetVisiblePlane(GX_PLANEMASK_BG2);
        G2S_SetBG2Priority(0);
        G2S_SetBG2ControlDCBmp(GX_BG_SCRSIZE_DCBMP_256x256,
                               GX_BG_AREAOVER_XLU, GX_BG_BMPSCRBASE_0x00000);
        G2S_BG2Mosaic(FALSE);
    }
    else
    {
        /*
         *   Show the main LCD below and the sub LCD above.
         *   Sub BG mode: 5
         *     BG-0: -
         *     BG-1: -
         *     BG-2: -
         *     BG-3: -
         *     OBJ: ON
         *   Capture mode: (256, 192), 3D only 100%
         *   VRAM-A: -
         *   VRAM-B: -
         *   VRAM-C: LCDC
         *   VRAM-D: Sub OBJ
         */
        (void)GX_DisableBankForLCDC();
        (void)GX_DisableBankForSubBG();
        GX_SetBankForSubOBJ(GX_VRAM_SUB_OBJ_128_D);
        GX_SetBankForLCDC(GX_VRAM_LCDC_C);
        GX_SetCapture(GX_CAPTURE_SIZE_256x192,
                      GX_CAPTURE_MODE_A, GX_CAPTURE_SRCA_3D, (GXCaptureSrcB)0,
                      GX_CAPTURE_DEST_VRAM_C_0x00000, 16, 0);
        GXS_SetVisiblePlane(GX_PLANEMASK_OBJ);
    }

    G3X_Reset();

    /* Configure the camera matrix */
    {
        const VecFx32 Eye = { 0, 0, FX32_ONE * 5 };
        const VecFx32 at = { 0, 0, 0 };
        const VecFx32 vUp = { 0, FX32_ONE, 0 };
        G3_LookAt(&Eye, &vUp, &at, NULL);
    }
}

/*---------------------------------------------------------------------------*
  Name:         setupSubOAM

  Description:  Arranges OAM into grid for display of VRAM-D on subscreen.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void setupSubOAM(void)
{
    int     i;
    int     x, y;
    int     idx = 0;

    GXS_SetOBJVRamModeBmp(GX_OBJVRAMMODE_BMP_2D_W256);

    for (i = 0; i < 128; ++i)
    {
        sOamBak[i].attr01 = 0;
        sOamBak[i].attr23 = 0;
    }

    for (y = 0; y < 192; y += 64)
    {
        for (x = 0; x < 256; x += 64, idx++)
        {
            G2_SetOBJAttr(&sOamBak[idx], x, y, 0,
                          GX_OAM_MODE_BITMAPOBJ, FALSE,
                          GX_OAM_EFFECT_NONE, GX_OAM_SHAPE_64x64, GX_OAM_COLOR_16,
                          (y / 8) * 32 + (x / 8), 15, 0);
        }
    }
    DC_FlushRange(&sOamBak[0], sizeof(sOamBak));
    /* I/O register is accessed using DMA operation, so cache wait is not needed */
    // DC_WaitWriteBufferEmpty();
    GXS_LoadOAM(&sOamBak[0], 0, sizeof(sOamBak));
}

/*---------------------------------------------------------------------------*
  Name:         GraphicMain

  Description:  Initialization and main loop for two-screen 3D display mode.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void GraphicMain(void)
{
    GX_DispOff();
    GXS_DispOff();

    /* Clear all VRAM, OAM, and palette RAM */
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
    MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);
    (void)GX_DisableBankForLCDC();
    MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);
    MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);
    MI_CpuFillFast((void *)HW_DB_OAM, 192, HW_DB_OAM_SIZE);
    MI_CpuClearFast((void *)HW_DB_PLTT, HW_DB_PLTT_SIZE);

    /* V-Blank interrupt handler configuration */
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    /* GX initialization */
    G3X_Init();
    G3X_InitTable();
    G3X_InitMtxStack();

    /* Initialization of the necessary render configurations only at startup */
    G3X_AntiAlias(TRUE);
    setupSubOAM();
    G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);
    G3X_SetClearColor(GX_RGB(0, 0, 0), 0, 0x7fff, 63, FALSE);
    G3_ViewPort(0, 0, 255, 191);

    /* Initialization of the projection matrix */
    {
        const fx32 right = FX32_ONE;
        const fx32 top = FX32_ONE * 3 / 4;
        const fx32 near = FX32_ONE;
        const fx32 far = FX32_ONE * 400;
        G3_Perspective(FX32_SIN30, FX32_COS30, FX32_ONE * 4 / 3, near, far, NULL);
        G3_StoreMtx(0);
    }

    /* Start render loop */
    OS_WaitVBlankIntr();
    GX_DispOn();
    GXS_DispOn();
    {
        u16     Rotate = 0;
        BOOL    bIsRight;

        (void)GetPadTrigger();
        for (bIsRight = TRUE;; bIsRight = !bIsRight)
        {
            if (GetPadTrigger() & PAD_BUTTON_SELECT)
            {
                break;
            }
            /* Frame render */
            setupFrame(bIsRight);
            drawCube(Rotate, bIsRight);

            Rotate += 256;

            G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);
            OS_WaitVBlankIntr();
            /*
             * Rendering results will be applied from the V-Blank after the buffers are swapped, so the LCD intended for the display is set at this time.
             * 
             * Specify that the cube on the right will appear in the lower screen.
             */
            GX_SetDispSelect(bIsRight ? GX_DISP_SELECT_SUB_MAIN : GX_DISP_SELECT_MAIN_SUB);

        }
    }
}
