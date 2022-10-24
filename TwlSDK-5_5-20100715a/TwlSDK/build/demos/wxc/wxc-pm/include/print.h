/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - demos - wxc-pm
  File:     print.h

  Copyright 2005-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-11-15#$
  $Rev: 2414 $
  $Author: hatamoto_minoru $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_BUILD_DEMOS_INCLUDE_PRINT_H_
#define NITRO_BUILD_DEMOS_INCLUDE_PRINT_H_

void ClearStringY(s16 Y);
void ClearString(void);
void PrintString(s16 x, s16 y, u8 palette, char *text, ...);

#endif /* NITRO_BUILD_DEMOS_INCLUDE_PRINT_H_ */