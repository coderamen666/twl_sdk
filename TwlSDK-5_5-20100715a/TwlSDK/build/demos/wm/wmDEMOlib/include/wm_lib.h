/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WM - demos - wmDEMOlib
  File:     wm_lib.h

  Copyright 2006-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef WMDEMOLIB_WM_LIB_H_
#define WMDEMOLIB_WM_LIB_H_

#ifdef  __cplusplus
extern "C" {
#endif

/*===========================================================================*/

/* wm_lib.h */
#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif

#include <nitro/wm/common/wm.h>

#define WM_MODE_MP_PARENT       1
#define WM_MODE_MP_CHILD        2
#define WM_MODE_INFRASTRUCTURE  3

#define WM_DMA_NO 2

#define WM_PORT_KEYSHARING      15

#define WM_TYPE_CHILD_CONNECTED             0
#define WM_TYPE_CHILD_DISCONNECTED          1
#define WM_TYPE_MP_PARENT_SENT              2
#define WM_TYPE_MP_PARENT_RECV              3
#define WM_TYPE_PARENT_FOUND                4
#define WM_TYPE_PARENT_NOT_FOUND            5
#define WM_TYPE_CONNECTED_TO_PARENT         6
#define WM_TYPE_DISCONNECTED                7
#define WM_TYPE_MP_CHILD_SENT               8
#define WM_TYPE_MP_CHILD_RECV               9
#define WM_TYPE_DISCONNECTED_FROM_PARENT    10
#define WM_TYPE_CONNECT_FAILED              11
#define WM_TYPE_DCF_CHILD_SENT              12
#define WM_TYPE_DCF_CHILD_SENT_ERR          13
#define WM_TYPE_DCF_CHILD_RECV              14
#define WM_TYPE_DISCONNECT_COMPLETE         15
#define WM_TYPE_DISCONNECT_FAILED           16
#define WM_TYPE_END_COMPLETE                17
#define WM_TYPE_MP_CHILD_SENT_ERR           18
#define WM_TYPE_MP_PARENT_SENT_ERR          19
#define WM_TYPE_MP_STARTED                  20
#define WM_TYPE_INIT_COMPLETE               21
#define WM_TYPE_END_MP_COMPLETE             22
#define WM_TYPE_SET_GAMEINFO_COMPLETE       23
#define WM_TYPE_SET_GAMEINFO_FAILED         24
#define WM_TYPE_MP_SEND_ENABLE              25
#define WM_TYPE_PARENT_STARTED              26
#define WM_TYPE_BEACON_LOST                 27
#define WM_TYPE_BEACON_SENT                 28
#define WM_TYPE_BEACON_RECV                 29
#define WM_TYPE_MP_SEND_DISABLE             30
#define WM_TYPE_DISASSOCIATE                31
#define WM_TYPE_REASSOCIATE                 32
#define WM_TYPE_AUTHENTICATE                33
#define WM_TYPE_SET_LIFETIME                34
#define WM_TYPE_DCF_STARTED                 35
#define WM_TYPE_DCF_SENT                    36
#define WM_TYPE_DCF_SENT_ERR                37
#define WM_TYPE_DCF_RECV                    38
#define WM_TYPE_DCF_END                     39
#define WM_TYPE_MPACK_IND                   40
#define WM_TYPE_MP_CHILD_SENT_TIMEOUT       41
#define WM_TYPE_SEND_QUEUE_FULL_ERR         42

#define WM_TYPE_API_ERROR                   255 // Error on the value returned from an API call
#define WM_TYPE_ERROR                       256 // Error on callback


#define WL_HEADER_LENGTH_P  48         // WL header length when the parent receives
#define WM_HEADER_LENGTH    (2+4)      // WM Header length
#define WL_HEADER_LENGTH_T  8          // Total WL header length
#define WL_HEADER_LENGTH_C  8          // WL header length of each child's data

// Macro; computes the size of the child receive buffer
#define WM_CalcChildSendBufSize(_pInfo_)        (u16)(WM_SIZE_MP_CHILD_SEND_BUFFER(((WMgameInfo *)(&(((WMbssDesc*)(_pInfo_))->gameInfo)))->childMaxSize, TRUE))
#define WM_CalcChildRecvBufSize(_pInfo_)        (u16)(WM_SIZE_MP_CHILD_RECEIVE_BUFFER(((WMgameInfo *)(&(((WMbssDesc*)(_pInfo_))->gameInfo)))->parentMaxSize, TRUE))

// Macro; computes the size of the parent send and receive buffers
#define WM_CalcParentSendBufSize(_libParam_)    (u16)(WM_SIZE_MP_PARENT_SEND_BUFFER(((WM_lib_param*)(&(_libParam_)))->parentParam->parentMaxSize, TRUE))
#define WM_CalcParentRecvBufSize(_libParam_)    (u16)(WM_SIZE_MP_PARENT_RECEIVE_BUFFER(((WM_lib_param*)(&(_libParam_)))->parentParam->childMaxSize, WM_NUM_MAX_CHILD, TRUE))


typedef void (*WMcallbackFunc2) (u16 type, void *arg);  // WM callback type (Part 2)

// wm_lib parameter structure
typedef struct
{
    void   *wmBuf;                     // WM system buffer
    WMpparam *parentParam;             // Parent information settings (used by the parent)
    WMbssDesc *parentInfoBuf;          // Buffer for getting parent information (used by the child)
    u16    *sendBuf;                   // Send buffer
    u16    *recvBuf;                   // MP receive buffer
    WMdcfRecvBuf *dcfBuf;              // DCF receive buffer
    WMstatus *statusBuf;               // Buffer for getting status (this should be deleted, if possible)
    WMcallbackFunc2 callback;          // Callback for WM_lib
    WMbssDesc *pInfo;                  // Pointer to information for the connected parent (used by the child)
    u16     sendBufSize;               // Size of send buffer
    u16     recvBufSize;               // Size of MP Receive buffer
    u16     dcfBufSize;                // DCF receive buffer size
    u16     mode;                      // WM_MODE_***
    u16     endReq;
    u16     mpStarted;                 // Flag indicating that MP has started
    u16     mpBusy;                    // MP transmit request flag
    u16     child_bitmap;              // Child connection state
    u16     parentInfoBufSize;         // Size of the buffer that gets the parent information (used by StartScanEx)

    // KeySharing function
    u16     keySharing;                // KeySharing execution flag (1: Run KS, 0: Do not run KS)
    WMkeySetBuf *ksBuf;                // KeySharing buffer

    // Related to continuous sends
    u16     contSend;                  // Flag indicating execution of continuous transmission (1: Continuous transmission, 0: Per-frame communication)

    // wep related
    u16     wepMode;                   // WEPmode used for connection
    u16     wepKeyId;                  // ID of WEPKey used for connection
    u8      wepKey[80];                // WEPkey used for connection

    // Connection authentication related
    u16     authMode;                  // WM_AUTHMODE_OPEN_SYSTEM or WM_AUTHMODE_SHARED_KEY

    // Related to gameinfo
    u8     *uname;                     // UserName
    u8     *gname;                     // GameName
    u16     currentTgid;               // TGID of the connected parent (checked with BeaconRecv.Ind)

    // user gameinfo related
    u16     userGameInfoLength;        // Length of user area
    u16    *userGameInfo;              // Pointer to user area buffer

    // MP resend related
    BOOL    no_retry;                  // Resend processing execution flag (FALSE: Resend, TRUE: No resend)

    // Related to ScanEx
    BOOL    scanExFlag;                // Set to TRUE when using ScanEx

    u32     ggidFilter;                // GGID filtering (everything passes through when 0xffffffff)

//    u16             rsv1;               // for 4byte padding
}
WM_lib_param;

//-----------------------------------
// WM_lib_Init()
int     WM_lib_Init(WM_lib_param * para);       // WM_lib initialization

//-----------------------------------
// WM_lib_SetMPData()
int     WM_lib_SetMPData(u16 *data, u16 size);  // MP data transmission

//-----------------------------------
// WM_lib_SetMPDataEX()
int     WM_lib_SetMPDataEX(u16 *data, u16 size, u16 pollbitmap);
                                                // Send MP data (support for pollbitmap)

//-----------------------------------
// WM_lib_End()
int     WM_lib_End(void);              // Request termination of parent mode

//-----------------------------------
// WM_lib_ConnectToParent()
int     WM_lib_ConnectToParent(WMbssDesc *pInfo);       // Request connection to parent

//-----------------------------------
// WM_lib_SetDCFData()
int     WM_lib_SetDCFData(const u8 *destAdr, u16 *data, u16 size);      // DCF data transmission

//-----------------------------------
// WM_lib_SetGameInfo()
int     WM_lib_SetGameInfo(u16 *userGameInfo, u16 size, u32 ggid, u16 tgid);

//-----------------------------------
// WM_lib_Disconnect()
int     WM_lib_Disconnect(u16 aid);    // Disconnect child (aid) from parent


//-----------------------------------
// WM_lib_CheckMPSend()
BOOL    WM_lib_CheckMPSend(void);      // Check MP transmit permission


//-----------------------------------
// WM_lib_GetKeySet()
int     WM_lib_GetKeySet(WMkeySet *keySet);

//-----------------------------------
// WM_lib_set_auto()
void    WM_lib_set_auto(BOOL flag);    // Automatically register both parent and child

//-----------------------------------
// WM_lib_set_bssid()
void    WM_lib_set_bssid(u16 bssid0, u16 bssid1, u16 bssid2);   // BssId setting

//-----------------------------------
// WM_lib_CurrentScanChannel()

void    WM_lib_set_mp_dcf_parallel(BOOL flag);
void    WM_lib_set_no_retry_flag(BOOL flag);

//-----------------------------------
// Functions for setting and getting parameters for Scan
void    WM_lib_set_max_scan_time(u16 time);     // Set continuous Scan time
u16     WM_lib_get_max_scan_time(void); // Get continuous Scan time
void    WM_lib_set_scan_channel_seek(BOOL flag);        // Channel seek configuration
void    WM_lib_set_channel(u16 channel);        // Channel configuration
u16     WM_lib_CurrentScanChannel(void);        // Return channel currently being scanning
void    WM_lib_set_scanBssid(u8 *bssid);        // BSSID filtering

//-----------------------------------
// Functions for setting and getting parameters for ScanEx
void    WM_lib_set_scanEx_maxChannelTime(u16 time);     // Configure maxChannelTime
u16     WM_lib_get_scanEx_maxChannelTime(void); // Get maxChannelTime
void    WM_lib_set_scanEx_channelList(u16 channel);     // Configure channelList(bitmap)
u16     WM_lib_get_scanEx_channelList(void);    // Get channelList(bitmap)
void    WM_lib_set_scanEx_scanType(u16 scanType);       // ScanType configuration WM_SCANTYPE_ACTIVE(0), WM_SCANTYPE_PASSIVE(1)
u16     WM_lib_get_scanEx_scanType(void);       // Get ScanType WM_SCANTYPE_ACTIVE(0), WM_SCANTYPE_PASSIVE(1)
void    WM_lib_set_scanEx_ssidLength(u16 ssidLength);   // Configure SSID length for SSID filtering
void    WM_lib_set_scanEx_ssid(u8 *ssid);       // SSID configuration for SSID filtering
void    WM_lib_set_scanEx_bssid(u8 *bssid);     // BSSID configuration for BSSID filtering

//-----------------------------------
// WM_lib_CalcRSSI()
u16     WM_lib_CalcRSSI(u16 rate_rssi, u16 aid);        // Convert rate_rssi to RSSI value


/************ Start where wm_tool.h was **********************/


#define WM_NUM_PARENT_INFORMATIONS 16
#define WM_NUM_CHILD_INFORMATIONS 15


typedef struct
{
    u16     found_parent_count;        // Number of parents found
    u16     my_aid;                    // AID assigned to self
    WMmpRecvBuf *recvBuf;              // MP buffer passed with WM_StartMP()
    u16    *sendBuf;                   // MP send data buffer passed with WM_SetMPData()
    u8      MacAdrList[WM_NUM_PARENT_INFORMATIONS][6];  // For saving Parent MacAddresses (for 4 machines)
    WMbssDesc *parentInfo[WM_NUM_PARENT_INFORMATIONS];  // BssDesc for storing Parent information (for 4 machines) and used during connection
    u16     channelList[WM_NUM_PARENT_INFORMATIONS];    // For saving Parent channels (for 4 machines)
    u16     ssidLength[WM_NUM_PARENT_INFORMATIONS];     // Parent SSID lengths
    u8      ssidList[WM_NUM_PARENT_INFORMATIONS][32];   // For saving Parent SSIDs (for 4 machines)
    u16     gameInfoLength[WM_NUM_PARENT_INFORMATIONS]; // Length of parent GameInfo

    u16     find_counter[WM_NUM_PARENT_INFORMATIONS];   // Number of times a parent was found

    u16     rsv[10];
    WMgameInfo gameInfoList[WM_NUM_PARENT_INFORMATIONS] ATTRIBUTE_ALIGN(32);    // For saving parent GameInfo (for 4 machines)
}
childInfo;


typedef struct
{
    u8      addr[6];
    u8      reserved[2];               // For 4 byte padding
}
MACADDRESS;

typedef struct
{
    int     valid;
    MACADDRESS macaddr;
}
CHILD_INFO;

typedef void (*WM_LIB_CALLBACK) (u16 type, void *arg);


int     wm_lib_get_last_found_parent_no(void);
void    save_parentInfo(WMstartScanCallback *buf, WM_lib_param * param, childInfo * info);
void    save_parentInfoEx(WMstartScanExCallback *buf, WM_lib_param * param, childInfo * info);


BOOL    wm_lib_get_own_macaddress(MACADDRESS * ma);





/* Initialization functions */



/* Functions planned to be deleted */
void    wm_lib_add_child_list(WMstartParentCallback *arg);
void    wm_lib_delete_child_list(WMstartParentCallback *arg);
void    wm_lib_add_parent_list(WMstartScanCallback *arg);
void    wm_lib_add_parent_listEx(WMStartScanExCallback *arg);
void    wm_lib_delete_parent_list(WMstartScanCallback *arg);
void    wm_lib_parent_found_count_reset(void);
void    wm_lib_set_my_aid(int aid);

/* Functions for both the parent and child devices */
void    wm_lib_comm_init(void);
int     wm_lib_start(void);
const char *wm_lib_get_wlib_version(void);
void    wm_lib_set_mode(u16 mode);
void    wm_lib_set_recvbuffer(u8 *buf);
void    wm_lib_set_sendbuffer(u8 *buf);
void    wm_lib_set_callback(WM_LIB_CALLBACK callback);
int     wm_lib_get_recvbuffer_size(void);
int     wm_lib_get_sendbuffer_size(void);
void    wm_lib_set_keySharing_mode(int flag);
void    wm_lib_set_contSend_mode(int flag);
void    wm_lib_set_multiboot_mode(int flag);
void    wm_lib_set_ggid(u32 ggid);
u32     wm_lib_get_ggid(void);
void    wm_lib_set_tgid(u16 tgid);
u16     wm_lib_get_tgid(void);
void    wm_lib_set_scan_bssid(u16 bssid0, u16 bssid1, u16 bssid2);


/* Parent Functions */
void    wm_lib_set_gameinfo_gamename(char *str);
void    wm_lib_set_gameinfo_username(char *user_name);
u32     wm_lib_get_parent_gameinfo_ggid(int id);
u16     wm_lib_get_parent_gameinfo_tgid(int id);
u16    *wm_lib_get_parent_gameinfo_usergameinfo(int id);
u16     wm_lib_get_parent_gameinfo_usergameinfosize(int id);

void    wm_lib_set_parent_send_size(u16 size);
void    wm_lib_set_parent_channel(u16 ch);
void    wm_lib_set_beacon_period(u16 period_ms);


void    wm_lib_set_parent_recv_size_per_child(u16 size);
BOOL    wm_lib_get_child_macaddress(int aid, MACADDRESS * ma);
BOOL    wm_lib_is_child_valid(int aid);
int     wm_lib_set_gameinfo(void);

/* Child Functions */

BOOL    wm_lib_is_parent_gameinfo_valid(int id);
int     wm_lib_get_parent_gameinfo_channel(int id);
int     wm_lib_get_parent_gameinfo_parent_sendmaxsize(int id);
int     wm_lib_get_parent_gameinfo_child_sendbufsize(int id);
int     wm_lib_get_parent_gameinfo_child_recvbufsize(int id);
const char *wm_lib_get_parent_gameinfo_username(int id);
const char *wm_lib_get_parent_gameinfo_gamename(int id);


void   *WM_lib_get_mp_parent_callback_ptr(void);
void   *WM_lib_get_mp_child_callback_ptr(void);


int     wm_lib_get_parent_found_count(void);
int     wm_lib_connect_parent(int parent_no);
BOOL    wm_lib_get_parent_macaddress(int id, MACADDRESS * ma);
int     wm_lib_get_my_aid(void);

/*  Get Keysharing Keyset   */
int     wm_lib_get_keyset(WMkeySet *keyset);
int     wm_lib_connect_parent_via_bssdesc(WMbssDesc *bssDesc);

void    wm_lib_set_ggidFilter(u32 ggidFilter);

/*
    Get pointer to WMbssDesc structure
*/
WMbssDesc *wm_get_parent_bssdesc(int id, WMbssDesc *bssDescp);

int     wm_lib_connect_parent_via_bssdesc(WMbssDesc *bssDescp);

/************ End where wm_tool.h was **********************/

/****************************************************/


/*===========================================================================*/

#ifdef  __cplusplus
}       /* extern "C" */
#endif

#endif /* WMDEMOLIB_WM_LIB_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
