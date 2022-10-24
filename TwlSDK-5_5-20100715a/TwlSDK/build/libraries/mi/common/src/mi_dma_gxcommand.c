/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MI
  File:     mi_dma_gxcommand.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-10-16#$
  $Rev: 8957 $
  $Author: yada $
 *---------------------------------------------------------------------------*/

#ifdef SDK_ARM9
#include <nitro.h>
#include "../include/mi_dma.h"

//---- flagmental DMA size
#define MIi_GX_LENGTH_ONCE  (118 * sizeof(u32)) //(byte)

//---- fmagment DMA parameters
typedef struct
{
    volatile BOOL isBusy;

    u32     dmaNo;
    u32     src;
    u32     length;
    MIDmaCallback callback;
    void   *arg;

    GXFifoIntrCond fifoCond;
    void    (*fifoFunc) (void);
}
MIiGXDmaParams;

static MIiGXDmaParams MIi_GXDmaParams = { FALSE };

static void MIi_FIFOCallback(void);
static void MIi_DMACallback(void *);
static void MIi_DMAFastCallback(void *);

//================================================================================
//         not use auto-DMA
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         MI_SendGXCommand

  Description:  send GX command with geometry FIFO DMA.
                sync version.
                send data with small fragmental DMA, so DMA autostart may not occur.

  Arguments:    dmaNo          : DMA channel No.
                src            : source address
                commandLength  : GX command length. (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
#include <nitro/itcm_begin.h>
void MI_SendGXCommand(u32 dmaNo, const void *src, u32 commandLength)
{
    vu32   *dmaCntp;
    u32     leftLength = commandLength;
    u32     currentSrc = (u32)src;

    MIi_ASSERT_DMANO(dmaNo);
    MIi_ASSERT_SRC_ALIGN4(src);
    MIi_WARNING_ADDRINTCM(src, commandLength);

    if (leftLength == 0)
    {
        return;
    }

    //---- check DMA0 source address
    MIi_CheckDma0SourceAddress(dmaNo, (u32)src, commandLength, MI_DMA_SRC_INC);

    MIi_Wait_BeforeDMA(dmaCntp, dmaNo);
    while (leftLength > 0)
    {
        u32     length = (leftLength > MIi_GX_LENGTH_ONCE) ? MIi_GX_LENGTH_ONCE : leftLength;
        MIi_DmaSetParameters(dmaNo, currentSrc, (u32)REG_GXFIFO_ADDR, MI_CNT_SEND32(length), 0);
        leftLength -= length;
        currentSrc += length;
    }
    MIi_Wait_AfterDMA(dmaCntp);
}

#include <nitro/itcm_end.h>

/*---------------------------------------------------------------------------*
  Name:         MI_SendGXCommandAsync

  Description:  send GX command with geometry FIFO DMA.
                async version.
                send data with small fragmental DMA, so DMA autostart may not occur.

  Arguments:    dmaNo          : DMA channel No.
                src            : source address
                commandLength  : GX command length. (byte)
                callback       : callback function
                arg            : callback argument

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_SendGXCommandAsync(u32 dmaNo, const void *src, u32 commandLength, MIDmaCallback callback,
                           void *arg)
{
    MIi_ASSERT_DMANO(dmaNo);
    MIi_ASSERT_SRC_ALIGN4(src);
    MIi_WARNING_ADDRINTCM(src, commandLength);

    //---- if size==0, call callback immediately
    if (commandLength == 0)
    {
        MIi_CallCallback(callback, arg);
        return;
    }

    //---- wait till other task end
    while (MIi_GXDmaParams.isBusy /*volatile valiable */ )
    {
    }

    //---- wait till geometryEngine FIFO is underhalf
    while (!(G3X_GetCommandFifoStatus() & GX_FIFOSTAT_UNDERHALF))
    {
    }

    //---- fragmental DMA params
    MIi_GXDmaParams.isBusy = TRUE;
    MIi_GXDmaParams.dmaNo = dmaNo;
    MIi_GXDmaParams.src = (u32)src;
    MIi_GXDmaParams.length = commandLength;
    MIi_GXDmaParams.callback = callback;
    MIi_GXDmaParams.arg = arg;

    //---- check DMA0 source address
    MIi_CheckDma0SourceAddress(dmaNo, (u32)src, commandLength, MI_DMA_SRC_INC);

    MI_WaitDma(dmaNo);
    {
        OSIntrMode enabled = OS_DisableInterrupts();

        //---- remember FIFO interrupt setting
        MIi_GXDmaParams.fifoCond = (GXFifoIntrCond)((reg_G3X_GXSTAT & REG_G3X_GXSTAT_FI_MASK) >> REG_G3X_GXSTAT_FI_SHIFT);
        MIi_GXDmaParams.fifoFunc = OS_GetIrqFunction(OS_IE_GXFIFO);

        //---- set FIFO interrupt
        G3X_SetFifoIntrCond(GX_FIFOINTR_COND_UNDERHALF);
        OS_SetIrqFunction(OS_IE_GXFIFO, MIi_FIFOCallback);
        (void)OS_EnableIrqMask(OS_IE_GXFIFO);

        MIi_FIFOCallback();

        (void)OS_RestoreInterrupts(enabled);
    }
}

/*---------------------------------------------------------------------------*
  Name:         MIi_FIFOCallback

  Description:  callback for GX FIFO
                (used for MI_SendGXCommandAsync)
  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void MIi_FIFOCallback(void)
{
    u32     length;
    u32     src;

    if (MIi_GXDmaParams.length == 0)
    {
        return;
    }

    //---- parameters for this fragment
    length = (MIi_GXDmaParams.length >= MIi_GX_LENGTH_ONCE) ? MIi_GX_LENGTH_ONCE : MIi_GXDmaParams.length;
    src = MIi_GXDmaParams.src;

    //---- arrange fragmental DMA params
    MIi_GXDmaParams.length -= length;
    MIi_GXDmaParams.src += length;

    //---- start fragmental DMA. if last, set DMA callback
    if (MIi_GXDmaParams.length == 0)
    {
        OSi_EnterDmaCallback(MIi_GXDmaParams.dmaNo, MIi_DMACallback, NULL);
        MIi_DmaSetParameters(MIi_GXDmaParams.dmaNo, src, (u32)REG_GXFIFO_ADDR, MI_CNT_SEND32_IF(length), 0);
        (void)OS_ResetRequestIrqMask(OS_IE_GXFIFO);
    }
    else
    {
        MIi_DmaSetParameters(MIi_GXDmaParams.dmaNo, src, (u32)REG_GXFIFO_ADDR, MI_CNT_SEND32(length), 0);
        (void)OS_ResetRequestIrqMask(OS_IE_GXFIFO);
    }
}

/*---------------------------------------------------------------------------*
  Name:         MIi_DMACallback

  Description:  callback for DMA
                (used for MI_SendGXCommandAsync)

  Arguments:    not use

  Returns:      None
 *---------------------------------------------------------------------------*/
static void MIi_DMACallback(void *)
{
    (void)OS_DisableIrqMask(OS_IE_GXFIFO);

    //---- restore FIFO interrupt setting
    G3X_SetFifoIntrCond(MIi_GXDmaParams.fifoCond);
    OS_SetIrqFunction(OS_IE_GXFIFO, MIi_GXDmaParams.fifoFunc);

    MIi_GXDmaParams.isBusy = FALSE;

    MIi_CallCallback(MIi_GXDmaParams.callback, MIi_GXDmaParams.arg);
}

//================================================================================
//         use auto-DMA
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         MI_SendGXCommandFast

  Description:  send GX command with geometry FIFO DMA.
                sync version.
                send data at once, so DMA autostart may occur.

  Arguments:    dmaNo          : DMA channel No.
                src            : source address
                commandLength  : GX command length. (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
#include <nitro/itcm_begin.h>
void MI_SendGXCommandFast(u32 dmaNo, const void *src, u32 commandLength)
{
    vu32   *dmaCntp;

    MIi_ASSERT_DMANO(dmaNo);
    MIi_ASSERT_SRC_ALIGN4(src);
    MIi_WARNING_ADDRINTCM(src, commandLength);

    if (commandLength == 0)
    {
        return;
    }

    MIi_Wait_BeforeDMA(dmaCntp, dmaNo);
    MIi_DmaSetParameters(dmaNo, (u32)src, (u32)REG_GXFIFO_ADDR, MI_CNT_GXCOPY(commandLength), 0);
    MIi_Wait_AfterDMA(dmaCntp);
}

#include <nitro/itcm_end.h>

/*---------------------------------------------------------------------------*
  Name:         MI_SendGXCommandAsyncFast

  Description:  send GX command with geometry FIFO DMA.
                async version.
                send data at once, so DMA autostart may occur.

  Arguments:    dmaNo          : DMA channel No.
                src            : source address
                commandLength  : GX command length. (byte)
                callback       : callback function
                arg            : callback argument

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_SendGXCommandAsyncFast(u32 dmaNo, const void *src, u32 commandLength,
                               MIDmaCallback callback, void *arg)
{
    MIi_ASSERT_DMANO(dmaNo);
    MIi_ASSERT_SRC_ALIGN4(src);
    MIi_WARNING_ADDRINTCM(src, commandLength);

    if (commandLength == 0)
    {
        MIi_CallCallback(callback, arg);
        return;
    }

    //---- wait till other task end
    while (MIi_GXDmaParams.isBusy /*volatile valiable */ )
    {
    }

    //---- DMA params
    MIi_GXDmaParams.isBusy = TRUE;
    MIi_GXDmaParams.dmaNo = dmaNo;
    MIi_GXDmaParams.callback = callback;
    MIi_GXDmaParams.arg = arg;

    MIi_CheckAnotherAutoDMA(dmaNo, MI_DMA_TIMING_GXFIFO);
    //---- check DMA0 source address
    MIi_CheckDma0SourceAddress(dmaNo, (u32)src, commandLength, MI_DMA_SRC_INC);

    MI_WaitDma(dmaNo);

    OSi_EnterDmaCallback(dmaNo, MIi_DMAFastCallback, NULL);
    MIi_DmaSetParameters(dmaNo, (u32)src, (u32)REG_GXFIFO_ADDR, MI_CNT_GXCOPY_IF(commandLength), 0);
}

/*---------------------------------------------------------------------------*
  Name:         MIi_DMAFastCallback

  Description:  callback for DMA
                (used for MI_SendGXCommandAsyncFast)

  Arguments:    not use

  Returns:      None
 *---------------------------------------------------------------------------*/
static void MIi_DMAFastCallback(void *)
{
    MIi_GXDmaParams.isBusy = FALSE;

    MIi_CallCallback(MIi_GXDmaParams.callback, MIi_GXDmaParams.arg);
}

#endif // SDK_ARM9
