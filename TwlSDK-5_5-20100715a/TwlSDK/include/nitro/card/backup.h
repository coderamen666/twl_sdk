/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - include
  File:     backup.h

  Copyright 2007-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/
#ifndef NITRO_CARD_BACKUP_H_
#define NITRO_CARD_BACKUP_H_


#include <nitro/card/types.h>

#include <nitro/mi/dma.h>


#ifdef __cplusplus
extern  "C"
{
#endif


/*---------------------------------------------------------------------------*/
/* Constants */

// Backup device type
#define	CARD_BACKUP_TYPE_DEVICE_SHIFT	0
#define	CARD_BACKUP_TYPE_DEVICE_MASK	0xFF
#define	CARD_BACKUP_TYPE_DEVICE_EEPROM	1
#define	CARD_BACKUP_TYPE_DEVICE_FLASH	2
#define	CARD_BACKUP_TYPE_DEVICE_FRAM	3
#define	CARD_BACKUP_TYPE_SIZEBIT_SHIFT	8
#define	CARD_BACKUP_TYPE_SIZEBIT_MASK	0xFF
#define	CARD_BACKUP_TYPE_VENDER_SHIFT	16
#define	CARD_BACKUP_TYPE_VENDER_MASK	0xFF
#define	CARD_BACKUP_TYPE_DEFINE(type, size, vender)	\
	(((CARD_BACKUP_TYPE_DEVICE_ ## type) << CARD_BACKUP_TYPE_DEVICE_SHIFT) |	\
	((size) << CARD_BACKUP_TYPE_SIZEBIT_SHIFT) |	\
	((vender) << CARD_BACKUP_TYPE_VENDER_SHIFT))

typedef enum CARDBackupType
{
    CARD_BACKUP_TYPE_EEPROM_4KBITS = CARD_BACKUP_TYPE_DEFINE(EEPROM, 9, 0),
    CARD_BACKUP_TYPE_EEPROM_64KBITS = CARD_BACKUP_TYPE_DEFINE(EEPROM, 13, 0),
    CARD_BACKUP_TYPE_EEPROM_512KBITS = CARD_BACKUP_TYPE_DEFINE(EEPROM, 16, 0),
    CARD_BACKUP_TYPE_EEPROM_1MBITS = CARD_BACKUP_TYPE_DEFINE(EEPROM, 17, 0),
    CARD_BACKUP_TYPE_FLASH_2MBITS = CARD_BACKUP_TYPE_DEFINE(FLASH, 18, 0),
    CARD_BACKUP_TYPE_FLASH_4MBITS = CARD_BACKUP_TYPE_DEFINE(FLASH, 19, 0),
    CARD_BACKUP_TYPE_FLASH_8MBITS = CARD_BACKUP_TYPE_DEFINE(FLASH, 20, 0),
    CARD_BACKUP_TYPE_FLASH_16MBITS = CARD_BACKUP_TYPE_DEFINE(FLASH, 21, 0),
    CARD_BACKUP_TYPE_FLASH_64MBITS = CARD_BACKUP_TYPE_DEFINE(FLASH, 23, 0),
    CARD_BACKUP_TYPE_FRAM_256KBITS = CARD_BACKUP_TYPE_DEFINE(FRAM, 15, 0),
    CARD_BACKUP_TYPE_NOT_USE = 0
}
CARDBackupType;

#define CARD_BACKUP_TYPE_FLASH_64MBITS_EX (CARDBackupType)CARD_BACKUP_TYPE_DEFINE(FLASH, 23, 1)


// (Used internally by the library)

// PXI command request to components and the ensata emulator
typedef enum CARDRequest
{
    CARD_REQ_INIT = 0,                 /* initialize (setting from ARM9) */
    CARD_REQ_ACK,                      /* request done (acknowledge from ARM7) */
    CARD_REQ_IDENTIFY,                 /* CARD_IdentifyBackup */
    CARD_REQ_READ_ID,                  /* CARD_ReadRomID (TEG && ARM9) */
    CARD_REQ_READ_ROM,                 /* CARD_ReadRom (TEG && ARM9) */
    CARD_REQ_WRITE_ROM,                /* (reserved) */
    CARD_REQ_READ_BACKUP,              /* CARD_ReadBackup */
    CARD_REQ_WRITE_BACKUP,             /* CARD_WriteBackup */
    CARD_REQ_PROGRAM_BACKUP,           /* CARD_ProgramBackup */
    CARD_REQ_VERIFY_BACKUP,            /* CARD_VerifyBackup */
    CARD_REQ_ERASE_PAGE_BACKUP,        /* CARD_EraseBackupPage */
    CARD_REQ_ERASE_SECTOR_BACKUP,      /* CARD_EraseBackupSector */
    CARD_REQ_ERASE_CHIP_BACKUP,        /* CARD_EraseBackupChip */
    CARD_REQ_READ_STATUS,              /* CARD_ReadStatus */
    CARD_REQ_WRITE_STATUS,             /* CARD_WriteStatus */
    CARD_REQ_ERASE_SUBSECTOR_BACKUP,   /* CARD_EraseBackupSubSector */
    CARD_REQ_MAX
}
CARDRequest;

// Command request operation type
typedef enum CARDRequestMode
{
    CARD_REQUEST_MODE_RECV,            /* Receive data */
    CARD_REQUEST_MODE_SEND,            /* Send data (including single verify) */
    CARD_REQUEST_MODE_SEND_VERIFY,     /* Send data + verify */
    CARD_REQUEST_MODE_SPECIAL          /* Special operations like sector deletion */
}
CARDRequestMode;

// Maximum number of retries
#define	CARD_RETRY_COUNT_MAX	10

// PXI protocol definition
#define CARD_PXI_COMMAND_MASK              0x0000003f   // command part
#define CARD_PXI_COMMAND_SHIFT             0
#define CARD_PXI_COMMAND_PARAM_MASK        0x01ffffc0   // parameter part
#define CARD_PXI_COMMAND_PARAM_SHIFT       6

//---- PXI command
#define CARD_PXI_COMMAND_TERMINATE         0x0001       // arm9->arm7 terminate command
#define CARD_PXI_COMMAND_PULLED_OUT        0x0011       // arm7->arm9 pulled out message
#define CARD_PXI_COMMAND_RESET_SLOT        0x0002       // arm7->arm9 reset-slot message


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         CARD_IdentifyBackup

  Description:  Specifies the type of the card backup device.

  Arguments:    type: A CARDBackupType device type

  Returns:      TRUE if a test read succeeded.
 *---------------------------------------------------------------------------*/
BOOL    CARD_IdentifyBackup(CARDBackupType type);

/*---------------------------------------------------------------------------*
  Name:         CARD_GetCurrentBackupType

  Description:  Gets the backup device type that is currently specified.

  Arguments:    None.

  Returns:      The backup device type that is currently specified.
 *---------------------------------------------------------------------------*/
CARDBackupType CARD_GetCurrentBackupType(void);

/*---------------------------------------------------------------------------*
  Name:         CARD_GetBackupTotalSize

  Description:  Gets the overall size of the currently specified backup device.

  Arguments:    None.

  Returns:      Overall size of the backup device.
 *---------------------------------------------------------------------------*/
u32     CARD_GetBackupTotalSize(void);

/*---------------------------------------------------------------------------*
  Name:         CARD_GetBackupSectorSize

  Description:  Gets the sector size of the currently specified backup device.

  Arguments:    None.

  Returns:      Sector size of the backup device
 *---------------------------------------------------------------------------*/
u32     CARD_GetBackupSectorSize(void);

/*---------------------------------------------------------------------------*
  Name:         CARD_GetBackupPageSize

  Description:  Gets the page size of the currently specified backup device.

  Arguments:    None.

  Returns:      Page size of the backup device
 *---------------------------------------------------------------------------*/
u32     CARD_GetBackupPageSize(void);

/*---------------------------------------------------------------------------*
  Name:         CARD_LockBackup

  Description:  Locks the card bus to access the backup device.

  Arguments:    lock_id  Mutex ID allocated by the OS_GetLockID function

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARD_LockBackup(u16 lock_id);

/*---------------------------------------------------------------------------*
  Name:         CARD_UnlockBackup

  Description:  Unlocks the card bus that was locked to access the backup device.

  Arguments:    lock_id  Mutex ID used by the CARD_LockBackup function

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARD_UnlockBackup(u16 lock_id);

/*---------------------------------------------------------------------------*
  Name:         CARD_TryWaitBackupAsync

  Description:  Determines whether a backup device access function has completed.

  Arguments:    None.

  Returns:      TRUE if a backup device access function has completed
 *---------------------------------------------------------------------------*/
BOOL    CARD_TryWaitBackupAsync(void);

/*---------------------------------------------------------------------------*
  Name:         CARD_WaitBackupAsync

  Description:  Waits for a backup device access function to complete.

  Arguments:    None.

  Returns:      TRUE if the last call to a backup device access function completed with CARD_RESULT_SUCCESS. Otherwise, FALSE.
                
 *---------------------------------------------------------------------------*/
BOOL    CARD_WaitBackupAsync(void);

/*---------------------------------------------------------------------------*
  Name:         CARD_CancelBackupAsync

  Description:  Sends a cancel request to the backup device access function being processed.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARD_CancelBackupAsync(void);


// internal chip command as Serial-Bus-Interface

/*---------------------------------------------------------------------------*
  Name:         CARDi_RequestStreamCommand

  Description:  Issues a command request to transfer data.

  Arguments:    src        Transfer source offset or memory address
                dst        Transfer destination offset or memory address
                len        Transfer size
                callback   Completion callback (NULL if not used)
                arg        Argument of completion callback (ignored if not used)
                is_async   If async operation was specified, TRUE
                req_type   Command request type
                req_retry  Maximum number of retries when command request fails
                req_mode   Command request operation mode

  Returns:      TRUE if processing was successful.
 *---------------------------------------------------------------------------*/
BOOL    CARDi_RequestStreamCommand(u32 src, u32 dst, u32 len,
                                   MIDmaCallback callback, void *arg, BOOL is_async,
                                   CARDRequest req_type, int req_retry, CARDRequestMode req_mode);

/*---------------------------------------------------------------------------*
  Name:         CARDi_RequestWriteSectorCommand

  Description:  Erases sectors and issues program requests.

  Arguments:    src        Transfer source memory address
                dst        Transfer destination offset
                len        Transfer size
                verify     TRUE when performing a verify
                callback   Completion callback (NULL if not used)
                arg        Argument of completion callback (ignored if not used)
                is_async   If async operation was specified, TRUE

  Returns:      TRUE if processing was successful.
 *---------------------------------------------------------------------------*/
BOOL    CARDi_RequestWriteSectorCommand(u32 src, u32 dst, u32 len, BOOL verify,
                                        MIDmaCallback callback, void *arg, BOOL is_async);

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadBackup

  Description:  Issues a "read" chip command.

  Arguments:    src        Transfer source offset or memory address
                dst        Transfer destination offset or memory address
                len        Transfer size
                callback   Completion callback (NULL if not used)
                arg        Argument of completion callback (ignored if not used)
                is_async   If async operation was specified, TRUE

  Returns:      TRUE if processing was successful.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARDi_ReadBackup(u32 src, void *dst, u32 len,
                                 MIDmaCallback callback, void *arg, BOOL is_async)
{
    return CARDi_RequestStreamCommand((u32)src, (u32)dst, len,
                                      callback, arg, is_async,
                                      CARD_REQ_READ_BACKUP, 1, CARD_REQUEST_MODE_RECV);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_ProgramBackup

  Description:  Issues a read command that uses the "program" chip command.

  Arguments:    src        Transfer source offset or memory address
                dst        Transfer destination offset or memory address
                len        Transfer size
                callback   Completion callback (NULL if not used)
                arg        Argument of completion callback (ignored if not used)
                is_async   If async operation was specified, TRUE

  Returns:      TRUE if processing was successful.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARDi_ProgramBackup(u32 dst, const void *src, u32 len,
                                    MIDmaCallback callback, void *arg, BOOL is_async)
{
    return CARDi_RequestStreamCommand((u32)src, (u32)dst, len, callback, arg, is_async,
                                      CARD_REQ_PROGRAM_BACKUP, CARD_RETRY_COUNT_MAX,
                                      CARD_REQUEST_MODE_SEND);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_WriteBackup

  Description:  Issues a write command that uses the "write" chip command.

  Arguments:    src        Transfer source offset or memory address
                dst        Transfer destination offset or memory address
                len        Transfer size
                callback   Completion callback (NULL if not used)
                arg        Argument of completion callback (ignored if not used)
                is_async   If async operation was specified, TRUE

  Returns:      TRUE if processing was successful.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARDi_WriteBackup(u32 dst, const void *src, u32 len,
                                  MIDmaCallback callback, void *arg, BOOL is_async)
{
    return CARDi_RequestStreamCommand((u32)src, (u32)dst, len, callback, arg, is_async,
                                      CARD_REQ_WRITE_BACKUP, CARD_RETRY_COUNT_MAX,
                                      CARD_REQUEST_MODE_SEND);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_VerifyBackup

  Description:  Issues a verify command that uses the "read" chip command.

  Arguments:    src        Transfer source offset or memory address
                dst        Transfer destination offset or memory address
                len        Transfer size
                callback   Completion callback (NULL if not used)
                arg        Argument of completion callback (ignored if not used)
                is_async   If async operation was specified, TRUE

  Returns:      TRUE if processing was successful.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARDi_VerifyBackup(u32 dst, const void *src, u32 len,
                                   MIDmaCallback callback, void *arg, BOOL is_async)
{
    return CARDi_RequestStreamCommand((u32)src, (u32)dst, len, callback, arg, is_async,
                                      CARD_REQ_VERIFY_BACKUP, 1, CARD_REQUEST_MODE_SEND);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_ProgramAndVerifyBackup

  Description:  Issues a program and verify command that uses the "program" chip command.

  Arguments:    src        Transfer source offset or memory address
                dst        Transfer destination offset or memory address
                len        Transfer size
                callback   Completion callback (NULL if not used)
                arg        Argument of completion callback (ignored if not used)
                is_async   If async operation was specified, TRUE

  Returns:      TRUE if processing was successful.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARDi_ProgramAndVerifyBackup(u32 dst, const void *src, u32 len,
                                             MIDmaCallback callback, void *arg, BOOL is_async)
{
    return CARDi_RequestStreamCommand((u32)src, (u32)dst, len, callback, arg, is_async,
                                      CARD_REQ_PROGRAM_BACKUP, CARD_RETRY_COUNT_MAX,
                                      CARD_REQUEST_MODE_SEND_VERIFY);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_WriteAndVerifyBackup

  Description:  Issues a program and verify command that uses the "write" chip command.

  Arguments:    src        Transfer source offset or memory address
                dst        Transfer destination offset or memory address
                len        Transfer size
                callback   Completion callback (NULL if not used)
                arg        Argument of completion callback (ignored if not used)
                is_async   If async operation was specified, TRUE

  Returns:      TRUE if processing was successful.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARDi_WriteAndVerifyBackup(u32 dst, const void *src, u32 len,
                                           MIDmaCallback callback, void *arg, BOOL is_async)
{
    return CARDi_RequestStreamCommand((u32)src, (u32)dst, len, callback, arg, is_async,
                                      CARD_REQ_WRITE_BACKUP, CARD_RETRY_COUNT_MAX,
                                      CARD_REQUEST_MODE_SEND_VERIFY);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_EraseBackupSector

  Description:  Issues a sector deletion command that uses the "erase sector" chip command.

  Arguments:    dst        the deletion target offset
                len        the deletion size
                callback   Completion callback (NULL if not used)
                arg        Argument of completion callback (ignored if not used)
                is_async   If async operation was specified, TRUE

  Returns:      TRUE if processing was successful.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARDi_EraseBackupSector(u32 dst, u32 len,
                                        MIDmaCallback callback, void *arg, BOOL is_async)
{
    return CARDi_RequestStreamCommand(0, (u32)dst, len, callback, arg, is_async,
                                      CARD_REQ_ERASE_SECTOR_BACKUP, CARD_RETRY_COUNT_MAX,
                                      CARD_REQUEST_MODE_SPECIAL);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_EraseBackupSubSector

  Description:  Issues a sector deletion command using the chip command, "erase subsector".

  Arguments:    dst        the deletion target offset
                len        the deletion size
                callback   Completion callback (NULL if not used)
                arg        Argument of completion callback (ignored if not used)
                is_async   If async operation was specified, TRUE

  Returns:      If processing is successful, TRUE.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARDi_EraseBackupSubSector(u32 dst, u32 len,
                                           MIDmaCallback callback, void *arg, BOOL is_async)
{
    return CARDi_RequestStreamCommand(0, (u32)dst, len, callback, arg, is_async,
                                      CARD_REQ_ERASE_SUBSECTOR_BACKUP, CARD_RETRY_COUNT_MAX,
                                      CARD_REQUEST_MODE_SPECIAL);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_EraseBackupChip

  Description:  Issues a sector deletion command that uses the "erase chip" chip command.

  Arguments:    callback   Completion callback (NULL if not used)
                arg        Argument of completion callback (ignored if not used)
                is_async   If async operation was specified, TRUE

  Returns:      TRUE if processing was successful.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARDi_EraseBackupChip(MIDmaCallback callback, void *arg, BOOL is_async)
{
    return CARDi_RequestStreamCommand(0, 0, 0, callback, arg, is_async,
                                      CARD_REQ_ERASE_CHIP_BACKUP, 1, CARD_REQUEST_MODE_SPECIAL);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_ReadBackupAsync

  Description:  Asynchronous backup read (equivalent to the chip command "read")

  Arguments:    src        Transfer source offset
                dst        Transfer destination memory address
                len        Transfer size
                callback   Completion callback (NULL if not used)
                arg        Argument of completion callback (ignored if not used)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_ReadBackupAsync(u32 src, void *dst, u32 len, MIDmaCallback callback, void *arg)
{
    (void)CARDi_ReadBackup(src, dst, len, callback, arg, TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_ReadBackup

  Description:  Synchronous backup read (equivalent to the chip command "read")

  Arguments:    src        Transfer source offset
                dst        Transfer destination memory address
                len        Transfer size

  Returns:      TRUE on completion with CARD_RESULT_SUCCESS and FALSE otherwise.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_ReadBackup(u32 src, void *dst, u32 len)
{
    return CARDi_ReadBackup(src, dst, len, NULL, NULL, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_ProgramBackupAsync

  Description:  Asynchronous backup program (equivalent to the chip command "program")

  Arguments:    dst:        Transfer destination offset
                src:        Transfer source memory address
                len        Transfer size
                callback   Completion callback (NULL if not used)
                arg        Argument of completion callback (ignored if not used)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_ProgramBackupAsync(u32 dst, const void *src, u32 len,
                                        MIDmaCallback callback, void *arg)
{
    (void)CARDi_ProgramBackup(dst, src, len, callback, arg, TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_ProgramBackup

  Description:  Synchronous backup program (equivalent to the chip command "program")

  Arguments:    dst        Transfer destination offset
                src        Transfer source memory address
                len        Transfer size

  Returns:      TRUE on completion with CARD_RESULT_SUCCESS and FALSE otherwise.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_ProgramBackup(u32 dst, const void *src, u32 len)
{
    return CARDi_ProgramBackup(dst, src, len, NULL, NULL, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteBackupAsync

  Description:  Asynchronous backup write (equivalent to the chip command "write")

  Arguments:    dst        Transfer destination offset
                src        Transfer source memory address
                len        Transfer size
                callback   Completion callback (NULL if not used)
                arg        Argument of completion callback (ignored if not used)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_WriteBackupAsync(u32 dst, const void *src, u32 len,
                                      MIDmaCallback callback, void *arg)
{
    (void)CARDi_WriteBackup(dst, src, len, callback, arg, TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteBackup

  Description:  Synchronous backup write (equivalent to the chip command "write")

  Arguments:    dst        Transfer destination offset
                src        Transfer source memory address
                len        Transfer size

  Returns:      TRUE on completion with CARD_RESULT_SUCCESS and FALSE otherwise.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_WriteBackup(u32 dst, const void *src, u32 len)
{
    return CARDi_WriteBackup(dst, src, len, NULL, NULL, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_VerifyBackupAsync

  Description:  Asynchronous backup verify (equivalent to the chip command "read")

  Arguments:    src        Comparison source offset
                dst        Comparison destination memory address
                len        Comparison size
                callback   Completion callback (NULL if not used)
                arg        Argument of completion callback (ignored if not used)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_VerifyBackupAsync(u32 dst, const void *src, u32 len,
                                       MIDmaCallback callback, void *arg)
{
    (void)CARDi_VerifyBackup(dst, src, len, callback, arg, TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_VerifyBackup

  Description:  Synchronous backup verify (equivalent to the chip command "read")

  Arguments:    src        Comparison source offset
                dst        Comparison destination memory address
                len        Comparison size

  Returns:      TRUE on completion with CARD_RESULT_SUCCESS and FALSE otherwise.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_VerifyBackup(u32 dst, const void *src, u32 len)
{
    return CARDi_VerifyBackup(dst, src, len, NULL, NULL, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_ProgramAndVerifyBackupAsync

  Description:  Asynchronous backup program + verify

  Arguments:    dst        Transfer destination offset
                src        Transfer source memory address
                len        Transfer size
                callback   Completion callback (NULL if not used)
                arg        Argument of completion callback (ignored if not used)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_ProgramAndVerifyBackupAsync(u32 dst, const void *src, u32 len,
                                                 MIDmaCallback callback, void *arg)
{
    (void)CARDi_ProgramAndVerifyBackup(dst, src, len, callback, arg, TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_ProgramAndVerifyBackup

  Description:  Synchronous backup program + verify

  Arguments:    dst        Transfer destination offset
                src        Transfer source memory address
                len        Transfer size

  Returns:      TRUE on completion with CARD_RESULT_SUCCESS and FALSE otherwise.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_ProgramAndVerifyBackup(u32 dst, const void *src, u32 len)
{
    return CARDi_ProgramAndVerifyBackup(dst, src, len, NULL, NULL, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteAndVerifyBackupAsync

  Description:  Asynchronous backup write + verify

  Arguments:    dst        Transfer destination offset
                src        Transfer source memory address
                len        Transfer size
                callback   Completion callback (NULL if not used)
                arg        Argument of completion callback (ignored if not used)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_WriteAndVerifyBackupAsync(u32 dst, const void *src, u32 len,
                                               MIDmaCallback callback, void *arg)
{
    (void)CARDi_WriteAndVerifyBackup(dst, src, len, callback, arg, TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteAndVerifyBackup

  Description:  Synchronous backup write + Verify

  Arguments:    dst        Transfer destination offset
                src        Transfer source memory address
                len        Transfer size

  Returns:      TRUE on completion with CARD_RESULT_SUCCESS and FALSE otherwise.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_WriteAndVerifyBackup(u32 dst, const void *src, u32 len)
{
    return CARDi_WriteAndVerifyBackup(dst, src, len, NULL, NULL, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_EraseBackupSectorAsync

  Description:  Asynchronous sector deletion.

  Arguments:    dst        Deletion target offset
                len        Deletion size
                callback   Completion callback (NULL if not used)
                arg        Argument of completion callback (ignored if not used)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_EraseBackupSectorAsync(u32 dst, u32 len, MIDmaCallback callback, void *arg)
{
    (void)CARDi_EraseBackupSector(dst, len, callback, arg, TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_EraseBackupSector

  Description:  Deletes sectors synchronously.

  Arguments:    dst        Deletion target offset
                len        Deletion size

  Returns:      TRUE on completion with CARD_RESULT_SUCCESS and FALSE otherwise.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_EraseBackupSector(u32 dst, u32 len)
{
    return CARDi_EraseBackupSector(dst, len, NULL, NULL, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_EraseBackupSubSectorAsync

  Description:  Asynchronous subsector deletion.

  Arguments:    dst        Deletion target offset
                len        Deletion size
                callback   Completion callback (NULL if not used)
                arg        Argument of completion callback (ignored if not used)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_EraseBackupSubSectorAsync(u32 dst, u32 len, MIDmaCallback callback, void *arg)
{
    (void)CARDi_EraseBackupSubSector(dst, len, callback, arg, TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_EraseBackupSubSector

  Description:  Synchronous subsector deletion.

  Arguments:    dst        Deletion target offset
                len        Deletion size

  Returns:      TRUE if completed with CARD_RESULT_SUCCESS, FALSE for everything else.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_EraseBackupSubSector(u32 dst, u32 len)
{
    return CARDi_EraseBackupSubSector(dst, len, NULL, NULL, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_EraseBackupChipAsync

  Description:  Deletes a chip asynchronously.

  Arguments:    callback   Completion callback (NULL if not used)
                arg        Argument of completion callback (ignored if not used)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_EraseBackupChipAsync(MIDmaCallback callback, void *arg)
{
    (void)CARDi_EraseBackupChip(callback, arg, TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_EraseBackupChip

  Description:  Deletes sectors synchronously.

  Arguments:    None.

  Returns:      TRUE on completion with CARD_RESULT_SUCCESS and FALSE otherwise.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_EraseBackupChip(void)
{
    return CARDi_EraseBackupChip(NULL, NULL, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteBackupSectorAsync

  Description:  Erases and programs by sector.

  Arguments:    dst        Transfer destination offset
                src        Transfer source memory address
                len        Transfer size
                callback   Completion callback (NULL if not used)
                arg        Argument of completion callback (ignored if not used)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_WriteBackupSectorAsync(u32 dst, const void *src, u32 len,
                                            MIDmaCallback callback, void *arg)
{
    (void)CARDi_RequestWriteSectorCommand((u32)src, dst, len, FALSE, callback, arg, TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteBackupSector

  Description:  Erases and programs by sector.

  Arguments:    dst        Transfer destination offset
                src        Transfer source memory address
                len        Transfer size

  Returns:      TRUE on completion with CARD_RESULT_SUCCESS and FALSE otherwise.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_WriteBackupSector(u32 dst, const void *src, u32 len)
{
    return CARDi_RequestWriteSectorCommand((u32)src, dst, len, FALSE, NULL, NULL, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteAndVerifyBackupSectorAsync

  Description:  Erases and programs by sectors, then verifies.

  Arguments:    dst        Transfer destination offset
                src        Transfer source memory address
                len        Transfer size
                callback   Completion callback (NULL if not used)
                arg        Argument of completion callback (ignored if not used)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_WriteAndVerifyBackupSectorAsync(u32 dst, const void *src, u32 len,
                                                     MIDmaCallback callback, void *arg)
{
    (void)CARDi_RequestWriteSectorCommand((u32)src, dst, len, TRUE, callback, arg, TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteAndVerifyBackupSector

  Description:  Erases and programs by sectors, then verifies.

  Arguments:    dst        Transfer destination offset
                src        Transfer source memory address
                len        Transfer size

  Returns:      TRUE on completion with CARD_RESULT_SUCCESS and FALSE otherwise.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_WriteAndVerifyBackupSector(u32 dst, const void *src, u32 len)
{
    return CARDi_RequestWriteSectorCommand((u32)src, dst, len, TRUE, NULL, NULL, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_AccessStatus

  Description:  Status read or write (for testing)

  Arguments:    command    CARD_REQ_READ_STATUS or CARD_REQ_WRITE_STATUS
                value      The value to write, if CARD_REQ_WRITE_STATUS

  Returns:      Returns a value of 0 or higher if successful; a negative number otherwise.
 *---------------------------------------------------------------------------*/
int CARDi_AccessStatus(CARDRequest command, u8 value);

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadStatus

  Description:  Status read (for testing)

  Arguments:    None.

  Returns:      Returns a value of 0 or higher if successful; a negative number otherwise.
 *---------------------------------------------------------------------------*/
SDK_INLINE int CARDi_ReadStatus(void)
{
    return CARDi_AccessStatus(CARD_REQ_READ_STATUS, 0);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_WriteStatus

  Description:  Status write (for testing)

  Arguments:    value      The value to be written

  Returns:      TRUE if completed with CARD_RESULT_SUCCESS, FALSE for everything else.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARDi_WriteStatus(u8 value)
{
    return (CARDi_AccessStatus(CARD_REQ_WRITE_STATUS, value) >= 0);
}


#ifdef __cplusplus
}
#endif // extern "C"


#endif // NITRO_CARD_BACKUP_H_
