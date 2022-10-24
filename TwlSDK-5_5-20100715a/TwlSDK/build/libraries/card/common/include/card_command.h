/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - libraries
  File:     card_command.h

  Copyright 2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-12-17#$
  $Rev: 11258 $
  $Author: yada $

 *---------------------------------------------------------------------------*/
#ifndef NITRO_LIBRARIES_CARD_COMMAND_H__
#define NITRO_LIBRARIES_CARD_COMMAND_H__


#include <nitro/card/common.h>
#include <nitro/card/backup.h>

#include <nitro/pxi.h>
#include <nitro/os.h>


#ifdef __cplusplus
extern  "C"
{
#endif


/*---------------------------------------------------------------------------*/
/* Constants */

// Backup device functionality (used internally)
#define CARD_BACKUP_CAPS_AVAILABLE          (u32)(CARD_BACKUP_CAPS_READ - 1)
#define CARD_BACKUP_CAPS_READ               (u32)(1 << CARD_REQ_READ_BACKUP)
#define CARD_BACKUP_CAPS_WRITE              (u32)(1 << CARD_REQ_WRITE_BACKUP)
#define CARD_BACKUP_CAPS_PROGRAM            (u32)(1 << CARD_REQ_PROGRAM_BACKUP)
#define CARD_BACKUP_CAPS_VERIFY             (u32)(1 << CARD_REQ_VERIFY_BACKUP)
#define CARD_BACKUP_CAPS_ERASE_PAGE         (u32)(1 << CARD_REQ_ERASE_PAGE_BACKUP)
#define CARD_BACKUP_CAPS_ERASE_SECTOR       (u32)(1 << CARD_REQ_ERASE_SECTOR_BACKUP)
#define CARD_BACKUP_CAPS_ERASE_CHIP         (u32)(1 << CARD_REQ_ERASE_CHIP_BACKUP)
#define CARD_BACKUP_CAPS_READ_STATUS        (u32)(1 << CARD_REQ_READ_STATUS)
#define CARD_BACKUP_CAPS_WRITE_STATUS       (u32)(1 << CARD_REQ_WRITE_STATUS)
#define CARD_BACKUP_CAPS_ERASE_SUBSECTOR    (u32)(1 << CARD_REQ_ERASE_SUBSECTOR_BACKUP)


/*---------------------------------------------------------------------------*/
/* Declarations */

// Parameters for sending and receiving PXI command requests.
typedef struct CARDiCommandArg
{
    CARDResult result;                 /* Return value */
    CARDBackupType type;               /* Device type */
    u32     id;                        /* Card ID. */
    u32     src;                       /* Transfer source */
    u32     dst;                       /* Transfer destination */
    u32     len;                       /* Transfer length */

    struct
    {
        /* Memory property */
        u32     total_size;            /* Memory capacity (BYTE) */
        u32     sect_size;             /* Write-sector (BYTE) */
        u32     subsect_size;          /* Write-sector (BYTE) */
        u32     page_size;             /* Write-page (BYTE) */
        u32     addr_width;            /* Address width on command (BYTE) */
        /* Maximum wait-time (as LCD-V-COUNT tick) */
        u32     program_page;
        u32     write_page;
        u32     write_page_total;
        u32     erase_chip;
        u32     erase_chip_total;
        u32     erase_sector;
        u32     erase_sector_total;
        u32     erase_subsector;
        u32     erase_subsector_total;
        u32     erase_page;
        /* Initial parameter of status register */
        u8      initial_status;
        u8      padding1[3];
        u32     caps;
        u8      padding2[4];
    }
    spec;

}
CARDiCommandArg;

SDK_COMPILER_ASSERT(sizeof(CARDiCommandArg) % 32 == 0);


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         CARDi_SendPxi

  Description:  Sends a PXI FIFO word.

  Arguments:    data       Data to send

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARDi_SendPxi(u32 data)
{
    while (PXI_SendWordByFifo(PXI_FIFO_TAG_FS, data, TRUE) < 0)
    {
    }
}

#if	defined(SDK_ARM7)
/*---------------------------------------------------------------------------*
  Name:         CARDi_LockMutexForARM7

  Description:  Mutex lock used for exclusive CARD access by ARM7

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_LockMutexForARM7(void);

/*---------------------------------------------------------------------------*
  Name:         CARDi_UnlockMutexForARM7

  Description:  Mutex unlock for exclusive CARD access by ARM7

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_UnlockMutexForARM7(void);
#endif // if defined(SDK_ARM7)

/*---------------------------------------------------------------------------*
  Name:         CARDi_OnFifoRecv

  Description:  PXI FIFO word receive callback.

  Arguments:    tag: PXI tag (always PXI_FIFO_TAG_FS)
                data: Receive data
                err: Error bit (according to old specs)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARDi_OnFifoRecv(PXIFifoTag tag, u32 data, BOOL err);


#ifdef __cplusplus
} // extern "C"
#endif


#endif // NITRO_LIBRARIES_CARD_COMMAND_H__
