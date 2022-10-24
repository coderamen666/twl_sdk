/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - libraries
  File:     card_pullOut.c

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-19#$
  $Rev: 10786 $
  $Author: okajima_manabu $

 *---------------------------------------------------------------------------*/


#include <nitro/card/rom.h>
#include <nitro/card/pullOut.h>

#include "card_rom.h"

//---- User callback for card pulled out
static CARDPulledOutCallback CARD_UserCallback;

//---- Flag to be pulled out
static u32 CARDiSlotResetCount;
static BOOL CARDi_IsPulledOutFlag = FALSE;

static void CARDi_PulledOutCallback(PXIFifoTag tag, u32 data, BOOL err);
static void CARDi_SendtoPxi(u32 data, u32 wait);

/*---------------------------------------------------------------------------*
  Name:         CARD_InitPulledOutCallback

  Description:  Initialize callback setting.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_InitPulledOutCallback(void)
{
    PXI_Init();

    CARDiSlotResetCount = 0;
    CARDi_IsPulledOutFlag = FALSE;

    //---- Set PXI callback
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_CARD, CARDi_PulledOutCallback);

    //---- Init user callback
    CARD_UserCallback = NULL;
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_PulledOutCallback

  Description:  Callback to receive data from PXI.

  Arguments:    tag: Tag from PXI (unused)
                data: Data from PXI
                err: Error bit (unused)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_PulledOutCallback(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused( tag, err )

    u32     command = data & CARD_PXI_COMMAND_MASK;

    //---- Receive message 'pulled out'
    if (command == CARD_PXI_COMMAND_PULLED_OUT)
    {
        if (CARDi_IsPulledOutFlag == FALSE)
        {
            BOOL    isTerminateImm = TRUE;

            CARDi_IsPulledOutFlag = TRUE;
            CARDi_NotifyEvent(CARD_EVENT_PULLEDOUT, NULL);

            //---- Call user callback
            if (CARD_UserCallback)
            {
                isTerminateImm = CARD_UserCallback();
            }

            //---- Terminate
            if (isTerminateImm)
            {
                CARD_TerminateForPulledOut();
            }
        }
    }
    else if (command == CARD_PXI_COMMAND_RESET_SLOT)
    {
        CARDiSlotResetCount += 1;
        CARDi_IsPulledOutFlag = FALSE;
        CARDi_NotifyEvent(CARD_EVENT_SLOTRESET, NULL);
    }
    else
    {
#ifndef SDK_FINALROM
        OS_TPanic("illegal card pxi command.");
#else
        OS_TPanic("");
#endif
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARD_SetPulledOutCallback

  Description:  Set user callback for card being pulled out.

  Arguments:    callback: Callback

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_SetPulledOutCallback(CARDPulledOutCallback callback)
{
    CARD_UserCallback = callback;
}

/*---------------------------------------------------------------------------*
  Name:         CARD_IsPulledOut

  Description:  Return if have detected card pulled out.

  Arguments:    None.

  Returns:      TRUE if detected.
 *---------------------------------------------------------------------------*/
BOOL CARD_IsPulledOut(void)
{
    return CARDi_IsPulledOutFlag;
}

//================================================================================
//         TERMINATION
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         CARD_TerminateForPulledOut

  Description:  Terminate for pulling out card.
                Send message to do termination to ARM7.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_TerminateForPulledOut(void)
{
    //---- If folding, power off
    if (PAD_DetectFold())
    {
        (void)PM_ForceToPowerOff();
        //---- Power off is always successful so this point is never reached
    }

    // When the cover is not closed, send Terminate command because the ARM7 processor must be stopped
#ifdef SDK_TWL
    if ( OS_IsRunOnTwl() )
    {
        // End processing
        // If making an ARM7 processor request, wait for the callback function to complete
        PMi_ExecutePostExitCallbackList();
    }
#endif // SDK_TWL
    //---- Send 'TERMINATE' command to ARM7, and terminate itself immediately
    CARDi_SendtoPxi(CARD_PXI_COMMAND_TERMINATE, 1);

    //---- Stop all dma
    MI_StopAllDma();
#ifdef SDK_TWL
	if ( OS_IsRunOnTwl() )
	{
		MI_StopAllNDma();
	}
#endif

    OS_Terminate();
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_CheckPulledOutCore

  Description:  Main processing for the card removal detection function.
                The card bus must be locked.

  Arguments:    id: ROM-ID read from the card

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_CheckPulledOutCore(u32 id)
{
    //---- Card ID IPL had read
    vu32    iplCardID = *(vu32 *)(HW_BOOT_CHECK_INFO_BUF);
    //---- If card removal has been detected, simulate PXI-notification from ARM7
    if (id != (u32)iplCardID)
    {
        OSIntrMode bak_cpsr = OS_DisableInterrupts();
        CARDi_PulledOutCallback(PXI_FIFO_TAG_CARD, CARD_PXI_COMMAND_PULLED_OUT, FALSE);
        (void)OS_RestoreInterrupts(bak_cpsr);
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARD_CheckPulledOut

  Description:  Get whether system has detected pulled out card
                by comparing IPL cardID with current cardID
                (notice that once a card is pulled out, IDs are absolutely different)

  Arguments:    None.

  Returns:      TRUE if current cardID is equal to IPL cardID.
 *---------------------------------------------------------------------------*/
void CARD_CheckPulledOut(void)
{
    CARDi_CheckPulledOutCore(CARDi_ReadRomID());
}

//================================================================================
//         SEND PXI COMMAND
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         CARDi_SendtoPxi

  Description:  Send data via PXI.

  Arguments:    data: Data to send

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_SendtoPxi(u32 data, u32 wait)
{
    while (PXI_SendWordByFifo(PXI_FIFO_TAG_CARD, data, FALSE) != PXI_FIFO_SUCCESS)
    {
        SVC_WaitByLoop((s32)wait);
    }
}


/*---------------------------------------------------------------------------*
 * Internal functions
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         CARDi_GetSlotResetCount

  Description:  Get the number of times the slot detects reinsertion.
                Simply return the number of times the CARDi_ResetSlotStatus function was called.

  Arguments:    None.

  Returns:      Number of times slot detects reinsertion. Initial value was 0.
 *---------------------------------------------------------------------------*/
u32     CARDi_GetSlotResetCount(void)
{
    return CARDiSlotResetCount;
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_IsPulledOutEx

  Description:  Determines whether the card was pulled from the slot.

  Arguments:    count: The number of times the slot was reinserted as confirmed last time.
                       Get using the CARDi_GetSlotResetCount function.

  Returns:      If the specified number of times the slot detected reinsertion is the same as the current and the card is currently pulled out, TRUE.
                
 *---------------------------------------------------------------------------*/
BOOL    CARDi_IsPulledOutEx(u32 count)
{
    BOOL    result = FALSE;
    OSIntrMode  bak = OS_DisableInterrupts();
    {
        result = ((count == CARDi_GetSlotResetCount()) && !CARD_IsPulledOut());
    }
    (void)OS_RestoreInterrupts(bak);
    return result;
}
