/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - multiboot-wfs - child
  File:     func_1.c

  Copyright 2005-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifdef SDK_TWL
#include	<twl.h>
#else
#include	<nitro.h>
#endif

#include	"func.h"


/******************************************************************************/
/* function */

#if	defined(__cplusplus)
extern  "C"
{
#endif


    void    func_1(char *dst)
    {
        static int i = 0;
              ++i;
                (void)OS_SPrintf(dst, "func_1. called %d time%s.\n", i, (i == 1) ? "" : "s", &i);
    }


#if	defined(__cplusplus)
}                                      /* extern "C" */
#endif


/******************************************************************************/
