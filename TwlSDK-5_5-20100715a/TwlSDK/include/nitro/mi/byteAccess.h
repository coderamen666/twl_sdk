/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MI - include
  File:     byteAccess.h

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

#ifndef NITRO_MI_BYTEACCESS_H_
#define NITRO_MI_BYTEACCESS_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SDK_TWL
#include <nitro/types.h>
#else
#include <twl/types.h>
#endif

/*---------------------------------------------------------------------------*
  Name:         MI_ReadByte

  Description:  read bytedata

  Arguments:    address : address to read bytedata

  Returns:      byte data
 *---------------------------------------------------------------------------*/
static inline u8 MI_ReadByte(const void *address)
{
    return *(u8 *)address;
}


/*---------------------------------------------------------------------------*
  Name:         MI_WriteByte

  Description:  write bytedata

  Arguments:    address : address to write bytedata

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void MI_WriteByte(void *address, u8 value)
{
    *(u8 *)address = value;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_MI_BYTEACCESS_H_ */
#endif
