/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - multiboot-PowerSave
  File:     common.h

  Copyright 2006-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#if !defined(NITRO_MB_DEMO_MULTIBOOT_POWERSAVE_COMMON_H_)
#define NITRO_MB_DEMO_MULTIBOOT_POWERSAVE_COMMON_H_


#ifdef SDK_TWL
#include	<twl.h>
#else
#include	<nitro.h>
#endif


/*****************************************************************************/
/* Variables */

/* Configures power-save mode */
extern BOOL g_power_save_mode;


/*****************************************************************************/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         ReadKeySetTrigger

  Description:  Detects key trigger.

  Arguments:    None.

  Returns:      A bit set showing the keys that were pressed during this invocation and not the previous one.
                
 *---------------------------------------------------------------------------*/
u16     ReadKeySetTrigger(void);

/*---------------------------------------------------------------------------*
  Name:         MenuMode

  Description:  Initial settings menu for the DS Single-Card parent device.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    MenuMode(void);

/*---------------------------------------------------------------------------*
  Name:         ParentMode

  Description:  DS Single-Card play parent processing.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    ParentMode(void);


#endif /* !defined(NITRO_MB_DEMO_MULTIBOOT_POWERSAVE_COMMON_H_) */
