/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MATH - libraries
  File:     dgt.h

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

#include <nitro/mi/memory.h>
#include <nitro/math/dgt.h>
#include "hmac.h"

#ifdef  SDK_WIN32
#include <string.h>
#define MI_CpuCopy8(_x_, _y_, _z_)  memcpy(_y_, _x_, _z_)
#define MI_CpuFill8                 memset
#endif

//
// Calculate SHA1 digest
//


/*
 * If giving priority to stack conservation rather than execution speed, make this option valid.
 * Specifically, this affects the following locations.
 *   - Increase/decrease the local variable's 256 bytes used by the MATHi_SHA1ProcessBlock function
 */
#define MATH_SHA1_SMALL_STACK

/*
 * To try using BSAFE SHA1, make this option valid.
 */
//#define MATH_SHA1_BSAFE_TEST

// Internal Function Declarations
#if !defined(MATH_SHA1_ASM)
static void MATHi_SHA1ProcessBlock(MATHSHA1Context *context);
#else
extern void MATHi_SHA1ProcessBlock(MATHSHA1Context *context);
#endif
static void MATHi_SHA1ProcessBlockForOverlay(MATHSHA1Context *context);
static void MATHi_SHA1Fill(MATHSHA1Context* context, u8 input, u32 length);


// Static Variables
static int MATHi_OverlayTableMode = 0;
static void (*MATHi_SHA1ProcessMessageBlockFunc)(MATHSHA1Context*) = MATHi_SHA1ProcessBlock;



// Internal Function Definitions

#if defined(PLATFORM_ENDIAN_LITTLE)
#define NETConvert32HToBE        NETSwapBytes32
#else
#define NETConvert32HToBE(val)   (val)
#endif

inline static u32 NETSwapBytes32( u32 val )
{
    return (u32)( (((val) >> 24UL) & 0x000000FFUL) | 
                  (((val) >>  8UL) & 0x0000FF00UL) | 
                  (((val) <<  8UL) & 0x00FF0000UL) | 
                  (((val) << 24UL) & 0xFF000000UL) );
}

inline static u32 NETRotateLeft32(int shift, u32 value)
{
    return (u32)((value << shift) | (value >> (u32)(32 - shift)));
}

#if !defined(MATH_SHA1_ASM)
/*---------------------------------------------------------------------------*
  Name:         MATHi_SHA1ProcessBlock

  Description:  Calculate hash using 1 block that expired in the SHA-1 context.
  
  Arguments:    context:   MATHSHA1Context structure
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MATHi_SHA1ProcessBlock(MATHSHA1Context *context)
{
    u32     a = context->h[0];
    u32     b = context->h[1];
    u32     c = context->h[2];
    u32     d = context->h[3];
    u32     e = context->h[4];
#if defined(MATH_SHA1_SMALL_STACK)
    u32     w[16];
#define w_alias(t)  w[(t) & 15]
#define w_update(t)                         \
        if (t >= 16)                        \
        {                                   \
            w_alias(t) = NETRotateLeft32(1, \
                w_alias(t - 16 +  0) ^      \
                w_alias(t - 16 +  2) ^      \
                w_alias(t - 16 +  8) ^      \
                w_alias(t - 16 + 13));      \
        }(void)0

#else
    u32     w[80];
#define w_alias(t)  w[t]
#define w_update(t) (void)0
#endif /* defined(MATH_SHA1_SMALL_STACK) */

    int     t;
	u32     tmp;
    for (t = 0; t < 16; ++t)
    {
        w[t] = NETConvert32HToBE(((u32*)context->block)[t]);
    }
#if !defined(MATH_SHA1_SMALL_STACK)
    for (; t < 80; ++t)
    {
        u32    *prev = &w[t - 16];
        w[t] = NETRotateLeft32(1, prev[ 0] ^ prev[ 2] ^ prev[ 8] ^ prev[13]);
    }
#endif /* !defined(MATH_SHA1_SMALL_STACK) */
    for (t = 0; t < 20; ++t)
    {
        tmp = 0x5A827999UL + ((b & c) | (~b & d));
        w_update(t);
        tmp += w_alias(t) + NETRotateLeft32(5, a) + e;
        e = d;
        d = c;
        c = NETRotateLeft32(30, b);
        b = a;
        a = tmp;
    }
    for (; t < 40; ++t)
    {
        tmp = 0x6ED9EBA1UL + (b ^ c ^ d);
        w_update(t);
        tmp += w_alias(t) + NETRotateLeft32(5, a) + e;
        e = d;
        d = c;
        c = NETRotateLeft32(30, b);
        b = a;
        a = tmp;
    }
    for (; t < 60; ++t)
    {
        tmp = 0x8F1BBCDCUL + ((b & c) | (b & d) | (c & d));
        w_update(t);
        tmp += w_alias(t) + NETRotateLeft32(5, a) + e;
        e = d;
        d = c;
        c = NETRotateLeft32(30, b);
        b = a;
        a = tmp;
    }
    for (; t < 80; ++t)
    {
        tmp = 0xCA62C1D6UL + (b ^ c ^ d);
        w_update(t);
        tmp += w_alias(t) + NETRotateLeft32(5, a) + e;
        e = d;
        d = c;
        c = NETRotateLeft32(30, b);
        b = a;
        a = tmp;
    }
    context->h[0] += a;
    context->h[1] += b;
    context->h[2] += c;
    context->h[3] += d;
    context->h[4] += e;
}
#endif /* !defined(MATH_SHA1_ASM) */


// Set the portion calculated from the end to 0 when finding the hash value with the overlay table
// Each entry size of the overlay table is a multiple of 64 so processing here is OK
static void MATHi_SHA1ProcessBlockForOverlay(MATHSHA1Context *context)
{
    u32 s0, s1;
    u32 *block = (u32*)context->block;

    // Save and clear file_id in OverlayTable
    s0 = block[6];      // 6   = location of file_id
    s1 = block[6+8];    // 6+8 = location of next file_id
    block[6]   = 0;
    block[6+8] = 0;

    MATHi_SHA1ProcessBlock(context);

    // Restore file_id
    block[6]   = s0;
    block[6+8] = s1;
}

static void MATHi_SHA1Fill(MATHSHA1Context* context, u8 input, u32 length)
{
    while (length > 0)
    {
        /* Fill data to the block margins */
        u32     rest = MATH_SHA1_BLOCK_SIZE - context->pool;
        if (rest > length)
        {
            rest = length;
        }
        MI_CpuFill8(&context->block[context->pool], input, rest);
        length -= rest;
        context->pool += rest;
        /* Execute the hash calculation if the block has expired. */
        if (context->pool >= MATH_SHA1_BLOCK_SIZE)
        {
            MATHi_SHA1ProcessMessageBlockFunc(context);
            context->pool = 0;
            ++context->blocks_low;
            if (!context->blocks_low)
            {
                ++context->blocks_high;
            }
        }
    }
}

int MATHi_SetOverlayTableMode( int flag )
{
    int prev = MATHi_OverlayTableMode;

    MATHi_OverlayTableMode = flag;

    if (MATHi_OverlayTableMode)
    {
        MATHi_SHA1ProcessMessageBlockFunc = MATHi_SHA1ProcessBlockForOverlay;
    }
    else
    {
        MATHi_SHA1ProcessMessageBlockFunc = MATHi_SHA1ProcessBlock;
    }

    return prev;
}

/*---------------------------------------------------------------------------*
  Name:         MATH_SHA1Init

  Description:  Initializes the MATHSHA1Context structure used for requesting the SHA1 value.
  
  Arguments:    context:   MATHSHA1Context structure
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void MATH_SHA1Init(MATHSHA1Context* context)
{
    context->blocks_low = 0;
    context->blocks_high = 0;
    context->pool = 0;
    context->h[0] = 0x67452301;
    context->h[1] = 0xEFCDAB89;
    context->h[2] = 0x98BADCFE;
    context->h[3] = 0x10325476;
    context->h[4] = 0xC3D2E1F0;
}

/*---------------------------------------------------------------------------*
  Name:         MATH_SHA1Update

  Description:  Updates the SHA1 value with given data.
  
  Arguments:    context:   MATHSHA1Context structure
                input:   Pointer to input data.
                length:  Length of input data.
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void MATH_SHA1Update(MATHSHA1Context* context, const void* input, u32 length)
{
    while (length > 0)
    {
        /* Fill data to the block margins */
        u32     rest = MATH_SHA1_BLOCK_SIZE - context->pool;
        if (rest > length)
        {
            rest = length;
        }
        MI_CpuCopy8(input, &context->block[context->pool], rest);
        input = (const u8 *)input + rest;
        length -= rest;
        context->pool += rest;
        /* Execute the hash calculation if the block has expired. */
        if (context->pool >= MATH_SHA1_BLOCK_SIZE)
        {
            MATHi_SHA1ProcessMessageBlockFunc(context);
            context->pool = 0;
            ++context->blocks_low;
            if (!context->blocks_low)
            {
                ++context->blocks_high;
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         MATH_SHA1GetHash

  Description:  Gets the final SHA1 value.
  
  Arguments:    context:   MATHSHA1Context structure
                digest:   Pointer to the location where the SHA1 value is stored.
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void MATH_SHA1GetHash(MATHSHA1Context *context, void *digest)
{
    u32    footer[2];
    static const u8 padlead[1] = { 0x80 };
    static const u8 padalign[sizeof(footer)] = { 0x00, };
    /* Set up footer field (in bits, big endian)*/
    footer[1] = NETConvert32HToBE((u32)
        (context->blocks_low << (6 + 3)) + (context->pool << (0 + 3)));
    footer[0] = NETConvert32HToBE((u32)
        (context->blocks_high << (6 + 3)) + (context->blocks_low >> (u32)(32 - (6 + 3))));
    /* Add leading padbyte '0x80'*/
    MATH_SHA1Update(context, padlead, sizeof(padlead));
    /* If necessary, add 2 padblocks*/
    if (MATH_SHA1_BLOCK_SIZE - context->pool < sizeof(footer))
    {
        MATH_SHA1Update(context, padalign, MATH_SHA1_BLOCK_SIZE - context->pool);
    }
    /* Add trailing padbytes '0x00'*/
    MATHi_SHA1Fill(context, 0x00, MATH_SHA1_BLOCK_SIZE - context->pool - sizeof(footer));
    /* Add footer length*/
    MATH_SHA1Update(context, footer, sizeof(footer));
    /* Copy registers to the dst*/
    context->h[0] = NETConvert32HToBE((u32)context->h[0]);
    context->h[1] = NETConvert32HToBE((u32)context->h[1]);
    context->h[2] = NETConvert32HToBE((u32)context->h[2]);
    context->h[3] = NETConvert32HToBE((u32)context->h[3]);
    context->h[4] = NETConvert32HToBE((u32)context->h[4]);
    MI_CpuCopy8(context->h, digest, sizeof(context->h));
}

#if defined(MATH_SHA1_BSAFE_TEST)
extern unsigned char *SHA1(const unsigned char *d, unsigned long n, unsigned char *md);
#endif
/*---------------------------------------------------------------------------*
  Name:         MATH_CalcSHA1

  Description:  Calculates SHA-1.
  
  Arguments:    digest:   Pointer to the location where SHA-1 is stored.
                data:    Pointer to input data.
                dataLength:   Length of input data.
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void MATH_CalcSHA1(void *digest, const void *data, u32 dataLength)
{
#if !defined(MATH_SHA1_BSAFE_TEST)
    MATHSHA1Context context;
    MATH_SHA1Init(&context);
    MATH_SHA1Update(&context, data, dataLength);
    MATH_SHA1GetHash(&context, digest);
#else
	SHA1((unsigned char*)data, dataLength, (unsigned char*)digest);
#endif
}



// HMAC

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
void MATH_CalcHMACSHA1(void *digest, const void *bin_ptr, u32 bin_len, const void *key_ptr, u32 key_len) 
{
    MATHSHA1Context context;
    unsigned char   hash_buf[ MATH_SHA1_DIGEST_SIZE ]; /* Hash value gotten from the hash function */
    
    MATHiHMACFuncs hash2funcs = {
        MATH_SHA1_DIGEST_SIZE,
        (512/8),
    };
    
    hash2funcs.context       = &context;
    hash2funcs.hash_buf      = hash_buf;
    hash2funcs.HashReset     = (void (*)(void*))                   MATH_SHA1Init;
    hash2funcs.HashSetSource = (void (*)(void*, const void*, u32)) MATH_SHA1Update;
    hash2funcs.HashGetDigest = (void (*)(void*, void*))            MATH_SHA1GetHash;
    
    MATHi_CalcHMAC(digest, bin_ptr, bin_len, key_ptr, key_len, &hash2funcs);
}
