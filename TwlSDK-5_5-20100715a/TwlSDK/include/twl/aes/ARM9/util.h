/*---------------------------------------------------------------------------*
  Project:  TwlSDK - AES - include
  File:     util.h

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef	TWL_AES_ARM9_UTIL_H_
#define	TWL_AES_ARM9_UTIL_H_

#ifdef	__cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include	<nitro/os/common/emulator.h>
#include	<twl/aes/common/types.h>


/*---------------------------------------------------------------------------*
    Constant Definitions
 *---------------------------------------------------------------------------*/

#define AES_ENCRYPT_HEADER_SIZE 16
#define AES_SIGN_HEADER_SIZE    32



/*---------------------------------------------------------------------------*
	Function Declarations
 *---------------------------------------------------------------------------*/

void AESi_InitRand(void);
void AESi_PrepairEncryptAndSign(AESNonce* pNonce, u32 srcSize, void* dst);
AESResult AESi_PrepairDecryptAndVerify(AESNonce* pNonce, const void* src, u32 srcSize);

AESResult AESi_Rand(void* pBuffer, u32 size);

// dst size = srcSize + 16
AESResult AESi_Encrypt(
            const void* src,
            u32         srcSize,
            void*       dst,
            AESCallback callback,
            void*       arg );

// dst size = srcSize - 16
AESResult AESi_Decrypt(
            const void* src,
            u32         srcSize,
            void*       dst,
            AESCallback callback,
            void*       arg );

// dst size = srcSize + 32
AESResult AESi_EncryptAndSign(
            const void* src,
            u32         srcSize,
            void*       dst,
            AESCallback callback,
            void*       arg );

// dst size = srcSize - 32
AESResult AESi_DecryptAndVerify(
            const void* src,
            u32         srcSize,
            void*       dst,
            AESCallback callback,
            void*       arg );



/*---------------------------------------------------------------------------*
	Inline Function Definitions
 *---------------------------------------------------------------------------*/

SDK_INLINE AESResult AES_Rand(void* pBuffer, u32 size)
{
    if( OS_IsRunOnTwl() )
    {
        return AESi_Rand(pBuffer, size);
    }
    else
    {
        return AES_RESULT_ON_DS;
    }
}

// dst size = srcSize + 16
SDK_INLINE AESResult AES_Encrypt(
            const void* src,
            u32         srcSize,
            void*       dst,
            AESCallback callback,
            void*       arg )
{
    if( OS_IsRunOnTwl() )
    {
        return AESi_Encrypt(src, srcSize, dst, callback, arg);
    }
    else
    {
        return AES_RESULT_ON_DS;
    }
}

// dst size = srcSize - 16
SDK_INLINE AESResult AES_Decrypt(
            const void* src,
            u32         srcSize,
            void*       dst,
            AESCallback callback,
            void*       arg )
{
    if( OS_IsRunOnTwl() )
    {
        return AESi_Decrypt(src, srcSize, dst, callback, arg);
    }
    else
    {
        return AES_RESULT_ON_DS;
    }
}

// dst size = srcSize + 32
SDK_INLINE AESResult AES_EncryptAndSign(
            const void* src,
            u32         srcSize,
            void*       dst,
            AESCallback callback,
            void*       arg )
{
    if( OS_IsRunOnTwl() )
    {
        return AESi_EncryptAndSign(src, srcSize, dst, callback, arg);
    }
    else
    {
        return AES_RESULT_ON_DS;
    }
}

// dst size = srcSize - 32
SDK_INLINE AESResult AES_DecryptAndVerify(
            const void* src,
            u32         srcSize,
            void*       dst,
            AESCallback callback,
            void*       arg )
{
    if( OS_IsRunOnTwl() )
    {
        return AESi_DecryptAndVerify(src, srcSize, dst, callback, arg);
    }
    else
    {
        return AES_RESULT_ON_DS;
    }
}


/*===========================================================================*/

#ifdef	__cplusplus
}          /* extern "C" */
#endif

#endif /* TWL_AES_ARM9_UTIL_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
