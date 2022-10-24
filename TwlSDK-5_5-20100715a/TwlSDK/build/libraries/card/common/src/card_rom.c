/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - libraries
  File:     card_rom.c

  Copyright 2007-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-07-14#$
  $Rev: 10904 $
  $Author: yosizaki $

 *---------------------------------------------------------------------------*/


#include <nitro/card/rom.h>
#include <nitro/card/pullOut.h>
#include <nitro/card/rom.h>


#include "../include/card_common.h"
#include "../include/card_rom.h"


#if defined(SDK_ARM9) && defined(SDK_TWL)
#define CARD_USING_HASHCHECK
#endif // defined(SDK_ARM9) && defined(SDK_TWL)

#if defined(SDK_ARM9) || (defined(SDK_ARM7) && defined(SDK_ARM7_READROM_SUPPORT))
#define CARD_USING_ROMREADER
#endif // defined(SDK_ARM9) || (defined(SDK_ARM7) && defined(SDK_ARM7_READROM_SUPPORT))


/*---------------------------------------------------------------------------*/
/* Constants */

#define CARD_COMMAND_PAGE           0x01000000
#define CARD_COMMAND_ID             0x07000000
#define CARD_COMMAND_REFRESH        0x00000000
#define CARD_COMMAND_STAT           CARD_COMMAND_ID
#define CARD_COMMAND_MASK           0x07000000
#define CARD_RESET_HI               0x20000000
#define CARD_COMMAND_OP_G_READID    0xB8
#define CARD_COMMAND_OP_G_READPAGE  0xB7
#define CARD_COMMAND_OP_G_READSTAT  0xD6
#define CARD_COMMAND_OP_G_REFRESH   0xB5
#ifdef SDK_TWL
#define CARD_COMMAND_OP_N_READID    0x90
#define CARD_COMMAND_OP_N_READPAGE  0x00
#define CARD_COMMAND_OP_N_READSTAT  CARD_COMMAND_OP_G_READSTAT
#define CARD_COMMAND_OP_N_REFRESH   CARD_COMMAND_OP_G_REFRESH
#endif

// ROM ID

#define CARD_ROMID_1TROM_MASK       0x80000000UL  // 1T-ROM type
#define CARD_ROMID_TWLROM_MASK      0x40000000UL  // TWL-ROM
#define CARD_ROMID_REFRESH_MASK     0x20000000UL  // Refresh support

// ROM STATUS

#define CARD_ROMST_RFS_WARN_L1_MASK 0x00000004UL
#define CARD_ROMST_RFS_WARN_L2_MASK 0x00000008UL
#define CARD_ROMST_RFS_READY_MASK   0x00000020UL


/*---------------------------------------------------------------------------*/
/* Variables */

// Base offset during read (normally 0)
u32         cardi_rom_base;

// Other internal information
static int                (*CARDiReadRomFunction) (void *userdata, void *buffer, u32 offset, u32 length);

static CARDTransferInfo     CARDiDmaReadInfo[1];
static CARDTransferInfo    *CARDiDmaReadRegisteredInfo;

static u32                  cache_page = 1;
static u8                   CARDi_cache_buf[CARD_ROM_PAGE_SIZE] ATTRIBUTE_ALIGN(32);
static BOOL                 CARDiEnableCacheInvalidationIC = FALSE;
static BOOL                 CARDiEnableCacheInvalidationDC = TRUE;

extern BOOL OSi_IsThreadInitialized;

static u8                   CARDiOwnSignature[CARD_ROM_DOWNLOAD_SIGNATURE_SIZE] ATTRIBUTE_ALIGN(4);


/*---------------------------------------------------------------------------*/
/* Functions */

void CARD_RefreshRom(void);

/*---------------------------------------------------------------------------*
  Name:         CARDi_SetRomOp

  Description:  Sets the card command.

  Arguments:    command: Command
                offset: Transfer page count

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_SetRomOp(u32 command, u32 offset)
{
    u32     cmd1 = (u32)((offset >> 8) | (command << 24));
    u32     cmd2 = (u32)((offset << 24));
    // Just in case, waiting for completion of prior ROM command
    while ((reg_MI_MCCNT1 & REG_MI_MCCNT1_START_MASK) != 0)
    {
    }
    // Master enable
    reg_MI_MCCNT0 = (u16)(REG_MI_MCCNT0_E_MASK | REG_MI_MCCNT0_I_MASK |
                          (reg_MI_MCCNT0 & ~REG_MI_MCCNT0_SEL_MASK));
    // Command settings
    reg_MI_MCCMD0 = MI_HToBE32(cmd1);
    reg_MI_MCCMD1 = MI_HToBE32(cmd2);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_GetRomFlag

  Description:  Gets card command control parameters.

  Arguments:    flag: Command type to issue to the card device
                           (CARD_COMMAND_PAGE / CARD_COMMAND_ID /
                            CARD_COMMAND_STAT / CARD_COMMAND_REFRESH)

  Returns:      Card command control parameters.
 *---------------------------------------------------------------------------*/
SDK_INLINE u32 CARDi_GetRomFlag(u32 flag)
{
    u32     rom_ctrl = *(vu32 *)(HW_CARD_ROM_HEADER + 0x60);
    return (u32)(flag | REG_MI_MCCNT1_START_MASK | CARD_RESET_HI | (rom_ctrl & ~CARD_COMMAND_MASK));
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_IsTwlRom

  Description:  Determines whether game card is installed with ROM that supports TWL.
                Returns TRUE when ROM is installed that supports TWL even for a NITRO application.

  Arguments:    None.

  Returns:      If ROM supports TWL, return TRUE.
 *---------------------------------------------------------------------------*/
BOOL CARDi_IsTwlRom(void)
{
    u32 iplCardID = *(u32 *)(HW_BOOT_CHECK_INFO_BUF);

    // If there is no card when starting up, always return FALSE
    if ( ! iplCardID )
    {
        return FALSE;
    }

    return (CARDi_ReadRomID() & CARD_ROMID_TWLROM_MASK) ? TRUE : FALSE;
}

#ifdef SDK_TWL

/*---------------------------------------------------------------------------*
  Name:         CARDi_IsNormalMode

  Description:  Determines whether game card is in NORMAL mode.

  Arguments:    None.

  Returns:      If game card is in NORMAL mode, returns TRUE.
 *---------------------------------------------------------------------------*/
static BOOL CARDi_IsNormalMode(void)
{
    const CARDRomHeaderTWL *oh = CARD_GetOwnRomHeaderTWL();

    return OS_IsRunOnTwl() &&
           (OS_GetBootType() != OS_BOOTTYPE_ROM) &&
           oh->access_control.game_card_on &&
           ! oh->access_control.game_card_nitro_mode;
}

#endif

/*---------------------------------------------------------------------------*
  Name:         CARDi_StartRomPageTransfer

  Description:  Starts ROM page transfers.

  Arguments:    offset: ROM offset of transfer source

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_StartRomPageTransfer(u32 offset)
{
    u8 op = CARD_COMMAND_OP_G_READPAGE;
#ifdef SDK_TWL
    if ( CARDi_IsNormalMode() )
    {
        op = CARD_COMMAND_OP_N_READPAGE;
    }
#endif

    CARDi_SetRomOp(op, offset);
    reg_MI_MCCNT1 = CARDi_GetRomFlag(CARD_COMMAND_PAGE);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadRomIDCore

  Description:  Reads the card ID.

  Arguments:    None.

  Returns:      Card ID.
 *---------------------------------------------------------------------------*/
u32 CARDi_ReadRomIDCore(void)
{
    u8 op = CARD_COMMAND_OP_G_READID;
#ifdef SDK_TWL
    if ( CARDi_IsNormalMode() )
    {
        op = CARD_COMMAND_OP_N_READID;
    }
#endif

    CARDi_SetRomOp(op, 0);
    reg_MI_MCCNT1 = (u32)(CARDi_GetRomFlag(CARD_COMMAND_ID) & ~REG_MI_MCCNT1_L1_MASK);
    while ((reg_MI_MCCNT1 & REG_MI_MCCNT1_RDY_MASK) == 0)
    {
    }
    return reg_MI_MCD1;
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadRomStatusCore

  Description:  Reads the card status.
                Issue only when detecting ROM that supports refresh.
                Required even for NITRO applications installed with supporting ROM.

  Arguments:    None.

  Returns:      Card status.
 *---------------------------------------------------------------------------*/
u32 CARDi_ReadRomStatusCore(void)
{
    u32 iplCardID = *(u32 *)(HW_BOOT_CHECK_INFO_BUF);

    if ( ! (iplCardID & CARD_ROMID_REFRESH_MASK) )
    {
        return CARD_ROMST_RFS_READY_MASK;
    }

    CARDi_SetRomOp(CARD_COMMAND_OP_G_READSTAT, 0);
    reg_MI_MCCNT1 = (u32)(CARDi_GetRomFlag(CARD_COMMAND_STAT) & ~REG_MI_MCCNT1_L1_MASK);
    while ((reg_MI_MCCNT1 & REG_MI_MCCNT1_RDY_MASK) == 0)
    {
    }
    return reg_MI_MCD1;
}

/*---------------------------------------------------------------------------*
  Name:         CARD_RefreshRom

  Description:  Refreshes bad blocks on card ROM.
                Issue only when detecting ROM that supports refresh.
                Required even for NITRO applications installed with this ROM.
                A forced refresh occurs in the extremely rare case that ECC correction is about to fail, even in the CARD_ReadRom function. However, the refresh may take approximately 50 ms, so call this function periodically at a timing that is safe for the application even if delays occur.
                
                
                

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_RefreshRom(void)
{
    SDK_ASSERT(CARD_IsAvailable());
    SDK_TASSERTMSG(CARDi_GetTargetMode() == CARD_TARGET_ROM, "must be locked by CARD_LockRom()");

#if defined(SDK_ARM9)
    (void)CARDi_WaitForTask(&cardi_common, TRUE, NULL, NULL);
    // Detect card removal here
    CARDi_CheckPulledOutCore(CARDi_ReadRomIDCore());
#endif // defined(SDK_ARM9)

    CARDi_RefreshRom(CARD_ROMST_RFS_WARN_L1_MASK | CARD_ROMST_RFS_WARN_L2_MASK);

#if defined(SDK_ARM9)
    cardi_common.cmd->result = CARD_RESULT_SUCCESS;
    CARDi_EndTask(&cardi_common);
#endif // defined(SDK_ARM9)
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_RefreshRom

  Description:  Refreshes bad blocks on card ROM.
                Issue only when detecting ROM that supports refresh.
                Required even for NITRO applications installed with this ROM.

  Arguments:    warn_mask: Warning level specification

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_RefreshRom(u32 warn_mask)
{
    if (CARDi_ReadRomStatusCore() & warn_mask)
    {
        CARDi_RefreshRomCore();
        // There is the possibility that completion of the refresh can take more than 100 ms
        while ( !(CARDi_ReadRomStatusCore() & CARD_ROMST_RFS_READY_MASK) )
        {
            // If that is possible, sleep
            if ( OSi_IsThreadInitialized && 
                 OS_IsAlarmAvailable() )
            {
                OS_Sleep(1);
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_RefreshRomCore

  Description:  Refreshes bad blocks on card ROM.
                Required even for NITRO applications installed with this ROM.
                Latency settings are invalid because the command is issued unilaterally to the card.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_RefreshRomCore(void)
{
    CARDi_SetRomOp(CARD_COMMAND_OP_G_REFRESH, 0);
    reg_MI_MCCNT1 = (u32)(CARDi_GetRomFlag(CARD_COMMAND_REFRESH) & ~REG_MI_MCCNT1_L1_MASK);
    while (reg_MI_MCCNT1 & REG_MI_MCCNT1_START_MASK)
    {
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadRomWithCPU

  Description:  Transfers ROM using the CPU.
                There is no need to consider cache or page unit restrictions, but note that the function will block until the transfer is complete.
                

  Arguments:    userdata: (Dummy to use as the other callback)
                buffer: Transfer destination buffer
                offset: Transfer source ROM offset
                length: Transfer size

  Returns:      None.
 *---------------------------------------------------------------------------*/
int CARDi_ReadRomWithCPU(void *userdata, void *buffer, u32 offset, u32 length)
{
    int     retval = (int)length;
    // Cache the global variables frequently used to the local variables
    u32         cachedPage = cache_page;
    u8  * const cacheBuffer = CARDi_cache_buf;
    while (length > 0)
    {
        // ROM transfer is always in page units
        u8     *ptr = (u8 *)buffer;
        u32     n = CARD_ROM_PAGE_SIZE;
        u32     pos = MATH_ROUNDDOWN(offset, CARD_ROM_PAGE_SIZE);
        // Use the cache if the same as the previous page
        if (pos == cachedPage)
        {
            ptr = cacheBuffer;
        }
        else
        {
            // Transfer to cache if not possible to transfer directly to the buffer
            if(((pos != offset) || (((u32)buffer & 3) != 0) || (length < n)))
            {
                cachedPage = pos;
                ptr = cacheBuffer;
            }
            // Read directly to the buffer guaranteed with 4-byte alignment using the CPU
            CARDi_StartRomPageTransfer(pos);
            {
                u32     word = 0;
                for (;;)
                {
                    // Wait for one word transfer to be completed
                    u32     ctrl = reg_MI_MCCNT1;
                    if ((ctrl & REG_MI_MCCNT1_RDY_MASK) != 0)
                    {
                        // Read data and store in buffer if necessary
                        u32     data = reg_MI_MCD1;
                        if (word < (CARD_ROM_PAGE_SIZE / sizeof(u32)))
                        {
                            ((u32 *)ptr)[word++] = data;
                        }
                    }
                    // End if 1 page transfer is completed
                    if ((ctrl & REG_MI_MCCNT1_START_MASK) == 0)
                    {
                        break;
                    }
                }
            }
        }
        // Transfer from cache if via cache
        if (ptr == cacheBuffer)
        {
            u32     mod = offset - pos;
            n = MATH_MIN(length, CARD_ROM_PAGE_SIZE - mod);
            MI_CpuCopy8(cacheBuffer + mod, buffer, n);
        }
        buffer = (u8 *)buffer + n;
        offset += n;
        length -= n;
    }
    // Determine whether card was removed while accessed
    CARDi_CheckPulledOutCore(CARDi_ReadRomIDCore());
    // If local variables, apply to global variables
    cache_page = cachedPage;
    (void)userdata;
    return retval;
}

#if defined(CARD_USING_ROMREADER)
/*---------------------------------------------------------------------------*
  Name:         CARDi_DmaReadPageCallback

  Description:  Callback when ROM page DMA transfer is completed.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_DmaReadPageCallback(void)
{
    CARDTransferInfo   *info = CARDiDmaReadRegisteredInfo;
    if (info)
    {
        info->src += CARD_ROM_PAGE_SIZE;
        info->dst += CARD_ROM_PAGE_SIZE;
        info->len -= CARD_ROM_PAGE_SIZE;
        // Transfer the next page if necessary
        if (info->len > 0)
        {
            CARDi_StartRomPageTransfer(info->src);
        }
        // Transfer completed
        else
        {
            cardi_common.DmaCall->Stop(cardi_common.dma);
            (void)OS_DisableIrqMask(OS_IE_CARD_DATA);
            (void)OS_ResetRequestIrqMask(OS_IE_CARD_DATA);
            CARDiDmaReadRegisteredInfo = NULL;
            // Determine whether card was removed while accessed
            CARDi_CheckPulledOutCore(CARDi_ReadRomIDCore());
            if (info->callback)
            {
                (*info->callback)(info->userdata);
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadRomWithDMA

  Description:  Transfers ROM using the DMA.
                After transfer of the first page starts, the function returns control immediately.

  Arguments:    info: Transfer information

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_ReadRomWithDMA(CARDTransferInfo *info)
{
    OSIntrMode bak_psr = OS_DisableInterrupts();
    CARDiDmaReadRegisteredInfo = info;
    // Set card transfer completion interrupt
    (void)OS_SetIrqFunction(OS_IE_CARD_DATA, CARDi_DmaReadPageCallback);
    (void)OS_ResetRequestIrqMask(OS_IE_CARD_DATA);
    (void)OS_EnableIrqMask(OS_IE_CARD_DATA);
    (void)OS_RestoreInterrupts(bak_psr);
    // Set CARD-DMA (transfer idling)
    cardi_common.DmaCall->Recv(cardi_common.dma, (void *)&reg_MI_MCD1, (void *)info->dst, CARD_ROM_PAGE_SIZE);
    // Start first ROM transfer
    CARDi_StartRomPageTransfer(info->src);
}

static void CARDi_DmaReadDone(void *userdata)
{
    (void)userdata;
#ifdef SDK_ARM9
    // Determine whether card was removed while accessed
    CARDi_CheckPulledOutCore(CARDi_ReadRomIDCore());
#endif
    // Refresh at final warning stage of ROM-ECC correction
    CARDi_RefreshRom(CARD_ROMST_RFS_WARN_L2_MASK);

    cardi_common.cmd->result = CARD_RESULT_SUCCESS;
    CARDi_EndTask(&cardi_common);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_IsRomDmaAvailable

  Description:  Determines whether asynchronous DMA is usable in ROM transfer.

  Arguments:    dma: DMA channel
                dst: Transfer destination buffer
                src: Transfer source ROM offset
                len: Transfer size

  Returns:      If DMA is usable in ROM transfer, return TRUE.
 *---------------------------------------------------------------------------*/
static BOOL CARDi_IsRomDmaAvailable(u32 dma, void *dst, u32 src, u32 len)
{
    return (dma <= MI_DMA_MAX_NUM) && (len > 0) && (((u32)dst & 31) == 0) &&
#ifdef SDK_ARM9
        (((u32)dst + len <= OS_GetITCMAddress()) || ((u32)dst >= OS_GetITCMAddress() + HW_ITCM_SIZE)) &&
        (((u32)dst + len <= OS_GetDTCMAddress()) || ((u32)dst >= OS_GetDTCMAddress() + HW_DTCM_SIZE)) &&
#endif
        (((src | len) & (CARD_ROM_PAGE_SIZE - 1)) == 0);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadRomSyncCore

  Description:  Common process for synchronous card reads.

  Arguments:    c: CARDiCommon structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_ReadRomSyncCore(CARDiCommon *c)
{
    // Run ROM access routine that corresponds to the execution environment
    (void)(*CARDiReadRomFunction)(NULL, (void*)c->dst, c->src, c->len);
#ifdef SDK_ARM9
    // Determine whether card was removed while accessed
    CARDi_CheckPulledOutCore(CARDi_ReadRomIDCore());
#endif
    // Refresh at final warning stage of ROM-ECC correction
    CARDi_RefreshRom(CARD_ROMST_RFS_WARN_L2_MASK);

    cardi_common.cmd->result = CARD_RESULT_SUCCESS;
}
#endif // defined(CARD_USING_ROMREADER)

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadRom

  Description:  Basic function of ROM read.

  Arguments:    dma: DMA channel to use
                src: Transfer source offset
                dst: Transfer destination memory address
                len: Transfer size
                callback: Completion callback (NULL if not used)
                arg: Argument of completion callback (ignored if not used)
                is_async: If set to asynchronous mode, TRUE

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_ReadRom(u32 dma,
                   const void *src, void *dst, u32 len,
                   MIDmaCallback callback, void *arg, BOOL is_async)
{
#if defined(CARD_USING_ROMREADER)
    CARDiCommon *const c = &cardi_common;

    SDK_ASSERT(CARD_IsAvailable());
    SDK_ASSERT(CARDi_GetTargetMode() == CARD_TARGET_ROM);
    SDK_TASSERTMSG((dma != 0), "cannot specify DMA channel 0");

    // Determine validity of card access
    CARD_CheckEnabled();
    if ((CARDi_GetAccessLevel() & CARD_ACCESS_LEVEL_ROM) == 0)
    {
        OS_TPanic("this program cannot access CARD-ROM!");
    }

    // Exclusive wait on ARM9 side
    (void)CARDi_WaitForTask(c, TRUE, callback, arg);

    // Get DMA interface
    c->DmaCall = CARDi_GetDmaInterface(dma);
    c->dma = (u32)((c->DmaCall != NULL) ? (dma & MI_DMA_CHANNEL_MASK) : MI_DMA_NOT_USE);
    if (c->dma <= MI_DMA_MAX_NUM)
    {
        c->DmaCall->Stop(c->dma);
    }

    // Set this time's transfer parameters
    c->src = (u32)((u32)src + cardi_rom_base);
    c->dst = (u32)dst;
    c->len = (u32)len;

    // Set transfer parameter
    {
        CARDTransferInfo   *info = CARDiDmaReadInfo;
        info->callback = CARDi_DmaReadDone;
        info->userdata = NULL;
        info->src = c->src;
        info->dst = c->dst;
        info->len = c->len;
        info->work = 0;
    }

    // DMA transfers can be used in some cases, if it is an environment where hash checks are invalid
    if ((CARDiReadRomFunction == CARDi_ReadRomWithCPU) &&
        CARDi_IsRomDmaAvailable(c->dma, (void *)c->dst, c->src, c->len))
    {
#if defined(SDK_ARM9)
        // Depending upon the size of the transfer range, switch the manner in which the cache is invalidated
        OSIntrMode bak_psr = OS_DisableInterrupts();
        if (CARDiEnableCacheInvalidationIC)
        {
            CARDi_ICInvalidateSmart((void *)c->dst, c->len, c->flush_threshold_ic);
        }
        if (CARDiEnableCacheInvalidationDC)
        {
            CARDi_DCInvalidateSmart((void *)c->dst, c->len, c->flush_threshold_dc);
        }
        (void)OS_RestoreInterrupts(bak_psr);
#endif
        // Start asynchronous transfer using DMA
        CARDi_ReadRomWithDMA(CARDiDmaReadInfo);
        // Wait until complete here if synchronous transfer is requested
        if (!is_async)
        {
            CARD_WaitRomAsync();
        }
    }
    else
    {
        // Even with CPU transfers, it is necessary to invalidate because the instruction cache is incompatible
        if (CARDiEnableCacheInvalidationIC)
        {
            CARDi_ICInvalidateSmart((void *)c->dst, c->len, c->flush_threshold_ic);
        }
        // Register task to thread if some form of direct processing is required by the CPU
        (void)CARDi_ExecuteOldTypeTask(CARDi_ReadRomSyncCore, is_async);
    }
#else
    (void)dma;
    (void)is_async;
    (void)CARDi_ReadRomWithCPU(NULL, dst, (u32)src, len);
    if (callback)
    {
        (*callback)(arg);
    }
#endif // defined(CARD_USING_ROMREADER)
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadRomID

  Description:  Reads the card ID.

  Arguments:    None.

  Returns:      Card ID.
 *---------------------------------------------------------------------------*/
u32 CARDi_ReadRomID(void)
{
    u32     ret = 0;

    SDK_ASSERT(CARD_IsAvailable());
    SDK_TASSERTMSG(CARDi_GetTargetMode() == CARD_TARGET_ROM, "must be locked by CARD_LockRom()");

#if defined(SDK_ARM9)
    (void)CARDi_WaitForTask(&cardi_common, TRUE, NULL, NULL);
#endif // defined(SDK_ARM9)

    /* Direct access is possible, so execute here */
    ret = CARDi_ReadRomIDCore();
#ifdef SDK_ARM9
    // Detect card removal here
    CARDi_CheckPulledOutCore(ret);
#endif

#if defined(SDK_ARM9)
    cardi_common.cmd->result = CARD_RESULT_SUCCESS;
    CARDi_EndTask(&cardi_common);
#endif // defined(SDK_ARM9)

    return ret;
}

#if defined(CARD_USING_HASHCHECK)
#include <twl/ltdmain_begin.h>

// ROM hash calculation buffer
u8     *CARDiHashBufferAddress = NULL;
u32     CARDiHashBufferLength = 0;
static CARDRomHashContext   context[1];

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadCardWithHash

  Description:  ROM transfer while confirming hash.

  Arguments:    buffer: Transfer destination buffer
                offset: Transfer source ROM offset
                length: Transfer size

  Returns:      None.
 *---------------------------------------------------------------------------*/
static int CARDi_ReadCardWithHash(void *userdata, void *buffer, u32 offset, u32 length)
{
    (void)userdata;
    CARD_ReadRomHashImage(context, buffer, offset, length);
    return (int)length;
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadCardWithHashInternalAsync

  Description:  Asynchronous DMA transfer for partial use internally with ROM transfers of the hash verified version.

  Arguments:    userdata: User-defined argument
                buffer: Transfer destination buffer
                offset: Transfer source ROM offset
                length: Transfer size

  Returns:      None.
 *---------------------------------------------------------------------------*/
static int CARDi_ReadCardWithHashInternalAsync(void *userdata, void *buffer, u32 offset, u32 length)
{
    if (cardi_common.dma == MI_DMA_NOT_USE)
    {
        length = 0;
    }
    else
    {
        CARDRomHashContext *context = (CARDRomHashContext *)userdata;
        static CARDTransferInfo card_hash_info[1];
        DC_FlushRange(buffer, length);
        card_hash_info->callback = (void(*)(void*))CARD_NotifyRomHashReadAsync;
        card_hash_info->userdata = context;
        card_hash_info->src = offset;
        card_hash_info->dst = (u32)buffer;
        card_hash_info->len = length;
        card_hash_info->command = 0;
        card_hash_info->work = 0;
        CARDi_ReadRomWithDMA(card_hash_info);
    }
    return (int)length;
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_InitRomHash

  Description:  If possible, switches to hash verification version ROM transfer.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_InitRomHash(void)
{
    // Because applications will often consume the entire arena before it is required by a function call such as FS_Init, always allocate a buffer for the hash context here
    // 
    // 
    u8     *lo = (u8 *)MATH_ROUNDUP((u32)OS_GetMainArenaLo(), 32);
    u8     *hi = (u8 *)MATH_ROUNDDOWN((u32)OS_GetMainArenaHi(), 32);
    u32     len = CARD_CalcRomHashBufferLength(CARD_GetOwnRomHeaderTWL());
    if (&lo[len] > hi)
    {
        OS_TPanic("cannot allocate memory for ROM-hash from ARENA");
    }
    else
    {
        OS_SetMainArenaLo(&lo[len]);
        CARDiHashBufferAddress = lo;
        CARDiHashBufferLength = len;
        // Actually load if it is an environment where the ROM boot and hash table exist
        if ((OS_GetBootType() == OS_BOOTTYPE_ROM) &&
            ((((const u8 *)HW_TWL_ROM_HEADER_BUF)[0x1C] & 0x01) != 0))
        {
            cardi_common.dma = MI_DMA_NOT_USE;
            CARDiReadRomFunction = CARDi_ReadCardWithHash;
            {
                u16     id = (u16)OS_GetLockID();
                CARD_LockRom(id);
                CARD_InitRomHashContext(context,
                                        CARD_GetOwnRomHeaderTWL(),
                                        CARDiHashBufferAddress,
                                        CARDiHashBufferLength,
                                        CARDi_ReadRomWithCPU,
                                        CARDi_ReadCardWithHashInternalAsync,
                                        context);
                // Destroy the pointer so that there is no competition for use of the same buffer
                CARDiHashBufferAddress = NULL;
                CARDiHashBufferLength = 0;
                CARD_UnlockRom(id);
                OS_ReleaseLockID(id);
            }
        }
    }
}
#include <twl/ltdmain_end.h>
#endif

/*---------------------------------------------------------------------------*
  Name:         CARDi_InitRom

  Description:  Initializes the ROM access management information.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_InitRom(void)
{
#if defined(CARD_USING_ROMREADER)
    CARDiReadRomFunction = CARDi_ReadRomWithCPU;
    // Hash verification is implemented for ROM data access in the TWL-SDK, but because signature data for DS Download Play is embedded after the hash is calculated, it is prohibited to carelessly read data from this point on with CARD_ReadRom(). It must be explicitly accessed using CARDi_GetOwnSignature().
    // 
    // 
    // 
    if ((OS_GetBootType() == OS_BOOTTYPE_ROM) && CARD_GetOwnRomHeader()->rom_size)
    {
        u16     id = (u16)OS_GetLockID();
        CARD_LockRom(id);
        (void)CARDi_ReadRomWithCPU(NULL, CARDiOwnSignature,
                                   CARD_GetOwnRomHeader()->rom_size,
                                   CARD_ROM_DOWNLOAD_SIGNATURE_SIZE);
        CARD_UnlockRom(id);
        OS_ReleaseLockID(id);
    }
#if defined(CARD_USING_HASHCHECK)
    // Adopts a routine that also verifies hash, if usable
    if (OS_IsRunOnTwl())
    {
		CARDi_InitRomHash();
    }
#endif
#else
    CARDiReadRomFunction = NULL;
#endif
}


/*---------------------------------------------------------------------------*
  Name:         CARD_WaitRomAsync

  Description:  Idles until the ROM access function is completed.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_WaitRomAsync(void)
{
    (void)CARDi_WaitAsync();
}

/*---------------------------------------------------------------------------*
  Name:         CARD_TryWaitRomAsync

  Description:  Determines whether the ROM access function is completed.

  Arguments:    None.

  Returns:      TRUE when the ROM access function is completed.
 *---------------------------------------------------------------------------*/
BOOL CARD_TryWaitRomAsync(void)
{
    return CARDi_TryWaitAsync();
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_GetOwnSignature

  Description:  Gets its own DS Download Play signature data.

  Arguments:    None.

  Returns:      Its own DS Download Play signature data.
 *---------------------------------------------------------------------------*/
const u8* CARDi_GetOwnSignature(void)
{
    return CARDiOwnSignature;
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_SetOwnSignature

  Description:  Sets its own DS Download Play signature data.
                Calls from the host library when booted without a card.

  Arguments:    DS Download Play signature data

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_SetOwnSignature(const void *signature)
{
    MI_CpuCopy8(signature, CARDiOwnSignature, CARD_ROM_DOWNLOAD_SIGNATURE_SIZE);
}

#if defined(SDK_ARM9)
/*---------------------------------------------------------------------------*
  Name:         CARD_GetCacheFlushFlag

  Description:  Gets setting flags of whether to invalidate cache automatically.

  Arguments:    icache: Pointer to the location where the automatic invalidation flag for the instruction cache is stored
                                  Ignore if NULL.
                dcache: Pointer to the location the automatic invalidation flag for the instruction cache is stored.
                                  Ignore if NULL.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_GetCacheFlushFlag(BOOL *icache, BOOL *dcache)
{
    SDK_ASSERT(CARD_IsAvailable());
    if (icache)
    {
        *icache = CARDiEnableCacheInvalidationIC;
    }
    if (dcache)
    {
        *dcache = CARDiEnableCacheInvalidationDC;
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARD_SetCacheFlushFlag

  Description:  Sets whether to automatically invalidate cache.
                By default, instruction cache is FALSE, and data cache is TRUE.

  Arguments:    icache: TRUE to enable automatic invalidation of the instruction cache
                dcache: TRUE to enable automatic invalidation of data caches

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_SetCacheFlushFlag(BOOL icache, BOOL dcache)
{
    SDK_ASSERT(CARD_IsAvailable());
    CARDiEnableCacheInvalidationIC = icache;
    CARDiEnableCacheInvalidationDC = dcache;
}
#endif
