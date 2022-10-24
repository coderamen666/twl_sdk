/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - include
  File:     fram.h

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
#ifndef NITRO_CARD_FRAM_H_
#define NITRO_CARD_FRAM_H_


#include <nitro/card/backup.h>


#ifdef __cplusplus
extern  "C"
{
#endif


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         CARD_IsBackupFram

  Description:  Determines whether the backup device type is currently configured to be FRAM.

  Arguments:    None.

  Returns:      TRUE if the backup device type is currently configured to be FRAM.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_IsBackupFram(void)
{
    const CARDBackupType t = CARD_GetCurrentBackupType();
    return (((t >> CARD_BACKUP_TYPE_DEVICE_SHIFT) &
              CARD_BACKUP_TYPE_DEVICE_MASK) == CARD_BACKUP_TYPE_DEVICE_FRAM);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_ReadFram

  Description:  Synchronous read.

  Arguments:    src        Transfer source offset
                dst        Transfer destination memory address
                len        Transfer size

  Returns:      TRUE on completion with CARD_RESULT_SUCCESS and FALSE otherwise.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_ReadFram(u32 src, void *dst, u32 len)
{
    return CARD_ReadBackup(src, dst, len);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_ReadFramAsync

  Description:  Asynchronous read.

  Arguments:    src        Transfer source offset
                dst        Transfer destination memory address
                len        Transfer size
                callback   Completion callback (NULL if not used)
                arg        Argument of completion callback (ignored if not used)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_ReadFramAsync(u32 src, void *dst, u32 len,
                                   MIDmaCallback callback, void *arg)
{
    CARD_ReadBackupAsync(src, dst, len, callback, arg);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteFram

  Description:  Synchronous write (equivalent to the chip command "program")

  Arguments:    dst        Transfer destination offset
                src        Transfer source memory address
                len        Transfer size

  Returns:      TRUE on completion with CARD_RESULT_SUCCESS and FALSE otherwise.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_WriteFram(u32 dst, const void *src, u32 len)
{
    return CARD_ProgramBackup(dst, src, len);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteFramAsync

  Description:  Asynchronous write (equivalent to the chip command "program")

  Arguments:    dst        Transfer destination offset
                src        Transfer source memory address
                len        Transfer size
                callback   Completion callback (NULL if not used)
                arg        Argument of completion callback (ignored if not used)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_WriteFramAsync(u32 dst, const void *src, u32 len,
                                    MIDmaCallback callback, void *arg)
{
    CARD_ProgramBackupAsync(dst, src, len, callback, arg);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_VerifyFram

  Description:  Synchronous verify (equivalent to the chip command "read")

  Arguments:    src        Comparison source offset
                dst        Comparison destination memory address
                len        Comparison size

  Returns:      TRUE on completion with CARD_RESULT_SUCCESS and FALSE otherwise.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_VerifyFram(u32 dst, const void *src, u32 len)
{
    return CARD_VerifyBackup(dst, src, len);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_VerifyFramAsync

  Description:  Asynchronous verify (equivalent to the chip command "read")

  Arguments:    src        Comparison source offset
                dst        Comparison destination memory address
                len        Comparison size
                callback   Completion callback (NULL if not used)
                arg        Argument of completion callback (ignored if not used)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_VerifyFramAsync(u32 dst, const void *src, u32 len,
                                     MIDmaCallback callback, void *arg)
{
    CARD_VerifyBackupAsync(dst, src, len, callback, arg);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteAndVerifyFram

  Description:  Synchronous write + verify

  Arguments:    dst        Transfer destination offset
                src        Transfer source memory address
                len        Transfer size

  Returns:      TRUE on completion with CARD_RESULT_SUCCESS and FALSE otherwise.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_WriteAndVerifyFram(u32 dst, const void *src, u32 len)
{
    return CARD_ProgramAndVerifyBackup(dst, src, len);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteAndVerifyFramAsync

  Description:  Asynchronous write + verify

  Arguments:    dst        Transfer destination offset
                src        Transfer source memory address
                len        Transfer size
                callback   Completion callback (NULL if not used)
                arg        Argument of completion callback (ignored if not used)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_WriteAndVerifyFramAsync(u32 dst, const void *src, u32 len,
                                             MIDmaCallback callback, void *arg)
{
    CARD_ProgramAndVerifyBackupAsync(dst, src, len, callback, arg);
}


#ifdef __cplusplus
} // extern "C"
#endif


#endif // NITRO_CARD_FRAM_H_
