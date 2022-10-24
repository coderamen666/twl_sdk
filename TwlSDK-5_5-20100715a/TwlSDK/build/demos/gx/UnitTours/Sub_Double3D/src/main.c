/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/Sub_Double3D
  File:     main.c

  Copyright 2003-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2010-07-10#$
  $Rev: 11353 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
// A sample that displays moving 3D models on both LCDs.
//
// Both LCDs display a different rotating 3D cube.
// The sub 2D engine displays the captured image rendered by the main engine,
// making use of direct color bitmap BG/bitmap OBJ.
// Note that the output LCDs are swapped every frame.
//
// HOWTO:
// 1. Set up OAM for the sub 2D engine, to display captured images correctly.
// 2. (Frame 2N):   Assign VRAM-C to BG of the sub 2D engine,
//                  and display the captured image on VRAM-C
//                  through direct color bitmap BG.
//                  Capture the 3D plane and store it on VRAM-D.
//    (Frame 2N+1): Assign VRAM-D to OBJ of the sub 2D engine,
//                  and display the captured image on VRAM-D
//                  through bitmap OBJs.
//                  Capture the 3D plane and store it on VRAM-C.
//---------------------------------------------------------------------------

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif
#include "DEMO.h"

#define    STACK_SIZE     1024
#define    THREAD1_PRIO   (OS_THREAD_LAUNCHER_PRIORITY - 6)
void    proc1(void *p1);
OSThread thread1;
u32     stack1[STACK_SIZE / sizeof(u32)];

static GXOamAttr sOamBak[128];

BOOL    flip_flag = TRUE;              // Flip switch flag
BOOL    swap = FALSE;                  // SwapBuffers execution flag
int     attr = GX_POLYGON_ATTR_MISC_NONE;

s16     gCubeGeometry[3 * 8] = {
    FX16_ONE, FX16_ONE, FX16_ONE,
    FX16_ONE, FX16_ONE, -FX16_ONE,
    FX16_ONE, -FX16_ONE, FX16_ONE,
    FX16_ONE, -FX16_ONE, -FX16_ONE,
    -FX16_ONE, FX16_ONE, FX16_ONE,
    -FX16_ONE, FX16_ONE, -FX16_ONE,
    -FX16_ONE, -FX16_ONE, FX16_ONE,
    -FX16_ONE, -FX16_ONE, -FX16_ONE
};

GXRgb   gCubeColor[8] = {
    GX_RGB(31, 31, 31),
    GX_RGB(31, 31, 0),
    GX_RGB(31, 0, 31),
    GX_RGB(31, 0, 0),
    GX_RGB(0, 31, 31),
    GX_RGB(0, 31, 0),
    GX_RGB(0, 0, 31),
    GX_RGB(0, 0, 0)
};

/* ---------------------------------------------------
**Operating modes
    This demo runs in 2 modes each time the Y Button is pressed.
    A 30-fps mode with deflickering is used first, followed by a 15-fps mode without deflickering.
    
    

*Flickering
    Flickering occurs when a display capture feature is used to capture a 3D image and the original image and the captured image are displayed one after the other.
    

    When the display capture feature is used to capture a 3D image, the least-significant bit of the original image's RGB (6:6:6) data will be 0, so the result will be RGB (5:5:5).
    
    Therefore, alternating between displaying the original image and the captured image will cause flickering.

*How to avoid problems
    You must display images captured in VRAM on both the upper and lower screens instead of displaying 3D images directly.
    To handle this, this demo displays the VRAM with its captured data unchanged.

*Polygon Edge Flickering
    Even if the aforementioned flickering countermeasures have been taken, some settings may cause polygon edges to flicker.
    This is caused by alpha values.

    An alpha value of 0 can be stored in the color buffer along with a valid RGB value according to the following formula: color buffer alpha = (polygon alpha x (antialias factor + correction value 1)) / 32.
    
    During 3D captures afterward, 0 will also be stored in the VRAM alpha bit only when the capture data's alpha value is 0.
    
    (You can actually dump VRAM and check polygon edges with an alpha value of 0.)
    
    This alpha bit is disabled in VRAM display mode, so pixels corresponding to the polygon edge will be displayed.
    The alpha bit is enabled for BG display (graphics display mode), so these bits will be handled as omitted and the backdrop color will be displayed.
    

    This difference causes polygon edges to flicker.
    (Although it is difficult to see when the clear color and the backdrop color are the same, if the clear color is changed, for example, it will be possible to see that these colors are displayed alternating as the background color.)
      

*How to avoid problems
    If the value specified as argument 'a' to the GX_SetCapture function is GX_CAPTURE_SRCA_2D3D, the display will be the same in the aforementioned scenario and when 2D and 3D graphics are combined, allowing you to avoid flickering.
    
---------------------------------------------------- */
static int operationMode = 0;

static void Color(int idx)
{
    G3_Color(gCubeColor[idx]);
}

static void Vtx(int idx)
{
    G3_Vtx(gCubeGeometry[idx * 3], gCubeGeometry[idx * 3 + 1], gCubeGeometry[idx * 3 + 2]);
}

static void Quad(int idx0, int idx1, int idx2, int idx3)
{
    Vtx(idx0);
    Vtx(idx1);
    Vtx(idx2);
    Vtx(idx3);
}

static void ColVtxQuad(int idx0, int idx1, int idx2, int idx3)
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

static void drawLeftCube(u16 Rotate)
{
    G3_PushMtx();

    // Rotate and translate
    G3_Translate(-3 << (FX32_SHIFT - 1), 0, 0);
    {
        fx16    s = FX_SinIdx(Rotate);
        fx16    c = FX_CosIdx(Rotate);

        G3_RotX(s, c);
        G3_RotY(s, c);
        G3_RotZ(s, c);
    }

    G3_MaterialColorDiffAmb(GX_RGB(31, 31, 31), // Diffuse
                            GX_RGB(16, 16, 16), // Ambient
                            FALSE      // Use diffuse as vtx color if TRUE
        );

    G3_MaterialColorSpecEmi(GX_RGB(16, 16, 16), // Specular
                            GX_RGB(0, 0, 0),    // Emission
                            FALSE      // Use shininess table if TRUE
        );

    G3_PolygonAttr(GX_LIGHTMASK_NONE,  // Disable lights
                   GX_POLYGONMODE_MODULATE,     // Modulation mode
                   GX_CULL_BACK,       // Cull back
                   0,                  // Polygon ID (0-63)
                   31,                 // Alpha (0-31)
                   attr                // OR of GXPolygonAttrMisc's value
        );

    //---------------------------------------------------------------------------
    // Draw a cube:
    // Specify different colors for the planes
    //---------------------------------------------------------------------------

    G3_Begin(GX_BEGIN_QUADS);

    {
        Color(3);
        Quad(2, 0, 4, 6);

        Color(4);
        Quad(7, 5, 1, 3);

        Color(5);
        Quad(6, 4, 5, 7);

        Color(2);
        Quad(3, 1, 0, 2);

        Color(6);
        Quad(5, 4, 0, 1);

        Color(1);
        Quad(6, 7, 3, 2);
    }

    G3_End();

    G3_PopMtx(1);
}

static void drawRightCube(u16 Rotate)
{
    G3_PushMtx();

    // Rotate and translate
    G3_Translate(3 << (FX32_SHIFT - 1), 0, 0);

    {
        fx16    s = FX_SinIdx(Rotate);
        fx16    c = FX_CosIdx(Rotate);

        G3_RotX(s, c);
        G3_RotY(s, c);
        G3_RotZ(s, c);
    }

    G3_MaterialColorDiffAmb(GX_RGB(31, 31, 31), // Diffuse
                            GX_RGB(16, 16, 16), // Ambient
                            FALSE      // Use diffuse as vtx color if TRUE
        );

    G3_MaterialColorSpecEmi(GX_RGB(16, 16, 16), // Specular
                            GX_RGB(0, 0, 0),    // Emission
                            FALSE      // Use shininess table if TRUE
        );

    G3_PolygonAttr(GX_LIGHTMASK_NONE,  // Disable lights
                   GX_POLYGONMODE_MODULATE,     // Modulation mode
                   GX_CULL_BACK,       // Cull back
                   0,                  // Polygon ID (0-63)
                   31,                 // Alpha (0-31)
                   attr                // OR of GXPolygonAttrMisc's value
        );

    //---------------------------------------------------------------------------
    // Draw a cube:
    // Specify different colors for the vertices.
    //---------------------------------------------------------------------------
    G3_Begin(GX_BEGIN_QUADS);
    {
        ColVtxQuad(2, 0, 4, 6);
        ColVtxQuad(7, 5, 1, 3);
        ColVtxQuad(6, 4, 5, 7);
        ColVtxQuad(3, 1, 0, 2);
        ColVtxQuad(5, 4, 0, 1);
        ColVtxQuad(6, 7, 3, 2);
    }
    G3_End();

    G3_PopMtx(1);

}

static void setupFrame2N(void)
{

    GX_SetDispSelect(GX_DISP_SELECT_MAIN_SUB);

    (void)GX_ResetBankForSubOBJ();
    GX_SetBankForSubBG(GX_VRAM_SUB_BG_128_C);
    GX_SetBankForLCDC(GX_VRAM_LCDC_D);
    GX_SetCapture(GX_CAPTURE_SIZE_256x192,
                  GX_CAPTURE_MODE_A,
                  GX_CAPTURE_SRCA_2D3D, (GXCaptureSrcB)0, GX_CAPTURE_DEST_VRAM_D_0x00000, 16, 0);
    switch (operationMode)
    {
    case 0:
        GX_SetGraphicsMode(GX_DISPMODE_VRAM_D, GX_BGMODE_0, GX_BG0_AS_3D); // Method that uses deflickering
        break;
    case 1:
        GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_3D); // Method that does not use deflickering
        break;
    }
    GX_SetVisiblePlane(GX_PLANEMASK_BG0);
    G2_SetBG0Priority(0);

    GXS_SetGraphicsMode(GX_BGMODE_5);
    GXS_SetVisiblePlane(GX_PLANEMASK_BG2);
    G2S_SetBG2ControlDCBmp(GX_BG_SCRSIZE_DCBMP_256x256,
                           GX_BG_AREAOVER_XLU, GX_BG_BMPSCRBASE_0x00000);
    G2S_SetBG2Priority(0);
    G2S_BG2Mosaic(FALSE);
}

static void setupFrame2N_1(void)
{
    GX_SetDispSelect(GX_DISP_SELECT_SUB_MAIN);
    (void)GX_ResetBankForSubBG();
    GX_SetBankForSubOBJ(GX_VRAM_SUB_OBJ_128_D);
    GX_SetBankForLCDC(GX_VRAM_LCDC_C);
    GX_SetCapture(GX_CAPTURE_SIZE_256x192,
                  GX_CAPTURE_MODE_A,
                  GX_CAPTURE_SRCA_2D3D, (GXCaptureSrcB)0, GX_CAPTURE_DEST_VRAM_C_0x00000, 16, 0);

    switch (operationMode)
    {
    case 0:
        GX_SetGraphicsMode(GX_DISPMODE_VRAM_C, GX_BGMODE_0, GX_BG0_AS_3D); // Method that uses deflickering
        break;
    case 1:
        GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_3D); // Method that does not use deflickering
        break;
    }
    GX_SetVisiblePlane(GX_PLANEMASK_BG0);
    G2_SetBG0Priority(0);

    GXS_SetGraphicsMode(GX_BGMODE_5);
    GXS_SetVisiblePlane(GX_PLANEMASK_OBJ);
}

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
            G2_SetOBJAttr(&sOamBak[idx],
                          x,
                          y,
                          0,
                          GX_OAM_MODE_BITMAPOBJ,
                          FALSE,
                          GX_OAM_EFFECT_NONE,
                          GX_OAM_SHAPE_64x64, GX_OAM_COLOR_16, (y / 8) * 32 + (x / 8), 15, 0);
        }
    }

    DC_FlushRange(&sOamBak[0], sizeof(sOamBak));
    /* I/O register is accessed using DMA operation, so cache wait is not needed */
    // DC_WaitWriteBufferEmpty();
    GXS_LoadOAM(&sOamBak[0], 0, sizeof(sOamBak));

}

//---------------------------------------------------------------------------
// Enable the SwapBuffers execution flag
//---------------------------------------------------------------------------
static void SetSwapBuffersflag(void)
{
    OSIntrMode old = OS_DisableInterrupts();    // Interrupts prohibited
    G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_Z);
    swap = TRUE;
    (void)OS_RestoreInterrupts(old);
}

static BOOL IgnoreRemoval(void)
{
    return FALSE;
}


#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
//#define CPU_TEST                      // Puts a load on the CPU (for testing)
//#define GE_TEST                      // Puts a load on the Geometry Engine (for testing)

//#define SET_FOG                       // Refer to the code shown when this define is enabled if you want to set different fogs for the two screens using Double3D
                                        // 

    u16     Rotate = 0;                // For rotating cubes (0-65535)

    int     i = 0;

#if defined(CPU_TEST) || defined(GE_TEST)
    MATHRandContext32 rnd;             // Variable to generate random number that applies load to CPU (for testing)
#endif
    //---------------------------------------------------------------------------
    // Initialize:
    // Enable IRQ interrupts, initialize VRAM, and set background #0 for 3D mode.
    //---------------------------------------------------------------------------
    DEMOInitCommon();
    DEMOInitVRAM();
    CARD_SetPulledOutCallback(IgnoreRemoval);

    OS_InitThread();
    OS_CreateThread(&thread1, proc1, NULL, stack1 + STACK_SIZE / sizeof(u32), STACK_SIZE,
                    THREAD1_PRIO);
    OS_WakeupThreadDirect(&thread1);

    G3X_Init();
    G3X_InitTable();
    G3X_InitMtxStack();
    G3X_AntiAlias(TRUE);
    setupSubOAM();

    G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_Z);
    G3X_SetClearColor(GX_RGB(0, 0, 0), // Clear color
                      0,               // Clear alpha
                      0x7fff,          // Clear depth
                      63,              // Clear polygon ID
                      FALSE            // Fog
        );

    G3_ViewPort(0, 0, 255, 191);       // Viewport

    //---------------------------------------------------------------------------
    // Set up the projection matrix
    //---------------------------------------------------------------------------
    {
        fx32    right = FX32_ONE;
        fx32    top = FX32_ONE * 3 / 4;
        fx32    near = FX32_ONE;
        fx32    far = FX32_ONE * 400;

        //---------------------------------------------------------------------------
        // Switch MTXMODE to GX_MTXMODE_PROJECTION, and
        // set a projection matrix on the current projection matrix on the matrix stack
        //---------------------------------------------------------------------------
        G3_Perspective(FX32_SIN30, FX32_COS30,  // Sine and cosine of FOVY
                       FX32_ONE * 4 / 3,        // Aspect
                       near,           // Near
                       far,            // Far
                       NULL            // A pointer to a matrix if you use one
            );

        G3_StoreMtx(0);
    }

#if defined(SET_FOG)
    /* Fog configuration */
    {
        u32     fog_table[8];
        int     i;

        // Fog attribute settings
        G3X_SetFog(TRUE,               // Enable fog
                   GX_FOGBLEND_COLOR_ALPHA,     // Apply fog to color and alpha
                   GX_FOGSLOPE_0x2000, // Fog gradient settings
                   0x4800);            // Fog calculation depth value
        G3X_SetFogColor(GX_RGB(31, 31, 31), 0); // Fog color settings

        // Fog table settings (increasing the values thickens the fog)
        for (i = 0; i < 8; i++)
        {
            fog_table[i] = (u32)(((i * 16) << 0) |
                                 ((i * 16 + 4) << 8) |
                                 ((i * 16 + 8) << 16) | ((i * 16 + 12) << 24));
        }
        G3X_SetFogTable(&fog_table[0]);
    }
#endif

    // The foreground (BG0) will be blended for a combination of 2D and 3D
    G2_SetBlendAlpha(GX_BLEND_PLANEMASK_BG0,
                     GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_BG1|GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_BG3|GX_BLEND_PLANEMASK_OBJ|GX_BLEND_PLANEMASK_BD,
                     16,
                     0
        );

    DEMOStartDisplay();

#if defined(CPU_TEST) || defined(GE_TEST)
    MATH_InitRand32(&rnd, OS_GetVBlankCount()); // Generates a random number that applies load to CPU (for testing)
#endif

    while (1)
    {
        DEMOReadKey();

        if(DEMO_IS_TRIG(PAD_BUTTON_Y))
        {
            operationMode = (operationMode+1)%2; // Switch the operating mode
        }

        G3X_Reset();

        Rotate += 256;

        //---------------------------------------------------------------------------
        // Set up camera matrix
        //---------------------------------------------------------------------------
        {
            VecFx32 Eye = { 0, 0, FX32_ONE * 5 };       // Eye position
            VecFx32 at = { 0, 0, 0 };  // Viewpoint
            VecFx32 vUp = { 0, FX32_ONE, 0 };   // Up

            G3_LookAt(&Eye, &vUp, &at, NULL);
        }
        G3_PushMtx();

#if defined(SET_FOG)
        if(DEMO_IS_TRIG(PAD_BUTTON_A))
        {
            if(attr)
            {
                // Disable fog
                attr = GX_POLYGON_ATTR_MISC_NONE;
            }
            else
            {
                // Enable fog
                attr = GX_POLYGON_ATTR_MISC_FOG;
            }
        }
#endif

        if (flip_flag)
            drawRightCube(Rotate);
        else
            drawLeftCube(Rotate);
        G3_PopMtx(1);

#ifdef GE_TEST
        while (GX_GetVCount() != 191)
        {;
        }
#endif

#ifdef CPU_TEST
        // Puts a load on the CPU with a random number (for testing)
        if (DEMO_IS_PRESS(PAD_BUTTON_R))
        {
            OS_SpinWait(MATH_Rand32(&rnd, 1000000));
        }
#endif

#ifdef GE_TEST
        // Puts a load on the Geometry Engine with a random number (for testing)
        while (i <= MATH_Rand32(&rnd, 1000000))
        {
            G3_PushMtx();
            G3_PopMtx(1);
            i++;
        }
#endif

#if defined(SET_FOG)
        // Wait until the V-Count is 180
        // (It is necessary to ensure that the fog process completes after this wait and before the start of the V-Blank.
        //  For now, an ample setting of 180 lines was used.)
        while (GX_GetVCount() != 180)
        {;
        }

        // Only issue in frames where the SwapBuffers command will not end in a processing falloff.
        // (Processing won't hang if the rendering is completed within a V-Count of 180.)
        if (!G3X_IsGeometryBusy())
        {
            SetSwapBuffersflag();          // Issue the SwapBuffers command
            
            if (flip_flag)                 // Flip switch flag
            {
                // Set the fog color for the lower screen
                G3X_SetFogColor(GX_RGB(0, 0, 0), 0);
            }
            else
            {
                // Set the fog color for the upper screen
                G3X_SetFogColor(GX_RGB(31, 31, 31), 0);
            }
        }
#else
        SetSwapBuffersflag();          // Enable the SwapBuffers execution flag
#endif
        OS_WaitVBlankIntr();           // Waiting for the end of the V-Blank interrupt
        switch (operationMode)
        {
        case 1:
            OS_WaitVBlankIntr();           // Waiting for the end of the V-Blank interrupt
            break;
        }
    }
}

//---------------------------------------------------------------------------
// V-Blank interrupt function:
//
// Interrupt handlers are registered on the interrupt table by OS_SetIRQFunction.
// OS_EnableIrqMask selects IRQ interrupts to enable, and
// OS_EnableIrq enables IRQ interrupts.
// Notice that you have to call OS_SetIrqCheckFlag to check a V-Blank interrupt.
//---------------------------------------------------------------------------
void VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK); // Checking V-Blank interrupt

    //---- Start thread 1
    OS_WakeupThreadDirect(&thread1);
}
/*
---------------------------------------------------------------------------------------------------

An explanation of the SwapBuffers command and the Geometry Engine follows

- The SwapBuffers command will be stored in the geometry command FIFO queue when the G3_SwapBuffers function is called.
  The buffers will be swapped at the start of the next V-Blank regardless of when the SwapBuffers command is called from the geometry command FIFO queue.
  (Basically, it can only be executed when a V-Blank starts.)
  

  - Therefore, if rendering or some other process overlaps with the V-Blank interval and the SwapBuffers command could not be run before the start of the V-Blank, the geometry engine will stay busy and wait until the next frame's V-Blank starts (approximately one frame).
  During this time, operations such as rendering graphics or swapping cannot be done, so the image from the last frame is displayed.
  


s: Issue SwapBuffers command
S: Mount the buffer swap operation
w: Wait for the SwapBuffers command to run before beginning the buffer swap operation
G: The Geometry Engine is busy rendering, etc.

                          |
-V-Blank(end)-----------------------------------------
               +-----+    |     +-----+
               |     |    |     |     |
               |     |    |     |     |
               |  G  |    |     |  G  |
               |     |    |     |     |
               |     |    |     |     |
               |     |    |     |     |
               |     |    |     |     |
               |     |    |     |     |
               +-----+    |     |     |
               |  s  |    |     |     |
               +-----+    |     |     |
               |  w  |    |     |     |
-V-Blank(start)+-----+----+-----+     +-----------
  * 784(cycle) |  S  |    |     |     |            * The number of CPU cycles taken by this swap operation is 784 cycles
               +-----+    |     +-----+           
    * check ->            |     |  s  |            * This check uses the G3X_IsGeometryBusy function to determine whether the swap operation has finished
                          |     +-----+                
-V-Blank(end)-------------+-----+     +---------
                          |     |     |
                          |     |     |
                          |     |     |
                          |     |     |
                          |     |     |
                          |     |     |
                          |     |  w  |
                          |     |     |
                          |     |     |
                          |     |     |
                          |     |     |
                          |     |     |
                          |     |     |
                          |     |     |
                          |     |     |
-V-Blank(start)-----------+-----+-----+----------
                          |     |  S  |
                          |     +-----+
                          |
                          |
-V-Blank(end)-------------+-----------------------


* Conditions that generate problems
    - Both the upper and lower screens will display the same image if they were swapped despite the fact that the buffers did not finish swapping during the V-Blank.
      

To avoid this problem, you must do the following inside the V-Blank.
    - Confirm that the next image has finished being rendered before entering a V-Blank.
      If rendering has finished, the buffers will be swapped at the start of the next V-Blank.
    - Confirm that the buffer swap operation has completed inside the V-Blank.
      If the swap operation is finished, switch the upper and lower screens.

    Therefore:
    - The next render is completed before entering a V-Blank, thus ensuring that the swap operation will happen at the start of the V-Blank.
    - The buffer swap operation completes inside the V-Blank.
    You must confirm both of the above.

**Specific processes run in a V-Blank

        -  if(GX_GetVCount() <= 193): The swap operation doesn't even take 1 line, so if 193 lines have been reached at this point we can determine that the swap has finished without the need to wait 784 cycles with the OS_SpinWait function.
                                                   
                                                   
                                                   
                                                   
        - OS_SpinWait(784):                        Waits 784 cycles.
        
        - if (!G3X_IsGeometryBusy() && swap):      If the SwapBuffers command has been run before the V-Blank and the Geometry Engine is not busy
                                                   (Not running a buffer swap operation or rendering graphics)
                                                   
                                                
[] Meaning of the operation
    784 (392 x 2) CPU cycles are required to swap the buffers (switch the data accessed by the rendering engine), so if the buffers started to swap at the start of a V-Blank, it can be inferred that they will have finished swapping after a 784-cycle wait.
    If the geometry engine is busy following a 784-cycle wait from the start of the V-Blank, the buffers have not yet finished being swapped because the rendering of graphics before the V-Blank was delayed.
    -> In this case, neither the swap operation nor a switch between the upper and lower screens should be run.
    If the geometry engine is not busy following a 784-cycle wait from the start of the V-Blank, the buffers have finished being swapped, and the graphics have finished being rendered.
-> Because the render and swap operations have both finished, you should switch the upper and lower screens.
    If you think that these 784 cycles during the V-Blank are a complete waste, add an operation of at least 784 cycles before calling OS_SpinWait(784).
        
        
    
        
        




Although this method cannot be used when no polygons are being rendered, here is another possibility.

void VBlankIntr(void)
{
    if ( (G3X_GetVtxListRamCount() == 0) && swap )
    {
        if (flag)  // Flip switch
        {
            setupFrame2N_1();
        }
        else
        {
            setupFrame2N();
        }


Making modifications as shown above will also prevent the issue.
This method determines that the buffers have been swapped using the G3X_GetVtxListRamCount function (which returns the number of vertices stored in vertex RAM) to confirm that there are no vertices stored in vertex RAM when the buffers have been swapped at the start of a V-Blank.

Note: If an extremely long interrupt occurs before a V-Blank interrupt and the V-Blank starts late, the V-Blank time becomes extremely short (approximately 3 lines or fewer). The upper and lower screens may be switched mid-display, and that frame may show only the previous image.




      
      

---------------------------------------------------------------------------------------------------
*/
void proc1(void *p1)
{
#pragma unused(p1)

    while (1)
    {
#define SPIN_WAIT                      // Switches between the method using the OS_SpinWait function and the method not using it

    // Image rendering also checks whether the swap operation for the buffer has completed
#ifdef SPIN_WAIT

    if (GX_GetVCount() <= 193)
    {
        OS_SpinWait(784);
    }
    if (!G3X_IsGeometryBusy() && swap)

#else

    if ((G3X_GetVtxListRamCount() == 0) && swap)

#endif
    // If the rendering and swap operations are both finished, the upper and lower screens switch
    {
        if (flip_flag)                 // Flip switch (operation to switch the upper and lower screens)
        {
            setupFrame2N_1();
        }
        else
        {
            setupFrame2N();
        }
        swap = FALSE;
        flip_flag = !flip_flag;
    }
    OS_SleepThread(NULL);   // Puts this thread to sleep. It will next wake when OS_WakeupThreadDirect is called
    }
}
