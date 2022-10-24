/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - libraries
  File:     card_backup.c

  Copyright 2007-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-26#$
  $Rev: 10827 $
  $Author: yosizaki $

 *---------------------------------------------------------------------------*/


#include <nitro.h>

#include "../include/card_common.h"
#include "../include/card_spi.h"


#ifndef SDK_ARM9
SDK_ERROR("this code is only for ARM9"!
#endif // SDK_ARM9


/*---------------------------------------------------------------------------*/
/* Constants */

#include <nitro/version_begin.h>
SDK_DEFINE_MIDDLEWARE(cardi_backup_assert, "NINTENDO", "BACKUP");
#include <nitro/version_end.h>
#define SDK_USING_BACKUP() SDK_USING_MIDDLEWARE(cardi_backup_assert)


/*---------------------------------------------------------------------------*/
/* Variables */

/* Cache of backup page last sent */
static u8 CARDi_backup_cache_page_buf[256] ATTRIBUTE_ALIGN(32);


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         CARDi_OnFifoRecv

  Description:  PXI FIFO word receive callback.

  Arguments:    tag: PXI tag (always PXI_FIFO_TAG_FS)
                data: Receive data
                err: Error bit (according to old specs)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_OnFifoRecv(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused(data)
    if ((tag == PXI_FIFO_TAG_FS) && err)
    {
        CARDiCommon *const p = &cardi_common;
        /* Receive reply from ARM7 and send a notification of completion */
        SDK_ASSERT(data == CARD_REQ_ACK);
        p->flag &= ~CARD_STAT_WAITFOR7ACK;
        OS_WakeupThreadDirect(p->current_thread_9);
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_Request

  Description:  Sends request from ARM9 to ARM7 and blocks completion.
                If the result is not CARD_RESULT_SUCCESS, retries the specified number of times.
                (Locking of the specified bus and exclusive control of the task thread are guaranteed by the caller of this function.)
                 
                Sub-process repeatedly called in another command

  Arguments:    req_type: Command request type
                retry_max: Max. number of times to retry

  Returns:      If the result is CARD_RESULT_SUCCESS, TRUE.
 *---------------------------------------------------------------------------*/
static BOOL CARDi_Request(CARDiCommon *p, int req_type, int retry_count)
{
    // Execute here if PXI not initialized
    if ((p->flag & CARD_STAT_INIT_CMD) == 0)
    {
        p->flag |= CARD_STAT_INIT_CMD;
        while (!PXI_IsCallbackReady(PXI_FIFO_TAG_FS, PXI_PROC_ARM7))
        {
            OS_SpinWait(100);
        }
        // Send the first INIT command (recursive)
        (void)CARDi_Request(p, CARD_REQ_INIT, 1);
    }
    // Flush the shared memory that has been set
    DC_FlushRange(p->cmd, sizeof(*p->cmd));
    DC_WaitWriteBufferEmpty();

    // Register the thread pointer to receive a command response
    p->current_thread_9 = OS_GetCurrentThread();

    do
    {
        // Send command request
        p->flag |= CARD_STAT_WAITFOR7ACK;
        CARDi_SendPxi((u32)req_type);
        // If there are more arguments, perform additional sends
        switch (req_type)
        {
        case CARD_REQ_INIT:
            CARDi_SendPxi((u32)p->cmd);
            break;
        }
        {
            // Wait for the completion of the request from ARM7
            OSIntrMode bak_psr = OS_DisableInterrupts();
            while ((p->flag & CARD_STAT_WAITFOR7ACK) != 0)
            {
                OS_SleepThread(NULL);
            }
            (void)OS_RestoreInterrupts(bak_psr);
        }
        DC_InvalidateRange(p->cmd, sizeof(*p->cmd));
    }
    while ((p->cmd->result == CARD_RESULT_TIMEOUT) && (--retry_count > 0));

    return (p->cmd->result == CARD_RESULT_SUCCESS);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_RequestStreamCommandCore

  Description:  Core of command request processing that transfers data.
                Called synchronously and asynchronously.

  Arguments:    p: Library's work buffer (passed by argument for efficiency)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_RequestStreamCommandCore(CARDiCommon * p)
{
    const int req_type = p->req_type;
    const int req_mode = p->req_mode;
    const int retry_count = p->req_retry;
    u32     size = sizeof(CARDi_backup_cache_page_buf);

    SDK_USING_BACKUP();

    /* Request at the page or sector level */
    if (req_type == CARD_REQ_ERASE_SECTOR_BACKUP)
    {
        size = CARD_GetBackupSectorSize();
    }
    else if (req_type == CARD_REQ_ERASE_SUBSECTOR_BACKUP)
    {
        size = cardi_common.cmd->spec.subsect_size;
    }
    do
    {
        const u32 len = (size < p->len) ? size : p->len;
        p->cmd->len = len;

        /* Stops here if there has been a cancel request */
        if ((p->flag & CARD_STAT_CANCEL) != 0)
        {
            p->flag &= ~CARD_STAT_CANCEL;
            p->cmd->result = CARD_RESULT_CANCELED;
            break;
        }
        switch (req_mode)
        {
        case CARD_REQUEST_MODE_RECV:
            /* Invalidate the buffer if command is receive-related */
            DC_InvalidateRange(CARDi_backup_cache_page_buf, len);
            p->cmd->src = (u32)p->src;
            p->cmd->dst = (u32)CARDi_backup_cache_page_buf;
            break;
        case CARD_REQUEST_MODE_SEND:
        case CARD_REQUEST_MODE_SEND_VERIFY:
            /* If command is send-related, copy the data to a temporary buffer */
            MI_CpuCopy8((const void *)p->src, CARDi_backup_cache_page_buf, len);
            DC_FlushRange(CARDi_backup_cache_page_buf, len);
            DC_WaitWriteBufferEmpty();
            p->cmd->src = (u32)CARDi_backup_cache_page_buf;
            p->cmd->dst = (u32)p->dst;
            break;
        case CARD_REQUEST_MODE_SPECIAL:
            /* Buffer operations are unnecessary */
            p->cmd->src = (u32)p->src;
            p->cmd->dst = (u32)p->dst;
            break;
        }
        /* Send a request */
        if (!CARDi_Request(p, req_type, retry_count))
        {
            break;
        }
        /* If specified, make another verify request with the same settings. */
        if (req_mode == CARD_REQUEST_MODE_SEND_VERIFY)
        {
            if (!CARDi_Request(p, CARD_REQ_VERIFY_BACKUP, 1))
            {
                break;
            }
        }
        else if (req_mode == CARD_REQUEST_MODE_RECV)
        {
            /* Copy from cache */
            MI_CpuCopy8(CARDi_backup_cache_page_buf, (void *)p->dst, len);
        }
        p->src += len;
        p->dst += len;
        p->len -= len;
    }
    while (p->len > 0);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_RequestWriteSectorCommandCore

  Description:  Erases sectors and is the core process for program requests.
                Called synchronously and asynchronously.

  Arguments:    p: Library's work buffer (passed by argument for efficiency)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_RequestWriteSectorCommandCore(CARDiCommon * p)
{
    const u32 sector_size = CARD_GetBackupSectorSize();
    SDK_USING_BACKUP();

    /* Return failure if the processing range is not an integer multiple of sector units */
    if ((((u32)p->dst | p->len) & (sector_size - 1)) != 0)
    {
        p->flag &= ~CARD_STAT_CANCEL;
        p->cmd->result = CARD_RESULT_INVALID_PARAM;
    }
    else
    {
        /* Sector unit processing */
        for (; p->len > 0; p->len -= sector_size)
        {
            u32     len = sector_size;
            /* Stops here if there has been a cancel request */
            if ((p->flag & CARD_STAT_CANCEL) != 0)
            {
                p->flag &= ~CARD_STAT_CANCEL;
                p->cmd->result = CARD_RESULT_CANCELED;
                break;
            }
            /* Sector deletion */
            p->cmd->dst = (u32)p->dst;
            p->cmd->len = len;
            if (!CARDi_Request(p, CARD_REQ_ERASE_SECTOR_BACKUP, 1))
            {
                break;
            }
            while (len > 0)
            {
                const u32 page = sizeof(CARDi_backup_cache_page_buf);
                /* Stops here if there has been a cancel request */
                if ((p->flag & CARD_STAT_CANCEL) != 0)
                {
                    p->flag &= ~CARD_STAT_CANCEL;
                    p->cmd->result = CARD_RESULT_CANCELED;
                    break;
                }
                /* Program */
                MI_CpuCopy8((const void *)p->src, CARDi_backup_cache_page_buf, page);
                DC_FlushRange(CARDi_backup_cache_page_buf, page);
                DC_WaitWriteBufferEmpty();
                p->cmd->src = (u32)CARDi_backup_cache_page_buf;
                p->cmd->dst = (u32)p->dst;
                p->cmd->len = page;
                if (!CARDi_Request(p, CARD_REQ_PROGRAM_BACKUP, CARD_RETRY_COUNT_MAX))
                {
                    break;
                }
                /* Verify if needed */
                if (p->req_mode == CARD_REQUEST_MODE_SEND_VERIFY)
                {
                    if (!CARDi_Request(p, CARD_REQ_VERIFY_BACKUP, 1))
                    {
                        break;
                    }
                }
                p->src += page;
                p->dst += page;
                len -= page;
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_AccessStatusCore

  Description:  Core access processing for status register.
                Called synchronously and asynchronously.

  Arguments:    p: Library's work buffer (passed by argument for efficiency)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_AccessStatusCore(CARDiCommon *p)
{
    CARDRequest command = (CARDRequest)CARDi_backup_cache_page_buf[1];
    DC_FlushRange(CARDi_backup_cache_page_buf, 1);
    p->cmd->src = (u32)CARDi_backup_cache_page_buf;
    p->cmd->dst = (u32)CARDi_backup_cache_page_buf;
    (void)CARDi_Request(p, command, 1);
    DC_InvalidateRange(CARDi_backup_cache_page_buf, 1);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_IdentifyBackupCore2

  Description:  Identifies the device type.

  Arguments:    type: Device type to be identified

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_IdentifyBackupCore2(CARDBackupType type)
{
    /*
     * Saves the obtained parameter in CARDiCommandArg.
     * Ultimately this is completed by discarding the table.
     */
    {
        CARDiCommandArg *const p = cardi_common.cmd;

        /* First, clear all parameters and set to NOT_USE state */
        MI_CpuFill8(&p->spec, 0, sizeof(p->spec));
        p->type = type;
        p->spec.caps = (CARD_BACKUP_CAPS_AVAILABLE | CARD_BACKUP_CAPS_READ_STATUS);
        if (type != CARD_BACKUP_TYPE_NOT_USE)
        {
            /*
             * Device type, total capacity and vendor can be obtained from 'type'.
             * The vendor number is 0 unless the same type was adopted by several manufacturers and these types needed to be distinguished for some reason (for example, due to problems or bugs for some of them).
             * 
             */
            const u32 size = (u32)(1 << ((type >> CARD_BACKUP_TYPE_SIZEBIT_SHIFT) &
                                         CARD_BACKUP_TYPE_SIZEBIT_MASK));
            const int device =
                ((type >> CARD_BACKUP_TYPE_DEVICE_SHIFT) & CARD_BACKUP_TYPE_DEVICE_MASK);
            const int vender =
                ((type >> CARD_BACKUP_TYPE_VENDER_SHIFT) & CARD_BACKUP_TYPE_VENDER_MASK);

            p->spec.total_size = size;
            /* Use 0xFF if the status register does not need to be corrected. (This is usually the case.) */
            p->spec.initial_status = 0xFF;
            if (device == CARD_BACKUP_TYPE_DEVICE_EEPROM)
            {
                switch (size)
                {
                default:
                    goto invalid_type;
                case 0x000200:        // CARD_BACKUP_TYPE_EEPROM_4KBITS
                    p->spec.page_size = 0x10;
                    p->spec.addr_width = 1;
                    p->spec.program_page = 5;
                    p->spec.initial_status = 0xF0;
                    break;
                case 0x002000:        // CARD_BACKUP_TYPE_EEPROM_64KBITS
                    p->spec.page_size = 0x0020;
                    p->spec.addr_width = 2;
                    p->spec.program_page = 5;
                    p->spec.initial_status = 0x00;
                    break;
                case 0x010000:        // CARD_BACKUP_TYPE_EEPROM_512KBITS
                    p->spec.page_size = 0x0080;
                    p->spec.addr_width = 2;
                    p->spec.program_page = 10;
                    p->spec.initial_status = 0x00;
                    break;
				case 0x020000:	      // CARD_BACKUP_TYPE_EEPROM_1MBITS
					p->spec.page_size = 0x0100;
					p->spec.addr_width = 3;
					p->spec.program_page = 5;
                    p->spec.initial_status = 0x00;
					break;
                }
                p->spec.sect_size = p->spec.page_size;
                p->spec.caps |= CARD_BACKUP_CAPS_READ;
                p->spec.caps |= CARD_BACKUP_CAPS_PROGRAM;
                p->spec.caps |= CARD_BACKUP_CAPS_VERIFY;
                p->spec.caps |= CARD_BACKUP_CAPS_WRITE_STATUS;
            }
            else if (device == CARD_BACKUP_TYPE_DEVICE_FLASH)
            {
                switch (size)
                {
                default:
                    goto invalid_type;
                case 0x040000:        // CARD_BACKUP_TYPE_FLASH_2MBITS
                case 0x080000:        // CARD_BACKUP_TYPE_FLASH_4MBITS
                case 0x100000:        // CARD_BACKUP_TYPE_FLASH_8MBITS
                    p->spec.write_page = 25;
                    p->spec.write_page_total = 300;
                    p->spec.erase_page = 300;
                    p->spec.erase_sector = 5000;
                    p->spec.caps |= CARD_BACKUP_CAPS_WRITE;
                    p->spec.caps |= CARD_BACKUP_CAPS_ERASE_PAGE;
                    break;
                case 0x200000:        // CARD_BACKUP_TYPE_FLASH_16MBITS
                    p->spec.write_page = 23;
                    p->spec.write_page_total = 300;
                    p->spec.erase_sector = 500;
                    p->spec.erase_sector_total = 5000;
                    p->spec.erase_chip = 10000;
                    p->spec.erase_chip_total = 60000;
                    p->spec.initial_status = 0x00;
                    p->spec.caps |= CARD_BACKUP_CAPS_WRITE;
                    p->spec.caps |= CARD_BACKUP_CAPS_ERASE_PAGE;
                    p->spec.caps |= CARD_BACKUP_CAPS_ERASE_CHIP;
                    p->spec.caps |= CARD_BACKUP_CAPS_WRITE_STATUS;
                    break;
                case 0x400000:        // CARD_BACKUP_TYPE_FLASH_32MBITS
                    p->spec.erase_sector = 600;
                    p->spec.erase_sector_total = 3000;
                    p->spec.erase_subsector = 70;
                    p->spec.erase_subsector_total = 150;
                    p->spec.erase_chip = 23000;
                    p->spec.erase_chip_total = 800000;
                    p->spec.initial_status = 0x00;
                    p->spec.subsect_size = 0x1000;
                    p->spec.caps |= CARD_BACKUP_CAPS_ERASE_SUBSECTOR;
                    p->spec.caps |= CARD_BACKUP_CAPS_ERASE_CHIP;
                    p->spec.caps |= CARD_BACKUP_CAPS_WRITE_STATUS;
                    break;
                case 0x800000:
                    if (vender == 0)  // CARD_BACKUP_TYPE_FLASH_64MBITS
                    {
                        p->spec.erase_sector = 1000;
                        p->spec.erase_sector_total = 3000;
                        p->spec.erase_chip = 68000;
                        p->spec.erase_chip_total = 160000;
                        p->spec.initial_status = 0x00;
                        p->spec.caps |= CARD_BACKUP_CAPS_ERASE_CHIP;
                        p->spec.caps |= CARD_BACKUP_CAPS_WRITE_STATUS;
                    }
                    else if (vender == 1)   // CARD_BACKUP_TYPE_FLASH_64MBITS_EX
                    {
                        p->spec.erase_sector = 1000;
                        p->spec.erase_sector_total = 3000;
                        p->spec.erase_chip = 68000;
                        p->spec.erase_chip_total = 160000;
                        p->spec.initial_status = 0x84;
                        p->spec.caps |= CARD_BACKUP_CAPS_ERASE_CHIP;
                        p->spec.caps |= CARD_BACKUP_CAPS_WRITE_STATUS;
                    }
                    break;
                }
                p->spec.sect_size = 0x010000;
                p->spec.page_size = 0x0100;
                p->spec.addr_width = 3;
                p->spec.program_page = 5;
                p->spec.caps |= CARD_BACKUP_CAPS_READ;
                p->spec.caps |= CARD_BACKUP_CAPS_PROGRAM;
                p->spec.caps |= CARD_BACKUP_CAPS_VERIFY;
                p->spec.caps |= CARD_BACKUP_CAPS_ERASE_SECTOR;
            }
            else if (device == CARD_BACKUP_TYPE_DEVICE_FRAM)
            {
                switch (size)
                {
                default:
                    goto invalid_type;
                case 0x002000:        // #CARD_BACKUP_TYPE_FRAM_64KBITS
                case 0x008000:        // #CARD_BACKUP_TYPE_FRAM_256KBITS
                    break;
                }
                p->spec.page_size = size;
                p->spec.sect_size = size;
                p->spec.addr_width = 2;
                p->spec.initial_status = 0x00;
                p->spec.caps |= CARD_BACKUP_CAPS_READ;
                p->spec.caps |= CARD_BACKUP_CAPS_PROGRAM;
                p->spec.caps |= CARD_BACKUP_CAPS_VERIFY;
                p->spec.caps |= CARD_BACKUP_CAPS_WRITE_STATUS;
            }
            else
            {
              invalid_type:
                p->type = CARD_BACKUP_TYPE_NOT_USE;
                p->spec.total_size = 0;
                cardi_common.cmd->result = CARD_RESULT_UNSUPPORTED;
                return;
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_IdentifyBackupCore

  Description:  Core process for identifying the device type.
                Called synchronously and asynchronously.

  Arguments:    p: Library's work buffer (passed by argument for efficiency)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_IdentifyBackupCore(CARDiCommon * p)
{
    (void)CARDi_Request(p, CARD_REQ_IDENTIFY, 1);
    /*
     * Issue a read command for the first byte and get the result value.
     * If there is a contact problem, damage, or lifespan problem, return TIMEOUT, regardless of the value.
     * (TIMEOUT can be determined using a Read-Status command regardless of the device type)
     */
    p->cmd->src = 0;
    p->cmd->dst = (u32)CARDi_backup_cache_page_buf;
    p->cmd->len = 1;
    (void)CARDi_Request(p, CARD_REQ_READ_BACKUP, 1);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_BeginBackupCommand

  Description:  Starting process for backup operation command.

  Arguments:    level: Access mode requested for backups
                callback: Completion callback (NULL if not used)
                arg: Argument of completion callback (ignored if not used)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_BeginBackupCommand(CARDAccessLevel level, MIDmaCallback callback, void *arg)
{
    SDK_USING_BACKUP();
    SDK_ASSERT(CARD_IsAvailable());
    SDK_TASSERTMSG(CARDi_GetTargetMode() == CARD_TARGET_BACKUP,
                  "[CARD] current locking target is not backup.");
    CARD_CheckEnabled();
    if ((CARDi_GetAccessLevel() & level) != level)
    {
        OS_TPanic("this program cannot %s%s CARD-backup!",
                (level & CARD_ACCESS_LEVEL_BACKUP_R) ? "READ" : "",
                (level & CARD_ACCESS_LEVEL_BACKUP_W) ? "WRITE" : "");
    }
    (void)CARDi_WaitForTask(&cardi_common, TRUE, callback, arg);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_RequestStreamCommand

  Description:  Issues request for command to transfer data.

  Arguments:    src: Transfer source offset or memory address
                dst: Transfer destination offset or memory address
                len: Transfer size
                callback: Completion callback (NULL if not used)
                arg: Argument of completion callback (ignored if not used)
                is_async: If async operation was specified, TRUE
                req_type: Command request type
                req_retry: Maximum number of retries when command request fails
                req_mode: Command request operation mode

  Returns:      TRUE if the process was successful.
 *---------------------------------------------------------------------------*/
BOOL CARDi_RequestStreamCommand(u32 src, u32 dst, u32 len,
                                MIDmaCallback callback, void *arg, BOOL is_async,
                                CARDRequest req_type, int req_retry, CARDRequestMode req_mode)
{
    CARDAccessLevel level = (req_mode == CARD_REQUEST_MODE_RECV) ? CARD_ACCESS_LEVEL_BACKUP_R : CARD_ACCESS_LEVEL_BACKUP_W;
    SDK_ASSERT(CARD_GetCurrentBackupType() != CARD_BACKUP_TYPE_NOT_USE);

    CARDi_BeginBackupCommand(level, callback, arg);

    {
        CARDiCommon *p = &cardi_common;
        p->src = src;
        p->dst = dst;
        p->len = len;
        p->req_type = req_type;
        p->req_retry = req_retry;
        p->req_mode = req_mode;
    }

    return CARDi_ExecuteOldTypeTask(CARDi_RequestStreamCommandCore, is_async);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_AccessStatus

  Description:  Status read or write (for testing).

  Arguments:    command: CARD_REQ_READ_STATUS or CARD_REQ_WRITE_STATUS
                value: Value to write, if CARD_REQ_WRITE_STATUS

  Returns:      Returns a value of 0 or higher if successful; a negative number otherwise.
 *---------------------------------------------------------------------------*/
int CARDi_AccessStatus(CARDRequest command, u8 value)
{
    SDK_ASSERT(CARD_GetCurrentBackupType() != CARD_BACKUP_TYPE_NOT_USE);

    CARDi_BeginBackupCommand(CARD_ACCESS_LEVEL_NONE, NULL, NULL);

    // Use temporary buffer instead of the task argument
    CARDi_backup_cache_page_buf[0] = value;
    CARDi_backup_cache_page_buf[1] = (u8)command;

    return CARDi_ExecuteOldTypeTask(CARDi_AccessStatusCore, FALSE) ?
           CARDi_backup_cache_page_buf[0] : -1;
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_RequestWriteSectorCommand

  Description:  Erases sectors and issues program requests.

  Arguments:    src: Transfer source memory address
                dst: Transfer destination offset
                len: Transfer size
                verify: TRUE when performing a verify
                callback: Completion callback (NULL if not used)
                arg: Argument of completion callback (ignored if not used)
                is_async: If async operation was specified, TRUE

  Returns:      TRUE if the process was successful.
 *---------------------------------------------------------------------------*/
BOOL CARDi_RequestWriteSectorCommand(u32 src, u32 dst, u32 len, BOOL verify,
                                     MIDmaCallback callback, void *arg, BOOL is_async)
{
    SDK_ASSERT(CARD_GetCurrentBackupType() != CARD_BACKUP_TYPE_NOT_USE);

    CARDi_BeginBackupCommand(CARD_ACCESS_LEVEL_BACKUP_W, callback, arg);

    {
        CARDiCommon *p = &cardi_common;
        p->src = src;
        p->dst = dst;
        p->len = len;
        p->req_mode = verify ? CARD_REQUEST_MODE_SEND_VERIFY : CARD_REQUEST_MODE_SEND;
    }

    return CARDi_ExecuteOldTypeTask(CARDi_RequestWriteSectorCommandCore, is_async);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_IdentifyBackup

  Description:  Specifies the card backup device type.

  Arguments:    type: Device classification of the CARDBackupType type

  Returns:      TRUE if the read test was successful.
 *---------------------------------------------------------------------------*/
BOOL CARD_IdentifyBackup(CARDBackupType type)
{
    if (type == CARD_BACKUP_TYPE_NOT_USE)
    {
        OS_TPanic("cannot specify CARD_BACKUP_TYPE_NOT_USE.");
    }

    CARDi_BeginBackupCommand(CARD_ACCESS_LEVEL_NONE, NULL, NULL);

    CARDi_IdentifyBackupCore2(type);

    return CARDi_ExecuteOldTypeTask(CARDi_IdentifyBackupCore, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_GetCurrentBackupType

  Description:  Gets backup device type currently specified.

  Arguments:    None.

  Returns:      Backup device type currently specified.
 *---------------------------------------------------------------------------*/
CARDBackupType CARD_GetCurrentBackupType(void)
{
    SDK_ASSERT(CARD_IsAvailable());

    return cardi_common.cmd->type;
}

/*---------------------------------------------------------------------------*
  Name:         CARD_GetBackupTotalSize

  Description:  Gets the overall size of the backup device currently specified.

  Arguments:    None.

  Returns:      Overall size of the backup device.
 *---------------------------------------------------------------------------*/
u32 CARD_GetBackupTotalSize(void)
{
    SDK_ASSERT(CARD_IsAvailable());

    return cardi_common.cmd->spec.total_size;
}

/*---------------------------------------------------------------------------*
  Name:         CARD_GetBackupSectorSize

  Description:  Gets sector size of the backup device currently specified.

  Arguments:    None.

  Returns:      Sector size of the backup device.
 *---------------------------------------------------------------------------*/
u32 CARD_GetBackupSectorSize(void)
{
    SDK_ASSERT(CARD_IsAvailable());

    return cardi_common.cmd->spec.sect_size;
}

/*---------------------------------------------------------------------------*
  Name:         CARD_GetBackupPageSize

  Description:  Gets page size of the backup device currently specified.

  Arguments:    None.

  Returns:      Page size of the backup device.
 *---------------------------------------------------------------------------*/
u32 CARD_GetBackupPageSize(void)
{
    SDK_ASSERT(CARD_IsAvailable());

    return cardi_common.cmd->spec.page_size;
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WaitBackupAsync

  Description:  Waits until the backup device access function is completed.

  Arguments:    None.

  Returns:      TRUE if the backup device access function that was called last is completed with CARD_RESULT_SUCCESS. FALSE, otherwise.
                
 *---------------------------------------------------------------------------*/
BOOL CARD_WaitBackupAsync(void)
{
    return CARDi_WaitAsync();
}

/*---------------------------------------------------------------------------*
  Name:         CARD_TryWaitBackupAsync

  Description:  Determines whether the backup device access function is completed.

  Arguments:    None.

  Returns:      TRUE if the backup device access function is completed.
 *---------------------------------------------------------------------------*/
BOOL CARD_TryWaitBackupAsync(void)
{
    return CARDi_TryWaitAsync();
}

/*---------------------------------------------------------------------------*
  Name:         CARD_CancelBackupAsync

  Description:  Requests termination to a backup device access function that is processing.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_CancelBackupAsync(void)
{
    OSIntrMode bak_cpsr = OS_DisableInterrupts();
    cardi_common.flag |= CARD_STAT_CANCEL;
    (void)OS_RestoreInterrupts(bak_cpsr);
}
