/*---------------------------------------------------------------------------*
  Project:  NitroSDK - CRYPTO - demos
  File:     rc4enc.h

  Copyright 2006 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: rc4enc.h,v $
  Revision 1.3  2006/03/08 09:15:35  seiki_masashi
  Revised comments

  Revision 1.2  2006/03/08 09:14:44  seiki_masashi
  Revised comments

  Revision 1.1  2006/03/08 08:53:41  seiki_masashi
  Added rc4-3 demo

  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef RC4ENC_H_
#define RC4ENC_H_

#ifdef  __cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include    <nitro/types.h>

#define RC4ENC_USER_KEY_LENGTH  12 // Key length specified by user
#define RC4ENC_TOTAL_KEY_LENGTH 16 // Final key length

// Size required when adding during encoding (24 bytes)
#define RC4ENC_ADDITIONAL_SIZE (sizeof(u32) /* iv */ + MATH_SHA1_DIGEST_SIZE /* message digest */)

/*---------------------------------------------------------------------------*
    Structure Definitions
 *---------------------------------------------------------------------------*/
typedef struct RC4EncoderContext
{
    CRYPTORC4Context rc4_context;
    u8 key[RC4ENC_TOTAL_KEY_LENGTH];
} RC4EncoderContext;

typedef struct RC4DecoderContext
{
    CRYPTORC4Context rc4_context;
    u8 key[RC4ENC_TOTAL_KEY_LENGTH];
} RC4DecoderContext;

/*---------------------------------------------------------------------------*
    function definitions
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
  Name:         InitRC4Encoder

  Description:  This function performs initialization for encryption using the RC4 algorithm.

  Arguments:    context - the context structure where the RC4 key information, etc., is kept
                key     - 12-byte key data

  Returns:      None.
 *---------------------------------------------------------------------------*/
void InitRC4Encoder(
    RC4EncoderContext* context,
    const void* key
);

/*---------------------------------------------------------------------------*
  Name:         EncodeRC4

  Description:  Performs encryption using the RC4 algorithm.

  Arguments:    context - the context structure where the RC4 key information, etc., is to be kept
                in      - input data
                in_len  - Data length
                out     - output data
                out_len - Size of memory secured as an ouptut buffer

  Returns:      Returns the output length if successful; otherwise, returns 0.
 *---------------------------------------------------------------------------*/
u32 EncodeRC4(
    RC4EncoderContext* context,
    const void* in,
    u32 in_len,
    void* out,
    u32 out_len
);

/*---------------------------------------------------------------------------*
  Name:         InitRC4Decoder

  Description:  Performs initialization for decryption using the RC4 algorithm.

  Arguments:    context - the context structure where the RC4 key information, etc., is kept
                key     - 12-byte key data

  Returns:      None.
 *---------------------------------------------------------------------------*/
void InitRC4Decoder(
    RC4DecoderContext* context,
    const void* key
);

/*---------------------------------------------------------------------------*
  Name:         DecodeRC4

  Description:  Performs decryption using the RC4 algorithm.
                Modification check of data is also performed. Fails if data has been altered.

  Arguments:    context - the context structure where the RC4 key information, etc., is to be kept
                in      - input data
                in_len  - Data length
                out     - output data
                out_len - Size of memory allocated as an ouptut buffer

  Returns:      Returns the output length if successful; otherwise, returns 0.
 *---------------------------------------------------------------------------*/
u32 DecodeRC4(
    RC4DecoderContext* context,
    const void* in,
    u32 in_len,
    void* out,
    u32 out_len
);


/*===========================================================================*/

#ifdef  __cplusplus
}       /* extern "C" */
#endif

#endif  /* RC4ENC_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
