/*---------------------------------------------------------------------------*
  Project:  TwlSDK - library - dsp
  File:     dsp_if.c

  Copyright 2007-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2010-01-12#$
  $Rev: 11277 $
  $Author: kakemizu_hironori $
 *---------------------------------------------------------------------------*/
#include <twl.h>
#include <twl/dsp.h>

#include <dsp_coff.h>
#include "dsp_process.h"

/*---------------------------------------------------------------------------*
    Constant definitions
 *---------------------------------------------------------------------------*/

#define reg_CFG_DSP_RST             *(vu8*)REG_RST_ADDR

#define REG_DSP_PCFG_RRIE_MASK      (REG_DSP_PCFG_PRIE0_MASK | REG_DSP_PCFG_PRIE1_MASK | REG_DSP_PCFG_PRIE2_MASK)
#define REG_DSP_PCFG_RRIE_SHIFT     REG_DSP_PCFG_PRIE0_SHIFT
#define REG_DSP_PSTS_RCOMIM_SHIFT   REG_DSP_PSTS_RCOMIM0_SHIFT
#define REG_DSP_PSTS_RRI_SHIFT      REG_DSP_PSTS_RRI0_SHIFT


#define DSP_DPRINTF(...)    ((void) 0)
//#define DSP_DPRINTF OS_TPrintf

/*---------------------------------------------------------------------------*
    Type Definitions
 *---------------------------------------------------------------------------*/
typedef struct DSPData
{
    u16 send;
    u16 reserve1;
    u16 recv;
    u16 reserve2;
}
DSPData;


/*---------------------------------------------------------------------------*
    Static Variable Definitions
 *---------------------------------------------------------------------------*/
static volatile DSPData *const dspData = (DSPData*)REG_COM0_ADDR;

/*---------------------------------------------------------------------------*
    Internal Function Definitions
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         DSP_PowerOn

  Description:  Turns on the DSP and configures it to be in the reset state.
                You must call DSP_ResetOff() to start the DSP.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_PowerOnCore(void)  // DSP_Init
{
    SCFG_ResetDSP();                                // Confirms reset of DSP block
    SCFG_SupplyClockToDSP(TRUE);                    // Power on for DSP block
    OS_SpinWaitSysCycles(2);                        // Wait 8 cycles @ 134 MHz
    SCFG_ReleaseResetDSP();                         // Release reset of DSP block
    DSP_ResetOnCore();                                  // Set reset of DSP core
}

/*---------------------------------------------------------------------------*
  Name:         DSP_PowerOff

  Description:  Turns off the DSP.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_PowerOffCore(void) // DSP_End
{
    SCFG_ResetDSP();                                // Sets reset of DSP block
    SCFG_SupplyClockToDSP(FALSE);                   // Power off for DSP block
}

/*---------------------------------------------------------------------------*
  Name:         DSP_ResetOn

  Description:  Resets the DSP.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_ResetOnCore(void)
{
    OS_SpinWaitSysCycles(4);
    if ((reg_DSP_PCFG & REG_DSP_PCFG_DSPR_MASK) == 0)
    {
        reg_DSP_PCFG |= REG_DSP_PCFG_DSPR_MASK;
        OS_SpinWaitSysCycles(4);
        while ( reg_DSP_PSTS & REG_DSP_PSTS_PRST_MASK )
        {
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_ResetOff

  Description:  Starts the DSP from the reset state.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_ResetOffCore(void)
{
    OS_SpinWaitSysCycles(4);
    while ( reg_DSP_PSTS & REG_DSP_PSTS_PRST_MASK )
    {
    }
    DSP_ResetInterfaceCore();   // Initialize DSP-A9IF
    OS_SpinWaitSysCycles(4);
    reg_DSP_PCFG &= ~REG_DSP_PCFG_DSPR_MASK;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_ResetOffEx

  Description:  Starts the DSP from the reset state.

  Arguments:    bitmap: Bitmap of the reply registers and semaphores that allow interrupts from the DSP to the ARM9

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_ResetOffExCore(u16 bitmap)
{
    SDK_ASSERT(bitmap >= 0 && bitmap <= 7);
    
    OS_SpinWaitSysCycles(4);
    while ( reg_DSP_PSTS & REG_DSP_PSTS_PRST_MASK )
    {
    }
    DSP_ResetInterfaceCore();   // Initialize DSP-A9IF
    OS_SpinWaitSysCycles(4);
    reg_DSP_PCFG |= (bitmap) << REG_DSP_PCFG_RRIE_SHIFT;
    OS_SpinWaitSysCycles(4);
    reg_DSP_PCFG &= ~REG_DSP_PCFG_DSPR_MASK;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_ResetInterface

  Description:  Initializes the registers.
                You must call this function when the DSP is reset.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_ResetInterfaceCore(void)
{
    OS_SpinWaitSysCycles(4);
    if (reg_DSP_PCFG & REG_DSP_PCFG_DSPR_MASK)
    {
        u16 dummy;
        reg_DSP_PCFG &= ~REG_DSP_PCFG_RRIE_MASK;
        reg_DSP_PSEM = 0;
        reg_DSP_PCLEAR = 0xFFFF;
        dummy = dspData[0].recv;
        dummy = dspData[1].recv;
        dummy = dspData[2].recv;
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_EnableRecvDataInterrupt

  Description:  Enables interrupts for the specified reply register.

  Arguments:    dataNo: Reply register number (0-2)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_EnableRecvDataInterruptCore(u32 dataNo)
{
    SDK_ASSERT(dataNo >= 0 && dataNo <= 2);
    OS_SpinWaitSysCycles(4);
    reg_DSP_PCFG |= (1 << dataNo) << REG_DSP_PCFG_RRIE_SHIFT;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_DisableRecvDataInterrupt

  Description:  Disables interrupts for the specified reply register.

  Arguments:    dataNo: Reply register number (0-2)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_DisableRecvDataInterruptCore(u32 dataNo)
{
    SDK_ASSERT(dataNo >= 0 && dataNo <= 2);
    OS_SpinWaitSysCycles(4);
    reg_DSP_PCFG &= ~((1 << dataNo) << REG_DSP_PCFG_RRIE_SHIFT);
}

/*---------------------------------------------------------------------------*
  Name:         DSP_SendDataIsEmpty

  Description:  Checks whether the DSP has received data for the specified command register.

  Arguments:    dataNo: Command register number (0-2)

  Returns:      TRUE if the DSP has already received data.
 *---------------------------------------------------------------------------*/
BOOL DSP_SendDataIsEmptyCore(u32 dataNo)
{
    SDK_ASSERT(dataNo >= 0 && dataNo <= 2);
    OS_SpinWaitSysCycles(4);
    return (reg_DSP_PSTS & ((1 << dataNo) << REG_DSP_PSTS_RCOMIM_SHIFT)) ? FALSE : TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_RecvDataIsReady

  Description:  Checks whether data was sent from the DSP to the specified reply register.

  Arguments:    dataNo: Reply register number (0-2)

  Returns:      TRUE if the DSP has sent data.
 *---------------------------------------------------------------------------*/
BOOL DSP_RecvDataIsReadyCore(u32 dataNo)
{
    SDK_ASSERT(dataNo >= 0 && dataNo <= 2);
    OS_SpinWaitSysCycles(4);
    return (reg_DSP_PSTS & ((1 << dataNo) << REG_DSP_PSTS_RRI_SHIFT)) ? TRUE : FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_SendData

  Description:  Sends data to the DSP.

  Arguments:    dataNo: Command register number (0-2)
                data: Data to send

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_SendDataCore(u32 dataNo, u16 data)
{
    SDK_ASSERT(dataNo >= 0 && dataNo <= 2);
    OS_SpinWaitSysCycles(4);
    while (reg_DSP_PSTS & ((1 << dataNo) << REG_DSP_PSTS_RCOMIM_SHIFT))
    {
    }
    dspData[dataNo].send = data;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_RecvData

  Description:  Receives data from the DSP.

  Arguments:    dataNo: Reply register number (0-2)

  Returns:      The received data.
 *---------------------------------------------------------------------------*/
u16 DSP_RecvDataCore(u32 dataNo)
{
    SDK_ASSERT(dataNo >= 0 && dataNo <= 2);
    OS_SpinWaitSysCycles(4);
    while (!(reg_DSP_PSTS & ((1 << dataNo) << REG_DSP_PSTS_RRI_SHIFT)))
    {
    }
    return dspData[dataNo].recv;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_EnableFifoInterrupt

  Description:  Enables FIFO interrupts.

  Arguments:    type: Cause of the FIFO interrupt

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_EnableFifoInterruptCore(DSPFifoIntr type)
{
    OS_SpinWaitSysCycles(4);
    reg_DSP_PCFG |= type;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_DisableFifoInterrupt

  Description:  Disables FIFO interrupts.

  Arguments:    type: Cause of the FIFO interrupt

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_DisableFifoInterruptCore(DSPFifoIntr type)
{
    OS_SpinWaitSysCycles(4);
    reg_DSP_PCFG &= ~type;
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
void DSP_SendFifoExCore(DSPFifoMemSel memsel, u16 dest, const u16 *src, int size, u16 flags)
{
    OSIntrMode  bak = OS_DisableInterrupts();
    u16 incmode = (u16)((flags & DSP_FIFO_FLAG_DEST_FIX) ? 0 : REG_DSP_PCFG_AIM_MASK);

    OS_SpinWaitSysCycles(4);
    reg_DSP_PCFG = (u16)((reg_DSP_PCFG & ~(REG_DSP_PCFG_MEMSEL_MASK|REG_DSP_PCFG_AIM_MASK))
                        | memsel | incmode);
    reg_DSP_PADR = dest;

    if (flags & DSP_FIFO_FLAG_SRC_FIX)
    {
        while (size-- > 0)
        {
            OS_SpinWaitSysCycles(4);
            while (reg_DSP_PSTS & REG_DSP_PSTS_WFFI_MASK)
            {
            }
            reg_DSP_PDATA = *src;
        }
    }
    else
    {
        while (size-- > 0)
        {
            OS_SpinWaitSysCycles(4);
            while (reg_DSP_PSTS & REG_DSP_PSTS_WFFI_MASK)
            {
            }
            reg_DSP_PDATA = *src++;
        }
    }
    (void)OS_RestoreInterrupts(bak);
}

/*---------------------------------------------------------------------------*
  Name:         DSP_RecvFifoEx

  Description:  Reads data from the DSP's memory space.

  Arguments:    memsel: Memory space to read data from (other than program memory)
                dest: Address at which to receive data
                src: Address from which the data was received (half-word)
                         If the most-significant 16 bits are set, values must be set in the DMA register in advance.
                         
                size: Data size to read (half-word)
                flags: Select the read mode from DSPFifoFlag

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_RecvFifoExCore(DSPFifoMemSel memsel, u16* dest, u16 src, int size, u16 flags)
{
    OSIntrMode  bak = OS_DisableInterrupts();
    DSPFifoRecvLength len;
    u16 incmode = (u16)((flags & DSP_FIFO_FLAG_SRC_FIX) ? 0 : REG_DSP_PCFG_AIM_MASK);

    SDK_ASSERT(memsel != DSP_FIFO_MEMSEL_PROGRAM);

    switch (flags & DSP_FIFO_FLAG_RECV_MASK)
    {
    case DSP_FIFO_FLAG_RECV_UNIT_2B:
        len = DSP_FIFO_RECV_2B;
        size = 1;
        break;
    case DSP_FIFO_FLAG_RECV_UNIT_16B:
        len = DSP_FIFO_RECV_16B;
        size = 8;
        break;
    case DSP_FIFO_FLAG_RECV_UNIT_32B:
        len = DSP_FIFO_RECV_32B;
        size = 16;
        break;
    default:
        len = DSP_FIFO_RECV_CONTINUOUS;
        break;
    }

    reg_DSP_PADR = src;
    OS_SpinWaitSysCycles(4);
    reg_DSP_PCFG = (u16)((reg_DSP_PCFG & ~(REG_DSP_PCFG_MEMSEL_MASK|REG_DSP_PCFG_DRS_MASK|REG_DSP_PCFG_AIM_MASK))
                        | memsel | len | incmode | REG_DSP_PCFG_RS_MASK);

    if (flags & DSP_FIFO_FLAG_DEST_FIX)
    {
        while (size-- > 0)
        {
            OS_SpinWaitSysCycles(4);
            while ((reg_DSP_PSTS & REG_DSP_PSTS_RFNEI_MASK) == 0)
            {
            }
            *dest = reg_DSP_PDATA;
        }
    }
    else
    {
        while (size-- > 0)
        {
            OS_SpinWaitSysCycles(4);
            while ((reg_DSP_PSTS & REG_DSP_PSTS_RFNEI_MASK) == 0)
            {
            }
            *dest++ = reg_DSP_PDATA;
        }
    }
    OS_SpinWaitSysCycles(4);
    reg_DSP_PCFG &= ~REG_DSP_PCFG_RS_MASK;
    (void)OS_RestoreInterrupts(bak);
}

/*---------------------------------------------------------------------------*
  Name:         DSP_SetCommandReg

  Description:  Writes a value to a command register.

  Arguments:    regNo: Command register number (0-2)
                data: Data

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_SetCommandRegCore(u32 regNo, u16 data)
{
    SDK_ASSERT(regNo >= 0 && regNo <= 2);
    OS_SpinWaitSysCycles(4);
    dspData[regNo].send = data;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_GetReplyReg

  Description:  Reads a value from a reply register.

  Arguments:    regNo: Reply register number (0-2)

  Returns:      The data read.
 *---------------------------------------------------------------------------*/
u16 DSP_GetReplyRegCore(u32 regNo)
{
    SDK_ASSERT(regNo >= 0 && regNo <= 2);
    OS_SpinWaitSysCycles(4);
    return dspData[regNo].recv;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_SetSemaphore

  Description:  Writes a value to the semaphore register used to send notifications from the ARM to the DSP.

  Arguments:    mask: Value to set

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_SetSemaphoreCore(u16 mask)
{
    reg_DSP_PSEM = mask;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_GetSemaphore

  Description:  Reads the value of the semaphore register for sending notifications from the DSP to an ARM processor.

  Arguments:    None.

  Returns:      The value written to the semaphore by the DSP.
 *---------------------------------------------------------------------------*/
u16 DSP_GetSemaphoreCore(void)
{
    OS_SpinWaitSysCycles(4);
    return reg_DSP_SEM;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_ClearSemaphore

  Description:  Clears the value of the semaphore register for sending notifications from the DSP to an ARM processor.

  Arguments:    mask: Bits to clear

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_ClearSemaphoreCore(u16 mask)
{
    reg_DSP_PCLEAR = mask;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_MaskSemaphore

  Description:  Sets bits that correspond to interrupts to disable in the semaphore register for sending notifications from the DSP to an ARM processor.

  Arguments:    mask: Bits corresponding to interrupts to disable

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_MaskSemaphoreCore(u16 mask)
{
    reg_DSP_PMASK = mask;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_CheckSemaphoreRequest

  Description:  Checks whether there is an interrupt request from the semaphore register.

  Arguments:    None.

  Returns:      TRUE if there is a request.
 *---------------------------------------------------------------------------*/
BOOL DSP_CheckSemaphoreRequestCore(void)
{
    return (reg_DSP_PSTS & REG_DSP_PSTS_PSEMI_MASK) >> REG_DSP_PSTS_PSEMI_SHIFT;
}


#if defined(DSP_SUPPORT_OBSOLETE_LOADER)
/*---------------------------------------------------------------------------*
 * The following shows candidate interfaces for termination because they are considered not to be currently in use
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 * Below is the old interface using a straight-mapping method
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         DSPi_MapProcessSlotAsStraight

  Description:  Initializes the slot map so the segment number and the WRAM slot number match.
                (This method is used only with the initial load function that does not have Ex applied.)

  Arguments:    context: DSPProcessContext structure
                slotB: WRAM-B allowed for use for code memory
                slotC: WRAM-C allowed for use for data memory

  Returns:      None.
 *---------------------------------------------------------------------------*/
static BOOL DSPi_MapProcessSlotAsStraight(DSPProcessContext *context, int slotB, int slotC)
{
    int     segment;
    for (segment = 0; segment < MI_WRAM_B_MAX_NUM; ++segment)
    {
        if (context->segmentCode & (1 << segment) != 0)
        {
            int     slot = segment;
            if ((slotB & (1 << slot)) == 0)
            {
                return FALSE;
            }
            context->slotOfSegmentCode[segment] = slot;
        }
    }
    for (segment = 0; segment < MI_WRAM_C_MAX_NUM; ++segment)
    {
        if (context->segmentData & (1 << segment) != 0)
        {
            int     slot = segment;
            if ((slotC & (1 << slot)) == 0)
            {
                return FALSE;
            }
            context->slotOfSegmentData[segment] = slot;
        }
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_LoadFileAuto

  Description:  Loads a COFF format DSP program and assigns the necessary WRAM to DSP.

  Arguments:    image: COFF file

  Returns:      TRUE on success.
 *---------------------------------------------------------------------------*/
BOOL DSP_LoadFileAutoCore(const void *image)
{
    // Temporarily convert to a memory file
    FSFile  memfile[1];
    if(DSPi_CreateMemoryFile(memfile, image))
    {
        DSPProcessContext   context[1];
        DSP_InitProcessContext(context, NULL);
        return DSP_StartupProcess(context, memfile, 0xFF, 0xFF, DSPi_MapProcessSlotAsStraight);
    }
    return FALSE;
}


#endif

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
