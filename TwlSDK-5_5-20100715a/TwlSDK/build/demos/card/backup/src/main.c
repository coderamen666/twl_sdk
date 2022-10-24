/*---------------------------------------------------------------------------*
  Project:  TwlSDK - demos - CARD - backup
  File:     main.c

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-11-27#$
  $Rev: 9430 $
  $Author: yosizaki $
 *---------------------------------------------------------------------------*/


#include <nitro.h>

#include "DEMO.h"


/*---------------------------------------------------------------------------*/
/* Variables */

// ID for locking the CARD backup.
// This is used by CARD_LockBackup() and CARD_UnlockBackup() because use of the CARD hardware resources is mutually exclusive with the FS library and other functions.
// 
static u16  card_lock_id;

// The resulting value when a CARD access function has an error.
// The return value from CARD_GetResultCode() will be changed by functions such as CARD_UnlockBackup(): take note of this when access processing and error handling are separated.
// 
static CARDResult last_result = CARD_RESULT_SUCCESS;

// Flag that indicates whether a write-test has been run.
static BOOL is_test_run;


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         GetCardResultString

  Description:  Gets a string for a CARD function's return value.

  Arguments:    result: CARDResult result value

  Returns:      Pointer to a string that describes the result value.
 *---------------------------------------------------------------------------*/
static const char *GetCardResultString(CARDResult result)
{
    switch (result)
    {
    case CARD_RESULT_SUCCESS:
        return "success";
    case CARD_RESULT_FAILURE:
        return "failure";
    case CARD_RESULT_INVALID_PARAM:
        return "invalid param";
    case CARD_RESULT_UNSUPPORTED:
        return "unsupported";
    case CARD_RESULT_TIMEOUT:
        return "timeout";
    case CARD_RESULT_CANCELED:
        return "canceled";
    case CARD_RESULT_NO_RESPONSE:
        return "no response";
    case CARD_RESULT_ERROR:
        return "error";
    default:
        return "unknown error";
    }
}

/*---------------------------------------------------------------------------*
  Name:         SelectDevice

  Description:  Device selection screen.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SelectDevice(void)
{
    /* *INDENT-OFF* */
    static const struct
    {
        CARDBackupType type;
        const char *comment;
    }
    types_table[] =
    {
        { CARD_BACKUP_TYPE_EEPROM_4KBITS,   "EEPROM    4 kb"},
        { CARD_BACKUP_TYPE_EEPROM_64KBITS,  "EEPROM   64 kb"},
        { CARD_BACKUP_TYPE_EEPROM_512KBITS, "EEPROM  512 kb"},
        { CARD_BACKUP_TYPE_EEPROM_1MBITS,   "EEPROM    1 Mb"},
        { CARD_BACKUP_TYPE_FLASH_2MBITS,    "FLASH     2 Mb"},
        { CARD_BACKUP_TYPE_FLASH_4MBITS,    "FLASH     4 Mb"},
        { CARD_BACKUP_TYPE_FLASH_8MBITS,    "FLASH     8 Mb"},
        { CARD_BACKUP_TYPE_FLASH_16MBITS,   "FLASH    16 Mb"},
        { CARD_BACKUP_TYPE_FLASH_64MBITS,   "FLASH    64 Mb"},
    };
    /* *INDENT-ON* */
    enum
    { types_table_max = sizeof(types_table) / sizeof(*types_table) };

    int     cur = 0;
    BOOL    error = FALSE;
    BOOL    end = FALSE;
    int     i;

    while (!end)
    {
        DEMOReadKey();
        // Move the cursor with the up and down buttons
        if (DEMO_IS_TRIG(PAD_KEY_DOWN))
        {
            error = FALSE;
            if (++cur >= types_table_max)
            {
                cur -= types_table_max;
            }
        }
        if (DEMO_IS_TRIG(PAD_KEY_UP))
        {
            error = FALSE;
            if (--cur < 0)
            {
                cur += types_table_max;
            }
        }

        // Set the currently selected device with the A Button.
        // Set this correctly because the library cannot determine whether the specified device has actually been loaded.
        // 
        if (DEMO_IS_TRIG(PAD_BUTTON_A))
        {
            CARD_LockBackup(card_lock_id);
            end = CARD_IdentifyBackup(types_table[cur].type);
            if (!end)
            {
                error = TRUE;
                last_result = CARD_GetResultCode();
            }
            CARD_UnlockBackup(card_lock_id);
        }

        // Displays screen
        DEMOFillRect(0, 0, GX_LCD_SIZE_X, GX_LCD_SIZE_Y, DEMO_RGB_CLEAR);
        DEMOSetBitmapTextColor(GX_RGBA(0, 31, 0, 1));
        DEMODrawText(10, 40, "select device!");
        for (i = 0; i < types_table_max; ++i)
        {
            DEMODrawText(10, 60 + 10 * i, "%c%s",
                         (cur == i) ? '>' : ' ', types_table[i].comment);
        }
        if (error)
        {
            DEMOSetBitmapTextColor(GX_RGBA(31, 0, 0, 1));
            DEMODrawText(10, 160, "error!");
            DEMODrawText(10, 170, "result:\"%s\"", GetCardResultString(last_result));
        }

        DEMO_DrawFlip();
        OS_WaitVBlankIntr();
    }
}

/*---------------------------------------------------------------------------*
  Name:         TestWriteAndVerify

  Description:  Write test screen.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void TestWriteAndVerify(void)
{
    const u32   page_size = CARD_GetBackupPageSize();
    const u32   sector_size = CARD_GetBackupSectorSize();
    const u32   total_size = CARD_GetBackupTotalSize();

    OSTick      erase_time = 0;
    u32         pos = 0;

    BOOL        end = FALSE;

    // Initialize the screen
    {
        DEMOFillRect(0, 0, GX_LCD_SIZE_X, GX_LCD_SIZE_Y, DEMO_RGB_CLEAR);
        DEMOSetBitmapTextColor(GX_RGBA(31, 31, 31, 1));
        if (CARD_IsBackupEeprom())
        {
            DEMODrawText(10, 10, "EEPROM");
        }
        else if (CARD_IsBackupFlash())
        {
            DEMODrawText(10, 10, "FLASH");
        }
        DEMODrawText(10, 20, "page:%d  sector:%d", page_size, sector_size);
        DEMODrawText(10, 30, "total:%d", total_size);
        DEMOFillRect(10, 50, GX_LCD_SIZE_X - 10 * 2, 100, GX_RGBA(0, 0, 0, 1));
        DEMODrawFrame(8, 45, GX_LCD_SIZE_X - 8 * 2, 182 - 8 - 45, GX_RGBA(0, 0, 31, 1));
        DEMO_DrawFlip();
        OS_WaitVBlankIntr();
    }

    while (!end)
    {
        DEMOReadKey();
        // Start with the A Button if nothing has started yet
        if (!is_test_run)
        {
            if (DEMO_IS_TRIG(PAD_BUTTON_A))
            {
                is_test_run = TRUE;
            }
        }
        //Stop test with B Button
        else if (DEMO_IS_TRIG(PAD_BUTTON_B))
        {
            end = TRUE;
        }

        // Perform a test write and onscreen display
        if (!is_test_run)
        {
            DEMOSetBitmapTextColor(GX_RGBA(31, 31, 31, 1));
            DEMODrawText(10, 50, "press A-BUTTON to test");
        }
        else
        {
            static u8 tmp_buf[65536];
            SDK_ASSERT(sizeof(tmp_buf) >= sector_size);

            DEMOFillRect(10, 50, 256 - 10 * 2, 100, GX_RGBA(0, 0, 0, 1));
            DEMOSetBitmapTextColor(GX_RGBA(0, 31, 0, 1));
            DEMODrawText(10, 50, "now testing...");
            DEMODrawText(10, 60, "address:%d-%d", pos, pos + page_size);

            // Lock CARD resources for subsequent backup accesses.
            // This blocks card access from the FS library and other modules until CARD_UnlockBackup() is called.
            // Be careful about process deadlocks.
            // 
            CARD_LockBackup(card_lock_id);
            {
                OSTick  tick;
                int     i;
                for (i = 0; i < page_size; ++i)
                {
                    tmp_buf[i] = (u8)(pos * 3 + i);
                }
                tick = OS_GetTick();
                // Write asynchronously + verify content check
                if (CARD_IsBackupEeprom())
                {
                    CARD_WriteAndVerifyEepromAsync(pos, tmp_buf, page_size, NULL, NULL);
                }
                else if (CARD_IsBackupFlash())
                {
                    // Write operations can be used on nearly all flash devices
                    if (CARD_GetCurrentBackupType() != CARD_BACKUP_TYPE_FLASH_64MBITS)
                    {
                        CARD_WriteAndVerifyFlashAsync(pos, tmp_buf, page_size, NULL, NULL);
                    }
                    // With some large-capacity FLASH devices, only EraseSector operations and Program operations can be used.
                    // Note that you must be cautious of the fact that erase operations can be carried out only in integer units of the sector size.
                    else
                    {
                        BOOL    programmable = FALSE;
                        if ((pos % sector_size) == 0)
                        {
                            // Each and every sector of the area that will now be written must be erased beforehand.
                            // This sample demo replaces the contents of an entire sector. If you want to replace only part of a sector, you must first back up the sector's data, erase the sector, and then rewrite the same data.
                            // 
                            // 
                            erase_time = tick;
                            programmable = CARD_EraseFlashSector(pos, sector_size);
                            tick = OS_GetTick();
                            erase_time = tick - erase_time;
                            last_result = CARD_GetResultCode();
                        }
                        else
                        {
                            // Areas which have already been erased may be written as-is.
                            programmable = TRUE;
                        }
                        if (programmable)
                        {
                            CARD_ProgramAndVerifyFlashAsync(pos, tmp_buf, page_size, NULL, NULL);
                        }
                    }
                }

                // This sample waits here for processing to complete and then checks the result.
                // Because some device types and specified sizes may cause this to block for a very long time, figure out how to wait in a way that is suited to the various user application frameworks.
                // (For example, use CARD_TryWaitBackupAsync() to check only once per frame.)
                // 
                (void)CARD_WaitBackupAsync();
                last_result = CARD_GetResultCode();
                if (last_result != CARD_RESULT_SUCCESS)
                {
                    // If there is an error, end for now
                    end = TRUE;
                }
                else
                {
                    // Display the time if successful
                    tick = OS_GetTick() - tick;
                    DEMODrawText(10, 70, "write:%6d[ms]/%d[BYTE]", (int)OS_TicksToMilliSeconds(tick), page_size);
                    if (erase_time != 0)
                    {
                        DEMODrawText(10, 80, "erase:%6d[ms]/%d[BYTE]", (int)OS_TicksToMilliSeconds(erase_time), sector_size);
                    }
                    // Move the test address to the next address
                    pos += page_size;
                    if (pos + page_size > total_size)
                    {
                        end = TRUE;
                    }
                }
            }
            CARD_UnlockBackup(card_lock_id);
        }

        DEMO_DrawFlip();
        OS_WaitVBlankIntr();
    }
}

/*---------------------------------------------------------------------------*
  Name:         ShowResult

  Description:  Test results display screen.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ShowResult(void)
{
    BOOL    end = FALSE;

    while (!end)
    {
        DEMOReadKey();
        // Go back with A / B / START Buttons
        if (DEMO_IS_TRIG(PAD_BUTTON_A | PAD_BUTTON_B | PAD_BUTTON_START))
        {
            end = TRUE;
        }

        // Displays screen
        if (last_result == CARD_RESULT_SUCCESS)
        {
            DEMOSetBitmapTextColor(GX_RGBA(31, 31, 31, 1));
            DEMODrawText(10, 100, "done! (success)");
        }
        else
        {
            DEMOSetBitmapTextColor(GX_RGBA(0, 31, 0, 1));
            DEMODrawText(10, 100, "error!");
            DEMODrawText(10, 110, "result:\"%s\"", GetCardResultString(last_result));
        }
        DEMO_DrawFlip();
        OS_WaitVBlankIntr();
    }
}

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Main entry point.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    // Initialize the SDK
    OS_Init();
    OS_InitTick();
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    // Initialize the display for the demo
    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplayBitmap();
    DEMOHookConsole();
    DEMOSetBitmapTextColor(GX_RGBA(31, 31, 0, 1));
    DEMOSetBitmapGroundColor(DEMO_RGB_CLEAR);
    DEMOStartDisplay();

    // When accessing a ROM or backup from a non-card application, you must determine that the inserted DS Card is a title from the same company and then explicitly call the CARD_Enable function to enable it.
    // See the Programming Guidelines for details.
    // 
    // 
    if (OS_GetBootType() != OS_BOOTTYPE_ROM)
    {
        const CARDRomHeader *own_header = (const CARDRomHeader *)HW_ROM_HEADER_BUF;
        const CARDRomHeader *rom_header = (const CARDRomHeader *)CARD_GetRomHeader();
        if (own_header->maker_code != rom_header->maker_code)
        {
            // The purpose of this sample is accessing backup devices, so it will stop here if a DS Card has not been inserted
            // 
            static const char *message = "cannot detect own-maker title DS-CARD!";
            DEMOFillRect(0, 0, GX_LCD_SIZE_X, GX_LCD_SIZE_Y, DEMO_RGB_CLEAR);
            DEMOSetBitmapTextColor(GX_RGBA(31, 0, 0, 1));
            DEMODrawText(10, 40, message);
            DEMO_DrawFlip();
            OS_WaitVBlankIntr();
            OS_TPanic(message);
        }
        else
        {
            CARD_Enable(TRUE);
        }
    }

    {
        // Secures an ID for locking the CARD library's data bus
        s32     ret = OS_GetLockID();
        if (ret == OS_LOCK_ID_ERROR)
        {
            OS_TPanic("demo fatal error! OS_GetLockID() failed");
        }
        card_lock_id = (u16)ret;
    }

    // Screen transition
    for (;;)
    {
        // Device selection screen
        SelectDevice();
        // Test start
        is_test_run = FALSE;
        TestWriteAndVerify();
        // Display results
        if (is_test_run)
        {
            ShowResult();
        }
    }

}
