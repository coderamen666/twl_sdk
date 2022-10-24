/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MI - include
  File:     endian.h

  Copyright 2006-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_MI_ENDIAN_H_
#define NITRO_MI_ENDIAN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/misc.h>
#include <nitro/types.h>
#include <nitro/platform.h>


/*****************************************************************************/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         MI_SwapEndian8

  Description:  A macro to convert a 8-bit value's endian type.
                This will actually never perform any conversions.

  Arguments:    val               the value for which to convert the endian type

  Returns:      The value for which the endian type has been converted
 *---------------------------------------------------------------------------*/
#define MI_SwapEndian8(val) (u8)(val)

/*---------------------------------------------------------------------------*
  Name:         MI_SwapEndian16

  Description:  A macro to convert a 16-bit value's endian type.
                Although this can be used for constants, be aware that the arguments will be evaluated multiple times.

  Arguments:    val               the value for which to convert the endian type

  Returns:      The value for which the endian type has been converted
 *---------------------------------------------------------------------------*/
#define MI_SwapEndian16(val) (u16)( \
                              (((val) & 0xFF00UL) >> 8UL) | \
                              (((val) & 0x00FFUL) << 8UL))

/*---------------------------------------------------------------------------*
  Name:         MI_SwapEndian32

  Description:  A macro to convert a 32-bit value's endian type.
                Although this can be used for constants, be aware that the arguments will be evaluated multiple times.

  Arguments:    val               the value for which to convert the endian type

  Returns:      The value for which the endian type has been converted
 *---------------------------------------------------------------------------*/
#define MI_SwapEndian32(val) (u32)( \
                              (((val) & 0xFF000000UL) >> 24UL) | \
                              (((val) & 0x00FF0000UL) >> 8UL) | \
                              (((val) & 0x0000FF00UL) << 8UL) | \
                              (((val) & 0x000000FFUL) << 24UL))

/*---------------------------------------------------------------------------*
  Name:         MI_SwapEndian64

  Description:  A macro to convert a 64-bit value's endian type.
                Although this can be used for constants, be aware that the arguments will be evaluated multiple times.

  Arguments:    val               the value for which to convert the endian type

  Returns:      The value for which the endian type has been converted
 *---------------------------------------------------------------------------*/
#define MI_SwapEndian64(val) (u64)(                                         \
                              (((val) & 0xFF00000000000000ULL) >> 56ULL)    \
                            | (((val) & 0x00FF000000000000ULL) >> 40ULL)    \
                            | (((val) & 0x0000FF0000000000ULL) >> 24ULL)    \
                            | (((val) & 0x000000FF00000000ULL) >>  8ULL)    \
                            | (((val) & 0x00000000FF000000ULL) <<  8ULL)    \
                            | (((val) & 0x0000000000FF0000ULL) << 24ULL)    \
                            | (((val) & 0x000000000000FF00ULL) << 40ULL)    \
                            | (((val) & 0x00000000000000FFULL) << 56ULL) )

#if (PLATFORM_BYTES_ENDIAN == PLATFORM_ENDIAN_LITTLE)
#define MIi_SwapEndian8IfLE(val)  MI_SwapEndian8(val)
#define MIi_SwapEndian16IfLE(val) MI_SwapEndian16(val)
#define MIi_SwapEndian32IfLE(val) MI_SwapEndian32(val)
#define MIi_SwapEndian64IfLE(val) MI_SwapEndian64(val)
#define MIi_SwapEndian8IfBE(val)  (val)
#define MIi_SwapEndian16IfBE(val) (val)
#define MIi_SwapEndian32IfBE(val) (val)
#define MIi_SwapEndian64IfBE(val) (val)
#else
#define MIi_SwapEndian8IfLE(val)  (val)
#define MIi_SwapEndian16IfLE(val) (val)
#define MIi_SwapEndian32IfLE(val) (val)
#define MIi_SwapEndian64IfLE(val) (val)
#define MIi_SwapEndian8IfBE(val)  MI_SwapEndian8(val)
#define MIi_SwapEndian16IfBE(val) MI_SwapEndian16(val)
#define MIi_SwapEndian32IfBE(val) MI_SwapEndian32(val)
#define MIi_SwapEndian64IfBE(val) MI_SwapEndian64(val)
#endif

/*---------------------------------------------------------------------------*
  Name:         MI_HTo*

  Description:  Value conversion from current environment to specified endian type.
                As the Nintendo DS uses a little-endian environment, conversion takes place with a do-nothing macro for *HToLe*.
                
                Although this can be used for constants, be aware that the arguments will be evaluated multiple times.

  Arguments:    val               the value for which to convert the endian type

  Returns:      The value converted to the specified endian type.
 *---------------------------------------------------------------------------*/
#define MI_HToLE8(val)  MIi_SwapEndian8IfBE(val)
#define MI_HToBE8(val)  MIi_SwapEndian8IfLE(val)
#define MI_HToLE16(val) MIi_SwapEndian16IfBE(val)
#define MI_HToBE16(val) MIi_SwapEndian16IfLE(val)
#define MI_HToLE32(val) MIi_SwapEndian32IfBE(val)
#define MI_HToBE32(val) MIi_SwapEndian32IfLE(val)
#define MI_HToBE64(val) MIi_SwapEndian64IfLE(val)

/*---------------------------------------------------------------------------*
  Name:         MI_*ToH*

  Description:  Value conversion from the specified endian type to the current environment.
                As the Nintendo DS uses a little-endian environment, conversion takes place with a do-nothing macro for *LeToH*.
                
                Although this can be used for constants, be aware that the arguments will be evaluated multiple times.

  Arguments:    val               the value for which to convert the endian type

  Returns:      The value converted from the specified endian type.
 *---------------------------------------------------------------------------*/
#define MI_LEToH8(val)  MIi_SwapEndian8IfBE(val)
#define MI_BEToH8(val)  MIi_SwapEndian8IfLE(val)
#define MI_LEToH16(val) MIi_SwapEndian16IfBE(val)
#define MI_BEToH16(val) MIi_SwapEndian16IfLE(val)
#define MI_LEToH32(val) MIi_SwapEndian32IfBE(val)
#define MI_BEToH32(val) MIi_SwapEndian32IfLE(val)
#define MI_BEToH64(val) MIi_SwapEndian64IfLE(val)

/*---------------------------------------------------------------------------*
  Name:         MI_LoadLE*

  Description:  Gets the little-endian value from the specified address.
                The content of this function is not dependent on the current environment's endian type.

  Arguments:    ptr               The data address read in as little-endian.
                                  You don't need to worry about alignment.

  Returns:      The value of the specified bit width gotten in little-endian.
 *---------------------------------------------------------------------------*/
SDK_INLINE u8 MI_LoadLE8(const void *ptr)
{
    const u8 *src = (const u8 *)ptr;
    return (u8)((src[0] << 0UL));
}
SDK_INLINE u16 MI_LoadLE16(const void *ptr)
{
    const u8 *src = (const u8 *)ptr;
    return (u16)((src[0] << 0UL) | (src[1] << 8UL));
}
SDK_INLINE u32 MI_LoadLE32(const void *ptr)
{
    const u8 *src = (const u8 *)ptr;
    return (u32)((src[0] << 0UL) | (src[1] << 8UL) | (src[2] << 16UL) | (src[3] << 24UL));
}
SDK_INLINE u64 MI_LoadLE64(const void *ptr)
{
    const u8 *src = (const u8 *)ptr;
    return ((u64)MI_LoadLE32(src + 4) << 32) | (u64)MI_LoadLE32(src);
}

/*---------------------------------------------------------------------------*
  Name:         MI_LoadBE*

  Description:  Gets the big-endian value from the specified address.
                The content of this function is not dependent on the current environment's endian type.

  Arguments:    ptr               The data address read in as big-endian.
                                 You don't need to worry about alignment.

  Returns:      The value of the specified bit width gotten in big-endian.
 *---------------------------------------------------------------------------*/
SDK_INLINE u8 MI_LoadBE8(const void *ptr)
{
    const u8 *src = (const u8 *)ptr;
    return (u8)((src[0] << 0UL));
}
SDK_INLINE u16 MI_LoadBE16(const void *ptr)
{
    const u8 *src = (const u8 *)ptr;
    return (u16)((src[0] << 8UL) | (src[1] << 0UL));
}
SDK_INLINE u32 MI_LoadBE32(const void *ptr)
{
    const u8 *src = (const u8 *)ptr;
    return (u32)((src[0] << 24UL) | (src[1] << 16UL) | (src[2] << 8UL) | (src[3] << 0UL));
}
SDK_INLINE u64 MI_LoadBE64(const void *ptr)
{
    const u8 *src = (const u8 *)ptr;
    return ((u64)MI_LoadLE32(src) << 32) | (u64)MI_LoadLE32(src + 4);
}

/*---------------------------------------------------------------------------*
  Name:         MI_StoreLE*

  Description:  Stores the little-endian value at the specified address.
                The content of this function is not dependent on the current environment's endian type.

  Arguments:    ptr               The data address written as little-endian.
                                  You don't need to worry about alignment.
                val               The value to be written

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void MI_StoreLE8(void *ptr, u8 val)
{
    u8     *src = (u8 *)ptr;
    src[0] = (u8)(val >> 0UL);
}
SDK_INLINE void MI_StoreLE16(void *ptr, u16 val)
{
    u8     *src = (u8 *)ptr;
    src[0] = (u8)(val >> 0UL);
    src[1] = (u8)(val >> 8UL);
}
SDK_INLINE void MI_StoreLE32(void *ptr, u32 val)
{
    u8     *src = (u8 *)ptr;
    src[0] = (u8)(val >> 0UL);
    src[1] = (u8)(val >> 8UL);
    src[2] = (u8)(val >> 16UL);
    src[3] = (u8)(val >> 24UL);
}
SDK_INLINE void MI_StoreLE64(void *ptr, u64 val)
{
    u8     *src = (u8 *)ptr;
    MI_StoreLE32(src,     (u32)(val >>  0));
    MI_StoreLE32(src + 4, (u32)(val >> 32));
}

/*---------------------------------------------------------------------------*
  Name:         MI_StoreBE*

  Description:  Stores the big-endian value at the specified address.
                The content of this function is not dependent on the current environment's endian type.

  Arguments:    ptr               The data address written as big-endian.
                                  You don't need to worry about alignment.
                val               The value to be written

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void MI_StoreBE8(void *ptr, u8 val)
{
    u8     *src = (u8 *)ptr;
    src[0] = (u8)(val >> 0UL);
}
SDK_INLINE void MI_StoreBE16(void *ptr, u16 val)
{
    u8     *src = (u8 *)ptr;
    src[0] = (u8)(val >> 8UL);
    src[1] = (u8)(val >> 0UL);
}
SDK_INLINE void MI_StoreBE32(void *ptr, u32 val)
{
    u8     *src = (u8 *)ptr;
    src[0] = (u8)(val >> 24UL);
    src[1] = (u8)(val >> 16UL);
    src[2] = (u8)(val >> 8UL);
    src[3] = (u8)(val >> 0UL);
}
SDK_INLINE void MI_StoreBE64(void *ptr, u64 val)
{
    u8     *src = (u8 *)ptr;
    MI_StoreLE32(src,     (u32)(val >> 32));
    MI_StoreLE32(src + 4, (u32)(val >>  0));
}


#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_MI_ENDIAN_H_ */
#endif
