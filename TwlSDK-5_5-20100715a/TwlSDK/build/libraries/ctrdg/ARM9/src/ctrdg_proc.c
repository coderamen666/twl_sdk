/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CTRDG - libraries - ARM9
  File:     ctrdg_proc.c

  Copyright 2008-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-02-02#$
  $Rev: 9951 $
  $Author: yada $
 *---------------------------------------------------------------------------*/
#include <nitro.h>
#include <../include/ctrdg_work.h>

extern void CTRDGi_InitCommon(void);
extern void CTRDGi_SendtoPxi(u32 data);

//---- PXI callbacks
static void CTRDGi_CallbackForInitModuleInfo(PXIFifoTag tag, u32 data, BOOL err);
static void CTRDGi_PulledOutCallback(PXIFifoTag tag, u32 data, BOOL err);
static void CTRDGi_CallbackForSetPhi(PXIFifoTag tag, u32 data, BOOL err);

extern CTRDGWork CTRDGi_Work;

//---- Lock for pxi
static int CTRDGi_Lock = FALSE;

//---- User callback
CTRDGPulledOutCallback CTRDG_UserCallback = NULL;

/* Game Pak removal callback is not called twice */
static BOOL isCartridgePullOut = FALSE;
static BOOL skipCheck = FALSE;
static BOOL ctrdg_already_pullout = FALSE;

#ifndef SDK_TWLLTD
//---- initialized flag
static BOOL isInitialized = FALSE;
#endif

//================================================================================
//            INIT
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         CTRDGi_IsInitialized

  Description:  check weather ctrdg library is initialized
                (for internal use)

  Arguments:    None

  Returns:      TRUE ... initialized already
                FALSE... not initialized
 *---------------------------------------------------------------------------*/
#ifndef SDK_TWLLTD
BOOL CTRDGi_IsInitialized(void)
{
    return isInitialized;
}
#endif

/*---------------------------------------------------------------------------*
  Name:         CTRDG_Init

  Description:  initialize cartridge functions

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
#ifndef SDK_TWLLTD
SDK_WEAK_SYMBOL void CTRDG_Init(void)
{
    if (isInitialized)
    {
        return;
    }
    isInitialized = TRUE;

    CTRDGi_InitCommon();

    /* So that the Game Pak removal callback cannot be called two or more times */
    ctrdg_already_pullout = FALSE;

#ifndef SDK_SMALL_BUILD
    //---- Setting PXI
    PXI_Init();
    while (!PXI_IsCallbackReady(PXI_FIFO_TAG_CTRDG, PXI_PROC_ARM7))
    {
    }
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_CTRDG, CTRDGi_CallbackForInitModuleInfo);

    CTRDGi_InitModuleInfo();

    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_CTRDG, NULL);  // To avoid warning by overriding
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_CTRDG, CTRDGi_PulledOutCallback);

    //---- Init user callback
    CTRDG_UserCallback = NULL;

    // Create a TaskThread to use the asynchronous version of the AGBBackup access function
    {
        static CTRDGiTaskWork CTRDGTaskList;
        CTRDGi_InitTaskThread(&CTRDGTaskList);
    }

    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_CTRDG_PHI, CTRDGi_CallbackForSetPhi);

    /*
     * Access is disabled, as long as the user has not explicitly specified 'enable'.
     * (If it is not an option Game Pak, writing is also not allowed)
     */
#if defined(SDK_ARM9)
    CTRDG_Enable(FALSE);
#endif
#endif // SDK_SMALL_BUILD
}
#endif // SDK_TWLLTD

/*---------------------------------------------------------------------------*
  Name:         CTRDGi_InitModuleInfo

  Description:  Initialize the information of peripheral device around cartridge.

                Because try lock cartridge, in case another processor had locked,
                wait until it was released.

                To avoid other bus master interrupts,
                DMA1 is used for accessing cartridge.
                This is a compatibility in future.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static CTRDGHeader headerBuf ATTRIBUTE_ALIGN(32);

void CTRDGi_InitModuleInfo(void)
{
#ifndef SDK_SMALL_BUILD

    static BOOL isInfoInitialized;
    CTRDGLockByProc lockInfo;
    OSIrqMask lastIE;
    BOOL    lastIME;

    if (isInfoInitialized)
    {
        return;
    }
    isInfoInitialized = TRUE;

    if (!(reg_OS_PAUSE & REG_OS_PAUSE_CHK_MASK))
    {
        return;
    }

    lastIE = OS_SetIrqMask(OS_IE_SPFIFO_RECV);
    lastIME = OS_EnableIrq();

    //---- Get privilege for accessing cartridge
    CTRDGi_LockByProcessor(CTRDGi_Work.lockID, &lockInfo);

    //---- Read information of peripheral devices
    {
        MIProcessor proc = MI_GetMainMemoryPriority();
        CTRDGRomCycle rc;

        // Set the latest access cycle
        CTRDGi_ChangeLatestAccessCycle(&rc);

        // Set main memory priority for avoiding DMA stole by ARM7 access
        MI_SetMainMemoryPriority(MI_PROCESSOR_ARM9);

        // 16-bit access to have much enough access interval
        DC_InvalidateRange(&((u8 *)&headerBuf)[0x80], sizeof(headerBuf) - 0x80);
        MI_DmaCopy16(1, (void *)(HW_CTRDG_ROM + 0x80),
                     &((u8 *)&headerBuf)[0x80], sizeof(headerBuf) - 0x80);

        // Restore main memory priority
        MI_SetMainMemoryPriority(proc);

        // Restore access cycle
        CTRDGi_RestoreAccessCycle(&rc);
    }

    //---- Release privilege for accessing Game Pak
    CTRDGi_UnlockByProcessor(CTRDGi_Work.lockID, &lockInfo);

    // Update Game Pak data only if a Game Pak was inserted during the previous check or if a software reset has not even been executed once
    //  
    if ((*(u8 *)HW_IS_CTRDG_EXIST) || !(*(u8 *)HW_SET_CTRDG_MODULE_INFO_ONCE))
    {
        //---- Copy the information of peripheral devices to system area
        int     i;
        CTRDGHeader *chb = &headerBuf;
        CTRDGModuleInfo *cip = CTRDGi_GetModuleInfoAddr();

        cip->moduleID.raw = chb->moduleID;
        for (i = 0; i < 3; i++)
        {
            cip->exLsiID[i] = chb->exLsiID[i];
        }
        cip->makerCode = chb->makerCode;
        cip->gameCode = chb->gameCode;

        // Check whether or not a Game Pak is inserted
        *(u8 *)HW_IS_CTRDG_EXIST = (u8)((CTRDG_IsExisting())? 1 : 0);
        // TRUE if the Game Pak information has been updated even once
        (*(u8 *)HW_SET_CTRDG_MODULE_INFO_ONCE) = TRUE;
    }

    //---- Copy Nintendo logo data in the ARM9 system ROM to main memory
    MI_CpuCopy32((void *)CTRDG_SYSROM9_NINLOGO_ADR, &headerBuf.nintendoLogo,
                 sizeof(headerBuf.nintendoLogo));
    DC_FlushAll();

    //---- Send message to ARM7
    CTRDGi_SendtoPxi(CTRDG_PXI_COMMAND_INIT_MODULE_INFO |
                     (((u32)&headerBuf - HW_MAIN_MEM) >> 5) << CTRDG_PXI_COMMAND_PARAM_SHIFT);

    //---- Wait until ARM7 finishes initializing
    while (CTRDGi_Work.subpInitialized != TRUE)
    {
        SVC_WaitByLoop(1);
    }

    (void)OS_RestoreIrq(lastIME);
    (void)OS_SetIrqMask(lastIE);

#endif // SDK_SMALL_BUILD
}

/*---------------------------------------------------------------------------*
  Name:         CTRDGi_CallbackForInitModuleInfo

  Description:  Callback to receive data from PXI.

  Arguments:    tag: Tag from PXI (unused)
                data: Data from PXI
                err: Error bit (unused)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CTRDGi_CallbackForInitModuleInfo(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused( tag, err )

    if ((data & CTRDG_PXI_COMMAND_MASK) == CTRDG_PXI_COMMAND_INIT_MODULE_INFO)
    {
        CTRDGi_Work.subpInitialized = TRUE;
    }
    else
    {
#ifndef SDK_FINALROM
        OS_TPanic("illegal Cartridge pxi command.");
#else
        OS_TPanic("");
#endif
    }
}

/*---------------------------------------------------------------------------*
  Name:         CTRDG_DummyInit

  Description:  Initialize for smaller CTRDG.
                Called from user defined CTRDG_Init.

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
#ifndef SDK_TWLLTD
static void CTRDGi_DummyCallback(PXIFifoTag tag, u32 data, BOOL err);
void CTRDG_DummyInit(void)
{
    CTRDGi_InitCommon();

    //---- set dummy callback
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_CTRDG, NULL);  // to avoid warning by overriding
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_CTRDG, CTRDGi_DummyCallback);

    //---- send INIT_MODULE_INFO for sync with ARM7
    CTRDGi_SendtoPxi(CTRDG_PXI_COMMAND_INIT_MODULE_INFO);
}

//---- dummy callback for smaller CTRDG.
static void CTRDGi_DummyCallback(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused( tag, data, err )
    // do nothing
}
#endif // ifndef SDK_TWLLTD

//================================================================================
//            PULLOUT PROCS
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         CTRDGi_PulledOutCallback

  Description:  Callback to receive data from PXI.

  Arguments:    tag: Tag from PXI (unused)
                data: Data from PXI
                err: Error bit (unused)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CTRDGi_PulledOutCallback(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused( tag, err )

    //---- Receive message 'pulled out'
    if ((data & CTRDG_PXI_COMMAND_MASK) == CTRDG_PXI_COMMAND_PULLED_OUT)
    {
        if (ctrdg_already_pullout == FALSE)
        {
            BOOL    isTerminateImm = FALSE;

            //---- Call user callback
            if (CTRDG_UserCallback)
            {
                isTerminateImm = CTRDG_UserCallback();
            }

            //---- Terminate
            if (isTerminateImm)
            {
                CTRDG_TerminateForPulledOut();
            }
            /* If called once, it is not called again */
            ctrdg_already_pullout = TRUE;
        }
    }
    else
    {
#ifndef SDK_FINALROM
        OS_TPanic("illegal Cartridge pxi command.");
#else
        OS_TPanic("");
#endif
    }
}

/*---------------------------------------------------------------------------*
  Name:         CTRDG_SetPulledOutCallback

  Description:  Set user callback for being pulled-out Game Pak.

  Arguments:    callback: Callback

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CTRDG_SetPulledOutCallback(CTRDGPulledOutCallback callback)
{
#ifndef SDK_TWLLTD
    SDK_ASSERT(CTRDGi_IsInitialized());
#endif
    CTRDG_UserCallback = callback;
}

/*---------------------------------------------------------------------------*
  Name:         CTRDG_TerminateForPulledOut

  Description:  Terminate for pulling out Game Pak.
                Send message to do termination to ARM7.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CTRDG_TerminateForPulledOut(void)
{
#ifndef SDK_TWLLTD
    SDK_ASSERT(CTRDGi_IsInitialized());
#endif

    //---- Send 'TERMINATE' command to ARM7 and terminate itself immediately
    CTRDGi_SendtoPxi(CTRDG_PXI_COMMAND_TERMINATE);

    OS_Terminate();
}

/*---------------------------------------------------------------------------*
  Name:         CTRDG_SendToARM7

  Description:  Send data to ARM7.

  Arguments:    arg: Data to send

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CTRDG_SendToARM7(void *arg)
{
    (void)PXI_SendWordByFifo(PXI_FIFO_TAG_CTRDG_Ex, (u32)arg, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         CTRDG_CheckPulledOut

  Description:  Game Pak is pulled out.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CTRDG_CheckPulledOut(void)
{
#ifndef SDK_TWLLTD
    SDK_ASSERT(CTRDGi_IsInitialized());
#endif

    //---- If skip flag or Game Pak already pulled out.
    if (skipCheck || isCartridgePullOut)
    {
        return;
    }

    //---------------- Check Game Pak pulled out
    //---- Check Game Pak
    isCartridgePullOut = CTRDG_IsPulledOut();

    //---- If no Game Pak...
    if (!CTRDG_IsExisting())
    {
        if (!isCartridgePullOut)
        {
            skipCheck = TRUE;
            return;
        }
    }

    //---------------- If Game Pak pulled out, tell that to ARM9
    if (isCartridgePullOut)
    {
        /* Call in the same way as functions that are called by PXI from ARM7. */
        /* Arguments forcibly set to those used when processing occurs. */
        CTRDGi_PulledOutCallback(PXI_FIFO_TAG_CTRDG, CTRDG_PXI_COMMAND_PULLED_OUT, NULL);
    }
}

/*---------------------------------------------------------------------------*
  Name:         CTRDG_SetPhiClock

  Description:  Set ARM9 and ARM7 PHI output clock.

  Arguments:    clock:    PHI clock to set (MIPhiClock value)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CTRDG_SetPhiClock(CTRDGPhiClock clock)
{
    u32     data = ((u32)clock << CTRDG_PXI_COMMAND_PARAM_SHIFT) | CTRDG_PXI_COMMAND_SET_PHI;

#ifndef SDK_TWLLTD
    SDK_ASSERT(CTRDGi_IsInitialized());
#endif

    //---- Check parameter range
    SDK_ASSERT((u32)clock <= CTRDG_PHI_CLOCK_16MHZ);

    //---- Set ARM9 PHI output clock
    MIi_SetPhiClock((MIiPhiClock) clock);

    //---- Send command to set ARM7 phi clock
    CTRDGi_Lock = TRUE;
    while (PXI_SendWordByFifo(PXI_FIFO_TAG_CTRDG_PHI, data, FALSE) != PXI_FIFO_SUCCESS)
    {
        SVC_WaitByLoop(1);
    }

    //---- Wait response
    while (CTRDGi_Lock)
    {
        SVC_WaitByLoop(1);
    }
}

/*---------------------------------------------------------------------------*
  Name:         CTRDG_CallbackForSetPhi

  Description:  Callback from ARM7 to receive PHI clock setting.

  Arguments:    tag: Tag from PXI (unused)
                data: Data from PXI
                err: Error bit (unused)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CTRDGi_CallbackForSetPhi(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused(tag, data, err)
    CTRDGi_Lock = FALSE;
}
