/*---------------------------------------------------------------------------*
  Project:  TwlSDK - FS - libraries
  File:     fs_api.c

  Copyright 2007-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-19#$
  $Rev: 10786 $
  $Author: okajima_manabu $

 *---------------------------------------------------------------------------*/


#include <nitro/types.h>
#include <nitro/misc.h>
#include <nitro/mi.h>
#include <nitro/os.h>
#include <nitro/pxi.h>
#include <nitro/card/common.h>

#include <nitro/fs.h>

#include "../include/util.h"
#include "../include/command.h"
#include "../include/rom.h"


/*---------------------------------------------------------------------------*/
/* Variables */

static BOOL is_init = FALSE;


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         FS_Init

  Description:  Initializes file system.

  Arguments:    default_dma_no:   Default DMA channel for FS
                                 If out of range (0-3),
                                 use CpuCopy instead of DMA.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FS_Init(u32 default_dma_no)
{
#if defined(FS_IMPLEMENT)
    SDK_TASSERTMSG(default_dma_no != 0, "cannot use DMA channel 0 for ROM access");
    if (!is_init)
    {
        is_init = TRUE;
#if defined(SDK_TWLLTD)
        // Changed to explicitly initialize the FAT driver at the application's discretion to prevent always linking unnecessary Unicode-related modules.
        // 
        // (NAND applications are expressly indicated within the FSi_OverrideRomArchive function)
        FS_InitFatDriver();
#endif
        FSi_InitRomArchive(default_dma_no);
        FSi_InitOverlay();
    }
#else
#pragma unused(default_dma_no)
    CARD_Init();
#endif /* FS_IMPLEMENT */
}

/*---------------------------------------------------------------------------*
  Name:         FS_IsAvailable

  Description:  Checks if file system is ready.

  Arguments:    None.

  Returns:      If file system is ready, TRUE.
 *---------------------------------------------------------------------------*/
BOOL FS_IsAvailable(void)
{
    return is_init;
}

/*---------------------------------------------------------------------------*
  Name:         FS_End

  Description:  Finalizes file system.
                Unloads and unregisters all the archives.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FS_End(void)
{
    OSIntrMode bak_psr = OS_DisableInterrupts();
    if (is_init)
    {
#if defined(FS_IMPLEMENT)
        FSi_EndArchive();
        FSi_EndRomArchive();
#endif /* FS_IMPLEMENT */
        is_init = FALSE;
    }
    (void)OS_RestoreInterrupts(bak_psr);
}

#if defined(FS_IMPLEMENT) && defined(SDK_TWL)

/*---------------------------------------------------------------------------*
  Name:         FS_InitFatDriver

  Description:  Initializes the FAT driver.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FS_InitFatDriver(void)
{
    static BOOL once = FALSE;
    if (!once)
    {
        once = TRUE;
        SDK_ASSERT(FS_IsAvailable());
        if (OS_IsRunOnTwl())
        {
            FSi_MountDefaultArchives();
        }
    }
}
#endif
