/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS
  File:     os_terminate_proc.c

  Copyright 2006-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#include <nitro/os/common/system.h>
#include <nitro/os/common/interrupt.h>
#include <nitro/os/common/emulator.h>
#include <nitro/os/common/pxi.h>
#include <nitro/os/common/reset.h>

#ifdef SDK_TWL
#include <twl/fatfs.h>
#endif

OSTerminateCallback OSi_TerminateCallback = NULL;
void* OSi_TerminateCallbackArg = NULL;

//============================================================================
//          TERMINATE and HALT
//============================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_Terminate

  Description:  finalize and terminate
  
  Arguments:    None

  Returns:      --  (Never return)
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL void OS_Terminate(void)
{
#ifdef SDK_TWL
    static BOOL terminated = FALSE;
#endif

	//---- callback
	if ( OSi_TerminateCallback )
	{
		OSTerminateCallback callback = OSi_TerminateCallback;
		OSi_TerminateCallback = NULL;
		callback( OSi_TerminateCallbackArg );
	}

#ifdef SDK_TWL
    if ( ! terminated )
    {
        terminated = TRUE;

        if ( OS_GetProcMode() != OS_PROCMODE_IRQ )
        {
            (void)OS_EnableIrq();
            (void)OS_EnableInterrupts();
        }

        if ( OS_IsRunOnTwl() )
        {
            if ( FATFSi_IsInitialized() )
            {
                (void)FATFS_UnmountAll();
            }
        }

        //---- send 'TERMINATE' command to ARM7, and terminate itself immediately
        OSi_SendToPxi(OS_PXI_COMMAND_TERMINATE);
    }
#endif

    OSi_TerminateCore();
}

/*---------------------------------------------------------------------------*
  Name:         OSi_TerminateCore

  Description:  Halt CPU and loop
  
  Arguments:    None

  Returns:      --  (Never return)
 *---------------------------------------------------------------------------*/
void OSi_TerminateCore(void)
{
    (void)OS_DisableInterrupts();
    while (1)
    {
        OS_Halt();
    }
}

/*---------------------------------------------------------------------------*
  Name:         OSi_SetTerminateCallback

  Description:  set terminate callback
                (internal function. don't use)
  
  Arguments:    callback : callback

  Returns:      None
 *---------------------------------------------------------------------------*/
void OSi_SetTerminateCallback( OSTerminateCallback callback, void* arg )
{
	OSi_TerminateCallback = callback;
	OSi_TerminateCallbackArg = arg;
}

/*---------------------------------------------------------------------------*
  Name:         OS_Halt

  Description:  Halt CPU
  
  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
#include <nitro/code32.h>
SDK_WEAK_SYMBOL asm void OS_Halt( void )
{
        mov     r0, #0
        mcr     p15, 0, r0, c7, c0, 4
        bx      lr
}
#include <nitro/codereset.h>

/*---------------------------------------------------------------------------*
  Name:         OS_Exit / OS_FExit

  Description:  Display exit string and Terminate.
                This is useful for 'loadrun' tool command.                 
  
  Arguments:    console : console to output string
                status  : exit status

  Returns:      --  (Never return)
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL void OS_Exit(int status)
{
	OS_FExit( OS_PRINT_OUTPUT_ERROR, status );
}

SDK_WEAK_SYMBOL void OS_FExit(int console, int status)
{
#ifdef SDK_FINALROM
#pragma unused( console, status )
#endif
    (void)OS_DisableInterrupts();
    OS_FPrintf( console, "\n" OS_EXIT_STRING, status);
    OS_Terminate();
}
