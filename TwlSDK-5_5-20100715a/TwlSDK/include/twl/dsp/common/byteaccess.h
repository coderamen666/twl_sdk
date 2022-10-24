/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include - dsp - common
  File:     byteaccess.h

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-07-03#$
  $Rev: 10855 $
  $Author: yosizaki $
 *---------------------------------------------------------------------------*/
#ifndef TWL_DSP_BYTEACCESS_H_
#define TWL_DSP_BYTEACCESS_H_


#ifdef SDK_TWL
#include <twl/types.h>
#include <twl/os.h>
#else
#include <dsp/types.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*/
/* Declarations */

// Type definitions to use when sharing data during inter-processor communication.
typedef u16 DSPAddr;        // This type expresses an address in the DSP (2 bytes, 1 word)
typedef u16 DSPWord;        // This type expresses a size in the DSP (2 bytes, 1 word)
typedef u16 DSPByte;        // This type expresses a single-byte unit in the DSP (1 byte, 1 word)
typedef u32 DSPWord32;      // This type expresses a size in the DSP (2 bytes, 1 word)
typedef u32 DSPByte32;      // This type expresses a single-byte unit in the DSP (1 byte, 1 word)
typedef u32 DSPAddrInARM;   // This type has converted DSP addresses into bytes

// Explicit Type-Conversion Macros
#define DSP_ADDR_TO_ARM(address)    (u32)((address) << 1)
#define DSP_ADDR_TO_DSP(address)    (u16)((u32)(address) >> 1)
#define DSP_WORD_TO_ARM(word)       (u16)((word) << 1)
#define DSP_WORD_TO_DSP(word)       (u16)((word) >> 1)
#define DSP_WORD_TO_ARM32(word)     (u32)((word) << 1)
#define DSP_WORD_TO_DSP32(word)     (u32)((word) >> 1)
#define DSP_32BIT_TO_ARM(value)     (u32)(((u32)(value) >> 16) | ((u32)(value) << 16))
#define DSP_32BIT_TO_DSP(value)     (u32)(((u32)(value) >> 16) | ((u32)(value) << 16))
#ifdef SDK_TWL
#define DSP_BYTE_TO_UNIT(byte)      (u16)(byte)
#define DSP_UNIT_TO_BYTE(unit)      (u16)(unit)
#else
#define DSP_BYTE_TO_UNIT(byte)      (u16)((byte) >> 1)
#define DSP_UNIT_TO_BYTE(unit)      (u16)((unit) << 1)
#endif

// The native size for sizeof(char) (this is 2 on the DSP and 1 on an ARM processor)
#define DSP_WORD_UNIT       (3 - sizeof(DSPWord))


/*---------------------------------------------------------------------------*/
/* Functions */

#ifdef SDK_TWL

/*---------------------------------------------------------------------------*
  Name:         DSP_LoadWord

  Description:  Loads a single word (an even number of 16-bit units) from the DSP's data memory space.

  Arguments:    offset: The DSP address to transfer from (in words)

  Returns:      The loaded 16-bit data.
 *---------------------------------------------------------------------------*/
SDK_INLINE u16 DSP_LoadWord(DSPAddr offset)
{
    u16     value;
    OSIntrMode  cpsr = OS_DisableInterrupts();
    DSP_RecvFifo(DSP_FIFO_MEMSEL_DATA, &value, offset, DSP_WORD_TO_DSP(sizeof(u16)));
    reg_DSP_PCFG &= ~(REG_DSP_PCFG_DRS_MASK | REG_DSP_PCFG_AIM_MASK);
    (void)OS_RestoreInterrupts(cpsr);
    return value;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_StoreWord

  Description:  Writes a single word (an even number of 16-bit units) to the DSP's data memory space.

  Arguments:    offset: The DSP address to transfer to (in words)
                value: The word value to write

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void DSP_StoreWord(DSPAddr offset, u16 value)
{
    OSIntrMode  cpsr = OS_DisableInterrupts();
    DSP_SendFifo(DSP_FIFO_MEMSEL_DATA, offset, &value, DSP_WORD_TO_DSP(sizeof(u16)));
//    reg_DSP_PCFG &= ~(REG_DSP_PCFG_DRS_MASK | REG_DSP_PCFG_AIM_MASK);
    (void)OS_RestoreInterrupts(cpsr);
}

/*---------------------------------------------------------------------------*
  Name:         DSP_Load8

  Description:  Loads 8 bits from the DSP's data memory space.

  Arguments:    offset: The DSP address to transfer from (in bytes)

  Returns:      The loaded 8-bit data.
 *---------------------------------------------------------------------------*/
SDK_INLINE u8 DSP_Load8(DSPAddrInARM offset)
{
    return (u8)(DSP_LoadWord(DSP_WORD_TO_DSP(offset)) >> ((offset & 1) << 3));
}

/*---------------------------------------------------------------------------*
  Name:         DSP_Load16

  Description:  Loads 16 bits from the DSP's data memory space.

  Arguments:    offset: The DSP address to transfer from (in bytes)

  Returns:      The loaded 16-bit data.
 *---------------------------------------------------------------------------*/
u16     DSP_Load16(DSPAddrInARM offset);

/*---------------------------------------------------------------------------*
  Name:         DSP_Load32

  Description:  Loads 32 bits from the DSP's data memory space.

  Arguments:    offset: The DSP address to transfer from (in bytes)

  Returns:      The loaded 32-bit data.
 *---------------------------------------------------------------------------*/
u32     DSP_Load32(DSPAddrInARM offset);

/*---------------------------------------------------------------------------*
  Name:         DSP_LoadData

  Description:  Loads data of an arbitrary length from the DSP's data memory space.

  Arguments:    offset: The DSP address to transfer from (in bytes)
                buffer: Buffer to transfer data to
                length: Size of the transmission

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    DSP_LoadData(DSPAddrInARM offset, void *buffer, u32 length);

/*---------------------------------------------------------------------------*
  Name:         DSP_Store8

  Description:  Writes 8 bits to the DSP's data memory space.

  Arguments:    offset: The DSP address to transfer to (in bytes)
                value: The 8-bit value to write

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    DSP_Store8(DSPAddrInARM offset, u8 value);

/*---------------------------------------------------------------------------*
  Name:         DSP_Store16

  Description:  Writes 16 bits to the DSP's data memory space.

  Arguments:    offset: The DSP address to transfer to (in bytes)
                value: The 16-bit value to write

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    DSP_Store16(DSPAddrInARM offset, u16 value);

/*---------------------------------------------------------------------------*
  Name:         DSP_Store32

  Description:  Writes 32 bits to the DSP's data memory space.

  Arguments:    offset: The DSP address to transfer to (in bytes)
                value: The 32-bit value to write

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    DSP_Store32(DSPAddrInARM offset, u32 value);

/*---------------------------------------------------------------------------*
  Name:         DSP_StoreData

  Description:  Writes data of an arbitrary length to the DSP's data memory space.

  Arguments:    offset: The DSP address to transfer to (in bytes)
                buffer: Buffer to transfer data from
                length: Size of the transmission

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    DSP_StoreData(DSPAddrInARM offset, const void *buffer, u32 length);


#endif // SDK_TWL


/*===========================================================================*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TWL_DSP_BYTEACCESS_H_ */
