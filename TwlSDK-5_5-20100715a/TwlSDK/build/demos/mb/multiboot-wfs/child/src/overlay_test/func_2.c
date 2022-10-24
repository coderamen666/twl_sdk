/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - multiboot-wfs - child
  File:     func_2.c

  Copyright 2005-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifdef SDK_TWL
#include	<twl.h>
#else
#include	<nitro.h>
#endif

#include	"func.h"


/******************************************************************************/
/* Functions */

#if	defined(__cplusplus)
extern  "C"
{
#endif


    void    func_2(char *dst)
    {
        static int i = 0;
              ++i;
                (void)OS_SPrintf(dst, "func_2. called %d time%s.\n", i, (i == 1) ? "" : "s", &i);

        /* This is dummy code used to redundantly increase the code size. */
        if      ((i > 0) && (i < 0))
        {
            volatile int v;
                    OVERLAY_CODE_BY_1000(++v);
        }
    }


#if	defined(__cplusplus)
}                                      /* Extern "C" */
#endif


/******************************************************************************/
