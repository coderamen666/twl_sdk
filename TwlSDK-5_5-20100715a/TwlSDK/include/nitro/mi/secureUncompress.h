/*---------------------------------------------------------------------------*
  Project:     TwlSDK - include - mi
  File:        secureUncompression.h

  Copyright 2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-06-20#$
  $Rev: 6675 $
  $Author: nishimoto_takashi $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_MI_SECURE_UNCOMPRESSION_H__
#define NITRO_MI_SECURE_UNCOMPRESSION_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/types.h>

#define MI_ERR_SUCCESS              0
#define MI_ERR_UNSUPPORTED          -1
#define MI_ERR_SRC_SHORTAGE         -2
#define MI_ERR_SRC_REMAINDER        -3
#define MI_ERR_DEST_OVERRUN         -4
#define MI_ERR_ILLEGAL_TABLE        -5

//======================================================================
//          Expanding Compressed Data
//======================================================================

/*---------------------------------------------------------------------------*
  Name:         MI_SecureUncompressAny

  Description:  Determines the compression type from the data header and runs the appropriate decompression process.
                
                Decompression processing will be linked for all compression types, so it may be better to run separate functions for each compression type if you are only using a specific compression format.
                
                

  Arguments:    srcp    Source address
                srcSize Source data size
                destp   Destination address
                dstSize: Destination size

  Returns:      0 if the conversion was successful
                A negative error code on failure
 *---------------------------------------------------------------------------*/
s32 MI_SecureUncompressAny( const void* srcp, u32 srcSize, void* destp, u32 dstSize );


/*---------------------------------------------------------------------------*
  Name:         MI_SecureUncompressRL

  Description:  8-bit decompression of run-length compressed data

  - Decompresses run-length compressed data, writing in 8 bit units.
  - Use 4 byte alignment for the source address.

  - Data header
      u32 :4  :                Reserved
          compType:4          Compression type( = 3)
          destSize:24         Data size after decompression
  
  - Flag data format
      u8  length:7            Decompressed data length - 1 (When not compressed)
                              Decompressed data length - 3 (only compress when the contiguous length is 3 bytes or greater)
          flag:1              (0, 1) = (not compressed, compressed)
  
  Arguments:    *srcp   Source address
                srcSize Source data size
                *destp   Destination address
                dstSize: Destination size

  Returns:      0 if the conversion was successful
                A negative error code on failure
 *---------------------------------------------------------------------------*/
s32 MI_SecureUncompressRL( const void *srcp, u32 srcSize, void *destp, u32 dstSize );


/*---------------------------------------------------------------------------*
  Name:         MI_SecureUncompressLZ
  
  Description:  8-bit decompression of LZ77 compressed data
  
  * Expands LZ77-compressed data and writes it in 8-bit units.
  - Use 4 byte alignment for the source address.
  
  - Data header
      u32 :4  :                Reserved
          compType:4          Compression type( = 1)
          destSize:24         Data size after decompression
  
  - Flag data format
      u8  flags               Compression/no compression flag
                              (0, 1) = (not compressed, compressed)
  - Code data format (Big Endian)
      u16 length:4            Decompressed data length - 3 (only compress when the match length is 3 bytes or greater)
          offset:12           Match data offset - 1
  
  Arguments:    *srcp   Source address
                srcSize Source data size
                *destp   Destination address
                dstSize: Destination size
  
  Returns:      0 if the conversion was successful
                A negative error code on failure
 *---------------------------------------------------------------------------*/
s32 MI_SecureUncompressLZ( const void *srcp, u32 srcSize, void *destp, u32 dstSize );


/*---------------------------------------------------------------------------*
  Name:         MI_SecureUncompressHuffman
  
  Description:  Decompression of Huffman compressed data
  
  - Decompresses Huffman compressed data, writing in 32 bit units.
  - Use 4 byte alignment for the source address.
  - Use 4-byte alignment for the destination address.
  - The target decompression buffer size must be prepared in 4-byte multiples.
  
  - Data header
      u32 bitSize:4           1 data bit size (Normally 4|8)
          compType:4          Compression type( = 2)
          destSize:24         Data size after decompression
  
  - Tree table
      u8           treeSize        Tree table size/2 - 1
      TreeNodeData nodeRoot        Root node
  
      TreeNodeData nodeLeft        Root left node
      TreeNodeData nodeRight       Root right node
  
      TreeNodeData nodeLeftLeft    Left left node
      TreeNodeData nodeLeftRight   Left right node
  
      TreeNodeData nodeRightLeft   Right left node
      TreeNodeData nodeRightRight  Right right node
  
              .
              .
  
    The compressed data itself follows
  
  - TreeNodeData structure
      u8  nodeNextOffset:6 :   Offset to the next node data - 1 (2 byte units)
          rightEndFlag:1      Right node end flag
          leftEndzflag:1      Left node end flag
                              When the end flag is set
                              There is data in next node
  
  Arguments:    *srcp   Source address
                srcSize Source data size
                *destp   Destination address
                dstSize: Destination size

  Returns:      0 if the conversion was successful
                A negative error code on failure
 *---------------------------------------------------------------------------*/
s32 MI_SecureUncompressHuffman( const void *srcp, u32 srcSize, void *destp, u32 dstSize );

/*---------------------------------------------------------------------------*
  Name:         MI_SecureUnfilterDiff

  Description:  8-bit decompression to restore differential filter conversion.
  
    - Restores a differential filter, writing in 8 bit units.
    - Cannot decompress directly into VRAM
    - If the compressed data size was not a multiple of four, adjust by padding with 0s as much as possible.
      
    - Use 4 byte alignment for the source address.

  Arguments:    *srcp   Source address
                srcSize Source size
                *destp   Destination address
                dstSize: Destination size

  Returns:      0 if the conversion was successful
                A negative error code on failure
 *---------------------------------------------------------------------------*/
s32 MI_SecureUnfilterDiff( register const void *srcp, u32 srcSize, register void *destp, u32 dstSize );

/*---------------------------------------------------------------------------*
  Name:         MI_SecureUncompressBLZ

  Description:  Uncompress BLZ ( backward LZ )

  Arguments:
      void *srcp              source address ( data buffer compressed by compBLZ.exe )
      u32   srcSize           source data size ( data size of compressed data )
      u32   dstSize           data size after uncompressing
      
  Returns:      None.
 *---------------------------------------------------------------------------*/
s32 MI_SecureUncompressBLZ(const void *srcp, u32 srcSize, u32 dstSize);

//================================================================================

#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_MI_SECURE_UNCOMPRESSION_H__ */
#endif
