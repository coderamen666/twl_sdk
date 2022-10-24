/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - g2d - src - internal
  File:     g2di_BitReader.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

#include <nnsys/g2d/g2d_config.h>
#include "g2di_BitReader.h"


/*---------------------------------------------------------------------------*
  Name:         BitReaderReload

  Description:  Abandons the current buffer inside BitReader, and loads the next byte.

  Arguments:    reader: Pointer to BitReader.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static NNS_G2D_INLINE void BitReaderReload(NNSiG2dBitReader* reader)
{
    reader->bits = *(reader->src)++;
    reader->availableBits = 8;
}



/*---------------------------------------------------------------------------*
  Name:         NNSi_G2dBitReaderRead

  Description:  Reads the bit string.

  Arguments:    reader: Pointer to BitReader.
                nBits:  The number of bits to read. It must be 8 or less.

  Returns:      The bit string that was read.
 *---------------------------------------------------------------------------*/
u32 NNSi_G2dBitReaderRead(NNSiG2dBitReader* reader, int nBits)
{
    u32 val = reader->bits;
    int nAvlBits = reader->availableBits;

    SDK_ASSERT(nBits <= 8);

    if( nAvlBits < nBits )
    // If the bits inside the byte buffer are insufficient...
    {
        int lack = nBits - nAvlBits;
        val <<= lack;
        BitReaderReload(reader);
        val |= NNSi_G2dBitReaderRead(reader, lack);
    }
    else
    // If the bits inside the byte buffer are sufficient...
    {
        val >>= (nAvlBits - nBits);

        reader->availableBits = (s8)(nAvlBits - nBits);
    }

    val &= 0xFF >> (8 - nBits);
    return val;
}

