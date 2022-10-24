/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CTRDG - libraries - ARM9
  File:     ctrdg_flash_LE39FW512.c

  Copyright 2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-11-15#$
  $Rev: 2414 $
  $Author: hatamoto_minoru $
 *---------------------------------------------------------------------------*/

#include <nitro.h>

//#define   __LE_DEBUG
/* For debugging */
//#ifdef    __LE_DEBUG
//#include "mylib.h"
//#endif

// Definition data------------------------------
/* For debugging */
//#ifndef   __FLASH_DEBUG
#define CTRDG_BACKUP_COM_ADR1   (CTRDG_AGB_FLASH_ADR+0x00005555)
#define CTRDG_BACKUP_COM_ADR2   (CTRDG_AGB_FLASH_ADR+0x00002aaa)
//#else
//#define   COM_ADR1    0x02003ffe
//#define COM_ADR2  0x02003fff
//#endif

#define FLASH_LOG_SECTOR_COUNT          16

#define ERASE_RETRY_MAX                 0x50

// Extern data----------------------------------
extern const CTRDGFlashType *AgbFlash;
extern u16 CTRDGi_PollingSR512kCOMMON(u16 phase, u8 *adr, u16 lastData);
/*Exclusive control*/
extern u16 ctrdgi_flash_lock_id;
extern BOOL ctrdgi_backup_irq;

// Function's prototype declaration-------------
u16     CTRDGi_EraseFlashChipLE(void);
u16     CTRDGi_EraseFlashSectorLE(u16 secNo);
u16     CTRDGi_ProgramFlashByteLE(u8 *src, u8 *dst);
u16     CTRDGi_WriteFlashSectorLE(u16 secNo, u8 *src);
u32     CTRDGi_VerifyFlashCoreFF(u8 *adr);
u32     CTRDGi_VerifyFlashErase(u8 *tgt, u32 (*func_p) (u8 *));
u32     CTRDGi_EraseFlashChipCoreLE(CTRDGTaskInfo * arg);
u32     CTRDGi_EraseFlashSectorCoreLE(CTRDGTaskInfo * arg);
u32     CTRDGi_WriteFlashSectorCoreLE(CTRDGTaskInfo * arg);

void    CTRDGi_EraseFlashChipAsyncLE(CTRDG_TASK_FUNC callback);
void    CTRDGi_EraseFlashSectorAsyncLE(u16 secNo, CTRDG_TASK_FUNC callback);
void    CTRDGi_WriteFlashSectorAsyncLE(u16 secNo, u8 *src, CTRDG_TASK_FUNC callback);

// Const data-----------------------------------
static const u16 leMaxTime[] = {
    10,                                // Common       10ms
    10,                                // Program      10ms(exactly:20usec)
    40,                                // Sector erase 25ms
    200,                               // Chip   erase 100ms
};

const CTRDGiFlashTypePlus defaultFlash512 = {
    CTRDGi_WriteFlashSectorLE,
    CTRDGi_EraseFlashChipLE,
    CTRDGi_EraseFlashSectorLE,
    CTRDGi_WriteFlashSectorAsyncLE,
    CTRDGi_EraseFlashChipAsyncLE,
    CTRDGi_EraseFlashSectorAsyncLE,
    CTRDGi_PollingSR512kCOMMON,
    leMaxTime,
    {
/* For debugging */
//#ifndef   __FLASH_DEBUG
        0x00010000,                       // ROM size
        {0x00001000, 12, 16, 0},          // Sector size, shift, count
//#else
//      0x00002000,
//      {0x00000200,  9, 16,  0},
//#endif
        {MI_CTRDG_RAMCYCLE_8, MI_CTRDG_RAMCYCLE_6},        // agb read cycle=3, write cycle=2
         0x00,                             // Maker ID
         0x00,                             // Device ID
    },
};

const CTRDGiFlashTypePlus LE39FW512 = {
    CTRDGi_WriteFlashSectorLE,
    CTRDGi_EraseFlashChipLE,
    CTRDGi_EraseFlashSectorLE,
    CTRDGi_WriteFlashSectorAsyncLE,
    CTRDGi_EraseFlashChipAsyncLE,
    CTRDGi_EraseFlashSectorAsyncLE,
    CTRDGi_PollingSR512kCOMMON,
    leMaxTime,
    {
/* For debugging */
//#ifndef   __FLASH_DEBUG
        0x00010000,                       // ROM size
        {0x00001000, 12, 16, 0},          // Sector size, shift, count, top
//#else
//      0x00002000,
//      {0x00000200,  9, 16,  0},
//#endif
        {MI_CTRDG_RAMCYCLE_8, MI_CTRDG_RAMCYCLE_6},        // agb read cycle=3, write cycle=2
        0xbf,                             // Maker ID
        0xd4,                             // Device ID
    },
};

// Program description**************************
u32 CTRDGi_EraseFlashChipCoreLE(CTRDGTaskInfo * arg)
{
/* For debugging */
//#ifdef    __FLASH_DEBUG
//  u32 i;
//#endif
    MICartridgeRamCycle ram_cycle;
    u32     result;
    (void)arg;

    /*Exclusive control (lock) */
    (void)OS_LockCartridge(ctrdgi_flash_lock_id);
    /*Access cycle settings */
    ram_cycle = MI_GetCartridgeRamCycle();
    MI_SetCartridgeRamCycle(AgbFlash->agbWait[0]);
    //*(vu16 *)REG_EXMEMCNT_ADDR=(*(vu16 *)REG_EXMEMCNT_ADDR & 0xfffc)|AgbFlash->agbWait[0]; // read 3cycles wait
    ctrdgi_backup_irq = OS_DisableIrq();

    *(vu8 *)CTRDG_BACKUP_COM_ADR1 = 0xaa;
    *(vu8 *)CTRDG_BACKUP_COM_ADR2 = 0x55;
    *(vu8 *)CTRDG_BACKUP_COM_ADR1 = 0x80;
    *(vu8 *)CTRDG_BACKUP_COM_ADR1 = 0xaa;
    *(vu8 *)CTRDG_BACKUP_COM_ADR2 = 0x55;
    *(vu8 *)CTRDG_BACKUP_COM_ADR1 = 0x10;
/* For debugging */
//#ifdef    __FLASH_DEBUG
//  adr=(u8 *)CTRDG_AGB_FLASH_ADR;
//  for(i=0;i<AgbFlash->romSize;i++)
//      *adr++=0xff;
//#endif
    (void)OS_RestoreIrq(ctrdgi_backup_irq);

    result = CTRDGi_PollingSR(CTRDG_BACKUP_PHASE_CHIP_ERASE, (u8 *)CTRDG_AGB_FLASH_ADR, 0xff);

    /*Access cycle settings */
    MI_SetCartridgeRamCycle(ram_cycle);
    /*Exclusive control (unlock) */
    (void)OS_UnlockCartridge(ctrdgi_flash_lock_id);
    return result;
}

u32 CTRDGi_EraseFlashSectorCoreLE(CTRDGTaskInfo * arg)
{
    u8     *adr;
    //u16 readFuncBuff[32];
    MICartridgeRamCycle ram_cycle;
    u32     result;
    CTRDGTaskInfo p = *arg;
    u16     secNo = p.sec_num;

    if (secNo >= FLASH_LOG_SECTOR_COUNT)
        return CTRDG_BACKUP_RESULT_ERROR | CTRDG_BACKUP_PHASE_PARAMETER_CHECK;

    /*Exclusive control (lock) */
    (void)OS_LockCartridge(ctrdgi_flash_lock_id);
    /*Access cycle settings */
    ram_cycle = MI_GetCartridgeRamCycle();
    MI_SetCartridgeRamCycle(AgbFlash->agbWait[0]);
    //*(vu16 *)REG_EXMEMCNT_ADDR=(*(vu16 *)REG_EXMEMCNT_ADDR & 0xfffc)|AgbFlash->agbWait[0]; // read 3cycles wait

    adr = (u8 *)(CTRDG_AGB_FLASH_ADR + (secNo << AgbFlash->sector.shift));

    ctrdgi_backup_irq = OS_DisableIrq();

    *(vu8 *)CTRDG_BACKUP_COM_ADR1 = 0xaa;
    *(vu8 *)CTRDG_BACKUP_COM_ADR2 = 0x55;
    *(vu8 *)CTRDG_BACKUP_COM_ADR1 = 0x80;
    *(vu8 *)CTRDG_BACKUP_COM_ADR1 = 0xaa;
    *(vu8 *)CTRDG_BACKUP_COM_ADR2 = 0x55;
    *(vu8 *)adr = 0x30;
/* For debugging */
//#ifdef    __FLASH_DEBUG
//  for(i=0;i<AgbFlash->sector.size;i++)
//      *adr++=0xff;
//  adr=(u8 *)(CTRDG_AGB_FLASH_ADR+(secNo<<AgbFlash->sector.shift));
//#endif
    (void)OS_RestoreIrq(ctrdgi_backup_irq);

    result = CTRDGi_PollingSR(CTRDG_BACKUP_PHASE_SECTOR_ERASE, adr, 0xff);

    /*Access cycle settings */
    MI_SetCartridgeRamCycle(ram_cycle);
    /*Exclusive control (unlock) */
    (void)OS_UnlockCartridge(ctrdgi_flash_lock_id);

    return result;
}

u16 CTRDGi_ProgramFlashByteLE(u8 *src, u8 *dst)
{
    u16     result;
    *(vu8 *)CTRDG_BACKUP_COM_ADR1 = 0xaa;
    *(vu8 *)CTRDG_BACKUP_COM_ADR2 = 0x55;
    *(vu8 *)CTRDG_BACKUP_COM_ADR1 = 0xa0;
    *dst = *src;

    result = CTRDGi_PollingSR(CTRDG_BACKUP_PHASE_PROGRAM, dst, *src);
    return result;
}

u32 CTRDGi_WriteFlashSectorCoreLE(CTRDGTaskInfo * arg)
{
    //u16 funcBuff[48],*func_src,*func_dst
    u8     *tgt;
    u16     retry, add_erase, j;
    MICartridgeRamCycle ram_cycle;
    u32     result;
    CTRDGTaskInfo p = *arg;
    u16     secNo = p.sec_num;
    u8     *src = p.data;

    /*Check parameters */
    if (secNo >= FLASH_LOG_SECTOR_COUNT)
        return CTRDG_BACKUP_RESULT_ERROR | CTRDG_BACKUP_PHASE_PARAMETER_CHECK;

    tgt = (u8 *)(CTRDG_AGB_FLASH_ADR + (secNo << AgbFlash->sector.shift));

    // Erase verify routine transmit
//  func_src=(u16 *)((u32)VerifyFlashCoreFF & 0xFFFFFFFE);
//  func_dst=funcBuff;
//  for(i=(u32)VerifyFlashErase-(u32)VerifyFlashCoreFF;i>0;i-=2)
//      *func_dst++=*func_src++;
    retry = 0;
    // Erase
/* For debugging */
    //++++++++++++++++++++
//#ifdef    __LE_DEBUG
//  if(secNo<8)
//      pos=0x0194+(secNo<<5);
//  else
//      pos=0x0199+((secNo-8)<<5);
//  DrawHexOnBgx(0,pos,BG_PLTT_WHITE,&retry,2);
//#endif
    //++++++++++++++++++++
    while (1)
    {
        result = CTRDGi_EraseFlashSectorLE(secNo);
        if (result == 0)
        {
            result = (u16)CTRDGi_VerifyFlashErase(tgt, CTRDGi_VerifyFlashCoreFF);
            if (result == 0)
                break;
        }
        if (retry++ == ERASE_RETRY_MAX)
            return result;
/* For debugging */
        //++++++++++++++++++++
//#ifdef    __LE_DEBUG
//      DrawHexOnBgx(0,pos,BG_PLTT_YELLOW,&retry,2);
//#endif
        //++++++++++++++++++++
    }

    // add Erase
    add_erase = 1;
    if (retry > 0)
        add_erase = 6;
    for (j = 1; j <= add_erase; j++)
    {
/* For debugging */
        //++++++++++++++++++++
//#ifdef    __LE_DEBUG
//      DrawHexOnBgx(0,pos+3,BG_PLTT_GREEN,&j,1);
//#endif
        //++++++++++++++++++++
        (void)CTRDGi_EraseFlashSectorLE(secNo);
    }

    /*Exclusive control (lock) */
    (void)OS_LockCartridge(ctrdgi_flash_lock_id);
    /*Access cycle settings */
    ram_cycle = MI_GetCartridgeRamCycle();
    MI_SetCartridgeRamCycle(AgbFlash->agbWait[0]);
    //*(vu16 *)REG_EXMEMCNT_ADDR=(*(vu16 *)REG_EXMEMCNT_ADDR & 0xfffc)|AgbFlash->agbWait[0]; // read 3cycles wait

    // Program

    ctrdg_flash_remainder = (u16)AgbFlash->sector.size;
    ctrdgi_backup_irq = OS_DisableIrq();
    while (ctrdg_flash_remainder)
    {
        result = CTRDGi_ProgramFlashByteLE(src, tgt);
        if (result)
            break;
        ctrdg_flash_remainder--;
        src++;
        tgt++;
    }
    (void)OS_RestoreIrq(ctrdgi_backup_irq);

    /*Access cycle settings */
    MI_SetCartridgeRamCycle(ram_cycle);
    /*Exclusive control (unlock) */
    (void)OS_UnlockCartridge(ctrdgi_flash_lock_id);

    return result;
}

u32 CTRDGi_VerifyFlashCoreFF(u8 *adr)
{
    u32     count;
    for (count = AgbFlash->sector.size; count > 0; count--)
    {
        if (*adr++ != 0xff)
        {
            break;
        }
    }
    return count;
}

u32 CTRDGi_VerifyFlashErase(u8 *tgt, u32 (*func_p) (u8 *))
{
    u32     result;
    MICartridgeRamCycle ram_cycle;

    /*Exclusive control (lock) */
    (void)OS_LockCartridge(ctrdgi_flash_lock_id);
    /*Access cycle settings */
    ram_cycle = MI_GetCartridgeRamCycle();
    MI_SetCartridgeRamCycle(AgbFlash->agbWait[0]);

    result = 0;
    if (func_p(tgt))
    {
        result = (CTRDG_BACKUP_RESULT_ERROR | CTRDG_BACKUP_PHASE_VERIFY_ERASE);
    }

    /*Access cycle settings */
    MI_SetCartridgeRamCycle(ram_cycle);
    /*Exclusive control (unlock) */
    (void)OS_UnlockCartridge(ctrdgi_flash_lock_id);

    return result;
}


u16 CTRDGi_EraseFlashChipLE(void)
{
    u16     result;
    CTRDGTaskInfo p;
    result = (u16)CTRDGi_EraseFlashChipCoreLE(&p);

    return result;
}

u16 CTRDGi_EraseFlashSectorLE(u16 secNo)
{
    u16     result;
    CTRDGTaskInfo p;
    p.sec_num = secNo;
    result = (u16)CTRDGi_EraseFlashSectorCoreLE(&p);

    return result;
}

u16 CTRDGi_WriteFlashSectorLE(u16 secNo, u8 *src)
{
    u16     result;
    CTRDGTaskInfo p;
    p.sec_num = secNo;
    p.data = src;
    result = (u16)CTRDGi_WriteFlashSectorCoreLE(&p);

    return result;
}

void CTRDGi_EraseFlashChipAsyncLE(CTRDG_TASK_FUNC callback)
{
    CTRDGTaskInfo p;

    CTRDGi_SetTask(&p, CTRDGi_EraseFlashChipCoreLE, callback);
}

void CTRDGi_EraseFlashSectorAsyncLE(u16 secNo, CTRDG_TASK_FUNC callback)
{
    CTRDGTaskInfo p;

    p.sec_num = secNo;
    CTRDGi_SetTask(&p, CTRDGi_EraseFlashSectorCoreLE, callback);
}

void CTRDGi_WriteFlashSectorAsyncLE(u16 secNo, u8 *src, CTRDG_TASK_FUNC callback)
{
    CTRDGTaskInfo p;

    p.sec_num = secNo;
    p.data = src;
    CTRDGi_SetTask(&p, CTRDGi_WriteFlashSectorCoreLE, callback);
}
