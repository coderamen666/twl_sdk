/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - multiboot-wfs - parent
  File:     parent.c

  Copyright 2005-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-12-08#$
  $Rev: 9555 $
  $Author: kitase_hirotake $
 *---------------------------------------------------------------------------*/


#ifdef SDK_TWL
#include    <twl.h>
#else
#include    <nitro.h>
#endif
#include <nitro/wm.h>
#include <nitro/wbt.h>
#include <nitro/fs.h>

#include    "wfs.h"
#include    "wh.h"

#include    "util.h"
#include    "common.h"


/*---------------------------------------------------------------------------*
    Internal Variable Definitions
 *---------------------------------------------------------------------------*/

/* Transfer configuration adjustment */
static BOOL is_parent_sync = FALSE;
static int parent_packet_size = parent_packet_max;


/*---------------------------------------------------------------------------*
    Function Definitions
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
  Name:         ModeSelect

  Description:  Process in parent/child selection screen.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ModeSelect(void)
{
    KeyInfo key[1];
    int     i;
    int     file_mode = 0;

    enum
    {
        mode_default, mode_shared, mode_max
    };
    static const char *(mode_name[]) =
    {
    "use child's own FS", "share parent's FS",};

    InitFrameLoop(key);
    while (WH_GetSystemState() == WH_SYSSTATE_IDLE)
    {
        WaitForNextFrame(key);

        PrintString(3, 10, COLOR_WHITE, "Press A to connect as PARENT");
        PrintString(2, 3, COLOR_GREEN, "<UP & DOWN key>");
        PrintString(2, 4, COLOR_WHITE, "child's FS mode:");
        for (i = 0; i < mode_max; ++i)
        {
            const BOOL focused = (i == file_mode);
            PrintString(5, 5 + i, focused ? COLOR_YELLOW : COLOR_WHITE,
                        "%c%s", focused ? '>' : ' ', mode_name[i]);
        }
        PrintString(5, 5 + mode_max, COLOR_GRAY, " (overlay is always own.)");

        /*
         * Press Up and Down on the +Control Pad to select one of the following file system modes.
         *   - File system of the child program for DS Download Play
         *   - File system of the parent program
         * The former is considered to be the normal mode, but the latter can be used to efficiently reduce ROM usage for applications that share most data resources between the parent and children.
         * 
         * 
         *
         * Note: In either case, however, the overlay information is always provided by the child program.
         *   
         *   Applications that share overlays must be configured to clone boot.
         *   
         */
        if ((key->trg & PAD_KEY_DOWN) && (++file_mode >= mode_max))
        {
            file_mode -= mode_max;
        }
        if ((key->trg & PAD_KEY_UP) && (--file_mode < 0))
        {
            file_mode += mode_max;
        }

        /*
         * Use the A button to start wireless processing as a parent.
         * In order for the WFS library to run using WM_SetPortCallback, call WFS_Init after WM_Initialize.
         * 
         */
        if (key->trg & PAD_BUTTON_A)
        {
            /*
             * Loading a table is only effective for a parent.
             * For children, the parent's file system table information is held by dynamically-allocated memory in the WFS library, so there is no reason to load this table again.
             * 
             * 
             */
            static BOOL table_loaded = FALSE;
            if (!table_loaded)
            {
                u32     need_size = FS_GetTableSize();
                void   *p_table = OS_Alloc(need_size);
                SDK_ASSERT(p_table != NULL);
                table_loaded = FS_LoadTable(p_table, need_size);
                if (!table_loaded && p_table)
                {
                    OS_Free(p_table);
                }
            }
            
            (void)WH_ParentConnect(WH_CONNECTMODE_MP_PARENT, 0x0000, 1);
            /*
             * The parent starts the WFS library and becomes the file server.
             * The files and overlays requested by children during DS Download Play are all provided automatically by the parent through the WFS library.
             * 
             */
            {
                FSFile  file[1];
                FS_InitFile(file);
                if (!FS_OpenFileEx(file, "data/main.srl", FS_FILEMODE_R))
                {
                    OS_TPanic("failed to open DS-downloaded program file!");
                }
                WFS_InitParent(port_wbt, StateCallbackForWFS, AllocatorForWFS,
                               NULL, parent_packet_max, file, (file_mode == mode_shared));
                WFS_EnableSync(0);
                (void)FS_CloseFile(file);
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         ModeParent

  Description:  Processing in parent communications screen.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ModeParent(void)
{
    KeyInfo key[1];

    InitFrameLoop(key);
    while (WH_GetSystemState() == WH_SYSSTATE_CONNECTED && WH_GetConnectMode() == WH_CONNECTMODE_MP_PARENT)
    {

        int     parent_packet_size = WFS_GetPacketSize();
        BOOL    is_parent_sync = ((WFS_GetSyncBitmap() & ~1) == (WFS_GetCurrentBitmap() & ~1));

        WaitForNextFrame(key);

        PrintString(8, 1, COLOR_WHITE, "Parent mode");
        PrintString(2, 20, COLOR_WHITE, "Press START to disconnect.");


        {
            /*
             * Switch the packet size by pressing Up and Down.
             *
             * This simply affects the transfer speed.
             * Note: When a change is made, it will not be applied to any processes in the middle of accessing data.
             */
            int     new_packet_size = parent_packet_size;

            if (key->trg & PAD_KEY_UP)
            {
                new_packet_size += 2;
                if (new_packet_size > parent_packet_max)
                    new_packet_size = WBT_PACKET_SIZE_MIN + 2;
            }
            if (key->trg & PAD_KEY_DOWN)
            {
                new_packet_size -= 2;
                if (new_packet_size < WBT_PACKET_SIZE_MIN + 2)
                    new_packet_size = (parent_packet_max & ~1);
            }
            if (parent_packet_size != new_packet_size)
            {
                parent_packet_size = new_packet_size;
                WFS_SetPacketSize(parent_packet_size);
            }
        }

        /*
         * Press the A button to switch to synchronous mode.
         *
         * This will cause access to be blocked for the first child if even one of the children specified to be synchronous does not receive the request.
         * 
         * This setting can make communications more efficient if it is clear that the same combination of files will be requested in the same order.
         * 
         *
         * This setting will be reset automatically and a warning message will be sent to debug output when a group of children has conflicting access requests.
         * 
         */
        if (key->trg & PAD_BUTTON_A)
        {
            is_parent_sync = !is_parent_sync;
            WFS_EnableSync(is_parent_sync ? WFS_GetCurrentBitmap() : 0);
        }

        if (key->trg & PAD_BUTTON_START)
        {
            WH_Finalize();
            while (WH_GetSystemState() != WH_SYSSTATE_IDLE) {}
        }

        /* Display the current configuration and connection state */
        PrintString(3, 4, COLOR_GREEN, "<UP & DOWN key>");
        PrintString(3, 5, COLOR_WHITE, "parent packet ... %d BYTE", parent_packet_size);

        PrintString(3, 7, COLOR_GREEN, "<toggle A button>");
        PrintString(3, 8, COLOR_WHITE, "sync mode     ... %s",
                    is_parent_sync ? "ENABLE" : "DISABLE");

        PrintString(3, 10, COLOR_GREEN, "bitmap status");
        PrintString(5, 11, COLOR_GREEN, "-  not connected");
        PrintString(5, 12, COLOR_GREEN, "+  idle");
        PrintString(5, 13, COLOR_GREEN, "*  busy");
        PrintString(5, 14, COLOR_GREEN, "!  sync-blocking");

        PrintString(12, 15, COLOR_BLUE, "0123456789ABCDEF");
        {
            int     i;
            const int cur_bitmap = WFS_GetCurrentBitmap();
            const int busy_bitmap = WFS_GetBusyBitmap();
            const int sync_bitmap = WFS_GetSyncBitmap();
            for (i = 0; i < sizeof(u16) * 8; ++i)
            {
                char    c;
                const int bit = (1 << i);
                if ((bit & busy_bitmap) != 0)
                    c = '*';
                else if ((bit & cur_bitmap) != 0)
                    c = '+';
                else
                    c = '-';
                PrintString(12 + i, 16, COLOR_WHITE, "%c", c);
                if ((bit & sync_bitmap) != 0)
                    PrintString(12 + i, 17, COLOR_WHITE, "!");
            }
        }
    }
}


/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
