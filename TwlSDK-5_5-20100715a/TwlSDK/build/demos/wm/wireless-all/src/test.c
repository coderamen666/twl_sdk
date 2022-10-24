/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WM - demos - wireless-all
  File:     test.c

  Copyright 2006-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-10-02#$
  $Rev: 8827 $
  $Author: yosizaki $
 *---------------------------------------------------------------------------*/

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif

#include "common.h"


/*****************************************************************************/
/* Variables */

/* Variable for parent WFS testing */
static BOOL is_parent_sync = FALSE;
static int parent_packet_size = WH_PARENT_MAX_SIZE;

/* Variable for child WFS testing */
static u8 tmp_buf[1024 * 128];
static int file_index = -1;
static OSTick read_tick;
static int read_byte;
static int read_time;
static BOOL file_sync_mode;
static FSFile file[1];

static const char *file_path[] = {
    "test/16kB.bin",
    "test/64kB.bin",
};
static const int file_max = sizeof(file_path) / sizeof(*file_path);


/*****************************************************************************/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         BeginFileTest

  Description:  Begins file read testing.

  Arguments:    None.

  Returns:      TRUE when asynchronous read begins normally.
 *---------------------------------------------------------------------------*/
static BOOL BeginFileTest(void)
{
    BOOL    retval = FALSE;
    if (FS_OpenFileEx(file, file_path[file_index], FS_FILEMODE_R))
    {
        read_tick = OS_GetTick();
        read_byte = (int)FS_GetFileLength(file);
        read_time = 0;
        if (read_byte > (int)sizeof(tmp_buf))
        {
            read_byte = (int)sizeof(tmp_buf);
        }
        OS_TPrintf("reading... %d BYTE\n", read_byte);
        if (FS_ReadFileAsync(file, tmp_buf, sizeof(tmp_buf)))
        {
            retval = TRUE;
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         EndFileTest

  Description:  Completes the file read testing.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void EndFileTest(void)
{
    if (FS_IsSucceeded(file))
    {
        u64     milli = OS_TicksToMilliSeconds(OS_GetTick() - read_tick);
        if (!milli)
        {
            milli = 1;
        }
        read_time = (int)(((u64)read_byte * 8 * 1000) / milli);
    }
    else
    {
        read_time = -1;
    }
    (void)FS_CloseFile(file);
}

/*---------------------------------------------------------------------------*
  Name:         UpdateFrame

  Description:  Frame update process.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void UpdateFrame(void)
{
    /* Parent operations */
    if (!MB_IsMultiBootChild())
    {
        /* Switch the transmission packet size */
        int     new_packet_size = parent_packet_size;
        if (IS_PAD_TRIGGER(PAD_KEY_UP))
        {
            new_packet_size += 2;
            if (new_packet_size > WH_PARENT_MAX_SIZE)
                new_packet_size = 16;
        }
        if (IS_PAD_TRIGGER(PAD_KEY_DOWN))
        {
            new_packet_size -= 2;
            if (new_packet_size < 16)
                new_packet_size = WH_PARENT_MAX_SIZE;
        }
        if (parent_packet_size != new_packet_size)
        {
            parent_packet_size = new_packet_size;
            WFS_SetPacketSize(parent_packet_size);
        }
        /*
         * Press the A Button to switch to synchronous mode.
         *
         * This will cause access to be blocked for the first child if even one of the children specified to be synchronous does not receive the request.
         * 
         * This setting can make communications more efficient if it is clear that the same combination of files will be requested in the same order.
         * 
         *
         * This setting will be reset automatically and a warning message will be sent to debug output when a group of children has conflicting access requests.
         * 
         */
        if (IS_PAD_TRIGGER(PAD_BUTTON_A))
        {
            is_parent_sync = !is_parent_sync;
            WFS_EnableSync(is_parent_sync ? WFS_GetCurrentBitmap() : 0);
        }
    }
    /* Child operations */
    else
    {
        /* Select the file that should be read */
        if (IS_PAD_TRIGGER(PAD_KEY_UP))
        {
            if (--file_index < 0)
            {
                file_index = file_max - 1;
            }
        }
        if (IS_PAD_TRIGGER(PAD_KEY_DOWN))
        {
            if (++file_index > file_max - 1)
            {
                file_index = 0;
            }
        }
        /* Switch between synchronous and asynchronous modes for testing */
        if (IS_PAD_TRIGGER(PAD_KEY_LEFT) || IS_PAD_TRIGGER(PAD_KEY_RIGHT))
        {
            file_sync_mode = !file_sync_mode;
        }
        /* Confirm completion during asynchronous read testing */
        if (FS_IsFile(file))
        {
            /* Measure if completed */
            if (!FS_IsBusy(file))
            {
                EndFileTest();
            }
        }
        /* If possible, use A button for file read testing */
        else if (IS_PAD_TRIGGER(PAD_BUTTON_A))
        {
            if (!BeginFileTest())
            {
                /* When the wireless connection is disconnected, etc. */
                EndFileTest();
            }
            /*
             * Wait for completion here if this is a synchronous test.
             * Synchronous operations are not stopped as long as the WFS_End function is not called, so polling is carried out as follows to detect disconnection.
             * 
             * Except for the fact that it occupies CPU time by polling without transferring threads, this process is essentially equivalent to the FS_ReadFile function.
             * 
             */
            else if (file_sync_mode)
            {
                while (FS_IsBusy(file))
                {
                    if (WH_GetSystemState() != WH_SYSSTATE_DATASHARING)
                    {
                        WFS_End();
                    }
#if defined(SDK_ENABLE_ARM7_PRINT)
                    // PrintServer for ARM7 (for debugging purposes)
                    OS_PrintServer();
#endif
                }
                EndFileTest();
            }
        }
    }
    /* End communications with the START button */
    if (IS_PAD_TRIGGER(PAD_BUTTON_START))
    {
        (void)WH_Finalize();
    }
}


/*---------------------------------------------------------------------------*
  Name:         DrawFrame

  Description:  Frame render process.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DrawFrame(void)
{
    int     i;

    /* Display data sharing reception status */
    PrintString(8, 1, 0x2, "data-sharing mode");
    PrintString(4, 5, 0x4, "Receive:");
    for (i = 0; i < WH_PLAYER_MAX; ++i)
    {
        GShareData *input = GetCurrentInput(i);
        if (input)
        {
            PrintString(4, (s16)(6 + i), 0x4,
                        "Player%02d: %04x-%04x", i, input->key, input->count & 0xFFFF);
        }
        else
        {
            PrintString(4, (s16)(6 + i), 0x7, "No child");
        }
    }

    PrintString(0, 11, PLTT_BLUE, "--------------------------------");

    /* Display WFS test */
    if (!MB_IsMultiBootChild())
    {
        /* Parent */
        PrintString(3, 13, PLTT_GREEN, "<UP & DOWN key>");
        PrintString(3, 14, PLTT_WHITE, "parent packet ... %d BYTE", parent_packet_size);

        PrintString(3, 15, PLTT_GREEN, "<toggle A button>");
        PrintString(3, 16, PLTT_WHITE, "sync mode     ... %s",
                    is_parent_sync ? "ENABLE" : "DISABLE");

        PrintString(3, 17, PLTT_GREEN, "bitmap status");
        PrintString(9, 18, PLTT_BLUE, "+  idle");
        PrintString(9, 19, PLTT_BLUE, "*  busy");
        PrintString(9, 20, PLTT_BLUE, "!  sync-blocking");
        PrintString(9, 21, PLTT_BLUE, "-  not connected");

        PrintString(3, 19, PLTT_BLUE, "0123");
        {
            const int cur_bitmap = WFS_GetCurrentBitmap();
            const int busy_bitmap = WFS_GetBusyBitmap();
            const int sync_bitmap = WFS_GetSyncBitmap();
            for (i = 0; i < WH_PLAYER_MAX; ++i)
            {
                char    c;
                const int bit = (1 << i);
                if ((bit & busy_bitmap) != 0)
                    c = '*';
                else if ((bit & cur_bitmap) != 0)
                    c = '+';
                else
                    c = '-';
                PrintString(3 + i, 20, PLTT_WHITE, "%c", c);
                if ((bit & sync_bitmap) != 0)
                    PrintString(3 + i, 21, PLTT_WHITE, "!");
            }
        }
    }
    else
    {
        /* Child */
        if (WFS_GetStatus() == WFS_STATE_READY)
        {
            /* Display operations overview and current setting items */
            PrintString(3, 13, PLTT_GREEN, "<LEFT & RIGHT key>");
            if (!file_sync_mode)
            {
                PrintString(3, 14, PLTT_WHITE, " sync ");
                PrintString(10, 14, PLTT_YELLOW, "[async]");
            }
            else
            {
                PrintString(3, 14, PLTT_YELLOW, "[sync]");
                PrintString(10, 14, PLTT_WHITE, " async ");
            }
            PrintString(3, 15, PLTT_GREEN, "<UP & DOWN key>");
            PrintString(3, 16 + file_index, PLTT_YELLOW, ">");
            for (i = 0; i < file_max; ++i)
            {
                PrintString(5, 16 + i, (i == file_index) ? PLTT_YELLOW : PLTT_WHITE, file_path[i]);
            }
            PrintString(3, 16 + file_max, PLTT_GREEN, "<press A button>");
            /* Display when read test fails */
            if (read_time == -1)
            {
                PrintString(3, 20, PLTT_WHITE, "read error!");
            }
            /* Display when read test succeeds */
            else if (read_time != 0)
            {
                PrintString(3, 20, PLTT_WHITE, "read done %d BYTE", read_byte);
                PrintString(3, 21, PLTT_WHITE, "%d [bps]", read_time);
            }
            /* Display during read test */
            else
            {
                int     current, total;
                if (WFS_GetCurrentDownloadProgress(&current, &total))
                {
                    PrintString(3, 20, PLTT_WHITE, "reading   %d BYTE", read_byte);
                    PrintString(3, 21, PLTT_WHITE, "%8d / %d", current, total);
                }
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         ExecuteDataSharing

  Description:  Main process for data sharing.
                WFS process in background.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ExecuteDataSharing(void)
{
    /* Work variable initialization */
    file_index = 0;
    FS_InitFile(file);

    /* Stand by until data sharing preparations are complete and then launch WFS */
    WaitWHState(WH_SYSSTATE_DATASHARING);
    WFS_Start();

    /* Frame processing as long as communication continues */
    while (WH_GetSystemState() == WH_SYSSTATE_DATASHARING)
    {
        if (WaitNextFrame())
        {
            /* Update the process if data sharing is successful */
            UpdateFrame();
        }
        DrawFrame();
    }

    /* Because MP communications are finished, terminate WFS */
    WFS_End();
    WaitWHState(WH_SYSSTATE_IDLE);

}
