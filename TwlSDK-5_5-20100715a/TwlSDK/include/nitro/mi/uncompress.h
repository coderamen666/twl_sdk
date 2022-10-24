/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MI - include
  File:     uncompress.h

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

#ifndef NITRO_MI_UNCOMPRESS_H_
#define NITRO_MI_UNCOMPRESS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/misc.h>
#include <nitro/types.h>


//---- Compression Types
typedef enum
{
    MI_COMPRESSION_LZ = 0x10,          // LZ77
    MI_COMPRESSION_HUFFMAN = 0x20,     // Huffman
    MI_COMPRESSION_RL = 0x30,          // Run Length
    MI_COMPRESSION_DIFF = 0x80,        // Differential filter

    MI_COMPRESSION_TYPE_MASK = 0xf0,
    MI_COMPRESSION_TYPE_EX_MASK = 0xff
}
MICompressionType;


//----------------------------------------------------------------
// Compressed Data Header
//
typedef struct
{
    u32     compParam:4;
    u32     compType:4;
    u32     destSize:24;

}
MICompressionHeader;



//----------------------------------------------------------------
// Parameters for expanding bit compressed data
//
typedef struct
{
    u16     srcNum;                    // Source data / Number of bytes
    u16     srcBitNum:8;               // Number of bits per one source data element
    u16     destBitNum:8;              // Number of bits per one destination data element
    u32     destOffset:31;             // Number to add to source data
    u32     destOffset0_on:1;          // Flag for whether to add an offset to 0 data.
}
MIUnpackBitsParam;


//======================================================================
//          Expanding Compressed Data
//======================================================================

//----------------------------------------------------------------------
//          Expanding Bit Compressed Data
//
//- Unpacks data padded with bits fixed to 0.
//- Align the destination address to a 4-byte boundary.
//
//Arguments:
//             void *srcp:              source address
//             void *destp:            destination address
//  MIUnpackBitsParam *paramp:  Address of MIUnpackBitsParam structure
//
//MIUnpackBitsParam Structure
//    u16 srcNum:              Number of bytes per source data element
//    u8  srcBitNum:           Number of bits per source data element
//    u8  destBitNum:          Number of bits per destination data element
//    u32 destOffset:31:       Offset number to add to source data.
//        destOffset0_On:1:    Flag for whether to add an offset to 0 data.
//
//- Return value: None
//----------------------------------------------------------------------

void    MI_UnpackBits(const void *srcp, void *destp, MIUnpackBitsParam *paramp);


//----------------------------------------------------------------------
//          8-bit decompression of LZ77 compressed data
//
//* Expands LZ77-compressed data and writes it in 8-bit units.
//- Cannot decompress directly into VRAM
//- If the compressed data size was not a multiple of four, adjust by padding with 0s as much as possible.
//   
//- Use 4-byte alignment for the source address.
//
//Arguments:
//    void *srcp:              source address
//    void *destp:            destination address
//
//- Data header
//    u32 :4  :                Reserved
//        compType:4          Compression type( = 1)
//        destSize:24         Data size after decompression
//
//- Flag data format
//    u8  flags               Compression/no compression flag
//                            (0, 1) = (not compressed, compressed)
//- Code data format (big endian)
//    u16 length:4            Decompressed data length - 3 (only compress when the match length is 3 bytes or greater)
//        offset:12           Match data offset - 1
//
//- Return value: None
//----------------------------------------------------------------------

void    MI_UncompressLZ8(const void *srcp, void *destp);


//----------------------------------------------------------------------
//          16-bit decompression of LZ77 compressed data
//
//* Expands LZ77-compressed data and writes it in 16-bit units.
//* Although it can also expand in data TCM and main memory, it is slower than MI_UncompressLZ778().
//   
//* For compressed data, search for a matching character string from a minimum of 2 bytes previous.
//- If the compressed data size was not a multiple of four, adjust by padding with 0s as much as possible.
//   
//- Use 4-byte alignment for the source address.
//
//Arguments:
//    void *srcp:              source address
//    void *destp:            destination address
//
//- Data header
//    u32 :4  :                Reserved
//        compType:4          Compression type( = 1)
//        destSize:24         Data size after decompression
//
//- Flag data format
//    u8  flags               Compression/no compression flag
//                            (0, 1) = (not compressed, compressed)
//- Code data format (big endian)
//    u16 length:4            Decompressed data length - 3 (only compress when the match length is 3 bytes or greater)
//        offset:12 :          Match data offset ( >= 2) - 1
//
//- Return value: None
//----------------------------------------------------------------------

void    MI_UncompressLZ16(const void *srcp, void *destp);


//----------------------------------------------------------------------
//          Decompression of Huffman compressed data
//
//- Decompresses Huffman compressed data, writing in 32-bit units.
//- If the compressed data size was not a multiple of four, adjust by padding with 0s as much as possible.
//   
//- Use 4-byte alignment for the source address.
//
//Arguments:
//    void *srcp:              source address
//    void *destp:            destination address
//
//- Data header
//    u32 bitSize:4           1 data bit size (Normally 4|8)
//        compType:4          Compression type( = 2)
//        destSize:24         Data size after decompression
//
//- Tree table
//    u8           treeSize        Tree table size/2 - 1
//    TreeNodeData nodeRoot        Root node
//
//    TreeNodeData nodeLeft        Root left node
//    TreeNodeData nodeRight       Root right node
//
//    TreeNodeData nodeLeftLeft    Left left node
//    TreeNodeData nodeLeftRight   Left right node
//
//    TreeNodeData nodeRightLeft   Right left node
//    TreeNodeData nodeRightRight  Right right node
//
//            .
//            .
//
//  The compressed data itself follows
//
//- TreeNodeData structure
//    u8  nodeNextOffset:6 :   Offset to the next node data - 1 (2 byte units)
//        rightEndFlag:1      Right node end flag
//        leftEndzflag:1      Left node end flag
//                            When end flag is set, there is data in next node.
//                             
//
//- Return value: None
//----------------------------------------------------------------------

void    MI_UncompressHuffman(const void *srcp, void *destp);


//----------------------------------------------------------------------
//          8-bit decompression of run-length compressed data
//
//- Decompresses run-length compressed data, writing in 8 bit units.
//- You cannot decompress directly to VRAM on a NITRO system.
//- If the compressed data size was not a multiple of four, adjust by padding with 0s as much as possible.
//  
//- Use 4-byte alignment for the source address.
//
//Arguments:
//    void *srcp:              source address
//    void *destp:            destination address
//
//- Data header
//    u32 :4  :                Reserved
//        compType:4          Compression type( = 3)
//        destSize:24         Data size after decompression
//
//- Flag data format
//    u8  length:7            Decompressed data length - 1 (When not compressed)
//                            Decompressed data length - 3 (only compress when the contiguous length is 3 bytes or greater)
//        flag:1              (0, 1) = (not compressed, compressed)
//
//- Return value: None
//----------------------------------------------------------------------

void    MI_UncompressRL8(const void *srcp, void *destp);


//----------------------------------------------------------------------
//          16-bit decompression of run-length compressed data
//
//- Decompresses run-length compressed data, writing in 16-bit units.
//- You can also decompress to data TCM and VRAM.
//- This is slower than MI_UncompressRL8() for decompression to main RAM.
//- This is faster than MI_UncompressRL8() for decompression to data TCM and VRAM.
//- If the compressed data size was not a multiple of four, adjust by padding with 0s as much as possible.
//  
//- Use 4-byte alignment for the source address.
//
//Arguments:
//    void *srcp:              source address
//    void *destp:            destination address
//
//- Data header
//    u32 :4  :                Reserved
//        compType:4          Compression type( = 3)
//        destSize:24         Data size after decompression
//
//- Flag data format
//    u8  length:7            Decompressed data length - 1 (When not compressed)
//                            Decompressed data length - 3 (only compress when the contiguous length is 3 bytes or greater)
//        flag:1              (0, 1) = (not compressed, compressed)
//
//- Return value: None
//----------------------------------------------------------------------

void    MI_UncompressRL16(const void *srcp, void *destp);


//----------------------------------------------------------------------
//          32-bit decompression of run-length compressed data
//
//- Decompresses run-length compressed data, writing in 32-bit units.
//- This is faster than MI_UncompressRL8() and MI_UncompressRL16().
//- If the compressed data size was not a multiple of four, adjust by padding with 0s as much as possible.
//  
//- Use 4-byte alignment for the source address.
//
//Arguments:
//    void *srcp:              source address
//    void *destp:            destination address
//
//- Data header
//    u32 :4  :                Reserved
//        compType:4          Compression type( = 3)
//        destSize:24         Data size after decompression
//
//- Flag data format
//    u8  length:7            Decompressed data length - 1 (When not compressed)
//                            Decompressed data length - 3 (only compress when the contiguous length is 3 bytes or greater)
//        flag:1              (0, 1) = (not compressed, compressed)
//
//- Return value: None
//----------------------------------------------------------------------

void    MI_UncompressRL32(register const void *srcp,register void *destp);


//----------------------------------------------------------------------
//          8-bit decompression to restore differential filter conversion.
//
//- Restores a differential filter, writing in 8-bit units.
//- You cannot decompress directly to VRAM on a NITRO system.
//- If the compressed data size was not a multiple of four, adjust by padding with 0s as much as possible.
//  
//- Use 4-byte alignment for the source address.
//
//Arguments:
//    void *srcp:              source address
//    void *destp:            destination address
//
//- Data header
//    u32 :4 :                 Bit size of unit
//        compType:4          Compression type( = 3)
//        destSize:24         Data size after decompression
//
//- Return value: None
//----------------------------------------------------------------------

void    MI_UnfilterDiff8(const void *srcp, void *destp);


//----------------------------------------------------------------------
//          16-bit decompression to restore differential filter conversion.
//
//- Restores a differential filter, writing in 16-bit units.
//- You can also decompress to data TCM and VRAM.
//- This is faster than MI_UnfilterDiff16().
//- If the compressed data size was not a multiple of four, adjust by padding with 0s as much as possible.
//  
//- Use 4-byte alignment for the source address.
//
//Arguments:
//    void *srcp:              source address
//    void *destp:            destination address
//
//- Data header
//    u32 :4 :                 Bit size of unit
//        compType:4          Compression type( = 3)
//        destSize:24         Data size after decompression
//
//- Return value: None
//----------------------------------------------------------------------

void    MI_UnfilterDiff16(const void *srcp, void *destp);

//----------------------------------------------------------------------
//          32-bit decompression to restore differential filter conversion.
//
//- Restores a differential filter, writing in 32-bit units.
//- You can also decompress to data TCM and VRAM.
//  This is faster than MI_Uncompress8().//---- 
//- If the compressed data size was not a multiple of four, adjust by padding with 0s as much as possible.
//  
//- Use 4-byte alignment for the source address.
//
//Arguments:
//    void *srcp:              source address
//    void *destp:            destination address
//
//- Data header
//    u32 :4 :                 Bit size of unit
//        compType:4          Compression type( = 3)
//        destSize:24         Data size after decompression
//
//- Return value: None
//----------------------------------------------------------------------

void    MI_UnfilterDiff32(register const void *srcp,register void *destp);


//----------------------------------------------------------------------
//          8-bit decompression for differential filter conversion
//
//- Runs differential filter conversion and writes data 8 bits at a time.
//- You cannot decompress directly to VRAM on a NITRO system.
//- If the compressed data size was not a multiple of four, adjust by padding with 0s as much as possible.
//  
//- Use 4-byte alignment for the source address.
//
//Arguments:
//    void *srcp:              source address
//    void *destp:            destination address
//    u32  size               Source size
//    BOOL bitsize            Differential size (TRUE: 16-bit, FALSE: 8-bit)
//
//- Data header
//    u32 :4 :                 Bit size of unit
//        compType:4          Compression type( = 3)
//        destSize:24         Data size after decompression
//
//- Return value: None
//----------------------------------------------------------------------

void    MI_FilterDiff8(register const void *srcp, register void *destp, register u32 size, register BOOL bitsize);


//----------------------------------------------------------------------
//          16-bit decompression for differential filter conversion
//
//- Runs differential filter conversion and writes data 16 bits at a time.
//- You can also decompress to data TCM and VRAM.
//- This is faster than MI_FilterDiff8().
//- If the compressed data size was not a multiple of four, adjust by padding with 0s as much as possible.
//  
//- Use 4-byte alignment for the source address.
//
//Arguments:
//    void *srcp:              source address
//    void *destp:            destination address
//    u32  size               Source size
//    BOOL bitsize            Differential size (TRUE: 16-bit, FALSE: 8-bit)
//
//- Data header
//    u32 :4 :                 Bit size of unit
//        compType:4          Compression type( = 3)
//        destSize:24         Data size after decompression
//
//- Return value: None
//----------------------------------------------------------------------

void    MI_FilterDiff16(register const void *srcp, register void *destp, register u32 size, register BOOL bitsize);


//----------------------------------------------------------------------
//          32-bit decompression for differential filter conversion
//
//- Runs differential filter conversion and writes data 32 bits at a time.
//- This is faster than MI_FilterDiff16().
//- If the compressed data size was not a multiple of four, adjust by padding with 0s as much as possible.
//  
//- Use 4-byte alignment for the source address.
//
//Arguments:
//    void *srcp:              source address
//    void *destp:            destination address
//    u32  size               Source size
//    BOOL bitsize            Differential size (TRUE: 16-bit, FALSE: 8-bit)
//
//- Data header
//    u32 :4 :                 Bit size of unit
//        compType:4          Compression type( = 3)
//        destSize:24         Data size after decompression
//
//- Return value: None
//----------------------------------------------------------------------

void    MI_FilterDiff32(register const void *srcp, register void *destp, register u32 size, register BOOL bitsize);


//================================================================================
/*---------------------------------------------------------------------------*
  Name:         MI_GetUncompressedSize

  Description:  Gets data size after uncompressing.
                This function can be used for all compression types
                (LZ8, LZ16, Huffman, RL8, RL16)

  Arguments:    srcp:  Compressed data address

  Returns:      size
 *---------------------------------------------------------------------------*/
static inline u32 MI_GetUncompressedSize(const void *srcp)
{
    return (*(u32 *)srcp >> 8);
}

/*---------------------------------------------------------------------------*
  Name:         MI_GetCompressionType

  Description:  Gets compression type from compressed data.

  Arguments:    srcp:  Compressed data address

  Returns:      Compression type.
                MI_COMPREESION_LZ: mean compressed by LZ77
                MI_COMPREESION_HUFFMAN: mean compressed by Huffman
                MI_COMPREESION_RL: mean compressed by Run Length
                MI_COMPRESSION_DIFF: mean converted by Differential filter
 *---------------------------------------------------------------------------*/
static inline MICompressionType MI_GetCompressionType(const void *srcp)
{
    return (MICompressionType)(*(u32 *)srcp & MI_COMPRESSION_TYPE_MASK);
}



/*---------------------------------------------------------------------------*
  Name:         MIi_UncompressBackward

  Description:  Uncompress special archive for module compression

  Arguments:    bottom = Bottom address of packed archive + 1
                bottom[-12] = offset for top of compressed data
                                 inp_top = bottom + bottom[-12]
                bottom[ -8] = offset for bottom of compressed data
                                 inp = bottom + bottom[ -8]
                bottom[ -4] = offset for bottom of original data
                                 outp = bottom + bottom[ -4]
  
                typedef struct
                {
                    int         bufferTop;
                    int         compressBottom;
                    int         originalBottom;
                }   CompFooter;

  Returns:      None.
 *---------------------------------------------------------------------------*/
// !!!! Coded in libraries/init/ARM9/crt0.c
void    MIi_UncompressBackward(void *bottom);

#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_MI_UNCOMPRESS_H_ */
#endif
