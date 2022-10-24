/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - libraries
  File:     card_spi.c

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-07-15#$
  $Rev: 10908 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/


#include <nitro.h>

#include "../include/card_common.h"
#include "../include/card_spi.h"

/* CARD-SPI control */


/*---------------------------------------------------------------------------*/
/* Constants */

/* The reg_MI_MCCNT0 bits*/

#define MCCNT0_SPI_CLK_MASK	0x0003 /* Baud rate settings mask */
#define MCCNT0_SPI_CLK_4M	0x0000 /* Baud rate 4.19 MHz */
#define MCCNT0_SPI_CLK_2M	0x0001 /* Baud rate 2.10 MHz */
#define MCCNT0_SPI_CLK_1M	0x0002 /* Baud rate 1.05 MHz */
#define MCCNT0_SPI_CLK_524K	0x0003 /* Baud rate 524 kHz */
#define MCCNT0_SPI_BUSY		0x0080 /* SPI busy flag */
#define	MMCNT0_SEL_MASK		0x2000 /* CARD ROM / SPI selection mask */
#define	MMCNT0_SEL_CARD		0x0000 /* CARD ROM selection */
#define	MMCNT0_SEL_SPI		0x2000 /* CARD SPI selection */
#define MCCNT0_INT_MASK		0x4000 /* Transfer completion interrupt mask */
#define MCCNT0_INT_ON		0x4000 /* Enable transfer completion interrupts */
#define MCCNT0_INT_OFF		0x0000 /* Disable transfer completion interrupts */
#define MCCNT0_MASTER_MASK	0x8000 /* CARD master mask */
#define MCCNT0_MASTER_ON	0x8000 /* CARD master ON */
#define MCCNT0_MASTER_OFF	0x0000 /* CARD master OFF */

#define CARD_BACKUP_TYPE_VENDER_IRC		(0xFF) // IRC vendor type
#define IRC_BACKUP_WAIT		50     // Interval at which IRC SPI sends commands

/*---------------------------------------------------------------------------*/
/* Variables */

typedef struct
{                                      /* SPI internal management parameter */
    u32     rest_comm;                 /* Total remaining number of bytes to send */
    u32     src;                       /* Transfer source */
    u32     dst;                       /* Transfer destination */
    BOOL    cmp;                       /* Comparison result */
}
CARDiParam;

static CARDiParam cardi_param;


/*---------------------------------------------------------------------------*/
/* Functions */

static BOOL CARDi_CommandCheckBusy(void);
static void CARDi_CommArray(const void *src, void *dst, u32 n, void (*func) (CARDiParam *));
static void CARDi_CommReadCore(CARDiParam * p);
static void CARDi_CommWriteCore(CARDiParam * p);
static void CARDi_CommVerifyCore(CARDiParam * p);

/*---------------------------------------------------------------------------*
  Name:         CARDi_CommArrayRead

  Description:  Post-read processing for read commands.

  Arguments:    dst: Read-target memory
                len: Read size

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARDi_CommArrayRead(void *dst, u32 len)
{
    CARDi_CommArray(NULL, dst, len, CARDi_CommReadCore);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_CommArrayWrite

  Description:  Post-write processing for write commands.

  Arguments:    dst: Write source memory
                len: Write size

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARDi_CommArrayWrite(const void *src, u32 len)
{
    CARDi_CommArray(src, NULL, len, CARDi_CommWriteCore);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_CommArrayVerify

  Description:  Post-read comparisons for (comparison) read commands.

  Arguments:    src: Comparison source memory
                len: Comparison size

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARDi_CommArrayVerify(const void *src, u32 len)
{
    CARDi_CommArray(src, NULL, len, CARDi_CommVerifyCore);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_EnableSpi

  Description:  Enables CARD-SPI.

  Arguments:    cont: Continuous transfer flag (CSPI_CONTINUOUS_ON or OFF)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARDi_EnableSpi(u32 cont)
{
    /*
     * If a device with a slow clock speed is introduced in the future, MCCNT0_SPI_CLK_4M will be added to the property array, resulting in a dynamic change
     * 
     */
    const u16 ctrl = (u16)(MCCNT0_MASTER_ON |
                           MCCNT0_INT_OFF | MMCNT0_SEL_SPI | MCCNT0_SPI_CLK_4M | cont);
    reg_MI_MCCNT0 = ctrl;
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_WaitBusy

  Description:  Waits for CARD-SPI completion.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARDi_WaitBusy(void)
{
    while ((reg_MI_MCCNT0 & MCCNT0_SPI_BUSY) != 0)
    {
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_CommandBegin

  Description:  Declaration for starting the issuing of commands.

  Arguments:    len: Length of the command to be issued

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_CommandBegin(int len)
{
    cardi_param.rest_comm = (u32)len;
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_CommandEnd

  Description:  Command send completion process.

  Arguments:    force_wait: Forced wait time (ms)
                timeout: Timeout duration (ms)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_CommandEnd(u32 force_wait, u32 timeout)
{
    if (force_wait + timeout > 0)
    {
        /*
         * If there is a forced wait, only wait for that time.
         * Unlike what was first talked about, the FLASH wait was also forced.
         */
        if (force_wait > 0)
        {
            OS_Sleep(force_wait);
        }
        if (timeout > 0)
        {
            /*
             * Only the PageWrite command is "forced to wait for just the first half," so this kind of special loop is used
             * 
             */
            int     rest = (int)(timeout - force_wait);
            while (!CARDi_CommandCheckBusy() && (rest > 0))
            {
                int     interval = (rest < 5) ? rest : 5;
                OS_Sleep((u32)interval);
                rest -= interval;
            }
        }
        /*
         * Other commands have waited the designated time, so one ReadStatus alone is acceptable
         * 
         */
        /* If busy at this point, then timeout */
        if (!CARDi_CommandCheckBusy())
        {
            cardi_common.cmd->result = CARD_RESULT_TIMEOUT;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_CommandReadStatus

  Description:  Read status.

  Arguments:    None.

  Returns:      Status value.
 *---------------------------------------------------------------------------*/
u8 CARDi_CommandReadStatus(void)
{
    const u8    buf = COMM_READ_STATUS;
    u8          dst;
    CARDi_CommandBegin(1 + 1);
    CARDi_CommArrayWrite(&buf, 1);
    CARDi_CommArrayRead(&dst, 1);
    CARDi_CommandEnd(0, 0);
    return dst;
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_CommandCheckBusy

  Description:  Determines from the read status command whether the device is still busy.

  Arguments:    None.

  Returns:      TRUE when not busy.
 *---------------------------------------------------------------------------*/
static BOOL CARDi_CommandCheckBusy(void)
{
    return ((CARDi_CommandReadStatus() & COMM_STATUS_WIP_BIT) == 0);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_WaitPrevCommand

  Description:  Busy check prior to the issuing of commands.
                Waits for 50 ms if a busy state from the prior command exists.
                (This is impossible under normal circumstances as long as the library is installed and the card correctly connected.)

  Arguments:    None.

  Returns:      TRUE if the command can be correctly issued.
 *---------------------------------------------------------------------------*/
static BOOL CARDi_WaitPrevCommand(void)
{
    CARDi_CommandEnd(0, 50);
    /* No response if a timeout occurs here */
    if (cardi_common.cmd->result == CARD_RESULT_TIMEOUT)
    {
        cardi_common.cmd->result = CARD_RESULT_NO_RESPONSE;
        return FALSE;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_WaitBusyforIRC

  Description:  Waits for IRC-SPI completion.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARDi_WaitBusyforIRC(void)
{
    u16 tick = OS_GetTickLo();
    while(OS_TicksToMicroSeconds(OS_GetTickLo() - tick) < IRC_BACKUP_WAIT)
    {
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_CommArray

  Description:  Common processes for the issuing of commands.

  Arguments:    src: Processing source memory (NULL if not used)
                dst: Processing destination memory (NULL if not used)
                len: Processing size
                func: Processing content

  Returns:      None.
 *---------------------------------------------------------------------------*/
static BOOL need_command = TRUE;

void CARDi_CommArray(const void *src, void *dst, u32 len, void (*func) (CARDiParam *))
{
    CARDiParam *const p = &cardi_param;
    CARDiCommandArg *const arg = cardi_common.cmd;
    // Determine whether it is IRC by determining whether its vendor code is 0xFF
    BOOL isIRC = ((u8)((arg->type >> CARD_BACKUP_TYPE_VENDER_SHIFT) & CARD_BACKUP_TYPE_VENDER_MASK) == CARD_BACKUP_TYPE_VENDER_IRC) ? TRUE : FALSE;
    p->src = (u32)src;
    p->dst = (u32)dst;

    // Clarified this, because for IRC support there were cases when the CLK was set to something other than 4M
    CARDi_EnableSpi(CSPI_CONTINUOUS_ON | MCCNT0_SPI_CLK_4M);

    for (; len > 0; --len)
    {
        if(need_command)
        {
            if(isIRC)
            {
                vu16 dummy_read;
                
                CARDi_WaitBusyforIRC(); // This wait is characteristic of the IRC's built-in CPU
                CARDi_EnableSpi(CSPI_CONTINUOUS_ON | MCCNT0_SPI_CLK_1M); // Set to 1 MHz when sending commands to SPI
                CARDi_WaitBusy();
                reg_MI_MCD0 = 0x00;  // Swap SSU and backup device
                CARDi_WaitBusy();
                dummy_read = reg_MI_MCD0;
                need_command = FALSE;
                CARDi_WaitBusyforIRC(); // This wait is characteristic of the IRC's built-in CPU
                CARDi_EnableSpi(CSPI_CONTINUOUS_ON | MCCNT0_SPI_CLK_4M);
            }
        }
        if (!--p->rest_comm)
        {
            CARDi_EnableSpi(CSPI_CONTINUOUS_OFF | MCCNT0_SPI_CLK_4M);
            need_command = TRUE;
        }
        CARDi_WaitBusy();
        (*func) (p);
    }
    if (!p->rest_comm)
    {
        reg_MI_MCCNT0 = (u16)(MCCNT0_MASTER_OFF | MCCNT0_INT_OFF | MCCNT0_SPI_CLK_4M);
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_CommReadCore

  Description:  Reads a single byte.

  Arguments:    p: Command parameter

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_CommReadCore(CARDiParam * p)
{
    reg_MI_MCD0 = 0;
    CARDi_WaitBusy();
    MI_WriteByte((void *)p->dst, (u8)reg_MI_MCD0);
    ++p->dst;
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_CommWriteCore

  Description:  Writes a single byte.

  Arguments:    p: Command parameter

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_CommWriteCore(CARDiParam * p)
{
    vu16    tmp;
    reg_MI_MCD0 = (u16)MI_ReadByte((void *)p->src);
    ++p->src;
    CARDi_WaitBusy();
    tmp = reg_MI_MCD0;
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_CommVerifyCore

  Description:  Single byte comparison.

  Arguments:    p: Command parameter

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_CommVerifyCore(CARDiParam * p)
{
    reg_MI_MCD0 = 0;
    CARDi_WaitBusy();
    /*
     * Read, and disconnect if no match.
     * However, you must exit from continuous transmissions, and therefore need to read at least once more than required.
     * 
     */
    if ((u8)reg_MI_MCD0 != MI_ReadByte((void *)p->src))
    {
        p->cmp = FALSE;
        if (p->rest_comm > 1)
        {
            p->rest_comm = 1;
        }
    }
    ++p->src;
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_WriteEnable

  Description:  Enables device write operations (this is needed once before every write-based command).

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_WriteEnable(void)
{
    static const u8 arg[1] = { COMM_WRITE_ENABLE, };
    CARDi_CommandBegin(sizeof(arg));
    CARDi_CommArrayWrite(arg, sizeof(arg));
    CARDi_CommandEnd(0, 0);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_SendSpiAddressingCommand

  Description:  Sets the address specification command settings.

  Arguments:    addr: Address on the device where searches will take place
                mode: Command to be issued

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_SendSpiAddressingCommand(u32 addr, u32 mode)
{
    const u32 width = cardi_common.cmd->spec.addr_width;
    u32     addr_cmd;
    switch (width)
    {
    case 1:
        /* For a 4-kbit device, [A:8] becomes part of the command */
        addr_cmd = (u32)(mode | ((addr >> 5) & 0x8) | ((addr & 0xFF) << 8));
        break;
    case 2:
        addr_cmd = (u32)(mode | (addr & 0xFF00) | ((addr & 0xFF) << 16));
        break;
    case 3:
        addr_cmd = (u32)(mode |
                         ((addr >> 8) & 0xFF00) | ((addr & 0xFF00) << 8) | ((addr & 0xFF) << 24));
        break;
    default:
        SDK_ASSERT(FALSE);
        break;
    }
    CARDi_CommArrayWrite(&addr_cmd, (u32)(1 + width));
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_InitStatusRegister

  Description:  For FRAM/EEPROM, adjusts the status register at launch.
                * Because changes in write protection may occur when the power is turned on when using FRAM.
                * Because initial values may be invalid at time of delivery when using EEPROM.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_InitStatusRegister(void)
{
    /*
     * The status register makes appropriate adjustments when used the first time for devices that could take erroneous defaults values.
     * 
     */
    const u8 stat = cardi_common.cmd->spec.initial_status;
    if (stat != 0xFF)
    {
        static BOOL status_checked = FALSE;
        if (!status_checked)
        {
            if (CARDi_CommandReadStatus() != stat)
            {
                CARDi_SetWriteProtectCore(stat);
            }
            status_checked = TRUE;
        }
    }
}


/********************************************************************/
/*
 * Direct internal process.
 * Exclusions, requests, and so on are all already completed with this step.
 * The limits on the size are assumed to have been taken into consideration.
 */

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadBackupCore

  Description:  The entire read command for the device.

  Arguments:    src: Read origin's device offset
                dst: Read target's memory address
                len: Read size

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_ReadBackupCore(u32 src, void *dst, u32 len)
{
    if (CARDi_WaitPrevCommand())
    {
        CARDiCommandArg *const cmd = cardi_common.cmd;
        /* The read process is done all at once because there is no page boundary */
        CARDi_CommandBegin((int)(1 + cmd->spec.addr_width + len));
        CARDi_SendSpiAddressingCommand(src, COMM_READ_ARRAY);
        CARDi_CommArrayRead(dst, len);
        CARDi_CommandEnd(0, 0);
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_ProgramBackupCore

  Description:  The entire Program (write without delete) command for the device.

  Arguments:    dst: Write target's device offset
                src: Write origin's memory address
                len: Write size

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_ProgramBackupCore(u32 dst, const void *src, u32 len)
{
    if (CARDi_WaitPrevCommand())
    {
        CARDiCommandArg *const cmd = cardi_common.cmd;
        /* Divide up the process so the write does not cross page boundaries */
        const u32 page = cmd->spec.page_size;
        while (len > 0)
        {
            const u32 mod = (u32)(dst & (page - 1));
            u32     size = page - mod;
            if (size > len)
            {
                size = len;
            }
            CARDi_WriteEnable();
            CARDi_CommandBegin((int)(1 + cmd->spec.addr_width + size));
            CARDi_SendSpiAddressingCommand(dst, COMM_PROGRAM_PAGE);
            CARDi_CommArrayWrite(src, size);
            CARDi_CommandEnd(cmd->spec.program_page, 0);
            if (cmd->result != CARD_RESULT_SUCCESS)
            {
                break;
            }
            src = (const void *)((u32)src + size);
            dst += size;
            len -= size;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_WriteBackupCore

  Description:  The entire Write (Delete + Program) command to the device.

  Arguments:    dst: Write target's device offset
                src: Write origin's memory address
                len: Write size

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_WriteBackupCore(u32 dst, const void *src, u32 len)
{
    if (CARDi_WaitPrevCommand())
    {
        CARDiCommandArg *const cmd = cardi_common.cmd;
        /* Divide up the process so the write does not cross page boundaries */
        const u32 page = cmd->spec.page_size;
        while (len > 0)
        {
            const u32 mod = (u32)(dst & (page - 1));
            u32     size = page - mod;
            if (size > len)
            {
                size = len;
            }
            CARDi_WriteEnable();
            CARDi_CommandBegin((int)(1 + cmd->spec.addr_width + size));
            CARDi_SendSpiAddressingCommand(dst, COMM_PAGE_WRITE);
            CARDi_CommArrayWrite(src, size);
            CARDi_CommandEnd(cmd->spec.write_page, cmd->spec.write_page_total);
            if (cmd->result != CARD_RESULT_SUCCESS)
            {
                break;
            }
            src = (const void *)((u32)src + size);
            dst += size;
            len -= size;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_VerifyBackupCore

  Description:  The entire Verify (actually this is Read + comparison) command to the device.

  Arguments:    dst: Comparison target's device offset
                src: Memory address of comparison source
                len: Comparison size

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_VerifyBackupCore(u32 dst, const void *src, u32 len)
{
    if (CARDi_WaitPrevCommand())
    {
        CARDiCommandArg *const cmd = cardi_common.cmd;
        /* The read process is done all at once because there is no page boundary */
        cardi_param.cmp = TRUE;
        CARDi_CommandBegin((int)(1 + cmd->spec.addr_width + len));
        CARDi_SendSpiAddressingCommand(dst, COMM_READ_ARRAY);
        CARDi_CommArrayVerify(src, len);
        CARDi_CommandEnd(0, 0);
        if ((cmd->result == CARD_RESULT_SUCCESS) && !cardi_param.cmp)
        {
            cmd->result = CARD_RESULT_FAILURE;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_EraseBackupSectorCore

  Description:  The entire Sector Delete command for the device.

  Arguments:    dst: Deletion target device offset
                len: Deletion size

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_EraseBackupSectorCore(u32 dst, u32 len)
{
    CARDiCommandArg *const cmd = cardi_common.cmd;
    const u32 sector = cmd->spec.sect_size;
    /* No processing takes place if the range is not aligned to an integer multiple of sectors. */
    if (((dst | len) & (sector - 1)) != 0)
    {
        cmd->result = CARD_RESULT_INVALID_PARAM;
    }
    else if (CARDi_WaitPrevCommand())
    {
        /* Processed in sector-aligned units */
        while (len > 0)
        {
            CARDi_WriteEnable();
            CARDi_CommandBegin((int)(1 + cmd->spec.addr_width + 0));
            CARDi_SendSpiAddressingCommand(dst, COMM_SECTOR_ERASE);
            CARDi_CommandEnd(cmd->spec.erase_sector, cmd->spec.erase_sector_total);
            if (cmd->result != CARD_RESULT_SUCCESS)
            {
                break;
            }
            dst += sector;
            len -= sector;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_EraseBackupSubSectorCore

  Description:  The entire sub-sector deletion command to the device.

  Arguments:    dst: Deletion target device offset
                len: Deletion size

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_EraseBackupSubSectorCore(u32 dst, u32 len)
{
    CARDiCommandArg *const cmd = cardi_common.cmd;
    const u32 sector = cmd->spec.subsect_size;
    /* No processing takes place if the processing range is not aligned to an integer multiple of sub-sectors */
    if (((dst | len) & (sector - 1)) != 0)
    {
        cmd->result = CARD_RESULT_INVALID_PARAM;
    }
    else if (CARDi_WaitPrevCommand())
    {
        /* Processed in sector-aligned units */
        while (len > 0)
        {
            CARDi_WriteEnable();
            CARDi_CommandBegin((int)(1 + cmd->spec.addr_width + 0));
            CARDi_SendSpiAddressingCommand(dst, COMM_SUBSECTOR_ERASE);
            CARDi_CommandEnd(cmd->spec.erase_subsector, cmd->spec.erase_subsector_total);
            if (cmd->result != CARD_RESULT_SUCCESS)
            {
                break;
            }
            dst += sector;
            len -= sector;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_EraseChipCore

  Description:  The entire chip delete command for the device.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_EraseChipCore(void)
{
    if (CARDi_WaitPrevCommand())
    {
        CARDiCommandArg *const cmd = cardi_common.cmd;
        static const u8 arg[1] = { COMM_CHIP_ERASE, };
        CARDi_WriteEnable();
        CARDi_CommandBegin(sizeof(arg));
        CARDi_CommArrayWrite(arg, sizeof(arg));
        CARDi_CommandEnd(cmd->spec.erase_chip, cmd->spec.erase_chip_total);
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_SetWriteProtectCore

  Description:  The entire write-protect command for the device.

  Arguments:    stat: Protect flag to be set

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_SetWriteProtectCore(u16 stat)
{
    if (CARDi_WaitPrevCommand())
    {
        CARDiCommandArg *const cmd = cardi_common.cmd;
        int     retry_count = 10;
        u8      arg[2];
        arg[0] = COMM_WRITE_STATUS;
        arg[1] = (u8)stat;
        do
        {
            CARDi_WriteEnable();
            CARDi_CommandBegin(1 + 1);
            CARDi_CommArrayWrite(&arg, sizeof(arg));
            CARDi_CommandEnd(5, 0);
        }
        while ((cmd->result == CARD_RESULT_TIMEOUT) && (--retry_count > 0));
    }
}


#if	0

/********************************************************************/
/********************************************************************/
/* Device-specific commands now being verified ***********************/
/********************************************************************/
/********************************************************************/


/* ID read */
static void CARDi_ReadIdCore(void)
{
    /*
     * These commands do not exist in EEPROM or FRAM.
     * Do they also not exist in the ST 2 Mbit FLASH?
     * Assume that an operation is not defined when incompatible command bytes are sent. For 2-megabit FLASH, should use of these commands always be denied, or should they be broken up into CARD_BACKUP_TYPE_FLASH_2MBITS_SANYO (for example)?
     * 
     * 
     * In either case, won't there be scenarios where an ID is required even if we've already categorized them?
     * 
     * If there is any reserve capacity, we plan to use it internally to determine the validity.
     */
    cardi_common.cmd->result = CARD_RESULT_UNSUPPORTED;
}

/* Commands that can only be used with 2M FLASH  *******************************/

/* Page erase (FLASH) */
static void CARDi_ErasePageCore(u32 dst)
{
    CARDi_WriteEnable();
    CARDi_CommandBegin(1 + cardi_common.cmd->spec.addr_width + 0);
    CARDi_SendSpiAddressingCommand(dst, COMM_PAGE_ERASE);
    CARDi_CommandEnd(cardi_common.cmd->spec.erase_page, 0);
}


#endif
