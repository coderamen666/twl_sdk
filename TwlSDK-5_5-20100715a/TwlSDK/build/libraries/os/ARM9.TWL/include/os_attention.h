/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS
  File:     os_attention.h

  Copyright 2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-10-19#$
  $Rev: 11099 $
  $Author: mizutani_nakaba $
 *---------------------------------------------------------------------------*/

#ifndef TWL_OS_ATTENTION_H_
#define TWL_OS_ATTENTION_H_

#include <nitro/misc.h>
#include <nitro/types.h>

#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*
  Name:         OS_ShowAttentionOfLimitedRom

  Description:  show string of attention to run limited mode on NITRO.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void OS_ShowAttentionOfLimitedRom(void);


#ifdef __cplusplus
}
#endif

#endif // #ifndef TWL_OS_ATTENTION_H_

