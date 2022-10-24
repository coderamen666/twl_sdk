/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include
  File:     crypto/rsa.h

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::$
  $Rev:$
  $Author:$
 *---------------------------------------------------------------------------*/

#ifndef NITRO_CRYPTO_RSA_H_
#define NITRO_CRYPTO_RSA_H_

#ifdef __cplusplus
extern "C" {
#endif


#define CRYPTO_RSA_VERIFY    // Define this to enable RSA signature verification


/*---------------------------------------------------------------------------*
    Constant Definitions
 *---------------------------------------------------------------------------*/

#define CRYPTO_RSA_CONTEXT_SIZE         (4 * 1)
#define CRYPTO_RSA_SIGN_CONTEXT_SIZE    (4 * 1)



/*---------------------------------------------------------------------------*
    Type Definitions
 *---------------------------------------------------------------------------*/

// RSA processing context
//   The size must match CRYPTORSAContext_local.
//   This must reflect any added or removed members (CRYPTO_RSA_CONTEXT_SIZE).
typedef struct CRYPTORSAContext
{
/* private: */
    u8      mem[CRYPTO_RSA_CONTEXT_SIZE];
}
CRYPTORSAContext;

// I/O stream parameters for CRYPTO_RSA_EncryptInit
typedef struct CRYPTORSAEncryptInitParam
{
    void    *key;       // [in] Public key string
    u32     key_len;    // [in] Public key string length
}
CRYPTORSAEncryptInitParam;

// I/O stream parameters for CRYPTO_RSA_EncryptInit_PrivateKey
typedef struct CRYPTORSAEncryptInitPKParam
{
    void    *privkey;       // [in] Private key string
    u32     privkey_len;    // [in] Private key string length
}
CRYPTORSAEncryptInitPKParam;

// I/O stream parameters for CRYPTO_RSA_Encrypt
typedef struct CRYPTORSAEncryptParam
{
    void    *in;        // [in] Encryption string
    u32     in_len;     // [in] Encryption string length
    void    *out;       // [out] Output string buffer
    u32     out_size;   // [in] Output string buffer size
}
CRYPTORSAEncryptParam;

// I/O stream parameters for CRYPTO_RSA_DecryptInit
typedef struct CRYPTORSADecryptInitParam
{
    void    *key;       // [in] Public key string
    u32     key_len;    // [in] Public key string length
}
CRYPTORSADecryptInitParam;

// I/O stream parameters for CRYPTO_RSA_Decrypt
typedef struct CRYPTORSADecryptParam
{
    void    *in;        // [in] Decryption string
    u32     in_len;     // [in] Decryption string length
    void    *out;       // [out] Output string buffer
    u32     out_size;   // [in] Output string buffer size
}
CRYPTORSADecryptParam;

// Processing context related to digital RSA signatures
//   The size must match CRYPTORSASignContext_local.
//   This must reflect any added or removed members (CRYPTO_RSA_SIGN_CONTEXT_SIZE).
typedef struct CRYPTORSASignContext
{
/* private: */
    u8      mem[CRYPTO_RSA_SIGN_CONTEXT_SIZE];
}
CRYPTORSASignContext;

// I/O stream parameters for CRYPTO_RSA_SignInit
typedef struct CRYPTORSASignInitParam
{
    void    *key;       // [in] Private key string
    u32     key_len;    // [in] Private key string length
}
CRYPTORSASignInitParam;

// I/O stream parameters for CRYPTO_RSA_Sign
typedef struct CRYPTORSASignParam
{
    void    *in;        // [in] Target signature string
    u32     in_len;     // [in] Target signature string length
    void    *out;       // [out] Buffer for the output signature string
    u32     out_size;   // [in] Buffer size for the output signature string
}
CRYPTORSASignParam;

#if defined(CRYPTO_RSA_VERIFY)
// I/O stream parameters for CRYPTO_RSA_VerifyInt
typedef struct CRYPTORSAVerifyInitParam
{
    void    *key;       // [in] Public key string
    u32     key_len;    // [in] Public key string length
}
CRYPTORSAVerifyInitParam;

// I/O stream parameters for CRYPTO_RSA_Verify
typedef struct CRYPTORSAVerifyParam
{
    void    *in;        // [in] String to verify
    u32     in_len;     // [in] String length to verify
    void    *sign;      // [in] Signature string
    u32     sign_len;   // [in] Signature string length
}
CRYPTORSAVerifyParam;
#endif


/*---------------------------------------------------------------------------*
    Constant Structure Declarations
 *---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*
    Function Declarations
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         CRYPTO_RSA_EncryptInit

  Description:  Runs initialization processing for RSA encryption.

  Arguments:    context:    Library context
                param:      I/O stream parameters

  Returns:      0 on success and a non-zero value on failure
 *---------------------------------------------------------------------------*/
s32 CRYPTO_RSA_EncryptInit(CRYPTORSAContext *context, CRYPTORSAEncryptInitParam *param);

/*---------------------------------------------------------------------------*
  Name:         CRYPTO_RSA_EncryptInit_PrivateKey

  Description:  Runs initialization processing (using private keys) for RSA encryption.

  Arguments:    context:    Library context
                param:      I/O stream parameters

  Returns:      0 on success and a non-zero value on failure
 *---------------------------------------------------------------------------*/
s32 CRYPTO_RSA_EncryptInit_PrivateKey(CRYPTORSAContext *context, CRYPTORSAEncryptInitPKParam *param);

/*---------------------------------------------------------------------------*
  Name:         CRYPTO_RSA_Encrypt

  Description:  Runs RSA encryption.

  Arguments:    context:    Library context
                param:      I/O stream parameters

  Returns:      A positive value indicates the string length and -1 indicates failure.
 *---------------------------------------------------------------------------*/
s32 CRYPTO_RSA_Encrypt(CRYPTORSAContext *context, CRYPTORSAEncryptParam *param);

/*---------------------------------------------------------------------------*
  Name:         CRYPTO_RSA_EncryptTerminate

  Description:  Runs shutdown processing for RSA encryption.

  Arguments:    context:    Library context

  Returns:      0 on success and a non-zero value on failure
 *---------------------------------------------------------------------------*/
s32 CRYPTO_RSA_EncryptTerminate(CRYPTORSAContext *context);

/*---------------------------------------------------------------------------*
  Name:         CRYPTO_RSA_DecryptInit

  Description:  Runs initialization processing for RSA decryption.

  Arguments:    context:    Library context
                param:      I/O stream parameters

  Returns:      0 on success and a non-zero value on failure
 *---------------------------------------------------------------------------*/
s32 CRYPTO_RSA_DecryptInit(CRYPTORSAContext *context, CRYPTORSADecryptInitParam *param);

/*---------------------------------------------------------------------------*
  Name:         CRYPTO_RSA_Decrypt

  Description:  Runs RSA decryption.

  Arguments:    context:    Library context
                param:      I/O stream parameters

  Returns:      A positive value indicates the string length and -1 indicates failure.
 *---------------------------------------------------------------------------*/
s32 CRYPTO_RSA_Decrypt(CRYPTORSAContext *context, CRYPTORSADecryptParam *param);

/*---------------------------------------------------------------------------*
  Name:         CRYPTO_RSA_DecryptTerminate

  Description:  Runs shutdown processing for RSA decryption.

  Arguments:    context:    Library context

  Returns:      0 on success and a non-zero value on failure
 *---------------------------------------------------------------------------*/
s32 CRYPTO_RSA_DecryptTerminate(CRYPTORSAContext *context);


/*---------------------------------------------------------------------------*
  Name:         CRYPTO_RSA_SignInit

  Description:  Initializes RSA signature processing.

  Arguments:    context:    Library context
                param:      I/O stream parameters

  Returns:      0 on success and a non-zero value on failure
 *---------------------------------------------------------------------------*/
s32 CRYPTO_RSA_SignInit(CRYPTORSASignContext *context, CRYPTORSASignInitParam *param);

/*---------------------------------------------------------------------------*
  Name:         CRYPTO_RSA_Sign

  Description:  Generates an RSA signature.

  Arguments:    context:    Library context
                param:      I/O stream parameters

  Returns:      A positive value indicates the string length of the generated signature and -1 indicates failure.
 *---------------------------------------------------------------------------*/
s32 CRYPTO_RSA_Sign(CRYPTORSASignContext *context, CRYPTORSASignParam *param);

/*---------------------------------------------------------------------------*
  Name:         CRYPTO_RSA_SignTerminate

  Description:  Shuts down RSA signature processing.

  Arguments:    context:    Library context

  Returns:      0 on success and a non-zero value on failure
 *---------------------------------------------------------------------------*/
s32 CRYPTO_RSA_SignTerminate(CRYPTORSASignContext *context);


#if defined(CRYPTO_RSA_VERIFY)
/*---------------------------------------------------------------------------*
  Name:         CRYPTO_RSA_VerifyInit

  Description:  Runs initialization processing for RSA signature verification.

  Arguments:    context:    Library context
                param:      I/O stream parameters

  Returns:      0 on success and a non-zero value on failure
 *---------------------------------------------------------------------------*/
s32 CRYPTO_RSA_VerifyInit(CRYPTORSASignContext *context, CRYPTORSAVerifyInitParam *param);

/*---------------------------------------------------------------------------*
  Name:         CRYPTO_RSA_Verify

  Description:  Verifies an RSA signature.

  Arguments:    context:    Library context
                param:      I/O stream parameters

  Returns:      0 on success and a non-zero value on failure
 *---------------------------------------------------------------------------*/
s32 CRYPTO_RSA_Verify(CRYPTORSASignContext *context, CRYPTORSAVerifyParam *param);

/*---------------------------------------------------------------------------*
  Name:         CRYPTO_RSA_VerifyTerminate

  Description:  Runs shutdown processing for RSA signature verification.

  Arguments:    context:    Library context

  Returns:      0 on success and a non-zero value on failure
 *---------------------------------------------------------------------------*/
s32 CRYPTO_RSA_VerifyTerminate(CRYPTORSASignContext *context);
#endif


/* for internal use */



#ifdef __cplusplus
}
#endif

#endif //NITRO_CRYPTO_RSA_H_
