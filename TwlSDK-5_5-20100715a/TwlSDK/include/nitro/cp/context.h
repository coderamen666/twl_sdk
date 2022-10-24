/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CP - include
  File:     context.h

  Copyright 2003-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_CP_CONTEXT_H_
#define NITRO_CP_CONTEXT_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SDK_TWL
#include <nitro/ioreg.h>
#else
#include <twl/ioreg.h>
#endif

#include <nitro/cp/divider.h>

//----------------------------------------------------------------
typedef struct CPContext
{
    u64     div_numer;
    u64     div_denom;
    u64     sqrt;
    u16     div_mode;
    u16     sqrt_mode;
}
CPContext;

/*---------------------------------------------------------------------------*
  Name:         CP_SaveContext

  Description:  Save current context into specified memory

  Arguments:    context   pointer to the memory to be stored the current context

  Returns:      None
 *---------------------------------------------------------------------------*/
void    CP_SaveContext(CPContext *context);


/*---------------------------------------------------------------------------*
  Name:         CP_RestoreContext

  Description:  Reload specified context as current context

  Arguments:    context   pointer to the memory to switch to the context

  Returns:      None
 *---------------------------------------------------------------------------*/
void    CPi_RestoreContext(const CPContext *context);
static inline void CP_RestoreContext(const CPContext *context)
{
    CPi_RestoreContext(context);
    CP_WaitDiv();
}



#ifdef __cplusplus
} /* extern "C" */
#endif


/* NITRO_CP_CONTEXT_H_ */
#endif
