/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     systemCall.h

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

#ifndef NITRO_OS_SYSTEMCALL_H_
#define NITRO_OS_SYSTEMCALL_H_

#include <nitro/mi/stream.h>
#include <nitro/mi/uncompress.h>

#ifdef SDK_TWL
#include <twl/os/common/systemCall.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


//================================================================================
/*---------------------------------------------------------------------------*
  Name:         SVC_WaitVBlankIntr

  Description:  Wait for VBlank.

              - Waits in hold state until VBlank interrupt is generated.
              - Using the interrupt process, set the flag corresponding to INTR_CHECK_BUF (DTCM_END - 0x8).
                
              At times when multiple interrupts are combined, it is possible to decrease call overhead compared to cases where SVC_Halt() is called repeatedly.
                
                
              -If Halt is entered when using a thread, other threads will also be stopped until an interrupt is generated. So we recommend using OS_WaitIrq(1, OS_IE_V_BLANK) for all threads other than the idle thread.
                
                

  Arguments:    None.

  Returns:      None.

 *---------------------------------------------------------------------------*/
#ifdef  SDK_SVC_WAITVBLANK_COMPATIBLE
void    SVC_WaitVBlankIntr(void);
#else
#define SVC_WaitVBlankIntr  OS_WaitVBlankIntr
#endif

/*---------------------------------------------------------------------------*
  Name:         SVC_WaitByLoop

  Description:  Loop specified number of times.

              - Performs loop processing in system ROM a specified number of times.
              - It takes four cycles for one loop.
              In states during normal operation where the sub-processor has priority to access main memory, it is possible to reduce stalling of the main processor by having the sub-processor run programs in system ROM.
                
                
              - When it is necessary to give priority to the main processor in modes such as main memory display mode, it may be a situation in which calling by the main processor is valid, but there is no need to call from the main processor as long as most of that time span is running in the cache or on the TCM.
                
                
                

              Arguments:
                  count:     loop count

  Arguments:    count: times to loop 

  Returns:      None.

 *---------------------------------------------------------------------------*/
void    SVC_WaitByLoop(s32 count);

/*---------------------------------------------------------------------------*
  Name:         SVC_CpuSet

  Description:  Clear or copy memory by cpu.

               - Clears or copies RAM using DmaSet macro compatible parameters.
               - If 32-bit transfer, it is accessed forcibly with 4-byte boundary, but with 16-bit transfer the arguments need to be matched up with a 2-byte boundary and passed.
                 

               Arguments:
                   srcp    Source address
                   destp   Destination address
                   dmaCntData:    only valid with MI_DMA_SRC_FIX, MI_DMA_32BIT_BUS, or MI_DMA_COUNT_MASK

                         MI_DMA_SRC_FIX(0, 1) = (source address increment, source address fixed)
                         MI_DMA_32BIT_BUS(0, 1) = (16-bit transfer, 32-bit transfer)
                         MI_DMA_COUNT_MASK & dmaCntData = number of transfers

               - High-level macros:
                   SVC_CpuClear, SVC_CpuClearArray, SVC_CpuCopy, SVC_CpuCopyArray

  Arguments:    srcp:       Source address
                destp:      Destination address
                dmaCntData: dma control data
                            (only MI_DMA_SRC_FIX, MI_DMA_32BIT_BUS, or MI_DMA_COUNT_MASK
                             is available)

  Returns:      None.

 *---------------------------------------------------------------------------*/
void    SVC_CpuSet(const void *srcp, void *destp, u32 dmaCntData);

/*---------------------------------------------------------------------------*
  Name:         SVC_CpuSetFast

  Description:  Clear or copy memory by cpu quickly.

              - Clears and copies RAM at high speed with DmaSet macro compatible parameters.
              - When access in 32-Byte units is possible, multiple load/store instructions are used, and the remainder is accessed in 4-Byte units.
                
              - Even if you provide an argument outside the 4-Byte boundary, access is forcibly within the 4-Byte boundary.

              Arguments:
                  srcp    Source address
                  destp   Destination address
                  dmaCntData:     only valid with MI_DMA_SRC_FIX and MI_DMA_COUNT_MASK

                        MI_DMA_SRC_FIX(0, 1) = (source address increment, source address fixed)
                        MI_DMA_COUNT_MASK & dmaCntData = number of transfers

              - High-level macros:
                  SVC_CpuClearFast, SVC_CpuClearArrayFast, SVC_CpuCopyFast, SVC_CpuCopyArrayFast

  Arguments:    srcp:       Source address
                destp:      Destination address
                dmaCntData: dma control data
                            (only MI_DMA_SRC_FIX and MI_DMA_COUNT_MASK
                             is available)

  Returns:      None.

 *---------------------------------------------------------------------------*/
void    SVC_CpuSetFast(const void *srcp, void *destp, u32 dmaCntData);

/*---------------------------------------------------------------------------*
  Name:         SVC_CpuClear

  Description:  Clear memory by SVC_CpuSet.

              - Calls system call that does RAM clear with the CPU.
              - Clear data is put in stack, and that is copied to the destination.

              Arguments:
                  data:        clear data
                  destp   Destination address
                  size:        number of cleared bytes
                  bit:         transfer bit width (16|32)

  Arguments:    data: clear data
                destp: destination address
                size: clear size ( by byte )
                bit: bit width ( 16 or 32 )

  Returns:      None.

 *---------------------------------------------------------------------------*/
#define SVC_CpuClear( data, destp, size, bit )                  \
do{                                                             \
    vu##bit tmp = (vu##bit )(data);                             \
    SVC_CpuSet((u8 *)&(tmp), (u8 *)(destp), (                   \
        MI_DMA_SRC_FIX        |                                 \
        MI_DMA_##bit##BIT_BUS | ((size)/((bit)/8) & 0x1fffff)));  \
} while(0)

/*---------------------------------------------------------------------------*
  Name:         SVC_CpuClearArray

  Description:  Clear memory by SVC_CpuSet.

              - Calls system call that does RAM clear with the CPU.
              - Clear data is put in stack, and that is copied to the destination.
              - SVC_CpuClearArray clears the entire destination array.

              Arguments:
                  data:        clear data
                  destp   Destination address
                  bit:         transfer bit width (16|32)

  Arguments:    data: clear data
                destp: destination address
                bit: bit width ( 16 or 32 )

  Returns:      None.

 *---------------------------------------------------------------------------*/
#define SVC_CpuClearArray( data, destp, bit )                   \
        SVC_CpuClear( data, destp, sizeof(destp), bit )

/*---------------------------------------------------------------------------*
  Name:         SVC_CpuCopy

  Description:  Copy memory by SVC_CpuSet.

              - Calls system call that does copy with the CPU.
              - SVC_CpuCopyArray copies the entire source array.

              Arguments:
                  srcp:  source address
                  destp:  destination address
                  size:  number of bytes transferred
                  bit:  transfer bit width (16|32)

  Arguments:    srcp: Source address
                destp: destination address
                size: size to copy ( by byte )
                bit: bit width ( 16 or 32 )

  Returns:      None.

 *---------------------------------------------------------------------------*/
#define SVC_CpuCopy( srcp, destp, size, bit )                   \
                                                                \
    SVC_CpuSet((u8 *)(srcp), (u8 *)(destp),  (                  \
        MI_DMA_SRC_INC        |                                 \
        MI_DMA_##bit##BIT_BUS | ((size)/((bit)/8) & 0x1fffff)))

/*---------------------------------------------------------------------------*
  Name:         SVC_CpuCopyArray

  Description:  Copy memory by SVC_CpuSet.

              - Calls system call that does copy with the CPU.
              - SVC_CpuCopyArray copies the entire source array.

              Arguments:
                  srcp:  source address
                  destp:  destination address
                  bit:  transfer bit width (16|32)

  Arguments:    srcp: Source address
                destp: destination address
                bit: bit width ( 16 or 32 )

  Returns:      None.

 *---------------------------------------------------------------------------*/
#define SVC_CpuCopyArray( srcp, destp, bit  )                  \
        SVC_CpuCopy( srcp, destp, sizeof(srcp), bit )

/*---------------------------------------------------------------------------*
  Name:         SVC_CpuClearFast

  Description:  Clear memory by SVC_CpuSetFast quickly.

              - Calls a system call that rapidly clears RAM in the CPU.
              - Clear data is put in stack, and that is copied to the destination.
              - When access is possible in 32-Byte units, multiple 32-Byte unit store instructions are used, and the remainder is accessed in 4-Byte units.
                

              Arguments:
                  data:        clear data
                  destp   Destination address
                  size:        number of cleared bytes

  Arguments:    data: clear data
                destp: destination address
                size: clear size ( by byte )

  Returns:      None.

 *---------------------------------------------------------------------------*/
#define SVC_CpuClearFast( data, destp, size )                   \
do{                                                             \
    vu32 tmp = (vu32 )(data);                                   \
    SVC_CpuSetFast((u8 *)&(tmp), (u8 *)(destp), (               \
        MI_DMA_SRC_FIX | ((size)/(32/8) & 0x1fffff)));   \
} while(0)

/*---------------------------------------------------------------------------*
  Name:         SVC_CpuClearArrayFast

  Description:  Clear memory by SVC_CpuSetFast quickly.

              - Calls a system call that rapidly clears RAM in the CPU.
              - Clear data is put in stack, and that is copied to the destination.
              - When access is possible in 32-Byte units, multiple store instructions are used, and the remainder is accessed in 4-Byte units.
                
              - SVC_CpuClearArrayFast clears the entire destination array.

              Arguments:
                  data:        clear data
                  destp   Destination address

  Arguments:    data: clear data
                size: size to clear ( by byte )

  Returns:      None.
 *---------------------------------------------------------------------------*/
#define SVC_CpuClearArrayFast( data, destp )                    \
        SVC_CpuClearFast( data, destp, sizeof(destp) )

/*---------------------------------------------------------------------------*
  Name:         SVC_CpuCopyFast

  Description:  Clear memory by SVC_CpuSetFast quickly.

              - Calls a system call that rapidly copies in the CPU.
              - When access in 32-Byte units is possible, multiple load/store instructions are used, and the remainder is accessed in 4-Byte units.
                

              Arguments:
                  srcp    Source address
                  destp   Destination address
                  size:        number of transfer bytes

  Arguments:    srcp: Source address
                destp: destination address
                size: size to copy ( by byte )

  Returns:      None.

 *---------------------------------------------------------------------------*/
#define SVC_CpuCopyFast( srcp, destp, size )                    \
                                                                \
    SVC_CpuSetFast((u8 *)(srcp), (u8 *)(destp),  (              \
        MI_DMA_SRC_INC | ((size)/(32/8) & 0x1fffff)))

/*---------------------------------------------------------------------------*
  Name:         SVC_CpuCopyArrayFast

  Description:  Clear memory by SVC_CpuSetFast quickly.

              - Calls a system call that rapidly copies in the CPU.
              - When access in 32-Byte units is possible, multiple load/store instructions are used, and the remainder is accessed in 4-Byte units.
                
              - SVC_CpuCopyArrayFast copies the entire source array.

              Arguments:
                  srcp    Source address
                  destp   Destination address

  Arguments:    srcp: Source address
                destp: destination address

  Returns:      None.

 *---------------------------------------------------------------------------*/
#define SVC_CpuCopyArrayFast( srcp, destp )                     \
        SVC_CpuCopyFast( srcp, destp, sizeof(srcp) )

/*---------------------------------------------------------------------------*
  Name:         SVC_UnpackBits

  Description:  Unpacks bits.

              - Unpacks data that was packed with the 0 fixed bit.
              - Align the destination address to a 4-byte boundary.

              Arguments:
                   srcp    Source address
                   destp   Destination address
                   paramp:        address of MIUnpackBitsParam structure

              MIUnpackBitsParam Structure
                    u16 srcNum:              Number of bytes per source data element
                    u8  srcBitNum:           Number of bits per source data element
                    u8  destBitNum:          Number of bits per destination data element
                    u32 destOffset:31:       Offset number to add to source data.
                        destOffset0_On:1:    Flag for whether to add an offset to 0 data.

  Arguments:    srcp: Source address
                destp: destination address
                paramp: parameter structure address

  Returns:      None.

 *---------------------------------------------------------------------------*/
void    SVC_UnpackBits(const void *srcp, void *destp, const MIUnpackBitsParam *paramp);

/*---------------------------------------------------------------------------*
  Name:         SVC_UncompressLZ8

  Description:  Uncompresses LZ77.

              * Expands LZ77-compressed data and writes it in 8-bit units.
              - You cannot directly uncompress VRAM, etc., that cannot be byte accessed.
              - If the compressed data size was not a multiple of four, adjust by padding with 0s as much as possible.
                
              - Use 4-byte alignment for the source address.

              Arguments:
                   srcp    Source address
                   destp   Destination address

               - Data header
                   u32 :4                Reserved
                        compType:4          Compression type( = 1)
                        destSize:24         Data size after decompression

               - Flag data format
                    u8  flags               Compression/no compression flag
                                            (0, 1) = (data not compressed, data compressed)
               - Code data format (big endian)
                    u16 length:4            Decompressed data length - 3 (only compress when the match length is 3 bytes or greater)
                        offset:12           Match data offset - 1

  Arguments:    srcp: Source address
                destp: destination address

  Returns:      None.

 *---------------------------------------------------------------------------*/
void    SVC_UncompressLZ8(const void *srcp, void *destp);

/*---------------------------------------------------------------------------*
  Name:         SVC_UncompressRL8

  Description:  Uncompresses run length encoding.

              - Decompresses run-length compressed data, writing in 8 bit units.
              - You cannot directly uncompress VRAM, etc., that cannot be byte accessed.
              - If the compressed data size was not a multiple of four, adjust by padding with 0s as much as possible.
                
              - Use 4-byte alignment for the source address.

              Arguments:
                   srcp    Source address
                   destp   Destination address

              - Data header
                   u32 :4                Reserved
                       compType:4          Compression type( = 3)
                       destSize:24         Data size after decompression

              - Flag data format
                   u8  length:7            Decompressed data length - 1 (When not compressed)
                                           Decompressed data length - 3 (only compress when the contiguous length is 3 bytes or greater)
                       flag:1              (0, 1) = (not compressed, compressed)

  Arguments:    srcp: Source address
                destp: destination address

  Returns:      None.

 *---------------------------------------------------------------------------*/
void    SVC_UncompressRL8(const void *srcp, void *destp);

/*---------------------------------------------------------------------------*
  Name:         SVC_UncompressLZ16FromDevice

  Description:  Uncompresses LZ77 from device.

              * Expands LZ77-compressed data and writes it in 16-bit units.
              - You can directly uncompress compressed data on a non-memory mapped device without using a temporary buffer.
                
              - You can also uncompress in RAM that cannot byte-access, but it is slower than SVC_UncompressLZ8().
                
              * For compressed data, search for a matching character string from a minimum of 2 bytes previous.
              - If the compressed data size was not a multiple of four, adjust by padding with 0s as much as possible.
                
              - Use 4-byte alignment for the source address.
              - At success, it returns the size after uncompression. At failure, it returns a negative value.
                Return a negative value when an error is detected in the initStream/terminateStream callback function.
                

              Arguments:
                   srcp    Source address
                   destp   Destination address
                   paramp:        address of parameter passed to the initStream function of the MIReadStreamCallbacks structure
                   callbacks:     address of the MIReadStreamCallbacks structure

               - Data header
                   u32 :4                Reserved
                       compType:4          Compression type( = 1)
                       destSize:23         Data size after decompression
                       :1                  unusable

               - Flag data format
                    u8  flags               Compression/no compression flag
                                            (0, 1) = (data not compressed, data compressed)
               - Code data format (big endian)
                    u16 length:4            Decompressed data length - 3 (only compress when the match length is 3 bytes or greater)
                        offset:12           Match data offset - 1

  Arguments:    srcp: Source address
                destp: destination address
                callbacks: address of stream callbacks structure

  Returns:      Uncompressed size >= 0
                error < 0

 *---------------------------------------------------------------------------*/
s32     SVC_UncompressLZ16FromDevice(const void *srcp, void *destp, const void *paramp,
                                     const MIReadStreamCallbacks *callbacks);

/*---------------------------------------------------------------------------*
  Name:         SVC_UncompressRL16FromDevice

  Description:  Uncompresses run length encoding from device.

              - Decompresses run-length compressed data, writing in 16 bit units.
              - You can directly uncompress compressed data on a non-memory mapped device without using a temporary buffer.
                
              - You can also uncompress in RAM that cannot byte-access, but it is slower than SVC_UncompressRL8().
                
              - If the compressed data size was not a multiple of four, adjust by padding with 0s as much as possible.
                
              - Use 4-byte alignment for the source address.
              - At success, it returns the size after uncompression. At failure, it returns a negative value.
                Return a negative value when an error is detected in the initStream/terminateStream callback function.
                

              Arguments:
                   srcp    Source address
                   destp   Destination address
                   paramp:        address of parameter passed to the initStream function of the MIReadStreamCallbacks structure
                   callbacks:     address of the MIReadStreamCallbacks structure

              - Data header
                   u32 :4                Reserved
                       compType:4          Compression type( = 3)
                       destSize:23         Data size after decompression
                       :1                  unusable

              - Flag data format
                   u8  length:7            Decompressed data length - 1 (When not compressed)
                                           Decompressed data length - 3 (only compress when the contiguous length is 3 bytes or greater)
                       flag:1              (0, 1) = (not compressed, compressed)

  Arguments:    srcp: Source address
                destp: destination address
                callbacks: address of stream callbacks structure

  Returns:      Uncompressed size >= 0
                error < 0

 *---------------------------------------------------------------------------*/
s32     SVC_UncompressRL16FromDevice(const void *srcp, void *destp, const void *paramp,
                                     const MIReadStreamCallbacks *callbacks);

/*---------------------------------------------------------------------------*
  Name:         SVC_UncompressHuffmanFromDevice

  Description:  Uncompresses Huffman encoding from device.

              - Decompresses Huffman compressed data, writing in 32-bit units.
              - You can directly uncompress compressed data on a non-memory mapped device without using a temporary buffer.
                
              - If the compressed data size was not a multiple of four, adjust by padding with 0s as much as possible.
                
              - Use 4-byte alignment for the source address.
              - At success, it returns the size after uncompression. At failure, it returns a negative value.
                Return a negative value when an error is detected in the initStream/terminateStream callback function.
                

              Arguments:
                   srcp    Source address
                   destp   Destination address
                   tableBufp:     tree table storage buffer  (maximum 512 Bytes)
                                 When you wish to pass parameters to the initStream function of the MIReadStreamCallbacks structure, you can pass them through this buffer.
                                 
                                 However, after calling the initStream function, the tree table is overwritten.
                   callbacks:     address of the MIReadStreamCallbacks structure

              - Data header
                   u32 bitSize:4           1 data bit size (Normally 4|8)
                       compType:4          Compression type( = 2)
                       destSize:23         Data size after decompression
                       :1                  unusable

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
                      leftEndflag:1       Left node end flag
                                          When the end flag is set, there is data in the next node.
                                          

  Arguments:    srcp: Source address
                destp: destination address
                callbacks: address of stream callbacks structure

  Returns:      Uncompressed size >= 0
                error < 0

 *---------------------------------------------------------------------------*/
s32     SVC_UncompressHuffmanFromDevice(const void *srcp, void *destp, u8 *tableBufp,
                                        const MIReadStreamCallbacks *callbacks);

/*---------------------------------------------------------------------------*
  Name:         SVC_GetCRC16

  Description:  Calculates CRC-16.

              - Calculates CRC-16.
              - Align the data address and size to 2 Byte boundary.

              Arguments:
                   start:         initial value
                   datap:         data address
                   size:          size (number of bytes)

  Arguments:    start: Start value
                datap: Data address
                size: Data size (by byte)

  Returns:      CRC-16

 *---------------------------------------------------------------------------*/
u16     SVC_GetCRC16(u32 start, const void *datap, u32 size);

/*---------------------------------------------------------------------------*
  Name:         SVC_Div

  Description:  Quotient of division.

              - Computes the quotient of the numerator divided by the denominator.
              - Returns with register values of: r0=numer/denom, r1=numer%denom, r3=|numer/denom|.
                
              - Because code size is kept small, this is not very fast.

              Arguments:
                   numer:        numerator
                   denom:         denominator

  Arguments:    numer: 
                denom: 

  Returns:      Quotient.

 *---------------------------------------------------------------------------*/
s32     SVC_Div(s32 number, s32 denom);

/*---------------------------------------------------------------------------*
  Name:         SVC_DivRem

  Description:  remainder of division

              - Calculates numer%denom.
              - Returns with register values of: r0=numer%denom, r1=numer%denom, r3=|numer/denom|.
                
              - Because code size is kept small, this is not very fast.

              Arguments:
                   numer:        numerator
                   denom:         denominator

  Arguments:    numer: 
                denom: 

  Returns:      Remainder.

 *---------------------------------------------------------------------------*/
s32     SVC_DivRem(s32 number, s32 denom);

/*---------------------------------------------------------------------------*
  Name:         SVC_Sqrt

  Description:  Square root.

              - Calculates square root.
              - To improve accuracy, make the argument a multiple of 2 only, left shift it, and pass it, then also shift the return value to align the digits.
                
              - Because code size is kept small, this is not very fast.

  Arguments:    src: 

  Returns:      Square root.

 *---------------------------------------------------------------------------*/
u16     SVC_Sqrt(u32 src);

/*---------------------------------------------------------------------------*
  Name:         SVC_Halt

  Description:  Halt.

                - Only stops CPU core
                - The relevant interrupt returns by the permitted (permission set in IE) interrupt request (IF set).
                  
                -If the CPSR's IRQ disable flag is set (OS_DisableInterrupts), it will return from halt, but interrupts will not occur.
                  
                 
                If a halt is entered during a state with IME cleared (OS_DisableIrq), return will become impossible.
                  

  Arguments:    None.

  Returns:      None.

 *---------------------------------------------------------------------------*/
void    SVC_Halt(void);

#ifdef SDK_ARM7
/*---------------------------------------------------------------------------*
  Name:         SVC_Sleep

  Description:  Sleep.

              - Stops source oscillation.
              - When enabled (set in IE), interrupts for the RTC, key input, Game Cards, Game Paks, or system open events will restore the system state.
                
                
              - Because source oscillation has been stopped, the IF flag will not be set immediately after the system state is restored; instead, the flag will be set when the CPU is restarted if there had already been an interrupt request signal at the terminals.
                
                
              - Before calling this function, zero out the POWCNT registers for both processors and stop all blocks; stop the sound amp and wireless module; and set the ARM9 state to 'halt'.
                
                
              - Set the POWCNT register's LCD enable flag to 0 at least 100 ms before calling this function.
                 If you do not keep this condition, the system may shut down.
                

  Arguments:    None.

  Returns:      None.

 *---------------------------------------------------------------------------*/
void    SVC_Sleep(void);

/*---------------------------------------------------------------------------*
  Name:         SVC_SetSoundBias

  Description:  Sets the sound bias.

              - Moves sound BIAS from 0 to intermediate value (0x200).

              Arguments:
                   stepLoops:     Number of loops to one sound bias change step (4 cycles/loop).
                                 The higher the value, the more gradually sound bias is changed.

  Arguments:    stepLoops : 

  Returns:      None.

 *---------------------------------------------------------------------------*/
void    SVC_SetSoundBias(s32 stepLoops);

/*---------------------------------------------------------------------------*
  Name:         SVC_ResetSoundBias

  Description:  Sets the sound bias.

              - Changes sound BIAS from the intermediate value (0x200) to 0.

              Arguments:
                   stepLoops:     Number of loops to one sound bias change step (4 cycles/loop).
                                 The higher the value, the more gradually sound bias is changed.

  Arguments:    stepLoops: 

  Returns:      None.

 *---------------------------------------------------------------------------*/
void    SVC_ResetSoundBias(s32 stepLoops);

/*---------------------------------------------------------------------------*
  Name:         SVC_GetSinTable
                SVC_GetPitchTable
                SVC_GetVolumeTable

  Description:  Get sound table data.

              - References the sound function table and returns value.

              Arguments:
                   index:         index

  Arguments:    index: 

  Returns:      Sound table data.

 *---------------------------------------------------------------------------*/
s16     SVC_GetSinTable(int index);
u16     SVC_GetPitchTable(int index);
u8      SVC_GetVolumeTable(int index);
#endif // SDK_ARM7


#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_OS_SYSTEMCALL_H_ */
#endif
