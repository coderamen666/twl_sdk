/*---------------------------------------------------------------------------*
  Project:  TWL-System - include - nnsys - gfd - VramManager
  File:     gfd_PlttVramMan_Types.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
#ifndef NNS_GFD_PLTTVRAMMAN_TYPES_H_
#define NNS_GFD_PLTTVRAMMAN_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro.h>
#include <nnsys/gfd/gfd_common.h>

#define NNS_GFD_PLTTMASK_16BIT              0xFFFF


#define NNS_GFD_PLTTKEY_SIZE_SHIFT 3
#define NNS_GFD_PLTTKEY_ADDR_SHIFT 3

#define NNS_GFD_PLTTSIZE_MIN ( 0x1 << NNS_GFD_PLTTKEY_SIZE_SHIFT )
#define NNS_GFD_PLTTSIZE_MAX ( NNS_GFD_PLTTMASK_16BIT << NNS_GFD_PLTTKEY_SIZE_SHIFT )


#define NNS_GFD_ALLOC_ERROR_PLTTKEY         (u32)0x0
#define NNS_GFD_4PLTT_MAX_ADDR              0x10000



//------------------------------------------------------------------------------
//
// NNSGfdPlttKey:
// A 32-bit value that can designate the region of the texture palette slot in 8-byte units.
// Values 0-0xffff can be used as the error value (because the size is 0).
//
// 31                    16  15                         0
// 3 bit right-shifted size  3 bit right-shifted offset
//------------------------------------------------------------------------------
typedef u32 NNSGfdPlttKey;


//------------------------------------------------------------------------------
// NNSGfdFuncAllocPlttVram
// szByte:  Specifies the size to allocate in bytes
// is4pltt: Whether the storage of 4-color palette needs to be possible
// opt:     Argument that depends on implementation (i.e., allocation from the front, allocation from the back, etc.)
//------------------------------------------------------------------------------
typedef NNSGfdPlttKey (*NNSGfdFuncAllocPlttVram)(u32 szByte, BOOL is4pltt, u32 opt);




//------------------------------------------------------------------------------
// NNSGfdFuncFreePlttVram
// Designates the key and deallocates the texture image slot region.
// It needs to be implemented in a way it is not going to be vague even without specifying is4pltt.
// Normal return if the return value is 0. Any other value will result in an implementation dependency error.
//------------------------------------------------------------------------------
typedef int (*NNSGfdFuncFreePlttVram)(NNSGfdPlttKey plttKey);


//------------------------------------------------------------------------------
//
// This may also be changed by the user.
//
//------------------------------------------------------------------------------
extern NNSGfdFuncAllocPlttVram  NNS_GfdDefaultFuncAllocPlttVram; 
extern NNSGfdFuncFreePlttVram   NNS_GfdDefaultFuncFreePlttVram;  


//------------------------------------------------------------------------------
//
// The library code will only be able to be accessed via this function.
//
//------------------------------------------------------------------------------
NNS_GFD_INLINE NNSGfdPlttKey
NNS_GfdAllocPlttVram(u32 szByte, BOOL is4pltt, u32 opt)
{
    return (*NNS_GfdDefaultFuncAllocPlttVram)( szByte, is4pltt, opt );
}

NNS_GFD_INLINE int
NNS_GfdFreePlttVram(NNSGfdPlttKey plttKey)
{
    return (*NNS_GfdDefaultFuncFreePlttVram)(plttKey);
}


//------------------------------------------------------------------------------
//
// NNSGfdPlttKey operations-related
//
//
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Gets the rounded size so that NNSGfdPlttKey can be expressed.
NNS_GFD_INLINE u32
NNSi_GfdGetPlttKeyRoundupSize( u32 size )
{
    if( size == 0 )
    {
        return NNS_GFD_PLTTSIZE_MIN;
    }else{
        return (((u32)(size) + ( NNS_GFD_PLTTSIZE_MIN-1 )) & ~( NNS_GFD_PLTTSIZE_MIN-1 ) );
    }
}

//------------------------------------------------------------------------------
NNS_GFD_INLINE NNSGfdPlttKey
NNS_GfdMakePlttKey( u32 addr, u32 size )
{
    // Has a rounding error been generated?
    SDK_ASSERT( (addr & (u32)((0x1 << NNS_GFD_PLTTKEY_ADDR_SHIFT) - 1 )) == 0 );
    SDK_ASSERT( (size & (u32)((0x1 << NNS_GFD_PLTTKEY_SIZE_SHIFT) - 1 )) == 0 );
    
    // Has an overflow been generated?
    SDK_ASSERT( ( (size >> NNS_GFD_PLTTKEY_SIZE_SHIFT) & ~NNS_GFD_PLTTMASK_16BIT ) == 0 );
    SDK_ASSERT( ( (addr >> NNS_GFD_PLTTKEY_ADDR_SHIFT) & ~NNS_GFD_PLTTMASK_16BIT ) == 0 );
    
    return    ( ( size >> NNS_GFD_PLTTKEY_SIZE_SHIFT ) << 16 ) 
            | ( 0xFFFF & ( addr >> NNS_GFD_PLTTKEY_ADDR_SHIFT ) );
}

//------------------------------------------------------------------------------
NNS_GFD_INLINE u32
NNS_GfdGetPlttKeyAddr( NNSGfdPlttKey plttKey )
{
    return (u32)( ( 0x0000FFFF & plttKey )  << NNS_GFD_PLTTKEY_ADDR_SHIFT );
}

//------------------------------------------------------------------------------
NNS_GFD_INLINE u32
NNS_GfdGetPlttKeySize( NNSGfdPlttKey plttKey )
{
    return (u32)( ( ( 0xFFFF0000 & plttKey ) >> 16 ) << NNS_GFD_PLTTKEY_SIZE_SHIFT );
}


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // NNS_GFD_PLTTVRAMMAN_TYPES_H_
