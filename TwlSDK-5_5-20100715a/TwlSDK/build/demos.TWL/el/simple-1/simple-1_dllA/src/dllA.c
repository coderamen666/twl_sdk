/*---------------------------------------------------------------------------*
  Project:  TwlSDK - EL - demos - dllA
  File:     dllA.c

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

#pragma optimization_level 0


int calu_func_A(int i)
{
    int j = i;
    
    OS_TPrintf("call calu_func_A %d\n", i);
    
    return j;
}

void global_func_A(void)
{
    int i;
    
    i = calu_func_A(0);
}

#pragma optimization_level reset
