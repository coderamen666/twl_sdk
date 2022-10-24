/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - include
  File:     eeprom.h

  Copyright 2007-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/
#ifndef NITRO_CARD_EEPROM_H_
#define NITRO_CARD_EEPROM_H_


#include <nitro/card/backup.h>


#ifdef __cplusplus
extern  "C"
{
#endif


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         CARD_IsBackupEeprom

  Description:  Determines whether the backup device type is currently configured to be EEPROM.

  Arguments:    None.

  Returns:      TRUE if the backup device type is currently configured to be EEPROM.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_IsBackupEeprom(void)
{
    const CARDBackupType t = CARD_GetCurrentBackupType();
    return (((t >> CARD_BACKUP_TYPE_DEVICE_SHIFT) &
              CARD_BACKUP_TYPE_DEVICE_MASK) == CARD_BACKUP_TYPE_DEVICE_EEPROM);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_ReadEeprom

  Description:  Performs a synchronous EEPROM read (equivalent to the "read" chip command).

  Arguments:    src: Transfer source offset
                dst: Transfer destination memory address
                len: Transfer size

  Returns:      TRUE on completion with CARD_RESULT_SUCCESS and FALSE otherwise.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_ReadEeprom(u32 src, void *dst, u32 len)
{
    return CARD_ReadBackup(src, dst, len);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_ReadEepromAsync

  Description:  Performs an asynchronous EEPROM read (equivalent to the "read" chip command).

  Arguments:    src: Transfer source offset
                dst: Transfer destination memory address
                len: Transfer size
                callback: Completion callback (NULL if not used)
                arg: Argument of completion callback (ignored if not used)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_ReadEepromAsync(u32 src, void *dst, u32 len,
                                     MIDmaCallback callback, void *arg)
{
    CARD_ReadBackupAsync(src, dst, len, callback, arg);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteEeprom

  Description:  Performs a synchronous EEPROM write (equivalent to the "program" chip command).

  Arguments:    dst: Transfer destination offset
                src: Transfer source memory address
                len: Transfer size

  Returns:      TRUE on completion with CARD_RESULT_SUCCESS and FALSE otherwise.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_WriteEeprom(u32 dst, const void *src, u32 len)
{
    return CARD_ProgramBackup(dst, src, len);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteFlashAsync

  Description:  Performs an asynchronous EEPROM write (equivalent to the "write" chip command).

  Arguments:    dst: Transfer destination offset
                src: Transfer source memory address
                len: Transfer size
                callback: Completion callback (NULL if not used)
                arg: Argument of completion callback (ignored if not used)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_WriteEepromAsync(u32 dst, const void *src, u32 len,
                                      MIDmaCallback callback, void *arg)
{
    CARD_ProgramBackupAsync(dst, src, len, callback, arg);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_VerifyEeprom

  Description:  Synchronously verifies an EEPROM.

  Arguments:    dst: Offset of the comparison target
                src: Memory address of the comparison source
                len: Comparison size

  Returns:      TRUE on completion with CARD_RESULT_SUCCESS and FALSE otherwise.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_VerifyEeprom(u32 dst, const void *src, u32 len)
{
    return CARD_VerifyBackup(dst, src, len);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_VerifyEepromAsync

  Description:  Asynchronously verifies an EEPROM.

  Arguments:    dst: Offset of the comparison target
                src: Memory address of the comparison source
                len: Comparison size
                callback: Completion callback (NULL if not used)
                arg: Argument of completion callback (ignored if not used)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_VerifyEepromAsync(u32 dst, const void *src, u32 len,
                                       MIDmaCallback callback, void *arg)
{
    CARD_VerifyBackupAsync(dst, src, len, callback, arg);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteAndVerifyEeprom

  Description:  Synchronously performs an EEPROM write and verification.

  Arguments:    dst: Transfer destination offset
                src: Transfer source memory address
                len: Transfer size

  Returns:      TRUE on completion with CARD_RESULT_SUCCESS and FALSE otherwise.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_WriteAndVerifyEeprom(u32 dst, const void *src, u32 len)
{
    return CARD_ProgramAndVerifyBackup(dst, src, len);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteAndVerifyEepromAsync

  Description:  Asynchronously performs an EEPROM write and verification.

  Arguments:    dst: Transfer destination offset
                src: Transfer source memory address
                len: Transfer size
                callback: Completion callback (NULL if not used)
                arg: Argument of completion callback (ignored if not used)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_WriteAndVerifyEepromAsync(u32 dst, const void *src, u32 len,
                                               MIDmaCallback callback, void *arg)
{
    CARD_ProgramAndVerifyBackupAsync(dst, src, len, callback, arg);
}


#ifdef __cplusplus
} // extern "C"
#endif


#endif // NITRO_CARD_EEPROM_H_
