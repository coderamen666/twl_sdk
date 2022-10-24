/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS
  File:     os_profile.c

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


#ifndef OS_PROFILE_AVAILABLE

SDK_WEAK_SYMBOL asm void __PROFILE_ENTRY( void )
{
    bx   lr
}
SDK_WEAK_SYMBOL asm void __PROFILE_EXIT( void )
{
    bx   lr
}
#endif // ifdef OS_PROFILE_AVAILABLE
