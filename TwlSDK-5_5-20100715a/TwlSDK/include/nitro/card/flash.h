/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - include
  File:     flash.h

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/
#ifndef NITRO_CARD_FLASH_H_
#define NITRO_CARD_FLASH_H_


#include <nitro/card/backup.h>


#ifdef __cplusplus
extern  "C"
{
#endif


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         CARD_IsBackupFlash

  Description:  Determines whether the backup device type is currently configured to be FLASH.

  Arguments:    None.

  Returns:      TRUE if the backup device type is currently configured to be FLASH.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_IsBackupFlash(void)
{
    const CARDBackupType t = CARD_GetCurrentBackupType();
    return (((t >> CARD_BACKUP_TYPE_DEVICE_SHIFT) &
              CARD_BACKUP_TYPE_DEVICE_MASK) == CARD_BACKUP_TYPE_DEVICE_FLASH);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_ReadFlash

  Description:  Synchronous FLASH read (equivalent to the chip command "read").

  Arguments:    src        Transfer source offset
                dst        Transfer destination memory address
                len        Transfer size

  Returns:      TRUE on completion with CARD_RESULT_SUCCESS and FALSE otherwise.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_ReadFlash(u32 src, void *dst, u32 len)
{
    return CARD_ReadBackup(src, dst, len);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_ReadFlashAsync

  Description:  Asynchronous FLASH read (equivalent to the chip command "read").

  Arguments:    src        Transfer source offset
                dst        Transfer destination memory address
                len        Transfer size
                callback   Completion callback (NULL if not used)
                arg        Argument of completion callback (ignored if not used)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_ReadFlashAsync(u32 src, void *dst, u32 len,
                                    MIDmaCallback callback, void *arg)
{
    CARD_ReadBackupAsync(src, dst, len, callback, arg);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteFlash

  Description:  Synchronous FLASH write (equivalent to the chip command "write").

  Arguments:    dst        Transfer destination offset
                src        Transfer source memory address
                len        Transfer size

  Returns:      TRUE on completion with CARD_RESULT_SUCCESS and FALSE otherwise.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_WriteFlash(u32 dst, const void *src, u32 len)
{
    return CARD_WriteBackup(dst, src, len);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteFlashAsync

  Description:  Asynchronous FLASH write (equivalent to the chip command "write").

  Arguments:    dst        Transfer destination offset
                src        Transfer source memory address
                len        Transfer size
                callback   Completion callback (NULL if not used)
                arg        Argument of completion callback (ignored if not used)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_WriteFlashAsync(u32 dst, const void *src, u32 len,
                                     MIDmaCallback callback, void *arg)
{
    CARD_WriteBackupAsync(dst, src, len, callback, arg);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_VerifyFlash

  Description:  Synchronous FLASH verify.

  Arguments:    dst        Offset of the comparison target
                src        Memory address of the comparison source
                len        Comparison size

  Returns:      TRUE on completion with CARD_RESULT_SUCCESS and FALSE otherwise.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_VerifyFlash(u32 dst, const void *src, u32 len)
{
    return CARD_VerifyBackup(dst, src, len);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_VerifyFlashAsync

  Description:  Asynchronous FLASH verify.

  Arguments:    dst        Offset of the comparison target
                src        Memory address of the comparison source
                len        Comparison size
                callback   Completion callback (NULL if not used)
                arg        Argument of completion callback (ignored if not used)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_VerifyFlashAsync(u32 dst, const void *src, u32 len,
                                      MIDmaCallback callback, void *arg)
{
    CARD_VerifyBackupAsync(dst, src, len, callback, arg);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteAndVerifyFlash

  Description:  Synchronous FLASH write and verification.

  Arguments:    dst        Transfer destination offset
                src        Transfer source memory address
                len        Transfer size

  Returns:      TRUE on completion with CARD_RESULT_SUCCESS and FALSE otherwise.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_WriteAndVerifyFlash(u32 dst, const void *src, u32 len)
{
    return CARD_WriteAndVerifyBackup(dst, src, len);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteAndVerifyFlashAsync

  Description:  Synchronous FLASH write and verification.

  Arguments:    dst        Transfer destination offset
                src        Transfer source memory address
                len        Transfer size
                callback   Completion callback (NULL if not used)
                arg        Argument of completion callback (ignored if not used)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_WriteAndVerifyFlashAsync(u32 dst, const void *src, u32 len,
                                              MIDmaCallback callback, void *arg)
{
    CARD_WriteAndVerifyBackupAsync(dst, src, len, callback, arg);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_EraseFlashSector

  Description:  Synchronous sector deletion.

  Arguments:    dst        Deletion target offset
                           Must be an integer multiple of the sector size.
                len        Deletion size
                           Must be an integer multiple of the sector size.

  Returns:      TRUE on completion with CARD_RESULT_SUCCESS and FALSE otherwise.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_EraseFlashSector(u32 dst, u32 len)
{
    return CARD_EraseBackupSector(dst, len);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_EraseFlashSectorAsync

  Description:  Asynchronous sector deletion.

  Arguments:    dst        Deletion target offset
                           Must be an integer multiple of the sector size.
                len        the deletion size
                           Must be an integer multiple of the sector size.
                callback   Completion callback (NULL if not used)
                arg        Argument of completion callback (ignored if not used)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_EraseFlashSectorAsync(u32 dst, u32 len,
                                           MIDmaCallback callback, void *arg)
{
    CARD_EraseBackupSectorAsync(dst, len, callback, arg);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_ProgramFlash

  Description:  Synchronous FLASH program (equivalent to the chip command "program").

  Arguments:    dst        Transfer destination offset
                src        Transfer source memory address
                len        Transfer size

  Returns:      TRUE on completion with CARD_RESULT_SUCCESS and FALSE otherwise.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_ProgramFlash(u32 dst, const void *src, u32 len)
{
    return CARD_ProgramBackup(dst, src, len);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_ProgramFlashAsync

  Description:  Asynchronous FLASH program (equivalent to the chip command "program").

  Arguments:    dst        Transfer destination offset
                src        Transfer source memory address
                len        Transfer size
                callback   Completion callback (NULL if not used)
                arg        Argument of completion callback (ignored if not used)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_ProgramFlashAsync(u32 dst, const void *src, u32 len,
                                       MIDmaCallback callback, void *arg)
{
    CARD_ProgramBackupAsync(dst, src, len, callback, arg);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_ProgramAndVerifyFlash

  Description:  Synchronous FLASH program and verification.

  Arguments:    dst        Transfer destination offset
                src        Transfer source memory address
                len        Transfer size

  Returns:      TRUE on completion with CARD_RESULT_SUCCESS and FALSE otherwise.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_ProgramAndVerifyFlash(u32 dst, const void *src, u32 len)
{
    return CARD_ProgramAndVerifyBackup(dst, src, len);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_ProgramAndVerifyFlashAsync

  Description:  Asynchronous FLASH program and verification.

  Arguments:    dst        Transfer destination offset
                src        Transfer source memory address
                len        Transfer size
                callback   Completion callback (NULL if not used)
                arg        Argument of completion callback (ignored if not used)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_ProgramAndVerifyFlashAsync(u32 dst, const void *src, u32 len,
                                                MIDmaCallback callback, void *arg)
{
    CARD_ProgramAndVerifyBackupAsync(dst, src, len, callback, arg);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteFlashSector

  Description:  Sector erasure and program.

  Arguments:    dst        Transfer destination offset
                           Must be an integer multiple of the sector size.
                src        Transfer source memory address
                len        Transfer size
                           Must be an integer multiple of the sector size.

  Returns:      TRUE on completion with CARD_RESULT_SUCCESS and FALSE otherwise.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_WriteFlashSector(u32 dst, const void *src, u32 len)
{
    return CARD_WriteBackupSector(dst, src, len);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteFlashSectorAsync

  Description:  Sector erasure and program.

  Arguments:    dst        Transfer destination offset
                           Must be an integer multiple of the sector size.
                src        Transfer source memory address
                len        Transfer size
                           Must be an integer multiple of the sector size.
                callback   Completion callback (NULL if not used)
                arg        Argument of completion callback (ignored if not used)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_WriteFlashSectorAsync(u32 dst, const void *src, u32 len,
                                           MIDmaCallback callback, void *arg)
{
    CARD_WriteBackupSectorAsync(dst, src, len, callback, arg);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteAndVerifyFlashSector

  Description:  Sector erasure, program, and verification.

  Arguments:    dst        Transfer destination offset
                           Must be an integer multiple of the sector size.
                src        Transfer source memory address
                len        Transfer size
                           Must be an integer multiple of the sector size.

  Returns:      TRUE on completion with CARD_RESULT_SUCCESS and FALSE otherwise.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_WriteAndVerifyFlashSector(u32 dst, const void *src, u32 len)
{
    return CARD_WriteAndVerifyBackupSector(dst, src, len);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteAndVerifyFlashSectorAsync

  Description:  Sector erasure, program, and verification.

  Arguments:    dst        Transfer destination offset
                           Must be an integer multiple of the sector size.
                src        Transfer source memory address
                len        Transfer size
                           Must be an integer multiple of the sector size.
                callback   Completion callback (NULL if not used)
                arg        Argument of completion callback (ignored if not used)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_WriteAndVerifyFlashSectorAsync(u32 dst, const void *src, u32 len,
                                                    MIDmaCallback callback, void *arg)
{
    CARD_WriteAndVerifyBackupSectorAsync(dst, src, len, callback, arg);
}


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* NITRO_CARD_EEPROM_H_ */
