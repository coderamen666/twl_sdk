/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CTRDG - include
  File:     ctrdg_sram.h

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
#ifndef NITRO_CTRDG_SRAM_H_
#define NITRO_CTRDG_SRAM_H_

#include <nitro.h>

#ifdef __cplusplus
extern "C" {
#endif

/* For debugging */
//#ifndef __SRAM_DEBUG
#define CTRDG_AGB_SRAM_ADR          0x0A000000  /* SRAM leading address */
#define CTRDG_AGB_SRAM_SIZE_256K    0x00008000  /* 256K SRAM size */
#define CTRDG_AGB_SRAM_SIZE_512K    0x00010000  /* 512K SRAM size */
//#else
//#define CTRDG_AGB_SRAM_ADR        0x02018000
//#define CTRDG_AGB_SRAM_SIZE_256K    0x00000400
//#define CTRDG_AGB_SRAM_SIZE_512K    0x00000800
//#endif

/* Max number of retries for the CTRDG_WriteAgbSramEx function */
#define CTRDG_AGB_SRAM_RETRY_MAX    3


/*------------------------------------------------------------------
--------------------------------------------------------------------*/

/*------------------------------------------------------------------*/
/*          Data read                                          */
/*------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
  Name:         CTRDG_ReadAgbSram
                CTRDG_ReadAgbSramAsync
  
                Corresponding AGB function: extern void ReadSram(u8 *src, u8 *dst, u32 size)

  Description:  From the SRAM address specified in the argument, reads 'size' bytes worth of data to the working memory starting at the 'dst' address.
                
                
  Arguments:    src: The read source SRAM address (address in the AGB memory map)
                dst: The work region address where the read data is stored (address in the AGB memory map)
                size: Read size in bytes
                callback: The callback function invoked when the read process is complete (only for asynchronous functions).

  Returns:      None.
 *---------------------------------------------------------------------------*/
extern void CTRDG_ReadAgbSram(u32 src, void *dst, u32 size);
extern void CTRDG_ReadAgbSramAsync(u32 src, void *dst, u32 size, CTRDG_TASK_FUNC callback);


/*------------------------------------------------------------------*/
/*          Writing the data                                          */
/*------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
  Name:         CTRDG_WriteAgbSram
                CTRDG_WriteAgbSramAsync
                
                Corresponding AGB function: extern void WriteSram(u8 *dst, u8 *src, u32 size)

  Description:  From the work region address specified in the argument, writes 'size' bytes worth of data to SRAM starting at the 'dst' address.
                
                
  Arguments:    dst: Write destination SRAM address (address in the AGB memory map)
                src: Write source work region address
                size: Write size in bytes
                callback: The callback function called when the write processes complete (only for asynchronous functions).

  Returns:      None.
 *---------------------------------------------------------------------------*/
extern void CTRDG_WriteAgbSram(u32 dst, const void *src, u32 size);
extern void CTRDG_WriteAgbSramAsync(u32 dst, const void *src, u32 size, CTRDG_TASK_FUNC callback);


/*------------------------------------------------------------------*/
/*          Data verification                                        */
/*------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
  Name:         CTRDG_VerifyAgbSram
                CTRDG_VerifyAgbSramAsync
                
                Corresponding AGB function: extern u32 VerifySram(u8 *tgt, u8 *src, u32 size)

  Description:  Verifies the data from the work region's 'src' address with the data at SRAM's 'tgt' address in 'size' byte portions.
                This function returns a 0 when verifications end normally, and when a verification error occurs, it returns the address where the error occurred.
                
                In addition, in the asynchronous versions, by referring to 'result', a member of the CTRDGTaskInfo structure that is an argument of the callback function returned after calling this routine, it is possible to know whether the verification process succeeded.
                
                
  Arguments:    tgt: Pointer to the verify-target SRAM address (write destination data, address in the AGB memory map)
                src: Pointer to the verify-source work region address (the original data)
                size: Verify size in bytes
                callback: The callback function called when the verification process completes (only for asynchronous functions)

  Returns:      0     : Normal end
                Non-zero: Returns the device-side error address for a verify error. (only for synchronous functions)
 *---------------------------------------------------------------------------*/
extern u32 CTRDG_VerifyAgbSram(u32 tgt, const void *src, u32 size);
extern void CTRDG_VerifyAgbSramAsync(u32 tgt, const void *src, u32 size, CTRDG_TASK_FUNC callback);


/*------------------------------------------------------------------*/
/*          Writing & Verifying the Data                              */
/*------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
  Name:         CTRDG_WriteAndVerifyAgbSram
                CTRDG_WriteAndVerifyAgbSramAsync
                
                Corresponding AGB function: extern u32 WriteSramEx(u8 *dst, u8 *src, u32 size)

  Description:  After performing writes internally with CTRDG_WriteAgbSram, performs verification with CTRDG_VerifyAgbSram; when errors occur, retries are made up to a maximum of CTRDG_AGB_SRAM_RETRY_MAX (defined in ctrdg_sram.h) times.
                
                
                In addition, in the asynchronous versions, by referring to 'result', a member of the CTRDGTaskInfo structure that is an argument of the callback function returned after calling this routine, it is possible to know whether the WriteAndVerify process succeeded.
                
                
  Arguments:    dst: Write destination SRAM address (address in the AGB memory map)
                src: Write source work region address
                size: Write/verify size in bytes
                callback: Callback function called when the WriteAndVerify process completes (only for asynchronous functions)

  Returns:      0     : Normal end
                Non-zero: Returns the device-side error address for a verify error. (only for synchronous functions)
 *---------------------------------------------------------------------------*/
extern u32 CTRDG_WriteAndVerifyAgbSram(u32 dst, const void *src, u32 size);
extern void CTRDG_WriteAndVerifyAgbSramAsync(u32 dst, const void *src, u32 size,
                                             CTRDG_TASK_FUNC callback);


#ifdef __cplusplus
}      /* extern "C" */
#endif

#endif /* NITRO_CTRDG_SRAM_H_ */
