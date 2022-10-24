/*---------------------------------------------------------------------------*
  Project:  TwlSDK - demos - FS - overlay
  File:     func_1.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-11-07#$
  $Rev: 2172 $
  $Author: yosizaki $
 *---------------------------------------------------------------------------*/


#include	<nitro.h>

#include	"func.h"


void func_1(void)
{
    static int i = 0;
    ++i;
    OS_TPrintf("here is func_1. called %d time%s.\n", i, (i == 1) ? "" : "s");
}
