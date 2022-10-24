/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     entropy.h

  Copyright 2003-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_OS_ENTROPY_H_
#define NITRO_OS_ENTROPY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/misc.h>
#include <nitro/types.h>
#ifndef SDK_TWL
#include <nitro/ioreg.h>
#else
#include <twl/ioreg.h>
#endif

#define OS_LOW_ENTROPY_DATA_SIZE 32

/*---------------------------------------------------------------------------*
  Name:         OS_GetLowEntropyData

  Description:  Gets low entropy data array that changes according to the system status.

  Arguments:    buffer - Pointer to the array that holds the data string.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    OS_GetLowEntropyData(u32 buffer[OS_LOW_ENTROPY_DATA_SIZE / sizeof(u32)]);



#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_OS_ENTROPY_H_ */
#endif
