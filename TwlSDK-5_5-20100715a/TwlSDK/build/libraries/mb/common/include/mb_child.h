/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - include
  File:     mb_child.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
/* ==========================================================================

    This is a header for the MB library's child device.
    When using this library in a multiboot child device as well as the IPL, additionally include this header in nitro/mb.h.
    

   ==========================================================================*/


#ifndef _MB_CHILD_H_
#define _MB_CHILD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/types.h>
#include <nitro/memorymap.h>
#include <nitro/mb.h>
#include "mb_fileinfo.h"
#include "mb_gameinfo.h"

/* ---------------------------------------------------------------------

        define constants

   ---------------------------------------------------------------------*/

#define MB_CHILD_SYSTEM_BUF_SIZE        (0x6000)

#define MB_MAX_SEND_BUFFER_SIZE         (0x400)
#define MB_MAX_RECV_BUFFER_SIZE         (0x80)

/* ----------------------------------------------------------------------------

    Multiboot RAM address definition (used internally)

   ----------------------------------------------------------------------------*/

/* 
    Possible placement range of a multiboot program.

    The maximum initial load size of the ARM9 code in wireless multiboot is 2.5 MB.
    
    The loadable region for ARM9 code is between MB_LOAD_AREA_LO and MB_LOAD_AREA_HI.
    
    
 */
#define MB_LOAD_AREA_LO                         ( HW_MAIN_MEM )
#define MB_LOAD_AREA_HI                         ( HW_MAIN_MEM + 0x002c0000 )
#define MB_LOAD_MAX_SIZE                        ( MB_LOAD_AREA_HI - MB_LOAD_AREA_LO )


/*
    Address definitions for the ARM7 static child-side temporary receive buffer during multi-boot.
    
    The loadable range of ARM7 code is any of the following.

    a) 0x02000000 - 0x02300000
       (MB_LOAD_AREA_LO - MB_ARM7_STATIC_RECV_BUFFER_END)
    b) 0x02300000 - 0x023fe000
       (MB_ARM7_STATIC_RECV_BUFFER_END - 0x023fe000)
    c) 0x037f8000 - 0x0380f000.
       (internal WRAM)

    
    
    * NOTE
    When loading ARM7 code to internal WRAM or after 0x02300000,

    0x022c0000 - 0x02300000
    (MB_LOAD_AREA_HI - MB_ARM7_STATIC_RECV_BUFFER_END)
    The buffer is configured as shown above, receives data, and then is relocated to the address specified at boot time.
    
    
    * ARM7 code whose position straddles the 0x02300000 address.
    For addresses after 0x02300000, ARM7 code larger than 0x40000 (MB_ARM7_STATIC_RECV_BUFFER_SIZE) cannot be guaranteed to run normally, so it is prohibited to create code like this.
      
      
 */

#define MB_ARM7_STATIC_RECV_BUFFER              MB_LOAD_AREA_HI
#define MB_ARM7_STATIC_RECV_BUFFER_END          ( HW_MAIN_MEM + 0x00300000 )
#define MB_ARM7_STATIC_RECV_BUFFER_SIZE         ( MB_ARM7_STATIC_RECV_BUFFER_END - MB_LOAD_AREA_HI )
#define MB_ARM7_STATIC_LOAD_AREA_HI             ( MB_LOAD_AREA_HI + MB_ARM7_STATIC_RECV_BUFFER_SIZE )
#define MB_ARM7_STATIC_LOAD_WRAM_MAX_SIZE       ( 0x18000 - 0x1000 )    // 0x18000 = HW_WRAM_SIZE + HW_PRV_WRAM_SIZE

/* Temporary storage location of BssDesc */
#define MB_BSSDESC_ADDRESS                      ( HW_MAIN_MEM + 0x003fe800 )
/* Temporary storage location of MBDownloadFileInfo */
#define MB_DOWNLOAD_FILEINFO_ADDRESS            ( MB_BSSDESC_ADDRESS + ( ( ( MB_BSSDESC_SIZE ) + (32) - 1 ) & ~((32) - 1) ) )
/* Place for saving the ROM header of an inserted ROM card at multiboot startup time */
#define MB_CARD_ROM_HEADER_ADDRESS              ( MB_DOWNLOAD_FILEINFO_ADDRESS + ( ( ( MB_DOWNLOAD_FILEINFO_SIZE ) + (32) - 1 ) & ~((32) - 1) ) )
/* Place for storing the ROM Header */
#define MB_ROM_HEADER_ADDRESS                   (HW_MAIN_MEM + 0x007ffe00)


/* ---------------------------------------------------------------------

        The API for the multiboot library (MB) child device. Used by IPL.

   ---------------------------------------------------------------------*/

/* Set child parameters and start */
int     MB_StartChild(void);

/* Get the size of the work memory used by the MB child */
int     MB_GetChildSystemBufSize(void);

/* Set the notification callback for the child state */
void    MB_CommSetChildStateCallback(MBCommCStateCallbackFunc callback);

/* Get the state of the child */
int     MB_CommGetChildState(void);

/* Start download */
BOOL    MB_CommStartDownload(void);

/* Get the status of the child's download progress as a percentage from 0 to 100 */
u16     MB_GetChildProgressPercentage(void);

/* Try connecting to the parent and request a download file */
int     MB_CommDownloadRequest(int index);      // Designate the parent device for which to request a connection with the list number of MbBeaconRecvStatus

#ifdef __cplusplus
}
#endif

#endif /*  _MB_CHILD_H_    */
