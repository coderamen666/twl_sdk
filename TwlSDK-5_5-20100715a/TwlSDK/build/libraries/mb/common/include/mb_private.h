/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - include
  File:     mb_private.h

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

/*
    Only this header file is included in each source file of the MB library
*/

#ifndef _MB_PRIVATE_H_
#define _MB_PRIVATE_H_

#ifdef __cplusplus
extern "C" {
#endif

#if	!defined(NITRO_FINALROM)
/*
#define PRINT_DEBUG
*/
#endif

#define CALLBACK_WM_STATE       0

#include <nitro/types.h>
#include <nitro/wm.h>
#include <nitro/mb.h>
#include "mb_wm_base.h"
#include "mb_common.h"
#include "mb_block.h"
#include "mb_rom_header.h"
#include "mb_gameinfo.h"
#include "mb_fileinfo.h"
#include "mb_child.h"

/* New framework for the caching method */
#include "mb_cache.h"
#include "mb_task.h"

/* Debugging output */
#ifdef  PRINT_DEBUG
#define MB_OUTPUT( ... )        OS_TPrintf( __VA_ARGS__ )
#define MB_DEBUG_OUTPUT( ... )  MBi_DebugPrint(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__ )

#else
#define MB_OUTPUT( ... )        ((void)0)
#define MB_DEBUG_OUTPUT( ... )  ((void)0)
#endif

// ----------------------------------------------------------------------------
// Definitions

/* ----------------------------------------------------------------------------

    Definitions related to block transfers

   ----------------------------------------------------------------------------*/

#define MB_IPL_VERSION                          (0x0001)
#define MB_MAX_BLOCK                            12288   // Max 12288 * 256 = 3.0 MB

#define MB_NUM_PARENT_INFORMATIONS              16      /* Maximum number of parents */

#define MB_SCAN_TIME_NORMAL                     WM_DEFAULT_SCAN_PERIOD
#define MB_SCAN_TIME_NORMAL_FOR_FAKE_CHILD      20
#define MB_SCAN_TIME_LOCKING                    220

//SDK_COMPILER_ASSERT( sizeof(MBDownloadFileInfo) <= MB_COMM_BLOCK_SIZE );


/* ----------------------------------------------------------------------------

    Definitions of internally used types

   ----------------------------------------------------------------------------*/

/*
 * The work region used by the MB library
 */

typedef struct MB_CommCommonWork
{
    //  --- Common    ---
    //  Send/receive buffer
    u32     sendbuf[MB_MAX_SEND_BUFFER_SIZE / sizeof(u32)] ATTRIBUTE_ALIGN(32);
    u32     recvbuf[MB_MAX_RECV_BUFFER_SIZE * MB_MAX_CHILD * 2 / sizeof(u32)] ATTRIBUTE_ALIGN(32);
    MBUserInfo user;                   // User information for this device
    u16     isMbInitialized;
    int     block_size_max;
    BOOL    start_mp_busy;             /* WM_StartMP() is operating */
    BOOL    is_started_ex;             /* WM_StartParentEX() called */
    u8      padding[28];
}
MB_CommCommonWork;

typedef struct MB_CommPWork
{
    MB_CommCommonWork common;

    //  --- For the parent device  ---
    MBUserInfo childUser[MB_MAX_CHILD] ATTRIBUTE_ALIGN(4);
    u16     childversion[MB_MAX_CHILD]; // Child device version information (in quantity equal to the number of child devices)
    u32     childggid[MB_MAX_CHILD];   // Child device GGID (in quantity equal to the number of child devices)
    MBCommPStateCallback parent_callback;       // Parent state callback function pointer
    int     p_comm_state[MB_MAX_CHILD]; // Holds the state of each of the children
    u8      file_num;                  // Number of registered files
    u8      cur_fileid;                // File number of the current file to send
    s8      fileid_of_child[MB_MAX_CHILD];      // File number that a child requested (-1 when there is no request)
    u8      child_num;                 // Number of entry children
    u16     child_entry_bmp;           // Child entry control bitmap
    MbRequestPieceBuf req_data_buf;    // Buffer for receiving child partition requests
    u16     req2child[MB_MAX_CHILD];   // The request to send to the child
    MBUserInfo childUserBuf;           // Child information buffer for passing to the application

    //  File information
    struct
    {
        // The DownloadFileInfo buffer
        MBDownloadFileInfo dl_fileinfo;
        MBGameInfo game_info;
        MB_BlockInfoTable blockinfo_table;
        MBGameRegistry *game_reg;
        void   *src_adr;               //  The starting address of the boot image on the parent device
        u16     currentb;              // The current block
        u16     nextb;                 // The next block to send
        u16     pollbmp;               // PollBitmap for sending data
        u16     gameinfo_child_bmp;    // The current bitmap member for updating the GameInfo
        u16     gameinfo_changed_bmp;  // The bitmap member that was generated by a change for updating the GameInfo
        u8      active;
        u8      update;

        /*
         * Added to support cache reads
         * These specify content in src_adr
         */
        MBiCacheList *cache_list;      /* Cache list */
        u32    *card_mapping;          /* CARD address for the start of each segment */

    }
    fileinfo[MB_MAX_FILE];

    BOOL    useWvrFlag;                // Flag indicating whether or not WVR is being used
    u8      padding2[20];

    /* Added for task thread */
    u8      task_work[2 * 1024];
    MBiTaskInfo cur_task;

}
MB_CommPWork;


typedef struct MB_CommCWork
{
    MB_CommCommonWork common;

    //  --- For child devices ---
    WMBssDesc bssDescbuf ATTRIBUTE_ALIGN(32);   // WMBssDesc backup
    MBDownloadFileInfo dl_fileinfo;    // The DownloadFileInfo buffer of the child device
    MBCommCStateCallbackFunc child_callback;    // The state callback function pointer of the child device
    int     c_comm_state;              // The state of the child device
    int     connectTargetNo;           // The MbBeaconRecvStatus list number of the connection target
    u8      fileid;                    // The requested file number
    u8      _padding1[1];
    u16     user_req;
    u16     got_block;                 // The number of downloaded blocks
    u16     total_block;               // The total number of blocks of the download file
    u8      recvflag[MB_MAX_BLOCK / 8]; // Flag that shows the block receive state
    MB_BlockInfoTable blockinfo_table;
    int     last_recv_seq_no;          // Sequence number of the block received previously
    u8      boot_end_flag;             // Flag for determining whether MB termination by BOOT_READY is being processed
    u8      _padding2[15];
}
MB_CommCWork;


/* ----------------------------------------------------------------------------

    Definitions of variables that are used internally

   ----------------------------------------------------------------------------*/

extern MB_CommCommonWork *mbc;
/* Pointer macro to the parent's working memory */
#define pPwork                                  ((MB_CommPWork*)mbc)
/* Pointer macro to the child's working memory */
#define pCwork                                  ((MB_CommCWork*)mbc)


#ifdef __cplusplus
}
#endif

#endif /*  _MB_PRIVATE_H_  */
