/*---------------------------------------------------------------------------*
  Project:  TwlSDK - DSP - demos - scaling-1
  File:     main.c

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2010-05-17#$
  $Rev: 11335 $
  $Author: kitase_hirotake $
*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
 The demo enlarges (or shrinks) 640x480 image data to fit a specified scale ratio and/or area.
 
 Three interpolation methods are available to choose from when scaling.
 
 - How to Use
     -- A : Change the interpolation mode
           Cycles in order through Nearest Neighbor, Bilinear, Bicubic, Nearest Neighbor, ....
           
     -- UP, DOWN, RIGHT, LEFT : Moves the area.
           Moves the area where the scaling will be performed.
           Movement cannot exceed the boundaries of the 640x480 image.
           
     -- START : Ends the demo.
     
 - Other Notes
     -- To change the scaling ratio, change the SCALING_FACTOR_X and SCALING_FACTOR_Y constants below and rebuild the demo.
       
     
     -- To change the DSP processing area size, change AREA_WIDTH and AREA_HEIGHT and rebuild the demo.
       
       
 ----------------------------------------------------------------------------*/

#include <twl.h>
#include <twl/dsp.h>

#include <DEMO.h>
#include <twl/dsp/common/graphics.h>

/* Settings */
#define RUN_ASYNC     0            // Whether to scale asynchronously
#define DMA_NO_FOR_FS 1            // For FS_Init

#define AREA_WIDTH   185            // Width of the area to scale
#define AREA_HEIGHT  140            // Height of the area to scale

#define SCALING_FACTOR_X 1.4f      // Scale ratio for the X direction (valid to the third decimal place, with an upper limit of 31)
#define SCALING_FACTOR_Y 1.4f      // Scale ratio for the Y direction (valid to the third decimal place, with an upper limit of 31)

#define OUTPUT_WIDTH  DSP_CALC_SCALING_SIZE(AREA_WIDTH, SCALING_FACTOR_X)  // Consider post-processing resolution and rounding errors caused by the f32 type
#define OUTPUT_HEIGHT DSP_CALC_SCALING_SIZE(AREA_HEIGHT, SCALING_FACTOR_Y)

/*---------------------------------------------------------------------------*
 Image data (640x480)
 *---------------------------------------------------------------------------*/
extern const u8 _binary_output_dat[];
extern const u8 _binary_output_dat_end[];

#define DATA_WIDTH    640
#define DATA_HEIGHT   480

/*---------------------------------------------------------------------------*
 Prototype Declarations
*---------------------------------------------------------------------------*/
void VBlankIntr(void);

static void ExecScaling(void);
static void InitializeGraphics(void);
static void WriteScreenBuffer(u16 *data, u32 width, u32 height, u16 *scr);
static void ScalingCallbackFunc(void);

/*---------------------------------------------------------------------------*
 Internal Variable Definitions
*---------------------------------------------------------------------------*/
// Screen buffers for main and sub-screens
static u16 ScrBuf[HW_LCD_WIDTH * HW_LCD_HEIGHT] ATTRIBUTE_ALIGN(32);
// Buffer storing results after transformation by the DSP
static u16 TmpBuf[OUTPUT_WIDTH * OUTPUT_HEIGHT] ATTRIBUTE_ALIGN(32);

static OSTick StartTick;       // Variable for measuring DSP processing time
static BOOL IsDspProcessing;   // Whether DSP is currently processing something (used when running this as an asynchronous process)

static u16 AreaX = 0;          // Upper-left x-coordinate of area to scale
static u16 AreaY = 0;          // Upper-left y-coordinate of area to scale

// Interpolation mode
static u16 ModeNames[3] = {
    DSP_GRAPHICS_SCALING_MODE_N_NEIGHBOR,
    DSP_GRAPHICS_SCALING_MODE_BILINEAR,
    DSP_GRAPHICS_SCALING_MODE_BICUBIC
    };

static u8 ModeNameStrings[3][24] = {
        "Nearest Neighbor",
        "Bilinear",
        "Bicubic"
        };

static u32 DspMode = 0;

/*---------------------------------------------------------------------------*
 Name:         TwlMain

 Description:  Initialization and main loop.

 Arguments:    None.

 Returns:      None.
*---------------------------------------------------------------------------*/
void TwlMain(void)
{
    FSFile file;
    
    DEMOInitCommon();
    OS_InitThread();
    OS_InitTick();
    OS_InitAlarm();             // This is required when using synchronous versions of DSP_Scaling* functions (because the OS_Sleep function is used internally)

    // When in NITRO mode, stopped by Panic
    DEMOCheckRunOnTWL();

    DEMOInitVRAM();
    InitializeGraphics();

    DEMOStartDisplay();

    // Because at first there is a possibility that WRAM might have been allocated to something as per the ROM header, clear it
    (void)MI_FreeWram_B( MI_WRAM_ARM9 );
    (void)MI_CancelWram_B( MI_WRAM_ARM9 );
    (void)MI_FreeWram_C( MI_WRAM_ARM9 );
    (void)MI_CancelWram_C( MI_WRAM_ARM9 );
    (void)MI_FreeWram_B( MI_WRAM_ARM7 );
    (void)MI_CancelWram_B( MI_WRAM_ARM7 );
    (void)MI_FreeWram_C( MI_WRAM_ARM7 );
    (void)MI_CancelWram_C( MI_WRAM_ARM7 );
    
    FS_Init(DMA_NO_FOR_FS);

    (void)OS_EnableInterrupts();

    IsDspProcessing = FALSE;
    
    // Clear the screen buffer
    MI_CpuClear8(ScrBuf, sizeof(ScrBuf));

    // Load graphics component
    DSP_OpenStaticComponentGraphics(&file);
    if(!DSP_LoadGraphics(&file, 0xFF, 0xFF))
    {
        OS_TPanic("failed to load graphics DSP-component! (lack of WRAM-B/C)");
    }
    
    // Initial execution
    ExecScaling();
    
    while (1)
    {
        DEMOReadKey();
        
        if (DEMO_IS_TRIG( PAD_BUTTON_START ))
        {
            break;    // Quit
        }
        
        // Move area targeted for processing
        if (DEMO_IS_PRESS( PAD_KEY_RIGHT ))
        {
            AreaX += 5;
            
            if (AreaX >= DATA_WIDTH - AREA_WIDTH - 1)
            {
                AreaX = DATA_WIDTH - AREA_WIDTH - 1;
            }
            ExecScaling();
        }
        else if (DEMO_IS_PRESS( PAD_KEY_LEFT ))
        {
            if (AreaX != 0)
            {
                if (AreaX <= 5)
                {
                    AreaX = 0;
                }
                else
                {
                    AreaX -= 5;
                }
            
                ExecScaling();
            }
        }
        
        if (DEMO_IS_PRESS( PAD_KEY_UP ))
        {
            if (AreaY != 0)
            {
                if (AreaY <= 5)
                {
                    AreaY = 0;
                }
                else
                {
                    AreaY -= 5;
                }
            
                ExecScaling();
            }
        }
        else if (DEMO_IS_PRESS( PAD_KEY_DOWN ))
        {
            AreaY += 5;
            if (AreaY >= DATA_HEIGHT - AREA_HEIGHT - 1)
            {
                AreaY = DATA_HEIGHT - AREA_HEIGHT - 1;
            }
            ExecScaling();
        }
        
        // Change the interpolation mode
        if (DEMO_IS_TRIG( PAD_BUTTON_A ))
        {
            DspMode++;
            if (DspMode >= 3)
            {
                DspMode = 0;
            }
            
            ExecScaling();
        }

        OS_WaitVBlankIntr();           // Waiting for the end of the V-Blank interrupt
    }
    
    OS_TPrintf("demo end.\n");

    // Unload graphics component
    DSP_UnloadGraphics();
    OS_Terminate();
}

//--------------------------------------------------------------------------------
//    V-Blank interrupt process
//
void VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK); // Checking V-Blank interrupt
}

/*--------------------------------------------------------------------------------
    Scale the image data using the DSP and the configured options
 ---------------------------------------------------------------------------------*/
static void ExecScaling()
{
    // Execute
#if RUN_ASYNC

	if ( !IsDspProcessing )
	{
		StartTick = OS_GetTick();

	    DSP_ScalingAsyncEx(_binary_output_dat, TmpBuf, DATA_WIDTH, DATA_HEIGHT,
	                       SCALING_FACTOR_X, SCALING_FACTOR_Y, ModeNames[DspMode], AreaX, AreaY, AREA_WIDTH, AREA_HEIGHT, ScalingCallbackFunc);

	    IsDspProcessing = TRUE;
	}

#else
	StartTick = OS_GetTick();
    (void)DSP_ScalingEx(_binary_output_dat, TmpBuf, DATA_WIDTH, DATA_HEIGHT,
                  SCALING_FACTOR_X, SCALING_FACTOR_Y, ModeNames[DspMode], AreaX, AreaY, AREA_WIDTH, AREA_HEIGHT);
    OS_TPrintf("mode: %s, time: %d microsec.\n", ModeNameStrings[DspMode], OS_TicksToMicroSeconds(OS_GetTick() - StartTick));

    // Adjust data for display on screen
    WriteScreenBuffer(TmpBuf, OUTPUT_WIDTH, OUTPUT_HEIGHT, ScrBuf);
    
    // Destroy the screen buffer cache
    DC_FlushAll();

    // Load processing results to VRAM
    GX_LoadBG3Bmp(ScrBuf, 0, HW_LCD_WIDTH * HW_LCD_HEIGHT * sizeof(u16));
#endif

}

static void InitializeGraphics()
{
    // VRAM allocation
    GX_SetBankForBG(GX_VRAM_BG_128_A);
    GX_SetBankForSubBG(GX_VRAM_SUB_BG_128_C);
    
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_5, GX_BG0_AS_2D);
    GX_SetVisiblePlane( GX_PLANEMASK_BG3 );
    GXS_SetGraphicsMode( GX_BGMODE_4 );
    GXS_SetVisiblePlane( GX_PLANEMASK_BG3 );
    
    GX_SetBGScrOffset(GX_BGSCROFFSET_0x00000);  // Set screen offset value
    GX_SetBGCharOffset(GX_BGCHAROFFSET_0x20000);  // Configure character base offset value
    
    G2_BlendNone();
    G2S_BlendNone();
    GX_Power2DSub(TRUE);    // Turn the sub 2D graphic engine off
    
    // Main-BG
    // BG3: scr 96KB
    {
        G2_SetBG3ControlDCBmp(GX_BG_SCRSIZE_DCBMP_256x256, GX_BG_AREAOVER_XLU, GX_BG_BMPSCRBASE_0x00000);
        G2_SetBG3Priority(2);
        G2_BG3Mosaic(FALSE);
    }
    
    // Sub-BG
    // BG3: scr 96KB
    {
        G2S_SetBG3ControlDCBmp(GX_BG_SCRSIZE_DCBMP_256x256, GX_BG_AREAOVER_XLU, GX_BG_BMPSCRBASE_0x00000);
        G2S_SetBG3Priority(2);
        G2S_BG3Mosaic(FALSE);
    }
}

/* Write the contents of 'data' to a 256x192 screen buffer */
static void WriteScreenBuffer(u16 *data, u32 width, u32 height, u16 *scr)
{
    int i;
    u32 lp_count;
    u32 tmp_linesize;
    
    // Create scrbuf
    if( height > HW_LCD_HEIGHT )
    {
        lp_count = HW_LCD_HEIGHT;
    }
    else
    {
        lp_count = height;
    }
    
    // Because it is displayed on the lower screen as a 256x256 BMP, you need to take size into account.
    // Copy line by line
    if( width > HW_LCD_WIDTH)
    {
        tmp_linesize = HW_LCD_WIDTH * sizeof(u16);
    }
    else
    {
        tmp_linesize = width * sizeof(u16);
    }
    
    for ( i=0; i < lp_count; i++ )
    {
        MI_CpuCopy( data + width * i, scr + HW_LCD_WIDTH * i, tmp_linesize );
    }
}

/* Callback function called when scaling is done */
static void ScalingCallbackFunc(void)
{
    OS_TPrintf("[Async]mode: %s, time: %d microsec.\n", ModeNameStrings[DspMode], OS_TicksToMicroSeconds(OS_GetTick() - StartTick));

	    // Adjust data for display on screen
    WriteScreenBuffer(TmpBuf, OUTPUT_WIDTH, OUTPUT_HEIGHT, ScrBuf);
	    
    // Destroy the screen buffer cache
    DC_FlushAll();

    // Load processing results to VRAM
    GX_LoadBG3Bmp(ScrBuf, 0, HW_LCD_WIDTH * HW_LCD_HEIGHT * sizeof(u16));

    IsDspProcessing = FALSE;
}

