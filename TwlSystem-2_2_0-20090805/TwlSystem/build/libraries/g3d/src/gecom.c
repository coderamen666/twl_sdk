/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - g3d
  File:     gecom.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

//
// AUTHOR: Kenji Nishida
//

#include <nnsys/g3d/gecom.h>

//
// NOTICE:
// ONLY FOR SINGLE THREADED CODE
//


////////////////////////////////////////////////////////////////////////////////
//
// Static Variables
//

/*---------------------------------------------------------------------------*
    NNS_G3dFlagGXDmaAsync

    Flag used by MI_SendGXCommandAsync, itself used by G3D.
    If nonzero, DMA transfer has not completed.
 *---------------------------------------------------------------------------*/
static volatile int NNS_G3dFlagGXDmaAsync  = 0;

//
// NOTICE:
// Even if the buffer for the Ge-related functions is NULL, this must operate correctly.
//

/*---------------------------------------------------------------------------*
    NNS_G3dGeBuffer

    Pointer to the geometry command buffer used by G3D.
    Allocated from the normal stack (DTCM) region memory.

    When there is a geometry command send request in the display list DMA transfer, the CPU can move on to the next process by writing the command to this buffer.
    
     Valid for objects with large display lists.
 *---------------------------------------------------------------------------*/
static NNSG3dGeBuffer* NNS_G3dGeBuffer = NULL;


////////////////////////////////////////////////////////////////////////////////
//
// APIs
//


/*---------------------------------------------------------------------------*
    NNS_G3dGeIsSendDLBusy

    Returns whether there are DMA transfers in process to the geometry engine for geometry commands running for G3D.
    
 *---------------------------------------------------------------------------*/
BOOL
NNS_G3dGeIsSendDLBusy(void)
{
    return NNS_G3dFlagGXDmaAsync;
}


/*---------------------------------------------------------------------------*
    NNS_G3dGeIsBufferExist

    Returns whether or nor the command buffer exists.
 *---------------------------------------------------------------------------*/
BOOL
NNS_G3dGeIsBufferExist(void)
{
    return (NNS_G3dGeBuffer != NULL);
}


/*---------------------------------------------------------------------------*
    NNS_G3dGeSetBuffer

    When the geometry command buffer has yet to be set by the NNS_G3dGeBuffer function, p is set as that buffer.
    
 *---------------------------------------------------------------------------*/
void
NNS_G3dGeSetBuffer(NNSG3dGeBuffer* p)
{
    NNS_G3D_NULL_ASSERT(p);

    if (NNS_G3dGeBuffer == NULL)
    {
        p->idx = 0;
        NNS_G3dGeBuffer = p;
    }
}


/*---------------------------------------------------------------------------*
    NNS_G3dGeReleaseBuffer

    Safely removes the geometry command buffer.
    The return value is a pointer to the removed command buffer.
 *---------------------------------------------------------------------------*/
NNSG3dGeBuffer*
NNS_G3dGeReleaseBuffer(void)
{
    NNSG3dGeBuffer* p;

    NNS_G3dGeFlushBuffer();

    p = NNS_G3dGeBuffer;
    NNS_G3dGeBuffer = NULL;
    return p;
}


static NNS_G3D_INLINE void
sendNB(const void* src, void* dst, u32 szByte)
{
    MI_CpuSend32(src, dst, szByte);
}


/*---------------------------------------------------------------------------*
    NNS_G3dGeFlushBuffer

    
    Waits until transfers are complete when a geometry command is in a DMA transfer, and if there is a geometry command buffer, writes the buffer's content to the geometry engine.
    
    Once this function ends, geometry commands can be sent directly to the geometry engine (that is, it is now safe to use the SDK's G3_XXX functions).
    
 *---------------------------------------------------------------------------*/
void
NNS_G3dGeFlushBuffer(void)
{
    if (NNS_G3dFlagGXDmaAsync)
    {
        NNS_G3dGeWaitSendDL();
    }

    if (NNS_G3dGeBuffer &&
        NNS_G3dGeBuffer->idx > 0)
    {
        sendNB(&NNS_G3dGeBuffer->data[0],
               (void*)&reg_G3X_GXFIFO,
               NNS_G3dGeBuffer->idx << 2);
        NNS_G3dGeBuffer->idx = 0;
    }
}


/*---------------------------------------------------------------------------*
    NNS_G3dGeWaitSendDL

    Waits until geometry command DMA transfer (using the NNS_G3dGeSendDL function) is complete.
    
 *---------------------------------------------------------------------------*/
void
NNS_G3dGeWaitSendDL(void)
{
    while(NNS_G3dFlagGXDmaAsync)
        ;
}


/*---------------------------------------------------------------------------*
    NNS_G3dGeIsImmOK

    Returns whether it is OK to send a command directly to the FIFO (g3imm.h).
    Returns TRUE if it is OK to send the command.
    The conditions here are that the command buffer does not exist or is empty, and GXDMA transfer is not currently underway.
 *---------------------------------------------------------------------------*/
BOOL
NNS_G3dGeIsImmOK(void)
{
    return (NNS_G3dGeBuffer == NULL || NNS_G3dGeBuffer->idx == 0) &&
           !NNS_G3dGeIsSendDLBusy();
}


/*---------------------------------------------------------------------------*
    NNS_G3dGeIsBufferOK

    Returns whether or not an amount of data equal to numWord can be added to the geometry command buffer.
    Returns TRUE if the data can be added.
 *---------------------------------------------------------------------------*/
BOOL
NNS_G3dGeIsBufferOK(u32 numWord)
{
    return (NNS_G3dGeBuffer != NULL) &&
           (NNS_G3dGeBuffer->idx + numWord <= NNS_G3D_SIZE_COMBUFFER);
}


//
// Exclusively used by NNS_G3dGeSendDL
//
static void
simpleUnlock_(void* arg)
{
    *((volatile int*)arg) = 0;
}

#ifdef NNS_G3D_USE_FASTGXDMA
static BOOL NNS_G3dFlagUseFastDma = TRUE;
#else
static BOOL NNS_G3dFlagUseFastDma = FALSE;
#endif


/*---------------------------------------------------------------------------*
    NNS_G3dGeUseFastDma

    When values other than FALSE are specified for the arguments, the MI_SendGXCommandAsyncFast function is used for geometry command DMA transfers.
    In that case, follow Chapter 7 of the Programming Manual, Cautions When Running Multiple DMA Channels in Parallel on the ARM9 System Bus, to the letter.
    
    
 *---------------------------------------------------------------------------*/
void
NNS_G3dGeUseFastDma(BOOL cond)
{
    NNS_G3dFlagUseFastDma = (cond);
}


/*---------------------------------------------------------------------------*
    NNS_G3dGeSendDL

    Safely writes the display list to the Geometry Engine.
 *---------------------------------------------------------------------------*/
void
NNS_G3dGeSendDL(const void* src, u32 szByte)
{
    NNS_G3D_NULL_ASSERT(src);
    NNS_G3D_ASSERT(szByte >= 4);

    // start transmission
    if (szByte < 256 || GX_DMAID == GX_DMA_NOT_USE)
    {
        NNS_G3dGeBufferOP_N(*(const u32*)src,
                            (const u32*)src + 1,
                            (szByte >> 2) - 1);
    }
    else
    {
        // The command buffer must be flushed in advance.
        NNS_G3dGeFlushBuffer();
        NNS_G3dFlagGXDmaAsync = 1;

        // When dmaNo is GX_DMA_NOT_USE, an assertion will occur within the MI_SendGXCommand* functions. This assertion is determined only due to DMA switching.
        // 
        if (NNS_G3dFlagUseFastDma)
        {
            if (OS_IsRunOnTwl())
            {
#ifdef SDK_TWL
                u32 dmaNo = GX_GetDefaultDMA();
                if (dmaNo == GX_DMA_NOT_USE)
                {
                    dmaNo = GX_GetDefaultNDMA();
                    MI_SendNDmaGXCommandAsyncFast(dmaNo,
                                                  src,
                                                  szByte,
                                                  &simpleUnlock_,
                                                  (void*)&NNS_G3dFlagGXDmaAsync);
                }
                else
                {
                    MI_SendGXCommandAsyncFast(dmaNo,
                                              src,
                                              szByte,
                                              &simpleUnlock_,
                                              (void*)&NNS_G3dFlagGXDmaAsync);
                }
#endif
            }
            else
            {
#ifndef SDK_TWLLTD
                u32 dmaNo = GX_GetDefaultDMA();
                MI_SendGXCommandAsyncFast(dmaNo,
                                          src,
                                          szByte,
                                          &simpleUnlock_,
                                          (void*)&NNS_G3dFlagGXDmaAsync);
#endif
            }
        }
        else
        {
            if (OS_IsRunOnTwl())
            {
#ifdef SDK_TWL
                u32 dmaNo = GX_GetDefaultDMA();
                if (dmaNo == GX_DMA_NOT_USE)
                {
                    dmaNo = GX_GetDefaultNDMA();
                    MI_SendNDmaGXCommandAsync(dmaNo,
                                              src,
                                              szByte,
                                              &simpleUnlock_,
                                              (void*)&NNS_G3dFlagGXDmaAsync);
                }
                else
                {
                    MI_SendGXCommandAsync(dmaNo,
                                          src,
                                          szByte,
                                          &simpleUnlock_,
                                          (void*)&NNS_G3dFlagGXDmaAsync);
                }
#endif
            }
            else
            {
#ifndef SDK_TWLLTD
                u32 dmaNo = GX_GetDefaultDMA();
                MI_SendGXCommandAsync(dmaNo,
                                      src,
                                      szByte,
                                      &simpleUnlock_,
                                      (void*)&NNS_G3dFlagGXDmaAsync);
#endif
            }
        }
    }
}


/*---------------------------------------------------------------------------*
    NNS_G3dGeBufferOP_N

    Sends a geometry command that takes N arguments.
 *---------------------------------------------------------------------------*/
void
NNS_G3dGeBufferOP_N(u32 op, const u32* args, u32 num)
{
    if (NNS_G3dGeBuffer)
    {
        if (NNS_G3dFlagGXDmaAsync)
        {
            // Is there sufficient empty space in the buffer?
            if (NNS_G3dGeBuffer->idx + 1 + num <= NNS_G3D_SIZE_COMBUFFER)
            {
                NNS_G3dGeBuffer->data[NNS_G3dGeBuffer->idx++] = op;
                if (num > 0)
                {
                    MI_CpuCopyFast(args,
                                   &NNS_G3dGeBuffer->data[NNS_G3dGeBuffer->idx],
                                   num << 2);

                    NNS_G3dGeBuffer->idx += num;
                }
                // buffering complete
                return;
            }
        }

        // In the end, nothing has been added to the buffer.
        // Write immediately after emptying the buffer.
        if (NNS_G3dGeBuffer->idx != 0)
        {
            NNS_G3dGeFlushBuffer();
        }
        else
        {
            if (NNS_G3dFlagGXDmaAsync)
            {
                NNS_G3dGeWaitSendDL();
            }
        }
    }
    else
    {
        if (NNS_G3dFlagGXDmaAsync)
        {
            NNS_G3dGeWaitSendDL();
        }
    }

    reg_G3X_GXFIFO = op;
    sendNB(args, (void*)&reg_G3X_GXFIFO, num << 2);
}
