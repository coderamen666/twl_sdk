/*---------------------------------------------------------------------------*
  Project:  TwlSDK - library - dsp
  File:     dsp_byteaccess.c

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <twl/dsp.h>
#include <twl/dsp/common/byteaccess.h>


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         DSP_Load16

  Description:  Reads 16 bits from the DSP data memory space.

  Arguments:    offset: Transfer source DSP address (in bytes)

  Returns:      The 16-bit data that was read.
 *---------------------------------------------------------------------------*/
u16 DSP_Load16(DSPAddrInARM offset)
{
    OSIntrMode  cpsr = OS_DisableInterrupts();
    u16     result = DSP_LoadWord(DSP_WORD_TO_DSP(offset));
    if ((offset & 1) != 0)
    {
        result = (u16)((DSP_LoadWord(DSP_WORD_TO_DSP(offset + 2)) << (16 - 8)) | (result >> 8));
    }
    (void)OS_RestoreInterrupts(cpsr);
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_Load32

  Description:  Reads 32 bits from the DSP data memory space.

  Arguments:    offset: Transfer source DSP address (in bytes)

  Returns:      The 32-bit data that was read.
 *---------------------------------------------------------------------------*/
u32 DSP_Load32(DSPAddrInARM offset)
{
    u32     result = (u32)((DSP_LoadWord(DSP_WORD_TO_DSP(offset + 0)) << 0) |
                           (DSP_LoadWord(DSP_WORD_TO_DSP(offset + 2)) << 16));
    if ((offset & 1) != 0)
    {
        result = (u32)((DSP_LoadWord(DSP_WORD_TO_DSP(offset + 4)) << (32 - 8)) | (result >> 8));
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_LoadData

  Description:  Reads an arbitrary length of data from the DSP data memory space.

  Arguments:    offset: Transfer source DSP address (in bytes)
                buffer: Transfer destination buffer
                length: Size of the transmission

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_LoadData(DSPAddrInARM offset, void *buffer, u32 length)
{
    if (length > 0)
    {
        // First, align leading end of transfer source.
        if ((offset & 1) != 0)
        {
            MI_StoreLE8(buffer, DSP_Load8(offset));
            buffer = (u8 *)buffer + 1;
            offset += 1;
            length -= 1;
        }
        // If the buffer is aligned, transmit the word as is; if not aligned, repeat word tranmissions repeatedly, although this is inefficient.
        // 
        // (This support can be implemented in the DSP_RecvFifo function, right?)
        if (((u32)buffer & 1) == 0)
        {
            u32     aligned = (u32)(length & ~1);
            if (aligned > 0)
            {
                OSIntrMode  cpsr = OS_DisableInterrupts();
                DSP_RecvFifo(DSP_FIFO_MEMSEL_DATA,
                             (u16*)buffer,
                             DSP_WORD_TO_DSP(offset),
                             DSP_WORD_TO_DSP(aligned));
                reg_DSP_PCFG &= ~(REG_DSP_PCFG_DRS_MASK | REG_DSP_PCFG_AIM_MASK);
                (void)OS_RestoreInterrupts(cpsr);
                buffer = (u8 *)buffer + aligned;
                offset += aligned; 
                length -= aligned;
            }
        }
        else
        {
            while (length >= 2)
            {
                MI_StoreLE16(buffer, DSP_LoadWord(DSP_WORD_TO_DSP(offset)));
                buffer = (u8 *)buffer + 2;
                offset += 2;
                length -= 2;
            }
        }
        // If necessary, load the end
        if (length > 0)
        {
            MI_StoreLE8(buffer, DSP_Load8(offset));
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_Store8

  Description:  Writes 8 bits to the DSP data memory space.

  Arguments:    offset: Transfer source DSP address (in bytes)
                value: The 8-bit value to write

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_Store8(DSPAddrInARM offset, u8 value)
{
    if ((offset & 1) == 0)
    {
        u16     tmp = DSP_LoadWord(DSP_WORD_TO_DSP(offset));
        tmp = (u16)((tmp & 0xFF00) | ((value << 0) & 0x00FF));
        DSP_StoreWord(DSP_WORD_TO_DSP(offset), tmp);
    }
    else
    {
        u16     tmp = DSP_LoadWord(DSP_WORD_TO_DSP(offset));
        tmp = (u16)((tmp & 0x00FF) | ((value << 8) & 0xFF00));
        DSP_StoreWord(DSP_WORD_TO_DSP(offset), tmp);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_Store16

  Description:  Writes 16 bits to the DSP data memory space.

  Arguments:    offset: Transfer source DSP address (in bytes)
                value: The 16-bit value to write

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_Store16(DSPAddrInARM offset, u16 value)
{
    if ((offset & 1) == 0)
    {
        DSP_StoreWord(DSP_WORD_TO_DSP(offset), value);
    }
    else
    {
        DSP_Store8(offset + 0, (u8)(value >> 0));
        DSP_Store8(offset + 1, (u8)(value >> 8));
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_Store32

  Description:  Writes 32 bits to the DSP data memory space.

  Arguments:    offset: Transfer source DSP address (in bytes)
                value: The 32-bit value to write

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_Store32(DSPAddrInARM offset, u32 value)
{
    if ((offset & 1) == 0)
    {
        DSP_StoreWord(DSP_WORD_TO_DSP(offset + 0), (u16)(value >> 0));
        DSP_StoreWord(DSP_WORD_TO_DSP(offset + 2), (u16)(value >> 16));
    }
    else
    {
        DSP_Store8(offset + 0, (u8)(value >> 0));
        DSP_StoreWord(DSP_WORD_TO_DSP(offset + 1), (u16)(value >> 8));
        DSP_Store8(offset + 3, (u8)(value >> 24));
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_StoreData

  Description:  Writes arbitrary length data to the DSP data memory space.

  Arguments:    offset: Transfer source DSP address (in bytes)
                buffer: Transfer source buffer
                length: Size of the transmission

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_StoreData(DSPAddrInARM offset, const void *buffer, u32 length)
{
    if (length > 0)
    {
        // First, align leading end of transfer destination
        if ((offset & 1) != 0)
        {
            DSP_Store8(offset, MI_LoadLE8(buffer));
            buffer = (const u8 *)buffer + 1;
            offset += 1;
            length -= 1;
        }
        // If the buffer is aligned, transmit the word as is; if not aligned, repeat word tranmissions repeatedly, although this is inefficient.
        // 
        // (This support can be implemented in the DSP_SendFifo function, right?)
        if (((u32)buffer & 1) == 0)
        {
            u32     aligned = (u32)(length & ~1);
            if (aligned > 0)
            {
                OSIntrMode  cpsr = OS_DisableInterrupts();
                DSP_SendFifo(DSP_FIFO_MEMSEL_DATA,
                             DSP_WORD_TO_DSP(offset),
                             (const u16 *)buffer,
                             DSP_WORD_TO_DSP(aligned));
            //    reg_DSP_PCFG &= ~(REG_DSP_PCFG_DRS_MASK | REG_DSP_PCFG_AIM_MASK);
                (void)OS_RestoreInterrupts(cpsr);
                buffer = (const u8 *)buffer + aligned;
                offset += aligned; 
                length -= aligned;
            }
        }
        else
        {
            while (length >= 2)
            {
                DSP_StoreWord(DSP_WORD_TO_DSP(offset), MI_LoadLE16(buffer));
                buffer = (const u8 *)buffer + 2;
                offset += 2;
                length -= 2;
            }
        }
        // If necessary, write the end
        if (length > 0)
        {
            DSP_Store8(offset, MI_LoadLE8(buffer));
        }
    }
}
