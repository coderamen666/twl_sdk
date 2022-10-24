/*---------------------------------------------------------------------------*
  Project:  TwlSDK - build - demos - tips - DisplayLogo
  File:     main.c

  Copyright 2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-11-24#$
  $Rev: 11184 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
// This demo sample displays the Licensed by Nintendo logo.
// It fades in (0.27 sec), then displays the logo for 1 second, then fades out (0.27 sec).
// 
// Press the A button during logo display to cancel the 1-second logo display and start the fade-out.
// 
//---------------------------------------------------------------------------

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif
#include "data.h"

/* Constants */
#define SCREEN_SIZE 256 * 192 * 2 // Data size (in bytes)
#define DISP_FRAME_NUM 60         // Constant used in the timer that measures 1 second (60 frames)
#define BRIGHTNESS_MAX 16         // Maximum brightness. Used as the initial value

typedef enum
{
    FADE_IN  = 0,
    DISPLAY  = 1,
    FADE_OUT = 2,
    END      = 3
}
DISP_STATE;

typedef struct
{
    u16     trigger;
    u16     press;
}
KeyWork;

/* Function Prototypes */
void Init(void);          // General initialization
void VBlankIntr(void);    // V-Blank interrupt definition
void ReadKey(KeyWork*);
void TwlMain(void);

/*---------------------------------------------------------------------------*
  Name:         Init

  Description:  Performs general initialization. Goes as far as loading the image data.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/

void Init(void)
{
    // Initialize each library
    OS_Init();
    FX_Init();

    GX_Init();

    GX_DispOff();
    GXS_DispOff();

    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();

    (void)GX_VBlankIntr(TRUE);

    //VRAM initialization
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);                          //Assign all VRAM banks to LCDC
    MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);     //Zero-clear the memory
    (void)GX_DisableBankForLCDC();                                //Deallocate the VRAM banks


/* Initialize the upper screen (this screen displays the Licensed by Nintendo logo in this demo)*/

    GX_SetBankForBG(GX_VRAM_BG_128_A);               // VRAM-A for BG

    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS,         // Graphics mode
                       GX_BGMODE_0,                  // BGMODE is 0
                       GX_BG0_AS_2D);                // BG #0 is for 2D

    G2_SetBG0Control(GX_BG_SCRSIZE_TEXT_256x256,     // 256pix x 256pix text
                     GX_BG_COLORMODE_16,             // Logo can be expressed in 16 colors
                     GX_BG_SCRBASE_0x0000,           // Screen base offset setting
                     GX_BG_CHARBASE_0x04000,         // Character base offset setting
                     GX_BG_EXTPLTT_01                // When using the BG extended palette, use slot 0
                     );

    GX_SetVisiblePlane(GX_PLANEMASK_BG0);            // Select BG0 for display
    G2_SetBG0Priority(0);                            // Set BG0 priority to top
    G2_BG0Mosaic(FALSE);                             // Do not apply mosaic effects to BG0

    GX_LoadBG0Char(data_Char, 0, sizeof(data_Char)); // Load logo images
    GX_LoadBGPltt(data_Palette, 0, sizeof(data_Palette));
    GX_LoadBG0Scr(data_Screen, 0, sizeof(data_Screen));

    GX_SetMasterBrightness(BRIGHTNESS_MAX);          // Fade in from a blank, white screen

/* Initialization of upper screen comes here and no further */

    OS_WaitVBlankIntr();
    GX_DispOn();

}

// V-Blank interrupt settings
void VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK); // Set V-Blank interrupt confirmation flag
}

// Key reading settings
void ReadKey(KeyWork* key)
{
    u16     readData = PAD_Read();
    key->trigger = (u16)(readData & (readData ^ key->press));
    key->press = readData;
}

/*---------------------------------------------------------------------------*
  Name:         Twl/NitroMain

  Description:  Main

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/

#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    KeyWork Key;                        // Variable for pressed buttons or keys
    DISP_STATE State = FADE_IN;         // Variable expressing state
    int DispFrame = DISP_FRAME_NUM;     // Variable to measure 1 second of display time
    int Brightness = BRIGHTNESS_MAX;    // Variable expressing brightness

    Init();                             // Initialization
    OS_Printf("***********************************************\n");
    OS_Printf("*    Start Displaying the Logo.               *\n"); // Display message
    OS_Printf("*    A BUTTON :  Stop displaying the Logo.    *\n");
    OS_Printf("***********************************************\n");
 
    while(1)
    {
        ReadKey(&Key);                  // Read keys
        switch (State) 
        {
            case FADE_IN:
                if ( (Brightness = GX_GetMasterBrightness()) > 0)    // Fade-in before display
                {
                    GX_SetMasterBrightness( Brightness - 1);
                }
                else
                {
                    State = DISPLAY;
                }
                break;
            case DISPLAY:                                      // Displaying. By default, display for 60 frames (1 second).
                if ( DispFrame > 0 )
                {
                    if ( Key.trigger & PAD_BUTTON_A )          // Press A Button to cancel (input of this button not accepted during fade-in or fade-out)
                    {
                        State = FADE_OUT;
                    }
                    else
                    {
                        DispFrame--;
                    }
                }
                else
                {
                    State = FADE_OUT;
                }
                break;
            case FADE_OUT:                                     // Fade out when display ends. Also fade out if display is canceled by button input.
                if ( (Brightness = GX_GetMasterBrightness()) < BRIGHTNESS_MAX )
                {
                    GX_SetMasterBrightness( Brightness + 1);
                }
                else
                {
                    State = END;
                }
                break;
            case END:
                break;
            default:
                break;
        }
        
        if ( State == END )
        {
            break;
        }
        
        OS_WaitVBlankIntr();
    }
    
    OS_Printf( "==== Finish sample ====\n" );
    OS_Terminate();
}
