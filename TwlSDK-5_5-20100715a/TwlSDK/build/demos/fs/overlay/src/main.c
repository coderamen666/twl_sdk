/*---------------------------------------------------------------------------*
  Project:  TwlSDK - demos - FS - overlay
  File:     main.c

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/


#include <nitro.h>

#include "DEMO.h"
#include "func.h"


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         LoadOverlayHigh

  Description:  Uses the simplest procedure to load an overlay.
                This process is normally sufficient.

  Arguments:    id: Overlay ID

  Returns:      Whether the load succeeded or failed.
 *---------------------------------------------------------------------------*/
static BOOL LoadOverlayHigh(MIProcessor target, FSOverlayID id)
{
    return FS_LoadOverlay(target, id);
}

/*---------------------------------------------------------------------------*
  Name:         LoadOverlayMiddle

  Description:  Uses a somewhat lengthy procedure to load an overlay.
                This sequence is used when the process of obtaining overlay information must be separate from the process of loading the actual module.
                

  Arguments:    id: Overlay ID

  Returns:      Whether the load succeeded or failed.
 *---------------------------------------------------------------------------*/
static BOOL LoadOverlayMiddle(MIProcessor target, FSOverlayID id)
{
    BOOL            retval = FALSE;
    FSOverlayInfo   info;
    if (FS_LoadOverlayInfo(&info, target, id))
    {
        if (FS_LoadOverlayImage(&info))
        {
            FS_StartOverlay(&info);
            retval = TRUE;
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         LoadOverlayLow

  Description:  Uses a combination of only the lowest-level functions to load an overlay.
                Refer to this sequence if you want the application to manage overall progress in detail, such as when it uses the WFS library to get an overlay wirelessly.
                
                

  Arguments:    id: Overlay ID

  Returns:      Whether the load succeeded or failed.
 *---------------------------------------------------------------------------*/
static BOOL LoadOverlayLow(MIProcessor target, FSOverlayID id)
{
    BOOL            retval = FALSE;
    FSOverlayInfo   info;
    if (FS_LoadOverlayInfo(&info, target, id))
    {
        FSFile  file;
        FS_InitFile(&file);
        (void)FS_LoadOverlayImageAsync(&info, &file);
        (void)FS_WaitAsync(&file);
        (void)FS_CloseFile(&file);
        FS_StartOverlay(&info);
        retval = TRUE;
    }
    return retval;
}

// The High, Middle, and Low load processes are all equivalent.
// Choose the appropriate method after considering the scenario in which it will be used.
#define	LoadOverlay(id)	LoadOverlayLow(MI_PROCESSOR_ARM9, id)

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Application's main entry.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    OS_Init();
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplayBitmap();
    DEMOHookConsole();

    DEMOSetBitmapTextColor(GX_RGBA(31, 31, 0, 1));
    DEMOSetBitmapGroundColor(DEMO_RGB_CLEAR);
    DEMOStartDisplay();

    FS_Init(FS_DMA_NOT_USE);

    OS_TPrintf("--------------------------------\n"
               "overlay sample.\n");

    {
        // An overlay ID written in the LSF file must be declared explicitly.
        // (The actual symbol is automatically created by the LCF template file at link time)
        FS_EXTERN_OVERLAY(main_overlay_1);
        FS_EXTERN_OVERLAY(main_overlay_2);
        FS_EXTERN_OVERLAY(main_overlay_3);

        /* *INDENT-OFF* */
        static struct
        {
            FSOverlayID     id;
            const char     *name;
        }
        list[] =
        {
            { FS_OVERLAY_ID(main_overlay_1), "main_overlay_1", },
            { FS_OVERLAY_ID(main_overlay_2), "main_overlay_2", },
            { FS_OVERLAY_ID(main_overlay_3), "main_overlay_3", },
        };
        static const int    overlay_max = sizeof(list) / sizeof(*list);
        /* *INDENT-ON* */

        int     i;
        int     loaded_overlay = -1;
        int     current = 0;

        // Display the address of the function in each overlay.
        // Note, in particular, that func_1 and func_2 have the same address.
        OS_TPrintf("func_1() : addr = 0x%08X.\n", func_1);
        OS_TPrintf("func_2() : addr = 0x%08X.\n", func_2);
        OS_TPrintf("func_3() : addr = 0x%08X.\n", func_3);

        // Select and overlay and load it
        for (;;)
        {
            DEMOReadKey();
            // Load or unload the currently selected overlay with the A Button
            if (DEMO_IS_TRIG(PAD_BUTTON_A))
            {
                if (loaded_overlay == -1)
                {
                    (void)LoadOverlay(list[current].id);
                    loaded_overlay = current;
                }
                else
                {
                    (void)FS_UnloadOverlay(MI_PROCESSOR_ARM9, list[loaded_overlay].id);
                    loaded_overlay = -1;
                }
            }
            // Call the function in the current overlay once with the B Button
            if (DEMO_IS_TRIG(PAD_BUTTON_B))
            {
                // If multiple overlay regions conflict, the FS library guarantees behavior only for the last one to be loaded.
                // Take note of this when using a function in an overlay.
                // 
                if (loaded_overlay == 0)
                {
                    func_1();
                }
                else if (loaded_overlay == 1)
                {
                    func_2();
                }
                else if (loaded_overlay == 2)
                {
                    func_3();
                }
            }
            // Move the cursor by pressing Up and Down
            if (DEMO_IS_TRIG(PAD_KEY_DOWN))
            {
                if (++current >= overlay_max)
                {
                    current -= overlay_max;
                }
            }
            else if (DEMO_IS_TRIG(PAD_KEY_UP))
            {
                if (--current < 0)
                {
                    current += overlay_max;
                }
            }
            // Displays screen
            {
                int     ox = 10;
                int     oy = 60;
                DEMOFillRect(0, 0, GX_LCD_SIZE_X, GX_LCD_SIZE_Y, DEMO_RGB_CLEAR);
                DEMOSetBitmapTextColor(GX_RGBA(0, 31, 0, 1));
                DEMODrawText(10, 10, "UP&DOWN :select overlay");
                DEMODrawText(10, 20, "A button:load/unload overlay");
                DEMODrawText(10, 30, "B button:call function");
                DEMOSetBitmapTextColor(GX_RGBA(31, 31, 31, 1));
                DEMODrawFrame(ox, oy, 240, 10 + overlay_max * 10, GX_RGBA( 0, 31, 0, 1));
                for (i = 0; i < overlay_max; ++i)
                {
                    BOOL    focus = (i == current);
                    BOOL    loaded = (list[i].id == loaded_overlay);
                    DEMOSetBitmapTextColor((GXRgb)(focus ? GX_RGBA(31, 31, 0, 1) : GX_RGBA(31, 31, 31, 1)));
                    DEMODrawText(ox + 10, oy + 5 + i * 10, "%s%s %s",
                                 focus ? ">" : " ", list[i].name, loaded ? "(loaded)" : "");
                }
            }
            DEMO_DrawFlip();
            OS_WaitVBlankIntr();
        }
    }

    OS_Terminate();
}
