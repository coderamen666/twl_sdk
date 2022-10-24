/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - libraries
  File:     card_spi.h

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/
#ifndef NITRO_LIBRARIES_CARD_SPI_H__
#define NITRO_LIBRARIES_CARD_SPI_H__


#ifdef __cplusplus
extern  "C"
{
#endif


/*---------------------------------------------------------------------------*/
/* Constants */

#define	CSPI_CONTINUOUS_ON	0x0040
#define	CSPI_CONTINUOUS_OFF	0x0000

/* Commands */
#define COMM_WRITE_STATUS	0x01   /* Write status */
#define COMM_PROGRAM_PAGE	0x02   /* Page program */
#define COMM_READ_ARRAY		0x03   /* Read */
#define COMM_WRITE_DISABLE	0x04   /* Disable writing (not used) */
#define COMM_READ_STATUS	0x05   /* Read status */
#define COMM_WRITE_ENABLE	0x06   /* Enable writing */

/* FLASH */
#define COMM_PAGE_WRITE		0x0A
#define COMM_PAGE_ERASE		0xDB
#define COMM_SECTOR_ERASE	0xD8
#define COMM_SUBSECTOR_ERASE	0x20
#define COMM_CHIP_ERASE		0xC7
#define CARDFLASH_READ_BYTES_FAST	0x0B    /* Unused */
#define CARDFLASH_DEEP_POWDOWN		0xB9    /* Unused */
#define CARDFLASH_WAKEUP			0xAB    /* Unused */

/* Status register bits */
#define	COMM_STATUS_WIP_BIT		0x01    /* WriteInProgress (bufy) */
#define	COMM_STATUS_WEL_BIT		0x02    /* WriteEnableLatch */
#define	COMM_STATUS_BP0_BIT		0x04
#define	COMM_STATUS_BP1_BIT		0x08
#define	COMM_STATUS_BP2_BIT		0x10
#define COMM_STATUS_WPBEN_BIT	0x80


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         CARDi_InitStatusRegister

  Description:  Corrects status registers at startup for FRAM and EEPROM.
                (Because FRAM write protection settings may change when the power is turned on)
                (Because there may be invalid values initially in EEPROM when it is delivered)

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARDi_InitStatusRegister(void);

/*---------------------------------------------------------------------------*
  Name:         CARDi_CommandReadStatus

  Description:  Read status.

  Arguments:    None.

  Returns:      Status value.
 *---------------------------------------------------------------------------*/
u8 CARDi_CommandReadStatus(void);

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadBackupCore

  Description:  The entire read command to the device.

  Arguments:    src         Device offset to read from
                dst         Memory address to read to
                len         Read size

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARDi_ReadBackupCore(u32 src, void *dst, u32 len);

/*---------------------------------------------------------------------------*
  Name:         CARDi_ProgramBackupCore

  Description:  The entire program (non-deleting write) command to the device.

  Arguments:    dst             Device offset to write to
                src             Memory address to write from
                len             Write size

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARDi_ProgramBackupCore(u32 dst, const void *src, u32 len);

/*---------------------------------------------------------------------------*
  Name:         CARDi_WriteBackupCore

  Description:  The entire write (delete and program) command to the device.

  Arguments:    dst             Device offset to write to
                src             Memory address to write from
                len             Write size

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARDi_WriteBackupCore(u32 dst, const void *src, u32 len);

/*---------------------------------------------------------------------------*
  Name:         CARDi_VerifyBackupCore

  Description:  The entire verify (actually read and compare) command to the device.

  Arguments:    dst        Device offset to compare
                src        Memory address to compare against
                len        Comparison size

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARDi_VerifyBackupCore(u32 dst, const void *src, u32 len);

/*---------------------------------------------------------------------------*
  Name:         CARDi_EraseBackupSectorCore

  Description:  The entire sector delete command to the device.

  Arguments:    dst        Device offset to delete
                len        Deletion size

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARDi_EraseBackupSectorCore(u32 dst, u32 len);

/*---------------------------------------------------------------------------*
  Name:         CARDi_EraseBackupSubSectorCore

  Description:  The entire sub-sector deletion command to the device.

  Arguments:    dst               Deletion target device offset
                len               Deletion size

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARDi_EraseBackupSubSectorCore(u32 dst, u32 len);

/*---------------------------------------------------------------------------*
  Name:         CARDi_EraseChipCore

  Description:  The entire chip delete command to the device.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARDi_EraseChipCore(void);

/*---------------------------------------------------------------------------*
  Name:         CARDi_SetWriteProtectCore

  Description:  The entire write-protect command to the device.

  Arguments:    stat              Protect flag to set

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARDi_SetWriteProtectCore(u16 stat);


#ifdef __cplusplus
} // extern "C"
#endif



#endif // NITRO_LIBRARIES_CARD_SPI_H__
