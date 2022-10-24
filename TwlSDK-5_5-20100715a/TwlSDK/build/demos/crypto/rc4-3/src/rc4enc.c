/*---------------------------------------------------------------------------*
  Project:  NitroSDK - CRYPTO - demos
  File:     rc4enc.c

  Copyright 2006-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-10-20#$
  $Rev: 9005 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
  Pseudorandom Number Generator
 *---------------------------------------------------------------------------*/

#include <nitro.h>
#include <nitro/crypto.h>

#include "prng.h"
#include "rc4enc.h"

/*---------------------------------------------------------------------------*
    Variable Definitions
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    Function Definitions
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         InitRC4Encoder

  Description:  Performs initialization for encryption using the RC4 algorithm.

  Arguments:    context: Context structure where the RC4 key information and so on is stored
                key: 12-byte key data

  Returns:      None.
 *---------------------------------------------------------------------------*/
void InitRC4Encoder(
    RC4EncoderContext* context,
    const void* key
)
{
    MI_CpuClear8(context, sizeof(RC4EncoderContext));

    // Key settings
    MI_CpuCopy8(key, context->key, RC4ENC_USER_KEY_LENGTH);
}

/*---------------------------------------------------------------------------*
  Name:         EncodeRC4

  Description:  Performs encryption using the RC4 algorithm.

  Arguments:    context: Context structure where the RC4 key information and so on is stored
                in: Input data
                in_len: Data length
                out: Output data
                out_len: Size of memory secured as an output buffer

  Returns:      Returns the output length if successful; otherwise, returns 0.
 *---------------------------------------------------------------------------*/
u32 EncodeRC4(
    RC4EncoderContext* context,
    const void* in,
    u32 in_len,
    void* out,
    u32 out_len
)
{
    u8 digest[MATH_SHA1_DIGEST_SIZE];
    u32 iv;
    u8* out_ptr = (u8*)out;

    if ((out_len < in_len)
        ||
        (out_len - in_len < RC4ENC_ADDITIONAL_SIZE))
    {
        // Output buffer is too small
        return 0;
    }
    
    // Create random IV (Initialization Vector)
    GetRandomBytes((u8*)(&iv), sizeof(iv));

    // TODO: Converts Endian to support network byte order
    MI_CpuCopy8(&iv, out_ptr, sizeof(iv));
    out_ptr += sizeof(iv);

    // Create the key to use this time by combining the specified key with the IV.
    MI_CpuCopy8(&iv, &context->key[RC4ENC_USER_KEY_LENGTH], sizeof(iv));
    CRYPTO_RC4Init(&context->rc4_context, context->key, sizeof(context->key));

    // Encrypt input data
    CRYPTO_RC4Encrypt(&context->rc4_context, in, in_len, out_ptr);
    out_ptr += in_len;

    // Calculate the message digest value of input data
    MATH_CalcSHA1(digest, in, in_len);

    // Encrypt digest value
    CRYPTO_RC4Encrypt(&context->rc4_context, digest, MATH_SHA1_DIGEST_SIZE, out_ptr);
    out_ptr += sizeof(digest);

    return (u32)(out_ptr - out);
}

/*---------------------------------------------------------------------------*
  Name:         InitRC4Decoder

  Description:  Performs initialization for decryption using the RC4 algorithm.

  Arguments:    context: Context structure where the RC4 key information, etc., is stored
                key: 12-byte key data

  Returns:      None.
 *---------------------------------------------------------------------------*/
void InitRC4Decoder(
    RC4DecoderContext* context,
    const void* key
)
{
    MI_CpuClear8(context, sizeof(RC4DecoderContext));

    // Key settings
    MI_CpuCopy8(key, context->key, RC4ENC_USER_KEY_LENGTH);
}

/*---------------------------------------------------------------------------*
  Name:         DecodeRC4

  Description:  Performs decryption using the RC4 algorithm.
                Modification check of data is also performed. This function fails if data has been altered.

  Arguments:    context: Context structure where the RC4 key information, etc., is stored
                in: Input data
                in_len: Data length
                out: Output data
                out_len: Size of memory secured as an ouptut buffer

  Returns:      Returns the output length if successful; otherwise, returns 0.
 *---------------------------------------------------------------------------*/
u32 DecodeRC4(
    RC4DecoderContext* context,
    const void* in,
    u32 in_len,
    void* out,
    u32 out_len
)
{
    u8 digest[MATH_SHA1_DIGEST_SIZE];
    u8 decrypted_digest[MATH_SHA1_DIGEST_SIZE];
    u32 iv;
    u8* in_ptr = (u8*)in;
    u32 data_len = in_len - RC4ENC_ADDITIONAL_SIZE;

    if ((in_len < RC4ENC_ADDITIONAL_SIZE)
        ||
        (out_len < data_len))
    {
        // Output buffer is too small
        return 0;
    }
    
    // Get IV
    // TODO: Converts Endian to support network byte order
    MI_CpuCopy8(in_ptr, &iv, sizeof(iv));
    in_ptr += sizeof(iv);

    // Create key to be used this time
    MI_CpuCopy8(&iv, &context->key[RC4ENC_USER_KEY_LENGTH], sizeof(iv));
    CRYPTO_RC4Init(&context->rc4_context, context->key, sizeof(context->key));

    // Decrypt input data
    CRYPTO_RC4Decrypt(&context->rc4_context, in_ptr, data_len, out);
    in_ptr += data_len;

    // Calculate the message digest value of decrypted data
    MATH_CalcSHA1(digest, out, data_len);

    // Decrypt the digest value
    CRYPTO_RC4Decrypt(&context->rc4_context, in_ptr, MATH_SHA1_DIGEST_SIZE, decrypted_digest);
    in_ptr += data_len;

    // Verifies that the digest value is correct
    {
        int i;
        for (i = 0; i < MATH_SHA1_DIGEST_SIZE; i++)
        {
            if (digest[i] != decrypted_digest[i])
            {
                // Verification failed
                return 0;
            }
        }
    }

    return data_len;
}


/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
