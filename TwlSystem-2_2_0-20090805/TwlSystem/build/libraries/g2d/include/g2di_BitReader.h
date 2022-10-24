/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - g2d
  File:     g2di_BitReader.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
#ifndef G2DI_BITREADER_H_
#define G2DI_BITREADER_H_

#include <nitro.h>
#include <nnsys/g2d/g2d_config.h>
#ifdef __cplusplus
extern "C" {
#endif


typedef struct NNSiG2dBitReader
{
    const u8* src;          // pointer to the read position
    s8 availableBits;       // number of unread bits in bits
    u8 bits;                // byte cache
    u8 padding_[2];         //
}
NNSiG2dBitReader;

//----------------------------------

/*---------------------------------------------------------------------------*
  Name:         NNSi_G2dBitReaderInit

  Description:  Initializes BitReader.

  Arguments:    reader: Pointer to the BitReader to initialize.
                src:    Pointer to the byte string that reads the bit.

  Returns:      None.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE void NNSi_G2dBitReaderInit(NNSiG2dBitReader* reader, const void* src)
{
    reader->availableBits   = 0;
    reader->src             = (const u8*)src;
    reader->bits            = 0;
}



/*---------------------------------------------------------------------------*
  Name:         NNSi_G2dBitReaderRead

  Description:  Reads the bit string.

  Arguments:    reader: Pointer to BitReader.
                nBits:  The number of bits to read. It must be 8 or less.

  Returns:      The bit string that was read.
 *---------------------------------------------------------------------------*/
u32 NNSi_G2dBitReaderRead(NNSiG2dBitReader* reader, int nBits);



/*---------------------------------------------------------------------------*
  Name:         NNSi_G2dBitReaderAlignByte

  Description:  Discards the remaining bits in the byte buffer.
                Subsequent bits read from this point become the head of each byte of the load source.
                

  Arguments:    reader: Pointer to BitReader.

  Returns:      None.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE void NNSi_G2dBitReaderAlignByte(NNSiG2dBitReader* reader)
{
    // Set to 0 if availableBits is not 8.
    // This assumes that availableBits is 8 or less.
    reader->availableBits &= 8;
}



#ifdef __cplusplus
}/* extern "C" */
#endif

#endif // G2DI_BITREADER_H_

