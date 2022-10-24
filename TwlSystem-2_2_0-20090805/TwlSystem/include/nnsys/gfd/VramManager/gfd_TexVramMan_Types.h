/*---------------------------------------------------------------------------*
  Project:  TWL-System - include - nnsys - gfd - VramManager
  File:     gfd_TexVramMan_Types.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
#ifndef NNS_GFD_TEXVRAMMAN_TYPES_H_
#define NNS_GFD_TEXVRAMMAN_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro.h>
#include <nnsys/gfd/gfd_common.h>

#define NNS_GFD_MASK_15BIT 0x7FFF
#define NNS_GFD_MASK_16BIT 0xFFFF

#define NNS_GFD_TEXKEY_SIZE_SHIFT 4
#define NNS_GFD_TEXKEY_ADDR_SHIFT 3


#define NNS_GFD_TEXSIZE_MIN ( 0x1 << NNS_GFD_TEXKEY_SIZE_SHIFT )
#define NNS_GFD_TEXSIZE_MAX ( 0x7FFF << NNS_GFD_TEXKEY_SIZE_SHIFT )




// The incorrect texture key that failed in allocation
#define NNS_GFD_ALLOC_ERROR_TEXKEY          (u32)0x0


//------------------------------------------------------------------------------
//
// NNSGfdTexKey:
// A 32-bit value which can designate the region for the texture image slot in an 8-byte unit.
// Values 0-0xffff can be used as the error value (because the size is 0).
//
// 31      30                    17  16                         0    
// 4x4Comp 4 bit right-shifted size, 3 bit right-shifted offset    
//
//------------------------------------------------------------------------------
typedef u32 NNSGfdTexKey;

//------------------------------------------------------------------------------
// NNSGfdFuncAllocTexVram
// szByte:    Specifies the size to allocate in bytes
// is4x4comp: Whether or not this is for the 4x4comp format
// opt:       Argument that depends on implementation (i.e., allocation from the front, allocation from the back, etc.)
//
// When is4x4comp is TRUE, the return value is a texture image region.
// The texture palette index region must also be separately allocated.
//------------------------------------------------------------------------------
typedef NNSGfdTexKey (*NNSGfdFuncAllocTexVram)(u32 szByte, BOOL is4x4comp, u32 opt);




//------------------------------------------------------------------------------
// NNSGfdFuncFreeTexVram
// Designates the key and deallocates the texture image slot region.
// There must be an implementation so that there is no vagueness even if is4x4comp is not designated.
// Normal return if the return value is 0. Any other value will result in an implementation dependency error.
//------------------------------------------------------------------------------
typedef int (*NNSGfdFuncFreeTexVram)(NNSGfdTexKey key);


//------------------------------------------------------------------------------
//
// This may also be changed by the user.
//
//------------------------------------------------------------------------------
extern NNSGfdFuncAllocTexVram   NNS_GfdDefaultFuncAllocTexVram; 
extern NNSGfdFuncFreeTexVram    NNS_GfdDefaultFuncFreeTexVram;  


//------------------------------------------------------------------------------
//
// Items similar to macros
// The library code will be accessed only via this function.
//
//------------------------------------------------------------------------------
NNS_GFD_INLINE NNSGfdTexKey
NNS_GfdAllocTexVram(u32 szByte, BOOL is4x4comp, u32 opt)
{
    return (*NNS_GfdDefaultFuncAllocTexVram)(szByte, is4x4comp, opt );
}

//------------------------------------------------------------------------------
//
// Items similar to macros
// The library code will be accessed only via this function.
//
//------------------------------------------------------------------------------
NNS_GFD_INLINE int
NNS_GfdFreeTexVram(NNSGfdTexKey memKey)
{
    return (*NNS_GfdDefaultFuncFreeTexVram)(memKey);
}



//------------------------------------------------------------------------------
//
// NNSGfdTexKey operations-related
//
//
//------------------------------------------------------------------------------




//------------------------------------------------------------------------------
// Gets the rounded size so that NNSGfdTexKey can be expressed.
NNS_GFD_INLINE u32
NNSi_GfdGetTexKeyRoundupSize( u32 size )
{
    if( size == 0 )
    {
        return NNS_GFD_TEXSIZE_MIN;
    }else{
        
        return (((u32)(size) + ( NNS_GFD_TEXSIZE_MIN-1 )) & ~( NNS_GFD_TEXSIZE_MIN-1 ) );
    }
}    

//------------------------------------------------------------------------------
NNS_GFD_INLINE NNSGfdTexKey
NNS_GfdMakeTexKey( u32 addr, u32 size, BOOL b4x4Comp )
{
    // Has a rounding error been generated?
    SDK_ASSERT( (addr & (u32)((0x1 << NNS_GFD_TEXKEY_ADDR_SHIFT) - 1 )) == 0 );
    SDK_ASSERT( (size & (u32)((0x1 << NNS_GFD_TEXKEY_SIZE_SHIFT) - 1 )) == 0 );
    
    // Has an overflow been generated?
    SDK_ASSERT( ( (size >> NNS_GFD_TEXKEY_SIZE_SHIFT) & ~NNS_GFD_MASK_15BIT ) == 0 );
    SDK_ASSERT( ( (addr >> NNS_GFD_TEXKEY_ADDR_SHIFT) & ~NNS_GFD_MASK_16BIT ) == 0 );
    
    return  ( ( size >> NNS_GFD_TEXKEY_SIZE_SHIFT ) << 16 ) 
            | ( ( NNS_GFD_MASK_16BIT & ( addr >> NNS_GFD_TEXKEY_ADDR_SHIFT ) ) ) 
            | b4x4Comp << 31;
}

//------------------------------------------------------------------------------
NNS_GFD_INLINE u32
NNS_GfdGetTexKeyAddr( NNSGfdTexKey memKey )
{
    return (u32)( ( ( 0x0000FFFF & memKey ) ) << NNS_GFD_TEXKEY_ADDR_SHIFT );
}

//------------------------------------------------------------------------------
NNS_GFD_INLINE u32
NNS_GfdGetTexKeySize( NNSGfdTexKey memKey )
{
    return (u32)( ( ( 0x7FFF0000 & memKey ) >> 16 ) << NNS_GFD_TEXKEY_SIZE_SHIFT );
}

//------------------------------------------------------------------------------
NNS_GFD_INLINE BOOL
NNS_GfdGetTexKey4x4Flag( NNSGfdTexKey memKey )
{
    return (BOOL)( (0x80000000 & memKey) >> 31 );
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // NNS_GFD_TEXVRAMMAN_TYPES_H_
