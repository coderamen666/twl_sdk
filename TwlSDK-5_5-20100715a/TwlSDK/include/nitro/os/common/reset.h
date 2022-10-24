/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     reset.h

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_OS_RESET_H_
#define NITRO_OS_RESET_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/misc.h>
#include <nitro/types.h>
#include <nitro/os/common/pxi.h>


//---- PXI command
#define OS_PXI_COMMAND_RESET      0x10
#define OS_PXI_COMMAND_TERMINATE  0x20

//---- PXI command field
#define OS_PXI_COMMAND_MASK       0x00007f00
#define OS_PXI_COMMAND_SHIFT      8
#define OS_PXI_DATA_MASK          0x000000ff
#define OS_PXI_DATA_SHIFT         0


//================================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_InitReset

  Description:  init reset system

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    OS_InitReset(void);

/*---------------------------------------------------------------------------*
  Name:         OS_ResetSystem

  Description:  start reset proc

  Arguments:    parameter : parameter to be sent to next boot.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifdef SDK_ARM9
void    OS_ResetSystem(u32 parameter);
#else
void    OS_ResetSystem(void);
#endif

/*---------------------------------------------------------------------------*
  Name:         OS_GetResetParameter

  Description:  get parameter from previous reset

  Arguments:    None.

  Returns:      reset parameter
 *---------------------------------------------------------------------------*/
static inline u32 OS_GetResetParameter(void)
{
    return (u32)*(u32 *)HW_RESET_PARAMETER_BUF;
}

//================================================================================
#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_OS_RESET_H_ */
#endif
