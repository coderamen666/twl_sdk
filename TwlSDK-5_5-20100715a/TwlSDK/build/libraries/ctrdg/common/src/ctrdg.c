/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CTRDG - include
  File:     ctrdg.c

  Copyright 2008-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-01-28#$
  $Rev: 9923 $
  $Author: yada $
 *---------------------------------------------------------------------------*/
#include <nitro/ctrdg.h>
#include <../include/ctrdg_work.h>
#include <nitro/os/ARM9/cache.h>

//----------------------------------------------------------------------------

CTRDGWork CTRDGi_Work;

/* cartridge permission */
static BOOL CTRDGi_EnableFlag = FALSE;


/*---------------------------------------------------------------------------*
  Name:         CTRDG_IsExistedAtInit

  Description:  get whether cartridge existed

  Arguments:    None

  Returns:      TRUE if existing
 *---------------------------------------------------------------------------*/
static BOOL CTRDG_IsExistedAtInit(void)
{
#ifdef SDK_TWLLTD
    return FALSE;
#else
    CTRDGModuleInfo *cip = CTRDGi_GetModuleInfoAddr();

    return cip->moduleID.raw != 0xffff ? TRUE : FALSE;
#endif
}

/*---------------------------------------------------------------------------*
  Name:         CTRDGi_InitCommon

  Description:  common routine called from CTRDG_Init.
                keep lockID for cartridge functions.

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CTRDGi_InitCommon(void)
{
    SVC_CpuClear(0, &CTRDGi_Work, sizeof(CTRDGi_Work), 32);     // use SVC_* intentionally.

    CTRDGi_Work.lockID = (u16)OS_GetLockID();
}

/*---------------------------------------------------------------------------*
  Name:         CTRDG_IsBitID

  Description:  return whether peripheral device which is specified by bitID exists
                in cartridge

  Arguments:    bitID  bit ID

  Returns:      TRUE if existed
 *---------------------------------------------------------------------------*/

BOOL CTRDG_IsBitID(u8 bitID)
{
    return (CTRDG_IsExisting() && CTRDGi_IsBitIDAtInit(bitID));
}

/*---------------------------------------------------------------------------*
  Name:         CTRDGi_IsBitIDAtInit

  Description:  return whether peripheral device which is specified by bitID existed
                in cartridge at boot time

  Arguments:    bitID  bit ID

  Returns:      TRUE if existed
 *---------------------------------------------------------------------------*/
BOOL CTRDGi_IsBitIDAtInit(u8 bitID)
{
    BOOL    retval = FALSE;

    CTRDGModuleInfo *cip = CTRDGi_GetModuleInfoAddr();

    if (cip->moduleID.raw != 0xffff && cip->moduleID.raw != 0x0000 && ~cip->moduleID.bitID & bitID)
    {
        retval = TRUE;
    }

    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         CTRDG_IsNumberID

  Description:  return whether peripheral device which is specified by numberID exists
                in cartridge

  Arguments:    numberID  number ID

  Returns:      TRUE if existed
 *---------------------------------------------------------------------------*/
BOOL CTRDG_IsNumberID(u8 numberID)
{
    return (CTRDG_IsExisting() && CTRDGi_IsNumberIDAtInit(numberID));
}

/*---------------------------------------------------------------------------*
  Name:         CTRDGi_IsNumberIDAtInit

  Description:  return whether peripheral device which is specified by numberID existed
                in cartridge at boot time

  Arguments:    numberID  number ID

  Returns:      TRUE if existed
 *---------------------------------------------------------------------------*/
BOOL CTRDGi_IsNumberIDAtInit(u8 numberID)
{
    BOOL    retval = FALSE;

    CTRDGModuleInfo *cip = CTRDGi_GetModuleInfoAddr();

    if (cip->moduleID.raw != 0xffff && cip->moduleID.raw != 0x0000)
    {
        if (cip->moduleID.numberID == numberID)
        {
            retval = TRUE;
        }
        else if (!cip->moduleID.disableExLsiID)
        {
            if (cip->exLsiID[0] == numberID
                || cip->exLsiID[1] == numberID || cip->exLsiID[2] == numberID)
            {
                retval = TRUE;
            }
        }
    }

    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         CTRDG_IsAgbCartridge

  Description:  return whether AGB cartridge exists

  Arguments:    None

  Returns:      TRUE if existed
 *---------------------------------------------------------------------------*/
BOOL CTRDG_IsAgbCartridge(void)
{
    return (CTRDG_IsExisting() && CTRDGi_IsAgbCartridgeAtInit());
}

/*---------------------------------------------------------------------------*
  Name:         CTRDG_IsOptionCartridge

  Description:  return whether option cartridge exists

  Arguments:    None

  Returns:      TRUE if existed
 *---------------------------------------------------------------------------*/
BOOL CTRDG_IsOptionCartridge(void)
{
    return (CTRDG_IsExisting() && !CTRDGi_IsAgbCartridgeAtInit());
}

/*---------------------------------------------------------------------------*
  Name:         CTRDGi_IsAgbCartridgeAtInit

  Description:  return whether AGB cartridge existed at boot time

  Arguments:    None

  Returns:      TRUE if existed
 *---------------------------------------------------------------------------*/
BOOL CTRDGi_IsAgbCartridgeAtInit(void)
{
    CTRDGModuleInfo *cip = CTRDGi_GetModuleInfoAddr();

    return cip->isAgbCartridge;
}

/*---------------------------------------------------------------------------*
  Name:         CTRDG_GetAgbGameCode

  Description:  get game code in AGB cartridge

  Arguments:    None

  Returns:      Game code if exist, FALSE if cartridge not exist
 *---------------------------------------------------------------------------*/
u32 CTRDG_GetAgbGameCode(void)
{
    u32     retval = FALSE;

    if (CTRDG_IsExisting())
    {
        retval = CTRDGi_GetAgbGameCodeAtInit();
    }

    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         CTRDGi_GetAgbGameCodeAtInit

  Description:  get game code in AGB cartridge read at boot time

  Arguments:    None

  Returns:      Game code if exist, FALSE if cartridge did not exist
 *---------------------------------------------------------------------------*/
u32 CTRDGi_GetAgbGameCodeAtInit(void)
{
    CTRDGModuleInfo *cip = CTRDGi_GetModuleInfoAddr();
    u32     retval = FALSE;

    if (CTRDGi_IsAgbCartridgeAtInit())
    {
        retval = cip->gameCode;
    }

    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         CTRDG_GetAgbMakerCode

  Description:  get maker code in AGB cartridge

  Arguments:    None

  Returns:      Maker code if exist, FALSE if cartridge not exist
 *---------------------------------------------------------------------------*/
u16 CTRDG_GetAgbMakerCode(void)
{
    u16     retval = FALSE;

    if (CTRDG_IsExisting())
    {
        retval = CTRDGi_GetAgbMakerCodeAtInit();
    }

    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         CTRDGi_GetAgbMakerCodeAtInit

  Description:  get maker code in AGB cartridge read at boot time

  Arguments:    None

  Returns:      Maker code if exist, FALSE if not exist
 *---------------------------------------------------------------------------*/
u16 CTRDGi_GetAgbMakerCodeAtInit(void)
{
    CTRDGModuleInfo *cip = CTRDGi_GetModuleInfoAddr();
    u16     retval = FALSE;

    if (CTRDGi_IsAgbCartridgeAtInit())
    {
        retval = cip->makerCode;
    }

    return retval;
}


/*---------------------------------------------------------------------------*
  Name:         CTRDG_IsPulledOut

  Description:  get whether system has detected pulled out cartridge

  Arguments:    None

  Returns:      TRUE if detect pull out
 *---------------------------------------------------------------------------*/
BOOL CTRDG_IsPulledOut(void)
{
    CTRDGModuleInfo *cip = CTRDGi_GetModuleInfoAddr();

#ifndef SDK_TWLLTD
    SDK_ASSERT(CTRDGi_IsInitialized());
#endif

    //---- cartridge not exist at boot time
    if (!CTRDG_IsExistedAtInit())
    {
        return FALSE;
    }

    //---- check existing when not detect pulled out
    if (!cip->detectPullOut)
    {
        (void)CTRDG_IsExisting();
    }

    return cip->detectPullOut;
}

/*---------------------------------------------------------------------------*
  Name:         CTRDG_IsAgbCartridgePulledOut

  Description:  get whether system has detected pulled out AGB cartridge

  Arguments:    None

  Returns:      TRUE if detect pull out
 *---------------------------------------------------------------------------*/
BOOL CTRDG_IsAgbCartridgePulledOut(void)
{
    return (CTRDG_IsPulledOut() && CTRDGi_IsAgbCartridgeAtInit());
}

/*---------------------------------------------------------------------------*
  Name:         CTRDG_IsOptionCartridgePulledOut

  Description:  get whether system has detected pulled out option cartridge

  Arguments:    None

  Returns:      TRUE if detect pull out
 *---------------------------------------------------------------------------*/
BOOL CTRDG_IsOptionCartridgePulledOut(void)
{
    return (CTRDG_IsPulledOut() && !CTRDGi_IsAgbCartridgeAtInit());
}

/*---------------------------------------------------------------------------*
  Name:         CTRDG_IsExisting

  Description:  get whether cartridge exists

  Arguments:    None

  Returns:      TRUE if existing
 *---------------------------------------------------------------------------*/
BOOL CTRDG_IsExisting(void)
{
#ifdef SDK_TWLLTD
    return FALSE;
#else
    BOOL    retval = TRUE;
    CTRDGLockByProc lockInfo;

    CTRDGHeader *chp = CTRDGi_GetHeaderAddr();
    CTRDGModuleInfo *cip = CTRDGi_GetModuleInfoAddr();

    SDK_ASSERT(CTRDGi_IsInitialized());

    //---- if cartridge not exist at boot time.
    if (!CTRDG_IsExistedAtInit())
    {
        return FALSE;
    }

    //---- if detect cartirdge pulled out
    if (cip->detectPullOut == TRUE)
    {
        return FALSE;
    }
#if defined(SDK_ARM7)
    //---- get privilege for accessing cartridge
    if (CTRDGi_LockByProcessor(CTRDGi_Work.lockID, &lockInfo) == FALSE)
    {
        (void)OS_RestoreInterrupts(lockInfo.irq);
        return TRUE;
    }
#else
    //---- get privilege for accessing cartridge
    CTRDGi_LockByProcessor(CTRDGi_Work.lockID, &lockInfo);
#endif

    //---- check if cartridge exists
    {
        CTRDGRomCycle rc;
        u8      isRomCode;

        // set the lastest access cycle
        CTRDGi_ChangeLatestAccessCycle(&rc);
        isRomCode = chp->isRomCode;

        // ( please observe comparison order to the following )
        if ((isRomCode == CTRDG_IS_ROM_CODE && cip->moduleID.raw != chp->moduleID)      // memory loaded
            || (isRomCode != CTRDG_IS_ROM_CODE && cip->moduleID.raw != *CTRDGi_GetModuleIDImageAddr())  // memory not loaded
            || ((cip->gameCode != chp->gameCode) && cip->isAgbCartridge))       // AGB cartridge comparison
        {
            cip->detectPullOut = TRUE;
            retval = FALSE;
        }

        //---- restore access cycle
        CTRDGi_RestoreAccessCycle(&rc);
    }

    //---- release privilege for accessing cartridge
    CTRDGi_UnlockByProcessor(CTRDGi_Work.lockID, &lockInfo);

    return retval;
#endif // SDK_TWLLTD
}

/*---------------------------------------------------------------------------*
  Name:         CTRDGi_ChangeLatestAccessCycle

  Description:  set access cycle to cartridge to latest setting

  Arguments:    r :  Cartridge ROM access cycle

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CTRDGi_ChangeLatestAccessCycle(CTRDGRomCycle *r)
{
    r->c1 = MI_GetCartridgeRomCycle1st();
    r->c2 = MI_GetCartridgeRomCycle2nd();

    MI_SetCartridgeRomCycle1st(MI_CTRDG_ROMCYCLE1_18);
    MI_SetCartridgeRomCycle2nd(MI_CTRDG_ROMCYCLE2_6);
}

/*---------------------------------------------------------------------------*
  Name:         CTRDGi_RestoreAccessCycle

  Description:  set access cycle to cartridge to the original setting

  Arguments:    r :  Cartridge ROM access cycle

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CTRDGi_RestoreAccessCycle(CTRDGRomCycle *r)
{
    MI_SetCartridgeRomCycle1st(r->c1);
    MI_SetCartridgeRomCycle2nd(r->c2);
}


/*---------------------------------------------------------------------------*
  Name:         CTRDGi_LockByProcessor

  Description:  get privilege for accessing cartridge to specified processor

                Because try lock cartridge, in case another processor had locked,
                wait till its was released.

                Status of interrupt in return is disable.

  Arguments:    lockID : lock ID for cartridge
                info   : info for lock by my processor

  Returns:      None.
 *---------------------------------------------------------------------------*/
#if defined(SDK_ARM7)
BOOL CTRDGi_LockByProcessor(u16 lockID, CTRDGLockByProc *info)
#else
void CTRDGi_LockByProcessor(u16 lockID, CTRDGLockByProc *info)
#endif
{
    while (1)
    {
        info->irq = OS_DisableInterrupts();

        if (((info->locked = OS_ReadOwnerOfLockCartridge() & CTRDG_LOCKED_BY_MYPROC_FLAG) != 0)
            || (OS_TryLockCartridge(lockID) == OS_LOCK_SUCCESS))
        {
#if defined(SDK_ARM7)
            return TRUE;
#else
            break;
#endif
        }
#if defined(SDK_ARM7)
        return FALSE;
#endif
        (void)OS_RestoreInterrupts(info->irq);

        SVC_WaitByLoop(1);
    }
}

/*---------------------------------------------------------------------------*
  Name:         CTRDGi_UnlockByProcessor

  Description:  release privilege for accessing cartirige 

  Arguments:    lockID : lock ID for cartridge
                info   : info for lock by my processor
                         it must be the value got in CTRDGi_LockCartridgeByProccesser.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CTRDGi_UnlockByProcessor(u16 lockID, CTRDGLockByProc *info)
{
    if (!info->locked)
    {
        (void)OS_UnLockCartridge(lockID);
    }

    (void)OS_RestoreInterrupts(info->irq);
}

/*---------------------------------------------------------------------------*
  Name:         CTRDGi_SendtoPxi

  Description:  send data via PXI

  Arguments:    data : data to send

  Returns:      None
 *---------------------------------------------------------------------------*/
void CTRDGi_SendtoPxi(u32 data)
{
    while (PXI_SendWordByFifo(PXI_FIFO_TAG_CTRDG, data, FALSE) != PXI_FIFO_SUCCESS)
    {
        SVC_WaitByLoop(1);
    }
}

//================================================================================
//       READ DATA FROM CARTRIDGE
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         CTRDG_DmaCopy16 / 32

  Description:  read cartridge data via DMA

  Arguments:    dmaNo : DMA No.
                src   : source address (in cartridge)
                dest  : destination address (in memory)
                size  : forwarding size

  Returns:      TRUE if success.
                FALSE if fail. ( no cartridge )
 *---------------------------------------------------------------------------*/
BOOL CTRDG_DmaCopy16(u32 dmaNo, const void *src, void *dest, u32 size)
{
#ifndef SDK_TWLLTD
    SDK_ASSERT(CTRDGi_IsInitialized());
#endif
    return CTRDGi_CopyCommon(dmaNo, src, dest, size, CTRDGi_FORWARD_DMA16);
}
BOOL CTRDG_DmaCopy32(u32 dmaNo, const void *src, void *dest, u32 size)
{
#ifndef SDK_TWLLTD
    SDK_ASSERT(CTRDGi_IsInitialized());
#endif
    return CTRDGi_CopyCommon(dmaNo, src, dest, size, CTRDGi_FORWARD_DMA32);
}

/*---------------------------------------------------------------------------*
  Name:         CTRDG_CpuCopy8 / 16 / 32

  Description:  read cartridge data by CPU access

  Arguments:    src   : source address (in cartridge)
                dest  : destination address (in memory)
                size  : forwarding size

  Returns:      TRUE if success.
                FALSE if fail. ( no cartridge )
 *---------------------------------------------------------------------------*/
BOOL CTRDG_CpuCopy8(const void *src, void *dest, u32 size)
{
#ifndef SDK_TWLLTD
    SDK_ASSERT(CTRDGi_IsInitialized());
#endif
    if (HW_CTRDG_ROM <= (u32)dest && (u32)dest < HW_CTRDG_RAM_END)
    {
        return CTRDGi_CopyCommon(0 /*dummy */ , (const void *)dest, (void *)src, size,
                                 CTRDGi_FORWARD_CPU8);
    }
    else
    {
        return CTRDGi_CopyCommon(0 /*dummy */ , src, dest, size, CTRDGi_FORWARD_CPU8);
    }
}
BOOL CTRDG_CpuCopy16(const void *src, void *dest, u32 size)
{
#ifndef SDK_TWLLTD
    SDK_ASSERT(CTRDGi_IsInitialized());
#endif
    return CTRDGi_CopyCommon(0 /*dummy */ , src, dest, size, CTRDGi_FORWARD_CPU16);
}
BOOL CTRDG_CpuCopy32(const void *src, void *dest, u32 size)
{
#ifndef SDK_TWLLTD
    SDK_ASSERT(CTRDGi_IsInitialized());
#endif
    return CTRDGi_CopyCommon(0 /*dummy */ , src, dest, size, CTRDGi_FORWARD_CPU32);
}

/*---------------------------------------------------------------------------*
  Name:         CTRDGi_CopyCommon

  Description:  read/write cartridge data.
                subroutine of CTRDG_CpuCopy*(), CTRDG_DmaCopy*() and CTRDG_WriteStream8().

  Arguments:    dmaNo       : DMA No.
                src         : source address
                dest        : destination address
                size        : forwarding size
                forwardType : action flag

  Returns:      TRUE if success.
                FALSE if fail. ( no cartridge )
 *---------------------------------------------------------------------------*/
BOOL CTRDGi_CopyCommon(u32 dmaNo, const void *src, void *dest, u32 size, u32 forwardType)
{
    //---- check cartridge existence
    if (!CTRDG_IsExisting())
    {
        return FALSE;
    }

    /* confirm whether application has certainly judged AGB-cartridge */
    CTRDG_CheckEnabled();

    (void)OS_LockCartridge(CTRDGi_Work.lockID);

    if ((forwardType & CTRDGi_FORWARD_TYPE_MASK) == CTRDGi_FORWARD_TYPE_DMA)
    {
        MI_StopDma(dmaNo);
        DC_FlushRange(dest, size);
    }

    switch (forwardType)
    {
    case CTRDGi_FORWARD_DMA16:
        MI_DmaCopy16(dmaNo, src, dest, size);
        break;
    case CTRDGi_FORWARD_DMA32:
        MI_DmaCopy32(dmaNo, src, dest, size);
        break;
    case CTRDGi_FORWARD_CPU16:
        MI_CpuCopy16(src, dest, size);
        break;
    case CTRDGi_FORWARD_CPU32:
        MI_CpuCopy32(src, dest, size);
        break;

    case CTRDGi_FORWARD_CPU8:
        {
            int     n;
            u8     *dest8 = (u8 *)dest;
            u8     *src8 = (u8 *)src;
            for (n = 0; n < size; n++)
            {
                *dest8++ = *src8++;
            }
        }
        break;
    }

    (void)OS_UnLockCartridge(CTRDGi_Work.lockID);

    //---- check to remove cartridge in the middle of reading
    if (!CTRDG_IsExisting())
    {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         CTRDG_Read8 / 16 / 32

  Description:  read cartridge data by CPU access

  Arguments:    address  : source address (in cartridge)
                rdata    : address to store read data

  Returns:      TRUE if success.
                FALSE if fail. ( no cartridge )
 *---------------------------------------------------------------------------*/
BOOL CTRDG_Read8(const u8 *address, u8 *rdata)
{
#ifndef SDK_TWLLTD
    SDK_ASSERT(CTRDGi_IsInitialized());
#endif
    return CTRDGi_AccessCommon((void *)address, 0 /*dummy */ , rdata, CTRDGi_ACCESS_READ8);
}
BOOL CTRDG_Read16(const u16 *address, u16 *rdata)
{
#ifndef SDK_TWLLTD
    SDK_ASSERT(CTRDGi_IsInitialized());
#endif
    return CTRDGi_AccessCommon((void *)address, 0 /*dummy */ , rdata, CTRDGi_ACCESS_READ16);
}
BOOL CTRDG_Read32(const u32 *address, u32 *rdata)
{
#ifndef SDK_TWLLTD
    SDK_ASSERT(CTRDGi_IsInitialized());
#endif
    return CTRDGi_AccessCommon((void *)address, 0 /*dummy */ , rdata, CTRDGi_ACCESS_READ32);
}

/*---------------------------------------------------------------------------*
  Name:         CTRDG_Write8 / 16 / 32

  Description:  write data to cartridge

  Arguments:    address  : destination address (in cartridge)
                data     : data to write

  Returns:      TRUE if success.
                FALSE if fail. ( no cartridge )
 *---------------------------------------------------------------------------*/
BOOL CTRDG_Write8(u8 *address, u8 data)
{
#ifndef SDK_TWLLTD
    SDK_ASSERT(CTRDGi_IsInitialized());
#endif
    return CTRDGi_AccessCommon(address, data, 0 /*dummy */ , CTRDGi_ACCESS_WRITE8);
}
BOOL CTRDG_Write16(u16 *address, u16 data)
{
#ifndef SDK_TWLLTD
    SDK_ASSERT(CTRDGi_IsInitialized());
#endif
    return CTRDGi_AccessCommon(address, data, 0 /*dummy */ , CTRDGi_ACCESS_WRITE16);
}
BOOL CTRDG_Write32(u32 *address, u32 data)
{
#ifndef SDK_TWLLTD
    SDK_ASSERT(CTRDGi_IsInitialized());
#endif
    return CTRDGi_AccessCommon(address, data, 0 /*dummy */ , CTRDGi_ACCESS_WRITE32);
}

/*---------------------------------------------------------------------------*
  Name:         CTRDGi_AccessCommon

  Description:  read/write cartridge data.
                subroutine of CTRDG_Read*() and CTRDG_Write*().

  Arguments:    address    : address to access
                data       : data to write (in write mode)
                rdata      : address to store read data (in read mode)
                accessType : action flag

  Returns:      TRUE if success.
                FALSE if fail. ( no cartridge )
 *---------------------------------------------------------------------------*/
BOOL CTRDGi_AccessCommon(void *address, u32 data, void *rdata, u32 accessType)
{
    //---- check cartridge existence
    if (!CTRDG_IsExisting())
    {
        return FALSE;
    }

    /* confirm whether application has certainly judged AGB-cartridge */
    CTRDG_CheckEnabled();

    (void)OS_LockCartridge(CTRDGi_Work.lockID);

    switch (accessType)
    {
    case CTRDGi_ACCESS_READ8:
        if (rdata)
        {
            *(u8 *)rdata = *(u8 *)address;
        }
        break;
    case CTRDGi_ACCESS_READ16:
        if (rdata)
        {
            *(u16 *)rdata = *(u16 *)address;
        }
        break;
    case CTRDGi_ACCESS_READ32:
        if (rdata)
        {
            *(u32 *)rdata = *(u32 *)address;
        }
        break;
    case CTRDGi_ACCESS_WRITE8:
        *(u8 *)address = (u8)data;
        break;
    case CTRDGi_ACCESS_WRITE16:
        *(u16 *)address = (u16)data;
        break;
    case CTRDGi_ACCESS_WRITE32:
        *(u32 *)address = (u32)data;
        break;
    }

    (void)OS_UnLockCartridge(CTRDGi_Work.lockID);

    //---- check to remove cartridge in the middle of reading
    if (!CTRDG_IsExisting())
    {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         CTRDG_IsEnabled

  Description:  check if CTRDG is accessable

  Arguments:    None.

  Returns:      Return cartridge access permission.
 *---------------------------------------------------------------------------*/
BOOL CTRDG_IsEnabled(void)
{
#if !defined(SDK_TWLLTD) && defined(SDK_ARM9)
    SDK_ASSERT(CTRDGi_IsInitialized());
#endif
    return CTRDGi_EnableFlag;
}

/*---------------------------------------------------------------------------*
  Name:         CTRDG_Enable

  Description:  Set cartridge access permission mode.

  Arguments:    enable       permission mode to be set.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CTRDG_Enable(BOOL enable)
{
    OSIntrMode bak_cpsr = OS_DisableInterrupts();
    CTRDGi_EnableFlag = enable;
#if defined(SDK_ARM9)
#ifndef SDK_TWLLTD
    SDK_ASSERT(CTRDGi_IsInitialized());
#endif
    if (CTRDG_IsExistedAtInit())
    {
        u32 dacc = (u32)(enable ? OS_PR3_ACCESS_RW : OS_PR3_ACCESS_RO);
        (void)OS_SetDPermissionsForProtectionRegion(OS_PR3_ACCESS_MASK, dacc);
        if ( enable )
        {
            // for CTRDG
            DC_FlushAll();
            DC_WaitWriteBufferEmpty();
            OS_DisableICacheForProtectionRegion(1<<3);
            OS_DisableDCacheForProtectionRegion(1<<3);
            OS_DisableWriteBufferForProtectionRegion(1<<3);
        }
        else
        {
            // for HW_TWL_MAIN_MEM_EX
            OS_EnableICacheForProtectionRegion(1<<3);
            OS_EnableDCacheForProtectionRegion(1<<3);
            OS_EnableWriteBufferForProtectionRegion(1<<3);
        }
    }
#endif /* defined(SDK_ARM9) */
    (void)OS_RestoreInterrupts(bak_cpsr);
}

/*---------------------------------------------------------------------------*
  Name:         CTRDG_CheckEnabled

  Description:  Terminate program if CTRDG is not accessable

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CTRDG_CheckEnabled(void)
{
#ifdef SDK_ARM9
#ifndef SDK_TWLLTD
    SDK_ASSERT(CTRDGi_IsInitialized());
#endif
    if (!CTRDG_IsOptionCartridge() && !CTRDG_IsEnabled())
    {
        OS_TPanic
            ("cartridge permission denied. (you must call CTRDG_Enable() under the guideline.)");
    }
#endif
}
