/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CTRDG - include
  File:     ctrdg_flash.h

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_CTRDG_FLASH_H_
#define NITRO_CTRDG_FLASH_H_

#include <nitro.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Definitions ****************************************************************/

/* Flash header address */
#define CTRDG_AGB_FLASH_ADR           0x0A000000

/* Maximum number of retries for the CTRDG_WriteAndVerifyAgbFlash function */
#define CTRDG_AGB_FLASH_RETRY_MAX     3


typedef struct CTRDGiFlashSectorTag
{
    u32     size;                      /* Sector size */
    u16     shift;                     /* Value after converting the sector size into a shift number */
    u16     count;                     /* Number of sectors */
    u16     top;                       /* The first usable sector number */
    u8      reserved[2];               /* Padding */
}
CTRDGiFlashSector;

typedef struct CTRDGFlashTypeTag
{
    u32     romSize;                   /* ROM size */
    CTRDGiFlashSector sector;          /* Sector data (* see CTRDGiFlashSector above) */
    MICartridgeRamCycle agbWait[2];    /* AGB cartridge bus RAM region's read/write weight value */
    u16     makerID;                   /* Vendor ID */
    u16     deviceID;                  /* Device ID */
}
CTRDGFlashType;

extern void CTRDGi_SetFlashBankMx(u16 bank);
extern u16 CTRDGi_ReadFlashID(void);
extern void StartFlashTimer(u16 phase);
extern void CheckFlashTimer(void);
extern void CTRDGi_SetFlashBankMx(u16 bank);
extern u16 CTRDGi_PollingSR512kCOMMON(u16 phase, u8 *adr, u16 lastData);
extern u16 CTRDGi_PollingSR1MCOMMON(u16 phase, u8 *adr, u16 lastData);

/*------------------------------------------------------------------*/
/*          Global variables                                          */
/*------------------------------------------------------------------*/

/*
 * Pointer to data that shows the structural content of flash memory.
 * (For details, see the data definitions above)
 */
extern const CTRDGFlashType *AgbFlash;

/*
 * Bar that shows progress during flash writes.
 * NOTE: ctrdg_flash_remainder decreases by 128 at a time for Atmel flash memory and 1 at a time for other devices.
 *   
 */
extern u16 ctrdg_flash_remainder;

/*------------------------------------------------------------------*/
/*          Data read                                          */
/*------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
  Name:         CTRDG_ReadAgbFlash
                CTRDG_ReadAgbFlashAsync
  
                Corresponding AGB function: extern void ReadFlash(u16 secNo,u32 offset,u8 *dst,u32 size)

  Description:  Starting from an address offset bytes into the target flash sector number, this function loads size bytes of data into the working memory area at dst.
                
                
                Access cycle settings are made within the function, and do not need to occur in advance.

                * Be aware that within this function the cartridge bus is locked for a fixed period of time.

                
  Arguments:    secNo: Target sector number
                offset: The offset, in bytes, within the sector
                dst: Address of the work region where the read data is stored
                size: Read size in bytes
                callback: The callback function invoked when the read process is complete (only for asynchronous functions).

  Returns:      None.
 *---------------------------------------------------------------------------*/
extern void CTRDG_ReadAgbFlash(u16 sec_num, u32 offset, u8 *dst, u32 size);
extern void CTRDG_ReadAgbFlashAsync(u16 sec_num, u32 offset, u8 *dst, u32 size,
                                    CTRDG_TASK_FUNC callback);


/*------------------------------------------------------------------*/
/*          Chip erasure                                              */
/*------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
  Name:         CTRDG_EraseAgbFlashChip
                CTRDG_EraseAgbFlashChipAsync
                
                Corresponding AGB function: extern u16 (*EraseFlashChip)()

  Description:  Completely erases the entire chip.
  
                Access cycle settings are made within the function, and do not need to occur in advance.
                As this function uses ticks to measure timeouts, the OS_InitTick function must be called in advance.
                
                
                In addition, in the asynchronous versions, by referring to 'result', a member of the CTRDGTaskInfo structure that is an argument of the callback function returned after calling this routine, it is possible to know whether the Erase process succeeded.
                

                * Be aware that within this function, all interrupts are prohibited for a certain period of time, and the cartridge bus is locked.
                In particular, when calling this function do not use DMAs that automatically launch at specific times, such as direct sound, V/H-Blank synchronization, display synchronization, cartridge requests, etc.
                

                
  Arguments:    callback: The callback function invoked when the erase process is complete (only for asynchronous functions).

  Returns:      0     : Normal completion (only for synchronous functions).
                Non-zero: Erase error
 *---------------------------------------------------------------------------*/
extern u16 CTRDG_EraseAgbFlashChip(void);
extern void CTRDG_EraseAgbFlashChipAsync(CTRDG_TASK_FUNC callback);


/*------------------------------------------------------------------*/
/*          Sector deletion                                              */
/*------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
  Name:         CTRDG_EraseAgbFlashSector
                CTRDG_EraseAgbFlashSectorAsync
                
                Corresponding AGB function: extern u16 (*EraseFlashSector)(u16 secNo)

  Description:  Erases the sector with the target sector number.
                This routine will be called by the write routine, so it does not usually need to be called before performing a write.
                
                An error is returned when the target sector number is out of range.
                
                In addition, in the asynchronous versions, by referring to 'result', a member of the CTRDGTaskInfo structure that is an argument of the callback function returned after calling this routine, it is possible to know whether the Erase process succeeded.
                
                
                Access cycle settings are made within the function, and do not need to occur in advance.
                As this function uses ticks to measure timeouts, the OS_InitTick function must be called in advance.
                

                * Be aware that within this function, all interrupts are prohibited for a certain period of time, and the cartridge bus is locked.
                In particular, when calling this function do not use DMAs that automatically launch at specific times, such as direct sound, V/H-Blank synchronization, display synchronization, cartridge requests, etc.
                

                
  Arguments:    sec_num: Target sector number
                callback: The callback function invoked when the erase process is complete (only for asynchronous functions).

  Returns:      0     : Normal completion (only for synchronous functions).
                Non-zero: Erase error
 *---------------------------------------------------------------------------*/
extern u16 CTRDG_EraseAgbFlashSector(u16 sec_num);
extern void CTRDG_EraseAgbFlashSectorAsync(u16 sec_num, CTRDG_TASK_FUNC callback);


/*------------------------------------------------------------------*/
/*          Per-sector data writes                                */
/*------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
  Name:         CTRDG_WriteAgbFlashSector
                CTRDG_WriteAgbFlashSectorAsync
                
                Corresponding AGB function: extern u16 (*ProgramFlashSector) (u16 secNo,u8 *src)

  Description:  Writes one sector's worth (4 KB) of data to the target sector number, starting at the src address.
                Writes within this routine occur after the above CTRDG_EraseAgbFlashSector is called and the sector erased.
                An error is returned when the target sector number is out of range.
                As this routine runs, the global variable, ctrdg_flash_remainder, can be referenced to know how many bytes remain.
                
                In addition, you can find out whether an asynchronous write process succeeded by checking the result member variable of the CTRDGTaskInfo structure, which is an argument to the callback function returned after this routine is called.
                
                
                Access cycle settings are made within the function, and do not need to occur in advance.
                As this function uses ticks to measure timeouts, the OS_InitTick function must be called in advance.
                

                * Be aware that within this function, all interrupts are prohibited for a certain period of time, and the cartridge bus is locked.
                In particular, when calling this function do not use DMAs that automatically launch at specific times, such as direct sound, V/H-Blank synchronization, display synchronization, cartridge requests, etc.
                

                
  Arguments:    sec_num: Target sector number
                src: Write source address
                callback: The callback function called when the write processes complete (only for asynchronous functions).

  Returns:      0     : Normal completion (only for synchronous functions).
                Non-zero: Write error
 *---------------------------------------------------------------------------*/
extern u16 CTRDG_WriteAgbFlashSector(u16 sec_num, u8 *src);
extern void CTRDG_WriteAgbFlashSectorAsync(u16 sec_num, u8 *src, CTRDG_TASK_FUNC callback);


/*------------------------------------------------------------------*/
/*          Per-byte data verification                              */
/*------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
  Name:         CTRDG_VerifyAgbFlash
                CTRDG_VerifyAgbFlashAsync
                
                Corresponding AGB function: extern u32 VerifyFlash(u16 secNo,u8 *src,u32 size)

  Description:  Verifies data starting from the 'src' address with target flash sector number data, in portions of 'size' bytes.
                This function returns a 0 when verifies end normally, and when a verify error occurs, it returns the address where the error occurred.
                In addition, in the asynchronous versions, by referring to 'result', a member of the CTRDGTaskInfo structure that is an argument of the callback function returned after calling this routine, it is possible to know whether the verification process succeeded.
                
                
  Arguments:    sec_num: Target sector number
                src: The original address to verify (original data).
                size: Verify size (bytes)
                callback: The callback function called when the verification process completes (only for asynchronous functions)

  Returns:      0     : Normal completion (only for synchronous functions).
                Non-zero: Returns the device-side error address for a verify error.
 *---------------------------------------------------------------------------*/
extern u32 CTRDG_VerifyAgbFlash(u16 sec_num, u8 *src, u32 size);
extern void CTRDG_VerifyAgbFlashAsync(u16 sec_num, u8 *src, u32 size, CTRDG_TASK_FUNC callback);


/*------------------------------------------------------------------*/
/*          Per-sector data writes and per-byte verification          */
/*------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
  Name:         CTRDG_WriteAndVerifyAgbFlash
                CTRDG_WriteAndVerifyAgbFlashAsync
                
                Corresponding AGB function: extern u32 ProgramFlashSectorEx2(u16 secNo, u8 *src, u32 verifySize)

  Description:  This function will call the CTRDG_WriteAgbFlashSector function to write data. After that, it will call the CTRDG_VerifyAgbFlash function to verify verifysize bytes.
                
                When errors occur, retries are made up to a maximum of CTRDG_AGB_FLASH_RETRY_MAX (defined in ctrdg_flash.h) times.
                
                NOTE: When there is a write error, this function uses 16 out of 32 bits to return the error code. When there is a verify error, however, a 32-bit device error address is returned. Take note of this when checking the error code.
                
                
                In addition, in the asynchronous versions, by referring to 'result', a member of the CTRDGTaskInfo structure that is an argument of the callback function returned after calling this routine, it is possible to know whether the WriteAndVerify process succeeded.
                
                
                Access cycle settings are made within the function, and do not need to occur in advance.
                As this function uses ticks to measure timeouts, the OS_InitTick function must be called in advance.
                

                * Be aware that within this function, all interrupts are prohibited for a certain period of time, and the cartridge bus is locked.
                In particular, when calling this function do not use DMAs that automatically launch at specific times, such as direct sound, V/H-Blank synchronization, display synchronization, cartridge requests, etc.
                

                
  Arguments:    sec_num: Target sector number
                src: Write source address
                verifysize : Verify size (bytes)
                callback: The callback function invoked when the WriteAndVerify process completes (only for asynchronous functions)

  Returns:      0     : Normal completion (only for synchronous functions).
                Non-zero: Returns the device-side error address for a verify error.
 *---------------------------------------------------------------------------*/
extern u32 CTRDG_WriteAndVerifyAgbFlash(u16 sec_num, u8 *src, u32 verifysize);
extern void CTRDG_WriteAndVerifyAgbFlashAsync(u16 sec_num, u8 *src, u32 verifysize,
                                              CTRDG_TASK_FUNC callback);


#ifdef __cplusplus
}      /* extern "C" */
#endif


#endif /* NITRO_CTRDG_FLASH_H_ */
