/*---------------------------------------------------------------------------*
  Project:  TwlSDK - EL - demos - dllB
  File:     dllB.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-10-30#$
  $Rev: 1932 $
  $Author: kitase_hirotake $
 *---------------------------------------------------------------------------*/

#include <twl.h>

#include "dllA.h"
#include "dllB.h"

#pragma optimization_level 0


int calu_func_B(int i)
{
    int j = i;
    
    OS_TPrintf("call calu_func_B %d\n", i);
    
    return j;
}

void global_func_B(void)
{
    int i;
    
    i = calu_func_B(1);
    i = calu_func_A(1);
}

#pragma optimization_level reset
