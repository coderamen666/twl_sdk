/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NA - include
  File:     archive.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef TWL_NA_ARM9_ARCHIVE_H_
#define TWL_NA_ARM9_ARCHIVE_H_

#include    <twl/misc.h>
#include    <twl/types.h>
#include    <nitro/os/common/emulator.h>

#ifdef  __cplusplus
extern "C" {
#endif

/*===========================================================================*/


typedef enum NATitleArchive
{
    NA_TITLE_ARCHIVE_DATAPUB,
    NA_TITLE_ARCHIVE_DATAPRV,
    NA_TITLE_ARCHIVE_MAX
}
NATitleArchive;


FSResult NAi_LoadOtherTitleArchive(const char* initialCode, NATitleArchive archive);
FSResult NAi_UnloadOtherTitleArchive(void);
FSResult NAi_FormatTitleArchive(NATitleArchive archive);

static inline FSResult NA_LoadOtherTitleArchive(const char* initialCode, NATitleArchive archive)
    { return OS_IsRunOnTwl() ? NAi_LoadOtherTitleArchive(initialCode, archive): FS_RESULT_UNSUPPORTED; }
static inline FSResult NA_UnloadOtherTitleArchive(void)
    { return OS_IsRunOnTwl() ? NAi_UnloadOtherTitleArchive(): FS_RESULT_UNSUPPORTED; }
static inline FSResult NA_FormatTitleArchive(NATitleArchive archive)
    { return OS_IsRunOnTwl() ? NAi_FormatTitleArchive(archive): FS_RESULT_UNSUPPORTED; }

/*===========================================================================*/

#ifdef  __cplusplus
}          /* extern "C" */
#endif

#endif /* TWL_NA_ARM9_ARCHIVE_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
