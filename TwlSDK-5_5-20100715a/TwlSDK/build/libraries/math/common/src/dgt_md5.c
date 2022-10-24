/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MATH - libraries
  File:     dgt_md5.c

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


#include <nitro/types.h>
#include <nitro/mi/memory.h>
#include <nitro/math/dgt.h>
#include "hmac.h"

#ifdef  SDK_WIN32
#include <string.h>
#define MI_CpuCopy8(_x_, _y_, _z_)  memcpy(_y_, _x_, _z_)
#define MI_CpuFill8                 memset
#endif

/*  The ARM7 is located on LTDWRAM for the TWL SDK's WPA */
#ifdef SDK_TWLHYB
#ifdef SDK_ARM7
#include <twl/ltdwram_begin.h>
#endif
#endif

//
// Calculate MD5 digest
//

#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))
#define ROTL(x, n) (((x) << (n)) | ((x) >> (32-(n))))

static void ProcessBlock(MATHMD5Context* context);

inline static u32 CalcRound1(u32 a, u32 b, u32 c, u32 d, u32 x, u32 s, u32 t)
{
    return b + ROTL((a + F(b,c,d) + x + t), s);
}

inline static u32 CalcRound2(u32 a, u32 b, u32 c, u32 d, u32 x, u32 s, u32 t)
{
    return b + ROTL((a + G(b,c,d) + x + t), s);
}

inline static u32 CalcRound3(u32 a, u32 b, u32 c, u32 d, u32 x, u32 s, u32 t)
{
    return b + ROTL((a + H(b,c,d) + x + t), s);
}

inline static u32 CalcRound4(u32 a, u32 b, u32 c, u32 d, u32 x, u32 s, u32 t)
{
    return b + ROTL((a + I(b,c,d) + x + t), s);
}

static void ProcessBlock(MATHMD5Context* context)
{
    static u32 t[] =
    {
        0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
        0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
        0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
        0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
#ifndef SDK_DIGEST_MD5_FAST
        0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
        0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
        0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
        0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
        0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
        0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
        0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
        0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
        0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
        0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
        0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
        0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391,
#else
     1, 0xf61e2562,
     6, 0xc040b340,
    11, 0x265e5a51,
     0, 0xe9b6c7aa,
     5, 0xd62f105d,
    10,  0x2441453,
    15, 0xd8a1e681,
     4, 0xe7d3fbc8,
     9, 0x21e1cde6,
    14, 0xc33707d6,
     3, 0xf4d50d87,
     8, 0x455a14ed,
    13, 0xa9e3e905,
     2, 0xfcefa3f8,
     7, 0x676f02d9,
    12, 0x8d2a4c8a,
     5, 0xfffa3942,
     8, 0x8771f681,
    11, 0x6d9d6122,
    14, 0xfde5380c,
     1, 0xa4beea44,
     4, 0x4bdecfa9,
     7, 0xf6bb4b60,
    10, 0xbebfbc70,
    13, 0x289b7ec6,
     0, 0xeaa127fa,
     3, 0xd4ef3085,
     6,  0x4881d05,
     9, 0xd9d4d039,
    12, 0xe6db99e5,
    15, 0x1fa27cf8,
     2, 0xc4ac5665,
     0, 0xf4292244,
     7, 0x432aff97,
    14, 0xab9423a7,
     5, 0xfc93a039,
    12, 0x655b59c3,
     3, 0x8f0ccc92,
    10, 0xffeff47d,
     1, 0x85845dd1,
     8, 0x6fa87e4f,
    15, 0xfe2ce6e0,
     6, 0xa3014314,
    13, 0x4e0811a1,
     4, 0xf7537e82,
    11, 0xbd3af235,
     2, 0x2ad7d2bb,
     9, 0xeb86d391,
#endif
    };

#ifndef SDK_DIGEST_MD5_FAST
    static u32 k[] =
    {
         1,  6, 11,  0,
         5, 10, 15,  4,
         9, 14,  3,  8,
        13,  2,  7, 12,
         5,  8, 11, 14,
         1,  4,  7, 10,
        13,  0,  3,  6,
         9, 12, 15,  2,
         0,  7, 14,  5,
        12,  3, 10,  1,
         8, 15,  6, 13,
         4, 11,  2,  9,
    };
    u32 *kp;
#endif

    u32 a, b, c, d;
    u32 *x;
    u32 *xp, *tp;
    int j;
    
    a = context->a;
    b = context->b;
    c = context->c;
    d = context->d;

#ifdef SDK_LITTLE_ENDIAN
    x = context->buffer32;
#else
#error "Big Endian is not supported."
#endif

    xp = x;
    tp = t;

    // /* Round 1. */
    // /* Let [abcd k s i] denote the operation
    //      a = b + ((a + F(b,c,d) + X[k] + T[i]) <<< s). */
    // /* Do the following 16 operations. */
    // [ABCD  0  7  1]  [DABC  1 12  2]  [CDAB  2 17  3]  [BCDA  3 22  4]
    // [ABCD  4  7  5]  [DABC  5 12  6]  [CDAB  6 17  7]  [BCDA  7 22  8]
    // [ABCD  8  7  9]  [DABC  9 12  10]  [CDAB  10 17  11]  [BCDA  11 22  12]
    // [ABCD  12  7  13]  [DABC  13 12  14]  [CDAB  14 17  15]  [BCDA  15 22  16]

    for (j = 0; j < 4; j++)
    {
        a = CalcRound1(a, b, c, d, *xp++,  7, *tp++);
        d = CalcRound1(d, a, b, c, *xp++, 12, *tp++);
        c = CalcRound1(c, d, a, b, *xp++, 17, *tp++);
        b = CalcRound1(b, c, d, a, *xp++, 22, *tp++);
    }

#ifndef SDK_DIGEST_MD5_FAST
    kp = k;
#endif

    // /* Round 2. */
    // /* Let [abcd k s i] denote the operation
    //      a = b + ((a + G(b,c,d) + X[k] + T[i]) <<< s). */
    // /* Do the following 16 operations. */
    // [ABCD  1  5  17]  [DABC  6 9  18]  [CDAB  11 14  19]  [BCDA  0 20  20]
    // [ABCD  5  5  21]  [DABC  10 9  22]  [CDAB  15 14  23]  [BCDA  4 20  24]
    // [ABCD  9  5  25]  [DABC  14 9  26]  [CDAB  3 14  27]  [BCDA  8 20  28]
    // [ABCD  13  5  29]  [DABC  2 9  30]  [CDAB  7 14  31]  [BCDA  12 20  32]

    for (j = 0; j < 4; j++)
    {
#ifndef SDK_DIGEST_MD5_FAST
        a = CalcRound2(a, b, c, d, x[*kp++],  5, *tp++);
        d = CalcRound2(d, a, b, c, x[*kp++],  9, *tp++);
        c = CalcRound2(c, d, a, b, x[*kp++], 14, *tp++);
        b = CalcRound2(b, c, d, a, x[*kp++], 20, *tp++);
#else
        a = CalcRound2(a, b, c, d, x[*(tp+0)],  5, *(tp+1));
        d = CalcRound2(d, a, b, c, x[*(tp+2)],  9, *(tp+3));
        c = CalcRound2(c, d, a, b, x[*(tp+4)], 14, *(tp+5));
        b = CalcRound2(b, c, d, a, x[*(tp+6)], 20, *(tp+7));
        tp += 8;
#endif
    }

    // /* Round 3. */
    // /* Let [abcd k s t] denote the operation
    //      a = b + ((a + H(b,c,d) + X[k] + T[i]) <<< s). */
    // /* Do the following 16 operations. */
    // [ABCD  5  4  33]  [DABC  8 11  34]  [CDAB  11 16  35]  [BCDA  14 23  36]
    // [ABCD  1  4  37]  [DABC  4 11  38]  [CDAB  7 16  39]  [BCDA  10 23  40]
    // [ABCD  13  4  41]  [DABC  0 11  42]  [CDAB  3 16  43]  [BCDA  6 23  44]
    // [ABCD  9  4  45]  [DABC  12 11  46]  [CDAB  15 16  47]  [BCDA  2 23  48]

    for (j = 0; j < 4; j++)
    {
#ifndef SDK_DIGEST_MD5_FAST
        a = CalcRound3(a, b, c, d, x[*kp++],  4, *tp++);
        d = CalcRound3(d, a, b, c, x[*kp++], 11, *tp++);
        c = CalcRound3(c, d, a, b, x[*kp++], 16, *tp++);
        b = CalcRound3(b, c, d, a, x[*kp++], 23, *tp++);
#else
        a = CalcRound3(a, b, c, d, x[*(tp+0)],  4, *(tp+1));
        d = CalcRound3(d, a, b, c, x[*(tp+2)], 11, *(tp+3));
        c = CalcRound3(c, d, a, b, x[*(tp+4)], 16, *(tp+5));
        b = CalcRound3(b, c, d, a, x[*(tp+6)], 23, *(tp+7));
        tp += 8;
#endif
    }

    // /* Round 4. */
    // /* Let [abcd k s t] denote the operation
    //      a = b + ((a + I(b,c,d) + X[k] + T[i]) <<< s). */
    // /* Do the following 16 operations. */
    // [ABCD  0  6  49]  [DABC  7 10  50]  [CDAB  14 15  51]  [BCDA  5 21  52]
    // [ABCD  12  6  53]  [DABC  3 10  54]  [CDAB  10 15  55]  [BCDA  1 21  56]
    // [ABCD  8  6  57]  [DABC  15 10  58]  [CDAB  6 15  59]  [BCDA  13 21  60]
    // [ABCD  4  6  61]  [DABC  11 10  62]  [CDAB  2 15  63]  [BCDA  9 21  64]

    for (j = 0; j < 4; j++)
    {
#ifndef SDK_DIGEST_MD5_FAST
        a = CalcRound4(a, b, c, d, x[*kp++],  6, *tp++);
        d = CalcRound4(d, a, b, c, x[*kp++], 10, *tp++);
        c = CalcRound4(c, d, a, b, x[*kp++], 15, *tp++);
        b = CalcRound4(b, c, d, a, x[*kp++], 21, *tp++);
#else
        a = CalcRound4(a, b, c, d, x[*(tp+0)],  6, *(tp+1));
        d = CalcRound4(d, a, b, c, x[*(tp+2)], 10, *(tp+3));
        c = CalcRound4(c, d, a, b, x[*(tp+4)], 15, *(tp+5));
        b = CalcRound4(b, c, d, a, x[*(tp+6)], 21, *(tp+7));
        tp += 8;
#endif
    }

    context->a += a;
    context->b += b;
    context->c += c;
    context->d += d;
}



/*---------------------------------------------------------------------------*
  Name:         MATH_MD5Init

  Description:  Initializes the MATHMD5Context structure used for requesting the MD5 value.
  
  Arguments:    context:   MATHMD5Context structure
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void
MATH_MD5Init(MATHMD5Context* context)
{
    context->a = 0x67452301;
    context->b = 0xefcdab89;
    context->c = 0x98badcfe;
    context->d = 0x10325476;
    context->length = 0;
}

/*---------------------------------------------------------------------------*
  Name:         MATH_MD5Update

  Description:  Updates the MD5 value with given data.
  
  Arguments:    context:   MATHMD5Context structure
                input:   Pointer to input data.
                length:  Length of input data.
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void
MATH_MD5Update(MATHMD5Context* context, const void* input, u32 length)
{
    u32 buffer_index, buffer_space;
    s32 i;
    u8* p;

    buffer_index = (u32)(context->length & 63); // (length % 64);
    context->length += length;
    buffer_space = 64 - buffer_index;

    if (buffer_space > length)
    {
        if (length > 0)
        {
            MI_CpuCopy8(input, &(context->buffer8[buffer_index]), length);
        }
        return;
    }

    MI_CpuCopy8(input, &(context->buffer8[buffer_index]), buffer_space);
    ProcessBlock(context);
    p = ((u8*)input + buffer_space);
    length -= buffer_space;

    i = (s32)(length >> 6); // length / 64
    for (; i > 0; i--)
    {
        MI_CpuCopy8(p, context->buffer8, 64);
        p += 64;
        ProcessBlock(context);
    }

    length &= 63; // length % 64
    if (length > 0)
    {
        MI_CpuCopy8(p, context->buffer8, length);
    }
}

/*---------------------------------------------------------------------------*
  Name:         MATH_MD5GetHash

  Description:  Gets the final MD5 value.
  
  Arguments:    context:   MATHMD5Context structure
                digest:   Pointer to the location where MD5 is stored.
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void
MATH_MD5GetHash(MATHMD5Context * context, void *digest)
{
    static u8 padding = 0x80;
    u64 total_length;
    u32 buffer_index, buffer_space;

    total_length = context->length << 3; // bytes to bits

    MATH_MD5Update(context, &padding, sizeof(u8));

    buffer_index = (u32)(context->length & 63); // (length % 64);
    buffer_space = 64 - buffer_index;
    if (buffer_space < sizeof(u64))
    {
        MI_CpuFill8(&(context->buffer8[buffer_index]), 0, buffer_space);
        ProcessBlock(context);
        buffer_index = 0;
        buffer_space = 64;
    }
    if (buffer_space > sizeof(u64))
    {
        MI_CpuFill8(&(context->buffer8[buffer_index]), 0, buffer_space - sizeof(u64));
    }
    *(u64*)&(context->buffer8[64-sizeof(u64)]) = total_length;

    ProcessBlock(context);

    MI_CpuCopy8(context->state, digest, sizeof(context->state));

    MI_CpuFill8(context, 0, sizeof(*context));
}

/*---------------------------------------------------------------------------*
  Name:         MATH_CalcMD5

  Description:  Calculates MD5.
  
  Arguments:    digest:   Pointer to the location where MD5 is stored.
                data:    Pointer to input data.
                dataLength:   Length of input data.
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void MATH_CalcMD5(void *digest, const void *data, u32 dataLength)
{
    MATHMD5Context context;
    MATH_MD5Init(&context);
    MATH_MD5Update(&context, data, dataLength);
    MATH_MD5GetHash(&context, digest);
}



// HMAC

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
void MATH_CalcHMACMD5(void *digest, const void *bin_ptr, u32 bin_len, const void *key_ptr, u32 key_len) 
{
    MATHMD5Context context;
    unsigned char   hash_buf[ MATH_MD5_BLOCK_SIZE ]; /* Hash value gotten from the hash function */
    
    MATHiHMACFuncs hash1funcs = {
        MATH_MD5_DIGEST_SIZE,
        (512/8),
    };
    
    hash1funcs.context       = &context;
    hash1funcs.hash_buf      = hash_buf;
    hash1funcs.HashReset     = (void (*)(void*))                   MATH_MD5Init;
    hash1funcs.HashSetSource = (void (*)(void*, const void*, u32)) MATH_MD5Update;
    hash1funcs.HashGetDigest = (void (*)(void*, void*))            MATH_MD5GetHash;
    
    MATHi_CalcHMAC(digest, bin_ptr, bin_len, key_ptr, key_len, &hash1funcs);
}

#ifdef SDK_TWLHYB
#ifdef SDK_ARM7
#include <twl/ltdwram_end.h>
#endif
#endif
