/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraries - spi
  File:     spi.h

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef LIBRARIES_SPI_H_
#define LIBRARIES_SPI_H_

#include    <nitro/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/

/*---------------------------------------------------------------------------*
    Inline Function Definitions
    Temporarily defined for ARM9.
    Should be defined in ARM9 library as static functions.
 *---------------------------------------------------------------------------*/
#ifdef  SDK_ARM9

/*---------------------------------------------------------------------------*
  Name:         SPI_TpSamplingNow

  Description:  Samples touch panel once.

  Arguments:    None.

  Returns:      BOOL - Returns TRUE when the command was successfully sent via PXI and FALSE on failure.
                       
 *---------------------------------------------------------------------------*/
static inline BOOL SPI_TpSamplingNow(void)
{
    // Send packet [0]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_TOUCHPANEL,
                               SPI_PXI_START_BIT |
                               SPI_PXI_END_BIT |
                               (0 << SPI_PXI_INDEX_SHIFT) | (SPI_PXI_COMMAND_TP_SAMPLING << 8), 0))
    {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         SPI_TpAutoSamplingOn

  Description:  Starts auto-sampling of touch panel.

  Arguments:    vCount -    V Count to carry out sampling.
                            When sampling multiple times per frame, a single frame's time divisions will start here.
                            
                frequency - Frequency of sampling for 1 frame.

  Returns:      BOOL - Returns TRUE when the command was successfully sent via PXI and FALSE on failure.
                       
 *---------------------------------------------------------------------------*/
static inline BOOL SPI_TpAutoSamplingOn(u16 vCount, u8 frequency)
{
    // Send packet [0]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_TOUCHPANEL,
                               SPI_PXI_START_BIT |
                               (0 << SPI_PXI_INDEX_SHIFT) |
                               (SPI_PXI_COMMAND_TP_AUTO_ON << 8) | (u32)frequency, 0))
    {
        return FALSE;
    }

    // Send packet [1]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_TOUCHPANEL,
                               SPI_PXI_END_BIT | (1 << SPI_PXI_INDEX_SHIFT) | (u32)vCount, 0))
    {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         SPI_TpAutoSamplingOff

  Description:  Stops auto-sampling of touch panel.

  Arguments:    None.

  Returns:      BOOL - Returns TRUE when the command was successfully sent via PXI and FALSE on failure.
                       
 *---------------------------------------------------------------------------*/
static inline BOOL SPI_TpAutoSamplingOff(void)
{
    // Send packet [0]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_TOUCHPANEL,
                               SPI_PXI_START_BIT |
                               SPI_PXI_END_BIT |
                               (0 << SPI_PXI_INDEX_SHIFT) | (SPI_PXI_COMMAND_TP_AUTO_OFF << 8), 0))
    {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         SPI_TpSetupStability

  Description:  Sets stability determination parameters for sampling

  Arguments:    range - For continuous sampling, error for which detected voltage is considered stable.
                        The detection value is 12 bits, 0 - 4095.

  Returns:      BOOL - Returns TRUE when the command was successfully sent via PXI and FALSE on failure.
                       
 *---------------------------------------------------------------------------*/
static inline BOOL SPI_TpSetupStability(u16 range)
{
    // Send packet [0]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_TOUCHPANEL,
                               SPI_PXI_START_BIT |
                               SPI_PXI_END_BIT |
                               (0 << SPI_PXI_INDEX_SHIFT) |
                               (SPI_PXI_COMMAND_TP_SETUP_STABILITY << 8) | (u32)range, 0))
    {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         SPI_NvramWriteEnable

  Description:  Issues "write permitted" instruction to NVRAM.

  Arguments:    None.

  Returns:      BOOL - Returns TRUE when the command was successfully sent via PXI and FALSE on failure.
                       
 *---------------------------------------------------------------------------*/
static inline BOOL SPI_NvramWriteEnable(void)
{
    // Send packet [0]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               SPI_PXI_START_BIT |
                               SPI_PXI_END_BIT |
                               (0 << SPI_PXI_INDEX_SHIFT) | (SPI_PXI_COMMAND_NVRAM_WREN << 8), 0))
    {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         SPI_NvramWriteDisable

  Description:  Issues "write prohibited" instruction to NVRAM.

  Arguments:    None.

  Returns:      BOOL - Returns TRUE when the command was successfully sent via PXI and FALSE on failure.
                       
 *---------------------------------------------------------------------------*/
static inline BOOL SPI_NvramWriteDisable(void)
{
    // Send packet [0]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               SPI_PXI_START_BIT |
                               SPI_PXI_END_BIT |
                               (0 << SPI_PXI_INDEX_SHIFT) | (SPI_PXI_COMMAND_NVRAM_WRDI << 8), 0))
    {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         SPI_NvramReadStatusRegister

  Description:  Issues "status register read" command to NVRAM.

  Arguments:    pData - Pointer to variable storing read value.
                        The ARM7 directly writes the value so be careful of the cache.

  Returns:      BOOL - Returns TRUE when the command was successfully sent via PXI and FALSE on failure.
                       
 *---------------------------------------------------------------------------*/
static inline BOOL SPI_NvramReadStatusRegister(u8 *pData)
{
    // Send packet [0]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               SPI_PXI_START_BIT |
                               (0 << SPI_PXI_INDEX_SHIFT) |
                               (SPI_PXI_COMMAND_NVRAM_RDSR << 8) | ((u32)pData >> 24), 0))
    {
        return FALSE;
    }

    // Send packet [1]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               (1 << SPI_PXI_INDEX_SHIFT) | (((u32)pData >> 8) & 0x0000ffff), 0))
    {
        return FALSE;
    }

    // Send packet [2]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               SPI_PXI_END_BIT |
                               (2 << SPI_PXI_INDEX_SHIFT) | (((u32)pData << 8) & 0x0000ff00), 0))
    {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         SPI_NvramReadDataBytes

  Description:  Issues a read command to NVRAM.

  Arguments:    address - Read start address on NVRAM. Only 24 bits valid.
                size -    Byte size to consecutively read.
                pData - Pointer to array storing read value.
                          The ARM7 directly writes the value so be careful of the cache.

  Returns:      BOOL - Returns TRUE when the command was successfully sent via PXI and FALSE on failure.
                       
 *---------------------------------------------------------------------------*/
static inline BOOL SPI_NvramReadDataBytes(u32 address, u32 size, u8 *pData)
{
    // Send packet [0]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               SPI_PXI_START_BIT |
                               (0 << SPI_PXI_INDEX_SHIFT) |
                               (SPI_PXI_COMMAND_NVRAM_READ << 8) |
                               ((address >> 16) & 0x000000ff), 0))
    {
        return FALSE;
    }

    // Send packet [1]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               (1 << SPI_PXI_INDEX_SHIFT) | (address & 0x0000ffff), 0))
    {
        return FALSE;
    }

    // Send packet [2]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM, (2 << SPI_PXI_INDEX_SHIFT) | (size >> 16), 0))
    {
        return FALSE;
    }

    // Send packet [3]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               (3 << SPI_PXI_INDEX_SHIFT) | (size & 0x0000ffff), 0))
    {
        return FALSE;
    }

    // Send packet [4]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               (4 << SPI_PXI_INDEX_SHIFT) | ((u32)pData >> 16), 0))
    {
        return FALSE;
    }

    // Send packet [5]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               SPI_PXI_END_BIT |
                               (5 << SPI_PXI_INDEX_SHIFT) | ((u32)pData & 0x0000ffff), 0))
    {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         SPI_NvramReadHigherSpeed

  Description:  Issues 'high speed read' command to NVRAM.
                After a 'high speed read' command is issued, ticks from an 8-bit dummy clock are used before data is read. This allows you to read with a clock as fast as 25 MHz.
                
                However, the SPI clock maximum is up to 4 MHz so using it there is meaningless.
                The normal 'read' command supports clocks up to a maximum of 20 MHz.
                The description above applies to M45PE40 devices. For the LE25FW203T, both a 'read' and a 'high-speed read' operate at 30 MHz.
                

  Arguments:    address - Read start address on NVRAM. Only 24 bits valid.
                size -    Byte size to consecutively read.
                pData - Pointer to array storing read value.
                          The ARM7 directly writes the value so be careful of the cache.

  Returns:      BOOL - Returns TRUE when the command was successfully sent via PXI and FALSE on failure.
                       
 *---------------------------------------------------------------------------*/
static inline BOOL SPI_NvramReadHigherSpeed(u32 address, u32 size, u8 *pData)
{
    // Send packet [0]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               SPI_PXI_START_BIT |
                               (0 << SPI_PXI_INDEX_SHIFT) |
                               (SPI_PXI_COMMAND_NVRAM_FAST_READ << 8) |
                               ((address >> 16) & 0x000000ff), 0))
    {
        return FALSE;
    }

    // Send packet [1]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               (1 << SPI_PXI_INDEX_SHIFT) | (address & 0x0000ffff), 0))
    {
        return FALSE;
    }

    // Send packet [2]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM, (2 << SPI_PXI_INDEX_SHIFT) | (size >> 16), 0))
    {
        return FALSE;
    }

    // Send packet [3]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               (3 << SPI_PXI_INDEX_SHIFT) | (size & 0x0000ffff), 0))
    {
        return FALSE;
    }

    // Send packet [4]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               (4 << SPI_PXI_INDEX_SHIFT) | ((u32)pData >> 16), 0))
    {
        return FALSE;
    }

    // Send packet [5]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               SPI_PXI_END_BIT |
                               (5 << SPI_PXI_INDEX_SHIFT) | ((u32)pData & 0x0000ffff), 0))
    {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         SPI_NvramPageWrite

  Description:  Issues 'page write' command to NVRAM.
                In NVRAM, a 'page erase' and a 'conditional page write' occur in succession.
                

  Arguments:    address - Write start address on NVRAM. Only 24 bits valid.
                size -    Consecutively written byte size.
                          If the sum of address and size exceeds the page boundary (256 bytes), the excess data is ignored.
                          
                pData -   Array where written value is stored.
                          The ARM7 loads directly, so you must ensure that data is written from the cache to memory in advance.
                          

  Returns:      BOOL - Returns TRUE when the command was successfully sent via PXI and FALSE on failure.
                       
 *---------------------------------------------------------------------------*/
static inline BOOL SPI_NvramPageWrite(u32 address, u16 size, const u8 *pData)
{
    // Send packet [0]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               SPI_PXI_START_BIT |
                               (0 << SPI_PXI_INDEX_SHIFT) |
                               (SPI_PXI_COMMAND_NVRAM_PW << 8) | ((address >> 16) & 0x000000ff), 0))
    {
        return FALSE;
    }

    // Send packet [1]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               (1 << SPI_PXI_INDEX_SHIFT) | (address & 0x0000ffff), 0))
    {
        return FALSE;
    }

    // Send packet [2]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM, (2 << SPI_PXI_INDEX_SHIFT) | (u32)size, 0))
    {
        return FALSE;
    }

    // Send packet [3]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               (3 << SPI_PXI_INDEX_SHIFT) | ((u32)pData >> 16), 0))
    {
        return FALSE;
    }

    // Send packet [4]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               SPI_PXI_END_BIT |
                               (4 << SPI_PXI_INDEX_SHIFT) | ((u32)pData & 0x0000ffff), 0))
    {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         SPI_NvramPageWrite

  Description:  Issues "page write (conditional)" command to NVRAM.
                Writes overwhelmingly faster than normal "page write", but the only operation you can do is lower the bit.
                 The '0' bit stays a '0' even if a '1' is written to it.

  Arguments:    address - Write start address on NVRAM. Only 24 bits valid.
                size -    Consecutively written byte size.
                          If the sum of address and size exceeds the page boundary (256 bytes), the excess data is ignored.
                          
                pData -   Array where written value is stored.
                          The ARM7 loads directly, so you must ensure that data is written from the cache to memory in advance.
                          

  Returns:      BOOL - Returns TRUE when the command was successfully sent via PXI and FALSE on failure.
                       
 *---------------------------------------------------------------------------*/
static inline BOOL SPI_NvramPageProgram(u32 address, u16 size, const u8 *pData)
{
    // Send packet [0]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               SPI_PXI_START_BIT |
                               (0 << SPI_PXI_INDEX_SHIFT) |
                               (SPI_PXI_COMMAND_NVRAM_PP << 8) | ((address >> 16) & 0x000000ff), 0))
    {
        return FALSE;
    }

    // Send packet [1]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               (1 << SPI_PXI_INDEX_SHIFT) | (address & 0x0000ffff), 0))
    {
        return FALSE;
    }

    // Send packet [2]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM, (2 << SPI_PXI_INDEX_SHIFT) | (u32)size, 0))
    {
        return FALSE;
    }

    // Send packet [3]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               (3 << SPI_PXI_INDEX_SHIFT) | ((u32)pData >> 16), 0))
    {
        return FALSE;
    }

    // Send packet [4]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               SPI_PXI_END_BIT |
                               (4 << SPI_PXI_INDEX_SHIFT) | ((u32)pData & 0x0000ffff), 0))
    {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         SPI_NvramPageErase

  Description:  Issues "page erase" instruction to NVRAM.
                All bits for erased page are all '1'.

  Arguments:    address - Address on NVRAM of page erased. Only 24 bits valid.
                          All of the 256 bytes of the page included in this address are erased.
                          In NVRAM, it is thought that an address with the lower 8 bits cleared will be the address to erase.
                          

  Returns:      BOOL - Returns TRUE when the command was successfully sent via PXI and FALSE on failure.
                       
 *---------------------------------------------------------------------------*/
static inline BOOL SPI_NvramPageErase(u32 address)
{
    // Send packet [0]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               SPI_PXI_START_BIT |
                               (0 << SPI_PXI_INDEX_SHIFT) |
                               (SPI_PXI_COMMAND_NVRAM_PE << 8) | ((address >> 16) & 0x000000ff), 0))
    {
        return FALSE;
    }

    // Send packet [1]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               SPI_PXI_END_BIT |
                               (1 << SPI_PXI_INDEX_SHIFT) | (address & 0x0000ffff), 0))
    {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         SPI_NvramSectorErase

  Description:  Issues "sector erase" instruction to NVRAM.
                All bits for erased sector are all '1'.

  Arguments:    address - Address on NVRAM of sector erased. Only 24 bits valid.
                          All of the 64 bytes of the sector included in this address are erased.
                          In NVRAM, it is thought that an address with the lower 16 bits cleared will be the address to erase.
                          

  Returns:      BOOL - Returns TRUE when the command was successfully sent via PXI and FALSE on failure.
                       
 *---------------------------------------------------------------------------*/
static inline BOOL SPI_NvramSectorErase(u32 address)
{
    // Send packet [0]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               SPI_PXI_START_BIT |
                               (0 << SPI_PXI_INDEX_SHIFT) |
                               (SPI_PXI_COMMAND_NVRAM_SE << 8) | ((address >> 16) & 0x000000ff), 0))
    {
        return FALSE;
    }

    // Send packet [1]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               SPI_PXI_END_BIT |
                               (1 << SPI_PXI_INDEX_SHIFT) | (address & 0x0000ffff), 0))
    {
        return FALSE;
    }

    return TRUE;
}

/* Use prohibited in conjunction with introduction of new model number device (2007/4/3 terui@nintendo.co.jp) */
#if 0
/*---------------------------------------------------------------------------*
  Name:         SPI_NvramPowerDown

  Description:  Issues "power save" instruction to NVRAM.
                Will not receive instruction other than "return from power save".

  Arguments:    None.

  Returns:      BOOL - Returns TRUE when the command was successfully sent via PXI and FALSE on failure.
                       
 *---------------------------------------------------------------------------*/
static inline BOOL SPI_NvramPowerDown(void)
{
    // Send packet [0]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               SPI_PXI_START_BIT |
                               SPI_PXI_END_BIT |
                               (0 << SPI_PXI_INDEX_SHIFT) | (SPI_PXI_COMMAND_NVRAM_DP << 8), 0))
    {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         SPI_NvramReleasePowerDown

  Description:  Issues "return from power save" instruction to NVRAM.
                This is ignored if the system is not in power-save mode.

  Arguments:    None.

  Returns:      BOOL - Returns TRUE when the command was successfully sent via PXI and FALSE on failure.
                       
 *---------------------------------------------------------------------------*/
static inline BOOL SPI_NvramReleasePowerDown(void)
{
    // Send packet [0]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               SPI_PXI_START_BIT |
                               SPI_PXI_END_BIT |
                               (0 << SPI_PXI_INDEX_SHIFT) | (SPI_PXI_COMMAND_NVRAM_RDP << 8), 0))
    {
        return FALSE;
    }

    return TRUE;
}
#endif

/*---------------------------------------------------------------------------*
  Name:         SPI_NvramChipErase

  Description:  Issues "chip erase" command to NVRAM.
                This is enabled for LE25FW203T devices.
                Ignored when device is M45PE40.

  Arguments:    None.

  Returns:      BOOL - Returns TRUE when the command was successfully sent via PXI and FALSE on failure.
                       
 *---------------------------------------------------------------------------*/
static inline BOOL SPI_NvramChipErase(void)
{
    // Send packet [0]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               SPI_PXI_START_BIT |
                               SPI_PXI_END_BIT |
                               (0 << SPI_PXI_INDEX_SHIFT) | (SPI_PXI_COMMAND_NVRAM_CE << 8), 0))
    {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         SPI_NvramReadSiliconId

  Description:  Issues "read silicon ID" command to NVRAM.
                This is enabled for LE25FW203T devices.
                Ignored when device is M45PE40.

  Arguments:    pData - Specifies a 2-byte buffer for storing the silicon ID.
                         The ARM7 directly writes the value so be careful of the cache.

  Returns:      BOOL - Returns TRUE when the command was successfully sent via PXI and FALSE on failure.
                        
 *---------------------------------------------------------------------------*/
static inline BOOL SPI_NvramReadSiliconId(u8 *pData)
{
    // Send packet [0]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               SPI_PXI_START_BIT |
                               (0 << SPI_PXI_INDEX_SHIFT) |
                               (SPI_PXI_COMMAND_NVRAM_RSI << 8) | ((u32)pData >> 24), 0))
    {
        return FALSE;
    }

    // Send packet [1]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               (1 << SPI_PXI_INDEX_SHIFT) | (((u32)pData >> 8) & 0x0000ffff), 0))
    {
        return FALSE;
    }

    // Send packet [2]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               SPI_PXI_END_BIT |
                               (2 << SPI_PXI_INDEX_SHIFT) | (((u32)pData << 8) & 0x0000ff00), 0))
    {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         SPI_NvramSoftwareReset

  Description:  Issues "software reset" command to NVRAM.
                This is enabled for LE25FW203T devices.
                Ignored when device is M45PE40.

  Arguments:    None.

  Returns:      BOOL - Returns TRUE when the command was successfully sent via PXI and FALSE on failure.
                       
 *---------------------------------------------------------------------------*/
static inline BOOL SPI_NvramSoftwareReset(void)
{
    // Send packet [0]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               SPI_PXI_START_BIT |
                               SPI_PXI_END_BIT |
                               (0 << SPI_PXI_INDEX_SHIFT) | (SPI_PXI_COMMAND_NVRAM_SR << 8), 0))
    {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         SPI_MicSamplingNow

  Description:  Samples the microphone once.

  Arguments:    type  - sampling type (0: 8-bit, 1: 12-bit)
                pData - Memory address that stores sampling results
                   -> Delete

  Returns:      BOOL - Returns TRUE when the command was successfully sent via PXI and FALSE on failure.
                       
 *---------------------------------------------------------------------------*/
static inline BOOL SPI_MicSamplingNow(u8 type)
{
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
                               SPI_PXI_START_BIT |
                               SPI_PXI_END_BIT |
                               (0 << SPI_PXI_INDEX_SHIFT) |
                               (SPI_PXI_COMMAND_MIC_SAMPLING << 8) | (u32)type, 0))
    {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         SPI_MicAutoSamplingOn

  Description:  Starts auto-sampling with the microphone.

  Arguments:    pData    - Buffer address that stores sampled data
                size     - Buffer size. Specified in byte units.
                span     - sampling interval (specify with ARM7 CPU clock)
                           Timer characteristics allow only 1, 64, 256, or 1024 times 16 bits to be be set accurately.
                            Bits at the end are truncated.
                middle   - Sampling times that returns response during process.
                           -> Delete
                adMode   - specify AD conversion bit ( 8 or 12 ).
                loopMode - Auto-sampling loop control determination.
                correct  - Tick correction determination during auto-sampling.

  Returns:      BOOL - Returns TRUE when the command was successfully sent via PXI and FALSE on failure.
                       
 *---------------------------------------------------------------------------*/
static inline BOOL SPI_MicAutoSamplingOn(void *pData, u32 size, u32 span,
//    u32     middle ,
                                         u8 adMode, BOOL loopMode, BOOL correct)
{
    u8      temp;

    // Combine flag types in "type" variable
    switch (adMode)                    // AD conversion bit width specification
    {
    case SPI_MIC_SAMPLING_TYPE_8BIT:
    case SPI_MIC_SAMPLING_TYPE_12BIT:
        temp = adMode;
        break;
    default:
        return FALSE;
    }
    if (loopMode)                      // Continuous sampling loop specification
    {
        temp |= (u8)SPI_MIC_SAMPLING_TYPE_LOOP_ON;
    }
    if (correct)                       // Apply correction logic specification due to Tick error
    {
        temp |= (u8)SPI_MIC_SAMPLING_TYPE_CORRECT_ON;
    }

    // Send packet [0]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
                               SPI_PXI_START_BIT |
                               (0 << SPI_PXI_INDEX_SHIFT) |
                               (SPI_PXI_COMMAND_MIC_AUTO_ON << 8) | (u32)temp, 0))
    {
        return FALSE;
    }

    // Send packet [1]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
                               (1 << SPI_PXI_INDEX_SHIFT) | ((u32)pData >> 16), 0))
    {
        return FALSE;
    }

    // Send packet [2]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
                               (2 << SPI_PXI_INDEX_SHIFT) | ((u32)pData & 0x0000ffff), 0))
    {
        return FALSE;
    }

    // Send packet [3]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC, (3 << SPI_PXI_INDEX_SHIFT) | (size >> 16), 0))
    {
        return FALSE;
    }

    // Send packet [4]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
                               (4 << SPI_PXI_INDEX_SHIFT) | (size & 0x0000ffff), 0))
    {
        return FALSE;
    }

    // Send packet [5]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC, (5 << SPI_PXI_INDEX_SHIFT) | (span >> 16), 0))
    {
        return FALSE;
    }

    // Send packet [6]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
                               SPI_PXI_END_BIT |
                               (6 << SPI_PXI_INDEX_SHIFT) | (span & 0x0000ffff), 0))
    {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         SPI_MicAutoSamplingOff

  Description:  Stops auto-sampling with the microphone.
                If loop playback was not specified at the beginning, it will stop automatically when the buffer is full.
                

  Arguments:    None.

  Returns:      BOOL - Returns TRUE when the command was successfully sent via PXI and FALSE on failure.
                       
 *---------------------------------------------------------------------------*/
static inline BOOL SPI_MicAutoSamplingOff(void)
{
    // Send packet [0]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
                               SPI_PXI_START_BIT |
                               SPI_PXI_END_BIT |
                               (0 << SPI_PXI_INDEX_SHIFT) | (SPI_PXI_COMMAND_MIC_AUTO_OFF << 8), 0))
    {
        return FALSE;
    }

    return TRUE;
}

#endif /* SDK_ARM9 */

/*===========================================================================*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LIBRARIES_SPI_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
