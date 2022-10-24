/*---------------------------------------------------------------------------*
  Project:  TwlSDK - AES - include
  File:     aes.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef	TWL_AES_H_
#define	TWL_AES_H_

#ifdef	__cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include	<twl/aes/common/types.h>

#ifdef SDK_ARM9
#include	<twl/aes/ARM9/aes_inline.h>
#include	<twl/aes/ARM9/util.h>
#else   // ifdef SDK_ARM9
#include	<twl/aes/ARM7/hi.h>
#endif  // ifdef SDK_ARM9 else

/*===========================================================================*/

#ifdef	__cplusplus
}          /* extern "C" */
#endif

#endif /* TWL_AES_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
