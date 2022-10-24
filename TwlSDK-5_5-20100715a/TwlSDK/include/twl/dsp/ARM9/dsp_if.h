/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include - dsp
  File:     dsp_if.h

  Copyright 2007-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef TWL_DSP_IF_H_
#define TWL_DSP_IF_H_

#include <twl/types.h>
#include <twl/hw/ARM9/ioreg_DSP.h>
#include <nitro/os/common/emulator.h>

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/

/*---------------------------------------------------------------------------*
    Constant definitions
 *---------------------------------------------------------------------------*/
typedef enum
{
    DSP_FIFO_MEMSEL_DATA    = (0x0 << REG_DSP_PCFG_MEMSEL_SHIFT),
    DSP_FIFO_MEMSEL_MMIO    = (0x1 << REG_DSP_PCFG_MEMSEL_SHIFT),
    DSP_FIFO_MEMSEL_PROGRAM = (0x5 << REG_DSP_PCFG_MEMSEL_SHIFT)
}
DSPFifoMemSel;

typedef enum
{
    DSP_FIFO_RECV_2B            = (0x0 << REG_DSP_PCFG_DRS_SHIFT),
    DSP_FIFO_RECV_16B           = (0x1 << REG_DSP_PCFG_DRS_SHIFT),
    DSP_FIFO_RECV_32B           = (0x2 << REG_DSP_PCFG_DRS_SHIFT),
    DSP_FIFO_RECV_CONTINUOUS    = (0x3 << REG_DSP_PCFG_DRS_SHIFT)
}
DSPFifoRecvLength;

typedef enum
{
    DSP_FIFO_INTR_SEND_EMPTY        = REG_DSP_PCFG_WFEIE_MASK,
    DSP_FIFO_INTR_SEND_FULL         = REG_DSP_PCFG_WFFIE_MASK,
    DSP_FIFO_INTR_RECV_NOT_EMPTY    = REG_DSP_PCFG_RFNEIE_MASK,
    DSP_FIFO_INTR_RECV_FULL         = REG_DSP_PCFG_RFFIE_MASK
}
DSPFifoIntr;

// For complex API
typedef enum
{
    DSP_FIFO_FLAG_SRC_INC   = (0UL << 0),
    DSP_FIFO_FLAG_SRC_FIX   = (1UL << 0),

    DSP_FIFO_FLAG_DEST_INC  = (0UL << 1),
    DSP_FIFO_FLAG_DEST_FIX  = (1UL << 1),

    DSP_FIFO_FLAG_RECV_UNIT_CONTINUOUS  = (0UL << 8),
    DSP_FIFO_FLAG_RECV_UNIT_2B          = (1UL << 8),
    DSP_FIFO_FLAG_RECV_UNIT_16B         = (2UL << 8),
    DSP_FIFO_FLAG_RECV_UNIT_32B         = (3UL << 8),
    DSP_FIFO_FLAG_RECV_MASK             = (3UL << 8)
}
DSPFifoFlag;

// Support the load process for old specifications a little longer.
#define DSP_SUPPORT_OBSOLETE_LOADER

/*---------------------------------------------------------------------------*
    Structure definitions
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    Function definitions
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         DSP_PowerOn

  Description:  Turns on the DSP and configures it to be in the reset state.
                You must call DSP_ResetOff() to start the DSP.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_PowerOnCore(void);
SDK_INLINE void DSP_PowerOn(void)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        DSP_PowerOnCore();
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_PowerOff

  Description:  Turns off the DSP.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_PowerOffCore(void);
SDK_INLINE void DSP_PowerOff(void)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        DSP_PowerOffCore();
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_ResetOn

  Description:  Resets the DSP.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_ResetOnCore(void);
SDK_INLINE void DSP_ResetOn(void)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        DSP_ResetOnCore();
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_ResetOff

  Description:  Starts the DSP from the reset state.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_ResetOffCore(void);
SDK_INLINE void DSP_ResetOff(void)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        DSP_ResetOffCore();
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_ResetOffEx

  Description:  Starts the DSP from the reset state.

  Arguments:    bitmap: Bitmap of the reply registers and semaphores that allow interrupts from the DSP to the ARM9

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_ResetOffExCore(u16 bitmap);
SDK_INLINE void DSP_ResetOffEx(u16 bitmap)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        DSP_ResetOffExCore(bitmap);
    }
}


/*---------------------------------------------------------------------------*
  Name:         DSP_ResetInterface

  Description:  Initializes the registers.
                You must call this function when the DSP is reset.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_ResetInterfaceCore(void);
SDK_INLINE void DSP_ResetInterface(void)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        DSP_ResetInterfaceCore();
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_EnableRecvDataInterrupt

  Description:  Enables interrupts for the specified reply register.

  Arguments:    dataNo: Reply register number (0-2)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_EnableRecvDataInterruptCore(u32 dataNo);
SDK_INLINE void DSP_EnableRecvDataInterrupt(u32 dataNo)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        DSP_EnableRecvDataInterruptCore(dataNo);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_DisableRecvDataInterrupt

  Description:  Disables interrupts for the specified reply register.

  Arguments:    dataNo: Reply register number (0-2)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_DisableRecvDataInterruptCore(u32 dataNo);
SDK_INLINE void DSP_DisableRecvDataInterrupt(u32 dataNo)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        DSP_DisableRecvDataInterruptCore(dataNo);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_SendDataIsEmpty

  Description:  Checks whether the DSP has received data for the specified command register.

  Arguments:    dataNo: Command register number (0-2)

  Returns:      TRUE if the DSP has already received data.
 *---------------------------------------------------------------------------*/
BOOL DSP_SendDataIsEmptyCore(u32 dataNo);
SDK_INLINE BOOL DSP_SendDataIsEmpty(u32 dataNo)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return DSP_SendDataIsEmptyCore(dataNo);
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_RecvDataIsReady

  Description:  Checks whether data was sent from the DSP to the specified reply register.

  Arguments:    dataNo: Reply register number (0-2)

  Returns:      TRUE if the DSP has sent data.
 *---------------------------------------------------------------------------*/
BOOL DSP_RecvDataIsReadyCore(u32 dataNo);
SDK_INLINE BOOL DSP_RecvDataIsReady(u32 dataNo)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return DSP_RecvDataIsReadyCore(dataNo);
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_SendData

  Description:  Sends data to the DSP.

  Arguments:    dataNo: Command register number (0-2)
                data: Data to send

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_SendDataCore(u32 dataNo, u16 data);
SDK_INLINE void DSP_SendData(u32 dataNo, u16 data)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        DSP_SendDataCore(dataNo, data);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_RecvData

  Description:  Receives data from the DSP.

  Arguments:    dataNo: Reply register number (0-2)

  Returns:      The received data.
 *---------------------------------------------------------------------------*/
u16 DSP_RecvDataCore(u32 dataNo);
SDK_INLINE u16 DSP_RecvData(u32 dataNo)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return DSP_RecvDataCore(dataNo);
    }
    return 0;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_EnableFifoInterrupt

  Description:  Enables FIFO interrupts.

  Arguments:    type: Cause of the FIFO interrupt

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_EnableFifoInterruptCore(DSPFifoIntr type);
SDK_INLINE void DSP_EnableFifoInterrupt(DSPFifoIntr type)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        DSP_EnableFifoInterruptCore(type);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_DisableFifoInterrupt

  Description:  Disables FIFO interrupts.

  Arguments:    type: Cause of the FIFO interrupt

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_DisableFifoInterruptCore(DSPFifoIntr type);
SDK_INLINE void DSP_DisableFifoInterrupt(DSPFifoIntr type)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        DSP_DisableFifoInterruptCore(type);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_SendFifoEx

  Description:  Writes data in the DSP's memory space.

  Arguments:    memsel: Memory space in which to write data
                dest: Address to write to (half-word).
                      If the most-significant 16 bits are set, values must be set in the DMA register in advance.
                         
                src: Data to write
                size: Length of the data to write (half-word)
                flags: You can select a write mode other than DSP_FIFO_FLAG_RECV_UNIT_* from DSPFifoFlag
                         

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_SendFifoExCore(DSPFifoMemSel memsel, u16 dest, const u16 *src, int size, u16 flags);
SDK_INLINE void DSP_SendFifoEx(DSPFifoMemSel memsel, u16 dest, const u16 *src, int size, u16 flags)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        DSP_SendFifoExCore(memsel, dest, src, size, flags);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_SendFifo

  Description:  Writes data in the DSP's memory space.

  Arguments:    memsel: Memory space in which to write data
                dest: Address to write to (half-word).
                      If the most-significant 16 bits are set, values must be set in the DMA register ahead of time.
                         
                src: Data to write
                size: Length of the data to write (half-word)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void DSP_SendFifo(DSPFifoMemSel memsel, u16 dest, const u16 *src, int size)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        DSP_SendFifoExCore(memsel, dest, src, size, 0);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_RecvFifoEx

  Description:  Reads data from the DSP's memory space.

  Arguments:    memsel: Memory space to read data from (other than program memory)
                dest: Address at which to receive data
                src: Address from which the data was received (half-word)
                     If the most-significant 16 bits are set, values must be set in the DMA register ahead of time.
                         
                size: Data size to read (half-word)
                flags: Select the read mode from DSPFifoFlag

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_RecvFifoExCore(DSPFifoMemSel memsel, u16 *dest, u16 src, int size, u16 flags);
SDK_INLINE void DSP_RecvFifoEx(DSPFifoMemSel memsel, u16 *dest, u16 src, int size, u16 flags)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        DSP_RecvFifoExCore(memsel, dest, src, size, flags);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_RecvFifo

  Description:  Reads data from the DSP's memory space.

  Arguments:    memsel: Memory space to read data from (other than program memory)
                dest: Address at which to receive data
                src: Address from which the data was received (half-word)
                     If the most-significant 16 bits are set, values must be set in the DMA register ahead of time.
                         
                size: Data size to read (half-word)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void DSP_RecvFifo(DSPFifoMemSel memsel, u16* dest, u16 src, int size)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        DSP_RecvFifoExCore(memsel, dest, src, size, 0);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_SetCommandReg

  Description:  Writes a value to a command register.

  Arguments:    regNo: Command register number (0-2)
                data: Data

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_SetCommandRegCore(u32 regNo, u16 data);
SDK_INLINE void DSP_SetCommandReg(u32 regNo, u16 data)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        DSP_SetCommandRegCore(regNo, data);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_GetReplyReg

  Description:  Reads a value from a reply register.

  Arguments:    regNo: Reply register number (0-2)

  Returns:      The data read.
 *---------------------------------------------------------------------------*/
u16 DSP_GetReplyRegCore(u32 regNo);
SDK_INLINE u16 DSP_GetReplyReg(u32 regNo)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return DSP_GetReplyRegCore(regNo);
    }
    return 0;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_SetSemaphore

  Description:  Writes a value to the semaphore register for sending notifications from an ARM processor to the DSP.

  Arguments:    mask: Value to set

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_SetSemaphoreCore(u16 mask);
SDK_INLINE void DSP_SetSemaphore(u16 mask)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        DSP_SetSemaphoreCore(mask);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_GetSemaphore

  Description:  Reads the value of the semaphore register for sending notifications from the DSP to an ARM processor.

  Arguments:    None.

  Returns:      The value written to the semaphore by the DSP.
 *---------------------------------------------------------------------------*/
u16 DSP_GetSemaphoreCore(void);
SDK_INLINE u16 DSP_GetSemaphore(void)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return DSP_GetSemaphoreCore();
    }
    return 0;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_ClearSemaphore

  Description:  Clears the value of the semaphore register for sending notifications from the DSP to an ARM processor.

  Arguments:    mask: Bits to clear

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_ClearSemaphoreCore(u16 mask);
SDK_INLINE void DSP_ClearSemaphore(u16 mask)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        DSP_ClearSemaphoreCore(mask);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_MaskSemaphore

  Description:  Sets bits that correspond to interrupts to disable in the semaphore register for sending notifications from the DSP to an ARM processor.

  Arguments:    mask: Bits corresponding to interrupts to disable

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_MaskSemaphoreCore(u16 mask);
SDK_INLINE void DSP_MaskSemaphore(u16 mask)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        DSP_MaskSemaphoreCore(mask);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_CheckSemaphoreRequest

  Description:  Checks whether there is an interrupt request from the semaphore register.

  Arguments:    None.

  Returns:      TRUE if there is a request.
 *---------------------------------------------------------------------------*/
BOOL DSP_CheckSemaphoreRequestCore(void);
SDK_INLINE BOOL DSP_CheckSemaphoreRequest(void)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return DSP_CheckSemaphoreRequestCore();
    }
    return FALSE;
}

#if defined(DSP_SUPPORT_OBSOLETE_LOADER)
/*---------------------------------------------------------------------------*
 * The following shows candidate interfaces for termination because they are considered not to be currently in use
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         DSP_LoadFileAuto

  Description:  Loads a COFF format DSP program and assigns the necessary WRAM to DSP.

  Arguments:    image: COFF file

  Returns:      TRUE on success.
 *---------------------------------------------------------------------------*/
BOOL DSP_LoadFileAutoCore(const void *image);
SDK_INLINE BOOL DSP_LoadFileAuto(const void *image)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return DSP_LoadFileAutoCore(image);
    }
    return FALSE;
}

#endif


/*===========================================================================*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TWL_DSP_IF_H_ */
