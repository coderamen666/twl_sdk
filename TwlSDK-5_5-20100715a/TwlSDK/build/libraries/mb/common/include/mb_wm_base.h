/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - include
  File:     mb_wm_base.h

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

#ifndef MB_WM_BASE_H_
#define MB_WM_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro.h>
#include <nitro/wm.h>

/* Maximum number of connections */
#define MB_NUM_PARENT_INFORMATIONS              16

#define MB_MAX_SEND_BUFFER_SIZE         (0x400)

/*
 * Multiboot parent/child selection
 * (Cannot be run for a non-IPL child)
 */
#define MB_MODE_PARENT                          1
#define MB_MODE_CHILD                           2

/*
 * Definitions specific to wm_lib
 */

#define MB_CALLBACK_CHILD_CONNECTED             0
#define MB_CALLBACK_CHILD_DISCONNECTED          1
#define MB_CALLBACK_MP_PARENT_SENT              2
#define MB_CALLBACK_MP_PARENT_RECV              3
#define MB_CALLBACK_PARENT_FOUND                4
#define MB_CALLBACK_PARENT_NOT_FOUND            5
#define MB_CALLBACK_CONNECTED_TO_PARENT         6
#define MB_CALLBACK_DISCONNECTED                7
#define MB_CALLBACK_MP_CHILD_SENT               8
#define MB_CALLBACK_MP_CHILD_RECV               9
#define MB_CALLBACK_DISCONNECTED_FROM_PARENT    10
#define MB_CALLBACK_CONNECT_FAILED              11
#define MB_CALLBACK_DCF_CHILD_SENT              12
#define MB_CALLBACK_DCF_CHILD_SENT_ERR          13
#define MB_CALLBACK_DCF_CHILD_RECV              14
#define MB_CALLBACK_DISCONNECT_COMPLETE         15
#define MB_CALLBACK_DISCONNECT_FAILED           16
#define MB_CALLBACK_END_COMPLETE                17
#define MB_CALLBACK_MP_CHILD_SENT_ERR           18
#define MB_CALLBACK_MP_PARENT_SENT_ERR          19
#define MB_CALLBACK_MP_STARTED                  20
#define MB_CALLBACK_INIT_COMPLETE               21
#define MB_CALLBACK_END_MP_COMPLETE             22
#define MB_CALLBACK_SET_GAMEINFO_COMPLETE       23
#define MB_CALLBACK_SET_GAMEINFO_FAILED         24
#define MB_CALLBACK_MP_SEND_ENABLE              25
#define MB_CALLBACK_PARENT_STARTED              26
#define MB_CALLBACK_BEACON_LOST                 27
#define MB_CALLBACK_BEACON_SENT                 28
#define MB_CALLBACK_BEACON_RECV                 29
#define MB_CALLBACK_MP_SEND_DISABLE             30
#define MB_CALLBACK_DISASSOCIATE                31
#define MB_CALLBACK_REASSOCIATE                 32
#define MB_CALLBACK_AUTHENTICATE                33
#define MB_CALLBACK_SET_LIFETIME                34
#define MB_CALLBACK_DCF_STARTED                 35
#define MB_CALLBACK_DCF_SENT                    36
#define MB_CALLBACK_DCF_SENT_ERR                37
#define MB_CALLBACK_DCF_RECV                    38
#define MB_CALLBACK_DCF_END                     39
#define MB_CALLBACK_MPACK_IND                   40
#define MB_CALLBACK_MP_CHILD_SENT_TIMEOUT       41
#define MB_CALLBACK_SEND_QUEUE_FULL_ERR         42

#define MB_CALLBACK_API_ERROR                   255     // Error on the value returned from an API call
#define MB_CALLBACK_ERROR                       256


/*
 * Individual parent information received by a beacon (managed internally by the child)
 */
typedef struct ParentInfo
{
    union
    {
        WMBssDesc parentInfo[1];
        u8      parentInfo_area[WM_BSS_DESC_SIZE] ATTRIBUTE_ALIGN(32);
    };
    /* Except for mac and GameInfo, probably not used at all */
    WMStartScanCallback scan_data;
    u8      reserved1[8];
}
ParentInfo;


/* Format of functions used for MB callbacks */
typedef void (*MBCallbackFunc) (u16 type, void *arg);


/*
 * wm_lib parameter structure
 * The part that bridges between wm_lib and wm_tool was temporarily forced to link, so some parts do not conform to ANSI-STRICT with respect to using unions
 * 
 * (This may simply be replaced later so it has been postponed)
 */
typedef struct
{
    /* Parent information settings (used by the parent) */
    union
    {
        WMParentParam parentParam;
        u8      parentParam_area[WM_PARENT_PARAM_SIZE] ATTRIBUTE_ALIGN(32);
    };

    /* WM internal buffer passed to StartMP (must not be used with SetMP) */
    u16     sendBuf[MB_MAX_SEND_BUFFER_SIZE / sizeof(u16)] ATTRIBUTE_ALIGN(32);

    /* Buffer for getting parent information (used by the child) */
    union
    {
        WMBssDesc parentInfoBuf;
        u8      parentInfoBuf_area[WM_BSS_DESC_SIZE] ATTRIBUTE_ALIGN(32);
    };

    u16     p_sendlen;
    u16     p_recvlen;

    WMMpRecvBuf *recvBuf;              /* Receive buffer */

    /* Parent/child callback */
    void    (*callback_ptr) (void *arg);

    u8      mpBusy;                    /* MP sending (busy) flag */
    u8      mbIsStarted;
    u8      reserved0[10];

    u16     sendBufSize;               // Size of send buffer
    u16     recvBufSize;               // Size of MP receive buffer

    MBCallbackFunc callback;           // Callback for WM_lib
    const WMBssDesc *pInfo;            // Pointer to information for the connected parent (used by the child)
    u16     mode;                      // MB_MODE_***
    u16     endReq;

    u16     mpStarted;                 // Flag indicating that MP has started
    u16     child_bitmap;              // Child connection state

    /* Related to continuous sends */
    u16     contSend;                  // Flag indicating execution of continuous transmission (1: Continuous transmission, 0: Per-frame communication)
    u8      reserved1[2];

    // Related to gameinfo
    u8      uname[WM_SIZE_USERNAME] ATTRIBUTE_ALIGN(4);
    u8      gname[WM_SIZE_GAMENAME] ATTRIBUTE_ALIGN(4);
    u16     currentTgid;               // TGID of the connected parent (checked with BeaconRecv.Ind)
    u8      reserved2[22];

    u16     userGameInfo[((WM_SIZE_USER_GAMEINFO + 32) & ~(32 - 1)) /
                         sizeof(u16)] ATTRIBUTE_ALIGN(32);

    /* Data unique to the child */
    struct
    {
        /*
         * The number of discovered parents.
         * It initially has a value of 0, is incremented when StartScan succeeds, and is set to 0 when StartConnect fails.
         * No one is currently looking at this, but it would probably be convenient for the user if it was included in BeconRecvState.
         * 
         */
        u16     found_parent_count;
        /*
         * AID assigned to self.
         * It initially has a value of 0 and is set to n when StartConnect succeeds.
         * No one is currently looking at this.
         */
        u16     my_aid;

        BOOL    scanning_flag;
        BOOL    scan_channel_flag;
        int     last_found_parent_no;

        u8      reserved8[16];

        /* Information array for 16 parent units */
        ParentInfo parent_info[MB_NUM_PARENT_INFORMATIONS];
    };

}
MBiParam;


// ===============================================================================
// Functions

/* Get the last parent to be found */
int     MBi_GetLastFountParent(void);

/* Get the parent's BSS */
WMBssDesc *MBi_GetParentBssDesc(int parent);

/* Set the maximum scan time */
void    MBi_SetMaxScanTime(u16 time);

int     MBi_SendMP(const void *buf, int len, int pollbmp);

int     MBi_GetSendBufSize(void);

int     MBi_GetRecvBufSize(void);

int     MBi_CommConnectToParent(const WMBssDesc *bssDescp);

u32     MBi_GetGgid(void);

u16     MBi_GetTgid(void);

u8      MBi_GetAttribute(void);

int     MBi_RestartScan(void);

int     MBi_GetScanChannel(void);

u16     MBi_GetAid(void);

BOOL    MBi_IsStarted(void);

int     MBi_CommEnd(void);

void    MBi_CommParentCallback(u16 type, void *arg);
void    MBi_CommChildCallback(u16 type, void *arg);

#ifdef __cplusplus
}
#endif

#endif /*  MB_WM_BASE_H_    */
