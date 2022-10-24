/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include
  File:     crypto/rc4.h

  Copyright 2005-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_CRYPTO_RC4_H_
#define NITRO_CRYPTO_RC4_H_

#ifdef __cplusplus
extern "C" {
#endif


#include <nitro/types.h>


typedef struct CRYPTORC4Context
{
    u8 i, j;
    u8 padd[2];
    u8 s[256];
} CRYPTORC4Context;

/*---------------------------------------------------------------------------*
  Name:         CRYPTO_RC4Init

  Description:  Performs initializations for encryption/decryption using the RC4 algorithm

  Arguments:    context - the context structure where the RC4 key information, etc., is kept
                key - the key data
                key_len - the key length

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CRYPTO_RC4Init(
    CRYPTORC4Context* context,
    const void* key,
    u32 key_len
);

/*---------------------------------------------------------------------------*
  Name:         CRYPTO_RC4Encrypt

  Description:  Performs encryption/decryption with the RC4 algorithm

  Arguments:    context - the context structure where the RC4 key information, etc., is to be kept
                in      - input data
                length  - data length
                out     - output data

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CRYPTO_RC4Encrypt(
    CRYPTORC4Context* context,
    const void* in,
    u32 length,
    void* out
);

/*---------------------------------------------------------------------------*
  Name:         CRYPTO_RC4Decrypt

  Description:  Performs encryption/decryption with the RC4 algorithm

  Arguments:    context - the context structure where the RC4 key information, etc., is to be kept
                in      - input data
                length  - data length
                out     - output data

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void CRYPTO_RC4Decrypt(
    CRYPTORC4Context* context,
    const void* in,
    u32 length,
    void* out
)
{
    CRYPTO_RC4Encrypt(context, in, length, out);
}

/*---------------------------------------------------------------------------*
  Name:         CRYPTO_RC4

  Description:  Performs encryption/decryption with the RC4 algorithm

  Arguments:    key - the key data
                key_len - the key data length
                data - the data to be converted (overwritten)
                data_len: Data length

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void CRYPTO_RC4(
    const void* key,
    u32 key_len,
    void* data,
    u32 data_len
)
{
    CRYPTORC4Context context;
    CRYPTO_RC4Init(&context, key, key_len);
    CRYPTO_RC4Encrypt(&context, data, data_len, data);
}


///////////////////////////////////////////////////////////////////////////////
// RC4 Fast
///////////////////////////////////////////////////////////////////////////////

// CRYPTORC4FastContext uses roughly 4 times as much memory as CRYPTORC4Context
typedef struct CRYPTORC4FastContext
{
    u32 i, j;
    u32 s[256];
} CRYPTORC4FastContext;

/*---------------------------------------------------------------------------*
  Name:         CRYPTO_RC4FastInit

  Description:  Performs initializations for encryption/decryption using the RC4 algorithm

  Arguments:    context - the context structure where the RC4 key information, etc., is kept
                key - the key data
                key_len - the key length

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CRYPTO_RC4FastInit(
    CRYPTORC4FastContext* context,
    const void* key,
    u32 key_len
);

/*---------------------------------------------------------------------------*
  Name:         CRYPTO_RC4FastEncrypt

  Description:  Performs encryption/decryption with the RC4Fast algorithm
                CRYPTO_RC4FastEncrypt uses roughly four times as much memory and processes at roughly 1.5 times the speed as CRYPTO_RC4Crypt

  Arguments:    context - the context structure where the RC4 key information, etc., is to be kept
                in      - input data
                length  - data length
                out     - output data

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CRYPTO_RC4FastEncrypt(
    CRYPTORC4FastContext* context,
    const void* in,
    u32 length,
    void* out
);

/*---------------------------------------------------------------------------*
  Name:         CRYPTO_RC4FastDecrypt

  Description:  Performs encryption/decryption with the RC4Fast algorithm
                CRYPTO_RC4FastDecrypt uses roughly four times as much memory and processes at roughly 1.5 times the speed as CRYPTO_RC4Decrypt.

  Arguments:    context - the context structure where the RC4 key information, etc., is to be kept
                in      - input data
                length  - data length
                out     - output data

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void CRYPTO_RC4FastDecrypt(
    CRYPTORC4FastContext* context,
    const void* in,
    u32 length,
    void* out
)
{
    CRYPTO_RC4FastEncrypt(context, in, length, out);
}

/*---------------------------------------------------------------------------*
  Name:         CRYPTO_RC4Fast

  Description:  Performs encryption/decryption with the RC4Fast algorithm
                CRYPTO_RC4Fast uses roughly four times as much memory and processes at roughly 1.5 times the speed as CRYPTO_RC4.

  Arguments:    key - the key data
                key_len - the key data length
                data - the data to be converted (overwritten)
                data_len: Data length

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void CRYPTO_RC4Fast(
    const void* key,
    u32 key_len,
    void* data,
    u32 data_len
)
{
    CRYPTORC4FastContext context;
    CRYPTO_RC4FastInit(&context, key, key_len);
    CRYPTO_RC4FastEncrypt(&context, data, data_len, data);
}

#ifdef __cplusplus
}
#endif

#endif //_NITRO_CRYPTO_RC4_H_
