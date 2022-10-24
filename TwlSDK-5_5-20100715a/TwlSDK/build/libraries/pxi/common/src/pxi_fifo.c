/*---------------------------------------------------------------------------*
  Project:  TwlSDK - PXI - libraries
  File:     pxi_fifo.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-19#$
  $Rev: 10786 $
  $Author: okajima_manabu $
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL
#include  <twl.h>
#else
#include  <nitro.h>
#endif

static u16 FifoCtrlInit = 0;
static PXIFifoCallback FifoRecvCallbackTable[PXI_MAX_FIFO_TAG];

/*********** function prototypes ******************/
static inline PXIFifoStatus PXIi_GetFromFifo(u32 *data_buf);
static inline PXIFifoStatus PXIi_SetToFifo(u32 data);


/*---------------------------------------------------------------------------*
  Name:         PXI_InitFifo

  Description:  initialize FIFO system

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PXI_InitFifo(void)
{
    int     i;
    OSIntrMode enabled;
    OSSystemWork *p = OS_GetSystemWork();

    enabled = OS_DisableInterrupts();

    if (!FifoCtrlInit)
    {
        FifoCtrlInit = TRUE;

        p->pxiHandleChecker[PXI_PROC_ARM] = 0UL;

        for (i = 0; i < PXI_MAX_FIFO_TAG; i++)
        {
            FifoRecvCallbackTable[i] = NULL;
        }

        reg_PXI_FIFO_CNT =
            (REG_PXI_FIFO_CNT_SEND_CL_MASK |
             REG_PXI_FIFO_CNT_RECV_RI_MASK | REG_PXI_FIFO_CNT_E_MASK | REG_PXI_FIFO_CNT_ERR_MASK);

        (void)OS_ResetRequestIrqMask(OS_IE_FIFO_RECV);
        (void)OS_SetIrqFunction(OS_IE_FIFO_RECV, PXIi_HandlerRecvFifoNotEmpty);
        (void)OS_EnableIrqMask(OS_IE_FIFO_RECV);

#ifndef SDK_FINALROM
        //
        // Send signal to let ensata know the time of PXI initialized
        //              Requested by ensata dev team 04/03/10
        //
        if (OSi_DetectEmulator())
        {
            reg_PXI_INTF = 0x100;
        }
        else
#endif
#ifdef  SDK_ARM7
        {
            //-------------------------------
            // PXI_INTF
            //   d14     RW  IE
            //   d13     W   IREQ
            //   d11-d08 RW  OUT Status
            //   d03-d00 R   IN  Status
            //-------------------------------
            for (i = 8; i >= 0; i--)
            {
                reg_PXI_INTF = (u16)(i << 8);
                OS_SpinWait(1000);     // Yield main bus to ARM9

                if ((reg_PXI_INTF & 15) != i)
                {
                    i = 8;
                }
            }
        }
#else
        {
            int     timeout;
            s32     c;

            for (i = 0;; i++)
            {
                c = reg_PXI_INTF & 15;
                reg_PXI_INTF = (u16)(c << 8);

                if (c == 0 && i > 4)
                {
                    break;
                }

                for (timeout = 1000; (reg_PXI_INTF & 15) == c; timeout--)
                {
                    if (timeout == 0)
                    {
                        i = 0;
                        break;
                    }
                }
            }
        }
#endif
    }
    (void)OS_RestoreInterrupts(enabled);
}


/*---------------------------------------------------------------------------*
  Name:         PXI_SetFifoRecvCallback

  Description:  set callback function when data arrive via FIFO

  Arguments:    fifotag    fifo tag NO (0-31)
                callback   callback function to be called

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PXI_SetFifoRecvCallback(int fifotag, PXIFifoCallback callback)
{
    OSIntrMode enabled;
    OSSystemWork *p = OS_GetSystemWork();

    SDK_WARNING(callback == NULL ||
                FifoRecvCallbackTable[fifotag] == NULL ||
                FifoRecvCallbackTable[fifotag] == callback,
                "Fifo Callback overridden [fifotag=%d]\n", fifotag);

    enabled = OS_DisableInterrupts();

    FifoRecvCallbackTable[fifotag] = callback;
    if (callback)
    {
        p->pxiHandleChecker[PXI_PROC_ARM] |= (1UL << fifotag);
    }
    else
    {
        p->pxiHandleChecker[PXI_PROC_ARM] &= ~(1UL << fifotag);
    }
    (void)OS_RestoreInterrupts(enabled);
}

/*---------------------------------------------------------------------------*
  Name:         PXI_IsCallbackReady

  Description:  check if callback is ready

  Arguments:    fifotag    fifo tag NO (0-31)
                proc       processor name PXI_PROC_ARM9 or PXI_PROC_ARM7

  Returns:      True if callback is ready
 *---------------------------------------------------------------------------*/
BOOL PXI_IsCallbackReady(int fifotag, PXIProc proc)
{
    OSSystemWork *p = OS_GetSystemWork();

    return (p->pxiHandleChecker[proc] & (1UL << fifotag)) ? TRUE : FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         PXI_SetFifoSendCallback

  Description:  set callback function when FIFO empty

  Arguments:    callback   callback function to be called

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PXI_SetFifoSendCallback(PXIFifoEmtpyCallback callback)
{
    OSIntrMode enabled;

    enabled = OS_DisableInterrupts();

    (void)OS_SetIrqFunction(OS_IE_FIFO_SEND, callback);
    (void)OS_EnableIrqMask(OS_IE_FIFO_SEND);

    reg_PXI_FIFO_CNT |= REG_PXI_FIFO_CNT_SEND_TI_MASK;

    (void)OS_RestoreInterrupts(enabled);
}


/*---------------------------------------------------------------------------*
  Name:         PXI_SendWordByFifo

  Description:  Send 32bit-word to anothre CPU via FIFO

  Arguments:    fifotag  fifo tag NO (0-31)
                data     data(26-bit) whichi is sent
                err      1 on error

  Returns:      if error occured, returns minus value
 *---------------------------------------------------------------------------*/
int PXI_SendWordByFifo(int fifotag, u32 data, BOOL err)
{
    PXIFifoMessage fifomsg;

    SDK_TASSERTMSG(0 <= fifotag && fifotag < PXI_MAX_FIFO_TAG, "[FifoTag] out of range");
    SDK_TASSERTMSG(data < (1UL << PXI_FIFOMESSAGE_BITSZ_DATA), "[data] out of range");

    fifomsg.e.tag = (PXIFifoTag)fifotag;
    fifomsg.e.err = (u32)err;
    fifomsg.e.data = data;

    return PXIi_SetToFifo(fifomsg.raw);
}


//======================================================================
//                      Write Send-FIFO reg.
//======================================================================
static inline PXIFifoStatus PXIi_SetToFifo(u32 data)
{
    OSIntrMode enabled;

    if (reg_PXI_FIFO_CNT & REG_PXI_FIFO_CNT_ERR_MASK)
    {
        reg_PXI_FIFO_CNT |= (REG_PXI_FIFO_CNT_E_MASK | REG_PXI_FIFO_CNT_ERR_MASK);
        return PXI_FIFO_FAIL_SEND_ERR;
    }

    enabled = OS_DisableInterrupts();
    if (reg_PXI_FIFO_CNT & REG_PXI_FIFO_CNT_SEND_FULL_MASK)
    {
        (void)OS_RestoreInterrupts(enabled);
        return PXI_FIFO_FAIL_SEND_FULL;
    }

    reg_PXI_SEND_FIFO = data;
    (void)OS_RestoreInterrupts(enabled);
    return PXI_FIFO_SUCCESS;
}


//======================================================================
//                      Read Send-FIFO reg.
//======================================================================
static inline PXIFifoStatus PXIi_GetFromFifo(u32 *data_buf)
{

    OSIntrMode enabled;

    if (reg_PXI_FIFO_CNT & REG_PXI_FIFO_CNT_ERR_MASK)
    {
        reg_PXI_FIFO_CNT |= (REG_PXI_FIFO_CNT_E_MASK | REG_PXI_FIFO_CNT_ERR_MASK);
        return PXI_FIFO_FAIL_RECV_ERR;
    }

    enabled = OS_DisableInterrupts();
    if (reg_PXI_FIFO_CNT & REG_PXI_FIFO_CNT_RECV_EMP_MASK)
    {
        (void)OS_RestoreInterrupts(enabled);
        return PXI_FIFO_FAIL_RECV_EMPTY;
    }

    *data_buf = reg_PXI_RECV_FIFO;
    (void)OS_RestoreInterrupts(enabled);

    return PXI_FIFO_SUCCESS;
}


//======================================================================
//      Interrupt handler called when SEND FIFO empty
//======================================================================
static void PXIi_HandlerSendFifoEmpty(void)
{
}

//======================================================================
//              Interrupt handler called when RECV FIFO not empty
//======================================================================
#ifdef  SDK_CW_WA_OPT_BLX
#pragma     optimization_level  1      // for BLX problem
#endif

void PXIi_HandlerRecvFifoNotEmpty(void)
{
    PXIFifoMessage fifomsg;
    PXIFifoStatus ret_code;
    PXIFifoTag tag;

    while (1)
    {
        ret_code = PXIi_GetFromFifo(&fifomsg.raw);

        if (ret_code == PXI_FIFO_FAIL_RECV_EMPTY)
            break;
        if (ret_code == PXI_FIFO_FAIL_RECV_ERR)
            continue;

        tag = (PXIFifoTag)fifomsg.e.tag;

        if (tag)
        {
            if (FifoRecvCallbackTable[tag])
            {
                (FifoRecvCallbackTable[tag]) (tag, fifomsg.e.data, (BOOL)fifomsg.e.err);
            }
            else
            {
                if (fifomsg.e.err)     /* reject if ping-pong message */
                {
                }
                else
                {
                    fifomsg.e.err = TRUE;       /* Set error & block send */
                    (void)PXIi_SetToFifo(fifomsg.raw);
                }
            }
        }
        else
        {
            /* Extended FIFO command */
        }
    }
}

#ifdef  SDK_CW_WA_OPT_BLX
#pragma     optimization_level  4
#endif
