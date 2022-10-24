/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS
  File:     os_pxit.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-05-15#$
  $Rev: 6002 $
  $Author: naksima $
 *---------------------------------------------------------------------------*/
#include        <nitro.h>


//---------------- local variables
static vu16 OSi_IsResetOccurred = FALSE;
static vu16 OSi_IsTerminateOccurred = FALSE;


//================================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_IsResetOccurred

  Description:  get status whethre reset event occurred

  Arguments:    None.

  Returns:      TRUE  : if reset event occurred
                FALSE : if not
 *---------------------------------------------------------------------------*/
BOOL OS_IsResetOccurred(void)
{
    return OSi_IsResetOccurred;
}

#ifdef SDK_TWL

/*---------------------------------------------------------------------------*
  Name:         OSi_IsTerminatePxiOccurred

  Description:  get status whethre terminate event occurred

  Arguments:    None.

  Returns:      TRUE  : if terminate event occurred
                FALSE : if not
 *---------------------------------------------------------------------------*/
BOOL OSi_IsTerminatePxiOccurred(void)
{
    return OSi_IsTerminateOccurred;
}

/*---------------------------------------------------------------------------*
  Name:         OSi_SetTerminatePxiOccurred

  Description:  set status of terminate event occurred

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void OSi_SetTerminatePxiOccurred(void)
{
    OSi_IsTerminateOccurred = TRUE;
}

#endif // SDK_TWL

/*---------------------------------------------------------------------------*
  Name:         OSi_CommonCallback

  Description:  PXI callback for OS tag

  Arguments:    tag  : not used
                data : data sent from another processor
                err  : not used

  Returns:      None.
 *---------------------------------------------------------------------------*/
void OSi_CommonCallback(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused( tag, err )
    u16     command;
    u16     commandArg;

    command = (u16)((data & OS_PXI_COMMAND_MASK) >> OS_PXI_COMMAND_SHIFT);
    commandArg = (u16)((data & OS_PXI_DATA_MASK) >> OS_PXI_DATA_SHIFT);

    //---------------- ARM9
#ifdef SDK_ARM9
    //---- reveice reset command (mean ARM7 is prepared to do reset)
    if (command == OS_PXI_COMMAND_RESET)
    {
        OSi_IsResetOccurred = TRUE;
    }
#ifdef SDK_TWL
    //---- reveice terminate command
    else if (command == OS_PXI_COMMAND_TERMINATE)
    {
        OS_TPrintf("[ARM9] received terminate command\n");
        OSi_IsTerminateOccurred = TRUE;
        OS_Terminate();
    }
#endif // SDK_TWL
    //---- unknown command
    else
    {
        OS_TPanic("[ARM9] unknown command");
    }

    //---------------- ARM7
#else
    //---- reveice reset command (mean ARM9 request for ARM7 to do reset)
    if (command == OS_PXI_COMMAND_RESET)
    {
        OSi_IsResetOccurred = TRUE;
    }
#ifdef SDK_TWL
    //---- reveice terminate command
    else if (command == OS_PXI_COMMAND_TERMINATE)
    {
        OSi_IsTerminateOccurred = TRUE;
        OS_Terminate();
    }
#endif // SDK_TWL
    //---- unknown command
    else
    {
        OS_TPanic("[ARM7] unknown command");
    }
#endif
}

/*---------------------------------------------------------------------------*
  Name:         OSi_SendToPxi

  Description:  send data to PXI interface

  Arguments:    data : data to be sent

  Returns:      None.
 *---------------------------------------------------------------------------*/
void OSi_SendToPxi(u16 data)
{
    u32     pxi_send_data;

    pxi_send_data = ((u32)data) << OS_PXI_COMMAND_SHIFT;
    while (PXI_SendWordByFifo(PXI_FIFO_TAG_OS, pxi_send_data, FALSE) != PXI_FIFO_SUCCESS)
    {
    }
}

