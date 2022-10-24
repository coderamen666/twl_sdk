/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     pxi.h

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-07-31#$
  $Rev: 387 $
  $Author: nakasima $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_OS_PXI_H_
#define NITRO_OS_PXI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/misc.h>
#include <nitro/types.h>
#include <nitro/pxi.h>

//================================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_IsResetOccurred

  Description:  get whethre reset event occurred

  Arguments:    None.

  Returns:      TRUE  : if reset occurred
                FALSE : if not
 *---------------------------------------------------------------------------*/
BOOL    OS_IsResetOccurred(void);

#ifdef SDK_TWL

/*---------------------------------------------------------------------------*
  Name:         OSi_IsTerminatePxiOccurred

  Description:  get status whethre terminate event occurred

  Arguments:    None.

  Returns:      TRUE  : if terminate event occurred
                FALSE : if not
 *---------------------------------------------------------------------------*/
BOOL    OSi_IsTerminatePxiOccurred(void);

#endif // SDK_TWL

/*---------------------------------------------------------------------------*
  Name:         OSi_SetTerminatePxiOccurred

  Description:  set status of terminate event occurred

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL
void OSi_SetTerminatePxiOccurred(void);
#else // SDK_NITRO
#define OSi_SetTerminatePxiOccurred()  ((void)0)
#endif // SDK_NITRO

//================================================================================
//-------- internal
void OSi_CommonCallback(PXIFifoTag tag, u32 data, BOOL err);
void OSi_SendToPxi(u16 data);


#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_OS_PXI_H_ */
#endif
