/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     protectionUnit.h

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

#ifndef NITRO_OS_PROTECTIONUNIT_H_
#define NITRO_OS_PROTECTIONUNIT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/misc.h>
#include <nitro/types.h>


//======================================================================
//          protection unit
//======================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_EnableProtectionUnit

  Description:  enable protection unit

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void    OS_EnableProtectionUnit(void);

/*---------------------------------------------------------------------------*
  Name:         OS_DisableProtectionUnit

  Description:  disable protection unit

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void    OS_DisableProtectionUnit(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_OS_PROTECTIONUNIT_H_ */
#endif
