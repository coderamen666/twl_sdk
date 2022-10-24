/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MATH - include
  File:     math/dgt.h

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev:$
  $Author:$
 *---------------------------------------------------------------------------*/

#ifndef NITRO_MATH_DGT_H_
#define NITRO_MATH_DGT_H_

#ifndef SDK_WIN32
#include <nitro/misc.h>
#endif
#include <nitro/types.h>

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------
// Constant Definitions
//----------------------------------------------------------------------------

// Digest length
#define MATH_MD5_DIGEST_SIZE    (128/8)     // 128-bit
#define MATH_SHA1_DIGEST_SIZE   (160/8)     // 160-bit
#define MATH_SHA256_DIGEST_SIZE 32          // 256-bit

// Maximum digest length
#define MATH_HASH_DIGEST_SIZE_MAX  MATH_SHA1_DIGEST_SIZE

// Process block length
#define MATH_HASH_BLOCK_SIZE    (512/8)
#define MATH_MD5_BLOCK_SIZE     MATH_HASH_BLOCK_SIZE     // 512-bit
#define MATH_SHA1_BLOCK_SIZE    MATH_HASH_BLOCK_SIZE     // 512-bit

//----------------------------------------------------------------------------
// Type Definitions
//----------------------------------------------------------------------------

typedef struct MATHMD5Context
{
    union
    {
        struct
        {
            unsigned long a, b, c, d;
        };
        unsigned long state[4];
    };
    unsigned long long length;
    union
    {
        unsigned long buffer32[16];
        unsigned char buffer8[64];
    };
} MATHMD5Context;

typedef struct MATHSHA1Context
{
    u32     h[5];                        /* H0,H1,H2,H3,H4 */
    u8      block[MATH_SHA1_BLOCK_SIZE]; /* current message block */
    u32     pool;                        /* message length in 'block' */
    u32     blocks_low;                  /* total blocks (in bytes) */
    u32     blocks_high;
}
MATHSHA1Context;

//----------------------------------------------------------------------------
// Function Declarations
//----------------------------------------------------------------------------

/*****************************************************************************/
/* MD5                                                                       */
/*****************************************************************************/

/*---------------------------------------------------------------------------*
  Name:         MATH_MD5Init

  Description:  Initializes the MATHMD5Context structure used for requesting the MD5 value.
  
  Arguments:    context:   MATHMD5Context structure
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void MATH_MD5Init(MATHMD5Context * context);

/*---------------------------------------------------------------------------*
  Name:         MATH_MD5Update

  Description:  Updates the MD5 value with given data.
  
  Arguments:    context:   MATHMD5Context structure
                input:   Pointer to input data.
                length:  Length of input data.
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void MATH_MD5Update(MATHMD5Context * context, const void *input, u32 length);

/*---------------------------------------------------------------------------*
  Name:         MATH_MD5GetHash

  Description:  Gets the final MD5 value.
  
  Arguments:    context:   MATHMD5Context structure
                digest:   Pointer to the location where MD5 is stored.
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void MATH_MD5GetHash(MATHMD5Context * context, void *digest);

// For forward compatibility
static inline void MATH_MD5GetDigest(MATHMD5Context * context, void *digest)
{
    MATH_MD5GetHash(context, digest);
}


/*****************************************************************************/
/* SHA-1                                                                     */
/*****************************************************************************/

/*---------------------------------------------------------------------------*
  Name:         MATH_SHA1Init

  Description:  Initializes the MATHSHA1Context structure used for requesting the SHA1 value.
  
  Arguments:    context:   MATHSHA1Context structure
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void MATH_SHA1Init(MATHSHA1Context * context);

/*---------------------------------------------------------------------------*
  Name:         MATH_SHA1Update

  Description:  Updates the SHA1 value with given data.
  
  Arguments:    context:   MATHSHA1Context structure
                input:   Pointer to input data.
                length:  Length of input data.
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void MATH_SHA1Update(MATHSHA1Context * context, const void *input, u32 length);

/*---------------------------------------------------------------------------*
  Name:         MATH_SHA1GetHash

  Description:  Gets the final SHA1 value.
  
  Arguments:    context:   MATHSHA1Context structure
                digest:   Pointer to the location where the SHA1 value is stored.
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void MATH_SHA1GetHash(MATHSHA1Context * context, void *digest);

// For forward compatibility
static inline void MATH_SHA1GetDigest(MATHSHA1Context * context, void *digest)
{
    MATH_SHA1GetHash(context, digest);
}


/*****************************************************************************/
/* SHA256                                                                     */
/*****************************************************************************/
#define MATHSHA256_CBLOCK	64
#define MATHSHA256_LBLOCK	16
#define MATHSHA256_BLOCK	16
#define MATHSHA256_LAST_BLOCK  56
#define MATHSHA256_LENGTH_BLOCK 8
#define MATHSHA256_DIGEST_LENGTH 32

typedef struct MATHSHA256Context MATHSHA256Context;
typedef void (MATHSHA256_BLOCK_FUNC)(MATHSHA256Context *c, u32 *W, int num); 

struct MATHSHA256Context
{
    u32 h[8];
    u32 Nl,Nh;
    u8 data[MATHSHA256_CBLOCK];
    int num;
};

void MATH_SHA256Init(MATHSHA256Context *c);
void MATH_SHA256Update(MATHSHA256Context *c, const void* data, u32 len);
void MATH_SHA256GetHash(MATHSHA256Context *c, void* digest);
void MATH_CalcSHA256(void* digest, const void* data, u32 dataLength);


/*****************************************************************************/
/* Utility Functions                                                        */
/*****************************************************************************/

/*---------------------------------------------------------------------------*
  Name:         MATH_CalcMD5

  Description:  Calculates MD5.
  
  Arguments:    digest:   Pointer to the location where MD5 is stored.
                data:    Pointer to input data.
                dataLength:   Length of input data.
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void    MATH_CalcMD5(void *digest, const void *data, u32 dataLength);

/*---------------------------------------------------------------------------*
  Name:         MATH_CalcSHA1

  Description:  Calculates SHA-1.
  
  Arguments:    digest:   Pointer to the location where SHA-1 is stored.
                data:    Pointer to input data.
                dataLength:   Length of input data.
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void    MATH_CalcSHA1(void *digest, const void *data, u32 dataLength);


/*---------------------------------------------------------------------------*
  Name:         MATH_CalcHMACMD5

  Description:  Calculates HMAC-MD5.
  
  Arguments:    digest:   Pointer to the location where HMAC-MD5 is stored.
                data:    Pointer to input data.
                dataLength:   Length of input data.
                key:    Pointer to the key
                keyLength:   Length of the key
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void
MATH_CalcHMACMD5(void *digest, const void *data, u32 dataLength, const void *key, u32 keyLength);

/*---------------------------------------------------------------------------*
  Name:         MATH_CalcHMACSHA1

  Description:  Calculates HMAC-SHA-1.
  
  Arguments:    digest:   Pointer to the location where the HMAC-SHA-1 value is stored.
                data:    Pointer to input data.
                dataLength:   Length of input data.
                key:    Pointer to the key
                keyLength:   Length of the key
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void
MATH_CalcHMACSHA1(void *digest, const void *data, u32 dataLength, const void *key, u32 keyLength);

/*---------------------------------------------------------------------------*
  Name:         MATH_CalcHMACSHA256

  Description:  Calculates HMAC-SHA-256.
  
  Arguments:    digest:   Pointer to the location where the HMAC-SHA-256 value is stored.
                data:    Pointer to input data.
                dataLength:   Length of input data.
                key:    Pointer to the key
                keyLength:   Length of the key
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void
MATH_CalcHMACSHA256(void *digest, const void *data, u32 dataLength, const void *key, u32 keyLength);

int MATHi_SetOverlayTableMode( int flag );

#ifdef __cplusplus
}/* extern "C" */
#endif

/* NITRO_MATH_DGT_H_ */
#endif
