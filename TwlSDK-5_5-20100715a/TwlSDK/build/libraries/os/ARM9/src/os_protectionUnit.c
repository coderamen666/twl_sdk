/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS
  File:     os_protectionUnit.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#include <nitro/os.h>
#include <nitro/code32.h>

//======================================================================
//          protection unit
//======================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_EnableProtectionUnit

  Description:  enable protection unit

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
asm void OS_EnableProtectionUnit( void )
{
    mrc     p15, 0, r0, c1, c0, 0
    orr     r0, r0, #HW_C1_PROTECT_UNIT_ENABLE
    mcr     p15, 0, r0, c1, c0, 0
    bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         OS_DisableProtectionUnit

  Description:  disable protection unit

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
asm void OS_DisableProtectionUnit( void )
{
    mrc     p15, 0, r0, c1, c0, 0
    bic     r0, r0, #HW_C1_PROTECT_UNIT_ENABLE
    mcr     p15, 0, r0, c1, c0, 0
    bx      lr
}

#include <nitro/codereset.h>
