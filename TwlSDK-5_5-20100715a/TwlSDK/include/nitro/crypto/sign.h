/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include
  File:     crypto/sign.h

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_CRYPTO_SIGN_H_
#define NITRO_CRYPTO_SIGN_H_

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
  Name:         CRYPTO_VerifySignatureWithHash

  Description:  verifies the electronic signature from the hash value

  Arguments:    hash_ptr - pointer to the hash value
                sign_ptr - pointer to the electronic signature
                mod_ptr - pointer to the public key
  Returns:      TRUE if verified
                FALSE if unable to verify
 *---------------------------------------------------------------------------*/
int CRYPTO_VerifySignatureWithHash(
    const void* hash_ptr,
    const void* sign_ptr,
    const void* mod_ptr
);

/*---------------------------------------------------------------------------*
  Name:         CRYPTO_VerifySignature

  Description:  verifies the electronic signature from the data

  Arguments:    data_ptr - pointer to the data
                data_len - Data size
                sign_ptr - pointer to the electronic signature
                mod_ptr - pointer to the public key

  Returns:      TRUE if verified
                FALSE if unable to verify
 *---------------------------------------------------------------------------*/
int CRYPTO_VerifySignature(
    const void* data_ptr,
    int   data_len,
    const void* sign_ptr,
    const void* mod_ptr
);


/*---------------------------------------------------------------------------*
  Name:         CRYPTO_SIGN_GetModulus

  Description:  Returns a pointer to the public key's modulus.
                (The address of the returned pointer is passed to the mod_ptr argument of the following functions.
                   CRYPTO_VerifySignature, CRYPTO_VerifySignatureWithHash )

  Arguments:    pub_ptr - Pointer to the public key
  Returns:      Returns the address of the modulus on success.
                Returns NULL on failure.
 *---------------------------------------------------------------------------*/
void *CRYPTO_SIGN_GetModulus(
	const void* pub_ptr
);


#ifdef __cplusplus
}
#endif

#endif //_NITRO_CRYPTO_SIGN_H_
