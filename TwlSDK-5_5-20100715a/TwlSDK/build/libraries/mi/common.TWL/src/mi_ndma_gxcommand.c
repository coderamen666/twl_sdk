/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MI
  File:     mi_ndma_gxcommand.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-00-00#$
  $Rev: 0 $
  $Author: x $
 *---------------------------------------------------------------------------*/
#ifdef SDK_ARM9
#include <twl.h>
#include "../include/mi_ndma.h"

#include    <twl/ltdmain_begin.h>

//---- flagmental DMA size
#define MIi_GX_LENGTH_ONCE  (118 * sizeof(u32)) //(byte)

//---- flagment NDMA parameters
typedef struct
{
    volatile BOOL isBusy;

    u32     ndmaNo;
    u32     src;
    u32     length;
    MINDmaCallback callback;
    void   *arg;

    GXFifoIntrCond fifoCond;
    void    (*fifoFunc) (void);
}
MIiGXNDmaParams;

static MIiGXNDmaParams MIi_GXNDmaParams = { FALSE };

static void MIi_FIFOCallback(void);
static void MIi_NDMACallback(void *);
static void MIi_NDMAFastCallback(void *);

/*---------------------------------------------------------------------------*
  Name:         MI_SendNDmaGXCommand

  Description:  send GX command with geometry FIFO NDMA.
                sync version.
                send data with small fragmental NDMA, so NDMA autostart may not occur.

  Arguments:    ndmaNo         : NDMA channel No. (0-3)
                src            : source address
                commandLength  : GX command length. (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void    MI_SendNDmaGXCommand(u32 ndmaNo, const void *src, u32 commandLength)
{
    vu32 *ndmaCntp;
    u32 leftLength = commandLength;
    u32 currentSrc = (u32)src;
    OSIntrMode enabled;

    MIi_ASSERT_DMANO( ndmaNo );

    if (leftLength == 0)
    {
        return;
    }

    enabled = OS_DisableInterrupts();
    ndmaCntp = (vu32*)MI_NDMA_REGADDR(ndmaNo, MI_NDMA_REG_CNT_WOFFSET );

    //---- wait while busy
    while( *ndmaCntp & REG_MI_NDMA0CNT_E_MASK)
    {
    }

    while(leftLength > 0)
    {
        u32 length = (leftLength > MIi_GX_LENGTH_ONCE)? MIi_GX_LENGTH_ONCE : leftLength;

        MIi_NDma_withConfig_Dev(MIi_NDMA_TYPE_GXCOPY, ndmaNo, (const void*)currentSrc, (void*)REG_GXFIFO_ADDR, 0/*not used*/,
                            length, &MIi_NDmaConfig[ndmaNo], MI_NDMA_TIMING_GXFIFO, MI_NDMA_ENABLE );

        leftLength -= length;
        currentSrc += length;
    }

    //---- wait while busy
    while( *ndmaCntp & REG_MI_NDMA0CNT_E_MASK)
    {
    }
    (void)OS_RestoreInterrupts(enabled);
}

/*---------------------------------------------------------------------------*
  Name:         MI_SendNDmaGXCommandAsync

  Description:  send GX command with geometry FIFO NDMA.
                async version.
                send data with small fragmental NDMA, so NDMA autostart may not occur.

  Arguments:    ndmaNo         : NDMA channel No. (0-3)
                src            : source address
                commandLength  : GX command length. (byte)
                callback       : callback function
                arg            : callback argument

  Returns:      None
 *---------------------------------------------------------------------------*/
void    MI_SendNDmaGXCommandAsync(u32 ndmaNo, const void *src, u32 commandLength, MINDmaCallback callback, void *arg)
{
    MIi_ASSERT_DMANO( ndmaNo );

    //---- if size==0, call callback immediately
    if ( commandLength == 0)
    {
        MIi_CallCallback(callback, arg);
        return;
    }

    //---- wait till other task end
    while(MIi_GXNDmaParams.isBusy)
    {
    }

    //---- wait till geometryEngine FIFO is underhalf
    while (!(G3X_GetCommandFifoStatus() & GX_FIFOSTAT_UNDERHALF))
    {
    }

    //---- fragmental DMA params
    MIi_GXNDmaParams.isBusy = TRUE;
    MIi_GXNDmaParams.ndmaNo = ndmaNo;
    MIi_GXNDmaParams.src = (u32)src;
    MIi_GXNDmaParams.length = commandLength;
    MIi_GXNDmaParams.callback = callback;
    MIi_GXNDmaParams.arg = arg;

    MI_WaitNDma(ndmaNo);
    {
        OSIntrMode enabled = OS_DisableInterrupts();

        //---- remember FIFO interrupt setting
        MIi_GXNDmaParams.fifoCond = (GXFifoIntrCond)((reg_G3X_GXSTAT & REG_G3X_GXSTAT_FI_MASK) >> REG_G3X_GXSTAT_FI_SHIFT);
        MIi_GXNDmaParams.fifoFunc = OS_GetIrqFunction(OS_IE_GXFIFO);

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
                (used for MI_SendNDmaGXCommandAsync)
  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void MIi_FIFOCallback(void)
{
    u32     length;
    u32     src;

    if (MIi_GXNDmaParams.length == 0)
    {
        return;
    }

    //---- parameters for this fragment
    length = (MIi_GXNDmaParams.length >= MIi_GX_LENGTH_ONCE) ? MIi_GX_LENGTH_ONCE : MIi_GXNDmaParams.length;
    src = MIi_GXNDmaParams.src;

    //---- arrange fragmental DMA params
    MIi_GXNDmaParams.length -= length;
    MIi_GXNDmaParams.src += length;

    //---- disable GX interrupt if last
    if ( MIi_GXNDmaParams.length == 0 )
    {
        (void)OS_DisableIrqMask(OS_IE_GXFIFO);
    }
    (void)OS_ResetRequestIrqMask(OS_IE_GXFIFO);

    //---- start fragmental DMA. if last, set DMA callback
    MIi_NDmaAsync( (MIi_GXNDmaParams.length == 0)? MIi_NDMA_TYPE_GXCOPY_IF: MIi_NDMA_TYPE_GXCOPY,
                   MIi_GXNDmaParams.ndmaNo,
                   (const void*)src,
                   (void*)REG_GXFIFO_ADDR,
                   0/*not used*/,
                   length,
                   (MIi_GXNDmaParams.length == 0)? MIi_NDMACallback: NULL,
                   NULL,
                   MI_NDMA_ENABLE );
}

/*---------------------------------------------------------------------------*
  Name:         MIi_NDMACallback

  Description:  callback for NDMA
                (used for MI_SendNDmaGXCommandAsync)

  Arguments:    not use

  Returns:      None
 *---------------------------------------------------------------------------*/
static void MIi_NDMACallback(void *)
{
    (void)OS_DisableIrqMask(OS_IE_GXFIFO);

    //---- restore FIFO interrupt setting
    G3X_SetFifoIntrCond(MIi_GXNDmaParams.fifoCond);
    OS_SetIrqFunction(OS_IE_GXFIFO, MIi_GXNDmaParams.fifoFunc);

    MIi_GXNDmaParams.isBusy = FALSE;

    MIi_CallCallback(MIi_GXNDmaParams.callback, MIi_GXNDmaParams.arg);
}

/*---------------------------------------------------------------------------*
  Name:         MI_SendNDmaGXCommandFast

  Description:  send GX command with geometry FIFO NDMA.
                sync version.
                send data at once, so NDMA autostart may occur.

  Arguments:    ndmaNo         : NDMA channel No.
                src            : source address
                commandLength  : GX command length. (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_SendNDmaGXCommandFast(u32 ndmaNo, const void *src, u32 commandLength)
{
    vu32   *ndmaCntp;
    OSIntrMode enabled;

    MIi_ASSERT_DMANO(ndmaNo);

    if (commandLength == 0)
    {
        return;
    }

    enabled = OS_DisableInterrupts();
    ndmaCntp = (vu32*)MI_NDMA_REGADDR(ndmaNo, MI_NDMA_REG_CNT_WOFFSET );

    //---- wait while busy
    while( *ndmaCntp & REG_MI_NDMA0CNT_E_MASK)
    {
    }

    MIi_NDmaAsync( MIi_NDMA_TYPE_GXCOPY,
                   ndmaNo,
                   (const void*)src,
                   (void*)REG_GXFIFO_ADDR,
                   0/*not used*/,
                   commandLength,
                   NULL,
                   NULL,
                   MI_NDMA_ENABLE );

    //---- wait while busy
    while( *ndmaCntp & REG_MI_NDMA0CNT_E_MASK)
    {
    }
    (void)OS_RestoreInterrupts(enabled);
}

/*---------------------------------------------------------------------------*
  Name:         MI_SendNDmaGXCommandAsyncFast

  Description:  send GX command with geometry FIFO NDMA.
                async version.
                send data at once, so NDMA autostart may occur.

  Arguments:    ndmaNo         : NDMA channel No.
                src            : source address
                commandLength  : GX command length. (byte)
                callback       : callback function
                arg            : callback argument

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_SendNDmaGXCommandAsyncFast(u32 ndmaNo, const void *src, u32 commandLength, MINDmaCallback callback, void *arg)
{
    MIi_ASSERT_DMANO(ndmaNo);

    if (commandLength == 0)
    {
        MIi_CallCallback(callback, arg);
        return;
    }

    //---- wait till other task end
    while (MIi_GXNDmaParams.isBusy /*volatile valiable */ )
    {
    }

    //---- DMA params
    MIi_GXNDmaParams.isBusy = TRUE;
    MIi_GXNDmaParams.ndmaNo = ndmaNo;
    MIi_GXNDmaParams.callback = callback;
    MIi_GXNDmaParams.arg = arg;

    MI_WaitNDma(ndmaNo);

    MIi_NDmaAsync( MIi_NDMA_TYPE_GXCOPY_IF,
                   ndmaNo,
                   (const void*)src,
                   (void*)REG_GXFIFO_ADDR,
                   0/*not used*/,
                   commandLength,
                   MIi_NDMAFastCallback,
                   NULL,
                   MI_NDMA_ENABLE );
}

/*---------------------------------------------------------------------------*
  Name:         MIi_NDMAFastCallback

  Description:  callback for NDMA
                (used for MI_SendNDmaGXCommandAsyncFast)

  Arguments:    not use

  Returns:      None
 *---------------------------------------------------------------------------*/
static void MIi_NDMAFastCallback(void *)
{
    MIi_GXNDmaParams.isBusy = FALSE;

    MIi_CallCallback(MIi_GXNDmaParams.callback, MIi_GXNDmaParams.arg);
}

//================================================================================
#endif // SDK_ARM9
