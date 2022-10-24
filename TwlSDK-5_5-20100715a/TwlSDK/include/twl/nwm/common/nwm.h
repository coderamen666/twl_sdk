/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WM - include
  File:     nwm.h

  Copyright 2007-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-24#$
  $Rev: 10812 $
  $Author: sato_masaki $
 *---------------------------------------------------------------------------*/

#ifndef LIBRARIES_NWM_ARM9_NWM_H__
#define LIBRARIES_NWM_ARM9_NWM_H__

#ifdef  __cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include <twl.h>
#include <nitro/wm.h>

/*---------------------------------------------------------------------------*
    Constant definitions
 *---------------------------------------------------------------------------*/

#define NWM_NUM_MAX_AP_AID              2007   // The maximum value possible for the AssociationID provided by the wireless router

#define NWM_SIZE_SSID                     32
#define NWM_SIZE_MACADDR                   6
#define NWM_SIZE_BSSID                    NWM_SIZE_MACADDR

#define NWM_SIZE_WEP_40BIT                 5
#define NWM_SIZE_WEP_104BIT               13
#define NWM_SIZE_WEP_128BIT               16

#define NWM_SIZE_WEP                      20   // For preserving WM (DS connection settings) compatibility

#define NWM_WEPMODE_OPEN                   WM_WEPMODE_NO
#define NWM_WEPMODE_40BIT                  WM_WEPMODE_40BIT
#define NWM_WEPMODE_104BIT                 WM_WEPMODE_104BIT
#define NWM_WEPMODE_128BIT                 WM_WEPMODE_128BIT

#define NWM_WPAMODE_WPA_TKIP               (WM_WEPMODE_128BIT + 1)
#define NWM_WPAMODE_WPA2_TKIP              (WM_WEPMODE_128BIT + 2)
#define NWM_WPAMODE_WPA_AES                (WM_WEPMODE_128BIT + 3)
#define NWM_WPAMODE_WPA2_AES               (WM_WEPMODE_128BIT + 4)


#define NWM_BSS_DESC_SIZE                 64   // Size of the buffer transferred with the NWM_StartScan function to store parent information
#define NWM_BSS_DESC_SIZE_MAX            512
#define NWM_FRAME_SIZE_MAX              1522   // Maximum data frame size: 1500(MTU) + 22(802.3 header)
#define NWM_SCAN_NODE_MAX                 32   // Maximum number of parent devices that can be found by a single Scan
#define NWM_SIZE_SCANBUF_MAX           65535   // Maximum buffer size

#define NWM_SYSTEM_BUF_SIZE           (0x00003000)

// Bitmask for capability information
#define NWM_CAPABILITY_ESS_MASK         0x0001
#define NWM_CAPABILITY_ADHOC_MASK       0x0002
#define NWM_CAPABILITY_CP_POLLABLE_MASK 0x0004
#define NWM_CAPABILITY_CP_POLL_REQ_MASK 0x0008
#define NWM_CAPABILITY_PRIVACY_MASK     0x0010
#define NWM_CAPABILITY_SPREAMBLE_MASK   0x0020
#define NWM_CAPABILITY_PBCC_MASK        0x0040
#define NWM_CAPABILITY_CH_AGILITY_MASK  0x0800
#define NWM_CAPABILITY_SPECTRUM_MASK    0x0100
#define NWM_CAPABILITY_QOS_MASK         0x0200
#define NWM_CAPABILITY_SSLOTTIME_MASK   0x0400
#define NWM_CAPABILITY_APSD_MASK        0x0800


#define NWM_SCANTYPE_PASSIVE    0
#define NWM_SCANTYPE_ACTIVE     1

#define NWM_DEFAULT_PASSIVE_SCAN_PERIOD 105 /* Note that the default NWM values are larger than the default WM values */
#define NWM_DEFAULT_ACTIVE_SCAN_PERIOD   30 /* Note that NWM uses different default values for active and passive scans */

#define NWM_RATESET_1_0M                0x0001
#define NWM_RATESET_2_0M                0x0002
#define NWM_RATESET_5_5M                0x0004
#define NWM_RATESET_6_0M                0x0008
#define NWM_RATESET_9_0M                0x0010
#define NWM_RATESET_11_0M               0x0020
#define NWM_RATESET_12_0M               0x0040
#define NWM_RATESET_18_0M               0x0080
#define NWM_RATESET_24_0M               0x0100
#define NWM_RATESET_36_0M               0x0200
#define NWM_RATESET_48_0M               0x0400
#define NWM_RATESET_54_0M               0x0800
#define NWM_RATESET_11B_MASK            ( NWM_RATESET_1_0M | NWM_RATESET_2_0M | NWM_RATESET_5_5M | NWM_RATESET_11_0M )
#define NWM_RATESET_11G_MASK            ( NWM_RATESET_1_0M | NWM_RATESET_2_0M | NWM_RATESET_5_5M | NWM_RATESET_11_0M     \
                                        | NWM_RATESET_6_0M | NWM_RATESET_9_0M | NWM_RATESET_12_0M | NWM_RATESET_18_0M    \
                                        | NWM_RATESET_24_0M | NWM_RATESET_36_0M | NWM_RATESET_48_0M | NWM_RATESET_54_0M )

#define NWM_WPA_PSK_LENGTH               32
#define NWM_WPA_PASSPHRASE_LENGTH_MAX    64


/* Link levels */
#define NWM_RSSI_INFRA_LINK_LEVEL_1  12
#define NWM_RSSI_INFRA_LINK_LEVEL_2  17
#define NWM_RSSI_INFRA_LINK_LEVEL_3  22

#define NWM_RSSI_ADHOC_LINK_LEVEL_1  12 /* [TODO] TBD */
#define NWM_RSSI_ADHOC_LINK_LEVEL_2  17 /* [TODO] TBD */
#define NWM_RSSI_ADHOC_LINK_LEVEL_3  22 /* [TODO] TBD */

/*---------------------------------------------------------------------------*
    Macro definitions
 *---------------------------------------------------------------------------*/


// ID for each API
typedef enum NWMApiid
{
    NWM_APIID_INIT = 0,                 // NWM_Init()
    NWM_APIID_RESET,                    // NWM_Reset()
    NWM_APIID_END,                      // NWM_End()

    NWM_APIID_LOAD_DEVICE,              // NWM_LoadDevice()
    NWM_APIID_UNLOAD_DEVICE,            // NWM_UnloadDevice()
    NWM_APIID_OPEN,                     // NWM_Open()
    NWM_APIID_CLOSE,                    // NWM_Close()

    NWM_APIID_START_SCAN,               // NWM_StartScan()
    NWM_APIID_CONNECT,                  // NWM_Connect()
    NWM_APIID_DISCONNECT,               // NWM_Disconnect()
    NWM_APIID_SET_RECEIVING_BUF,        // NWM_SetReceivingFrameBuffer()
    NWM_APIID_SEND_FRAME,               // NWM_SendFrame()
    NWM_APIID_UNSET_RECEIVING_BUF,      // NWM_UnsetReceivingFrameBuffer()
    NWM_APIID_SET_WEPKEY,               // NWM_SetWEPKey()
    NWM_APIID_SET_PS_MODE,              // NWM_SetPowerSaveMode()

    NWM_APIID_SET_WPA_KEY,              //
    NWM_APIID_SET_WPA_PARAMS,           //

    NWM_APIID_CREATE_QOS,               //
    NWM_APIID_SET_WPA_PSK,              // NWM_SetWPAPSK()
    NWM_APIID_INSTALL_FIRMWARE,         // NWMi_InstallFirmware()
    NWM_APIID_ASYNC_KIND_MAX,           //  : Type of asynchronous process
  
    NWM_APIID_INDICATION = 128,         //  : For indication callback

    NWM_APIID_UNKNOWN = 255             //  : Value returned from ARM7 at unknown command number
}
NWMApiid;


// API result codes
typedef enum NWMRetCode
{
    NWM_RETCODE_SUCCESS         =  0,
    NWM_RETCODE_FAILED          =  1,
    NWM_RETCODE_OPERATING       =  2,
    NWM_RETCODE_ILLEGAL_STATE   =  3,
    NWM_RETCODE_NWM_DISABLE     =  4,
    NWM_RETCODE_INVALID_PARAM   =  5,
    NWM_RETCODE_FIFO_ERROR      =  6,
    NWM_RETCODE_FATAL_ERROR     =  7,   // Error that the software cannot handle
    NWM_RETCODE_NETBUF_ERROR    =  8,
    NWM_RETCODE_WMI_ERROR       =  9,
    NWM_RETCODE_SDIO_ERROR      = 10,
    NWM_RETCODE_RECV_IND        = 11,
    NWM_RETCODE_INDICATION      = 12,   // Internal use only
  
    NWM_RETCODE_MAX
} NWMRetCode;

// NWM state codes
typedef enum NWMState
{
    NWM_STATE_NONE            = 0x0000,
    NWM_STATE_INITIALIZED     = 0x0001,    // INITIALIZED state
    NWM_STATE_LOADED          = 0x0002,    // LOADED state
    NWM_STATE_DISCONNECTED    = 0x0003,    // DISCONNECTED state
    NWM_STATE_INFRA_CONNECTED = 0x0004,    // CONNECTED STA (infrastructure) state
    NWM_STATE_ADHOC_CONNECTED = 0x0005     // CONNECTED STA (ad hoc) state
}
NWMState;

typedef enum NWMReasonCode
{
    NWM_REASON_API_SUCCESS          = 0,
  
    /* For infra mode */
    NWM_REASON_NO_NETWORK_AVAIL     = 1,
    NWM_REASON_LOST_LINK            = 2,
    NWM_REASON_DISCONNECT_CMD       = 3,
    NWM_REASON_BSS_DISCONNECTED     = 4,
    NWM_REASON_AUTH_FAILED          = 5,
    NWM_REASON_ASSOC_FAILED         = 6,
    NWM_REASON_NO_RESOURCES_AVAIL   = 7,
    NWM_REASON_CSERV_DISCONNECT     = 8,
    NWM_REASON_INVAILD_PROFILE      = 9,

    NWM_REASON_WEP_KEY_ERROR        =10,

    /* For WPA supplicant */
    NWM_REASON_WPA_KEY_ERROR        =11,
    NWM_REASON_TKIP_MIC_ERROR       =12,
    
    /* For Wireless QoS (802.11e) */
    NWM_REASON_NO_QOS_RESOURCES_AVAIL   = 13,
    
    NWM_REASON_UNKNOWN

} NWMReasonCode;

typedef enum NWMAuthMode
{
  NWM_AUTHMODE_OPEN,    /* DOT11 authentication */
  NWM_AUTHMODE_SHARED,  /* DOT11 authentication */
  NWM_AUTHMODE_WPA_PSK_TKIP         = NWM_WPAMODE_WPA_TKIP,   /* Be careful to keep this from overlapping with WCM_WEPMODE_* */
  NWM_AUTHMODE_WPA2_PSK_TKIP        = NWM_WPAMODE_WPA2_TKIP,
  NWM_AUTHMODE_WPA_PSK_AES          = NWM_WPAMODE_WPA_AES,
  NWM_AUTHMODE_WPA2_PSK_AES         = NWM_WPAMODE_WPA2_AES
}
NWMAuthMode, NWMauthMode;


// Element ID of Information Elements
typedef enum NWMElementID {
  NWM_ELEMENTID_SSID               = 0,
  NWM_ELEMENTID_SUPPORTED_RATES    = 1,
  NWM_ELEMENTID_FH_PARAMETER_SET   = 2,
  NWM_ELEMENTID_DS_PARAMETER_SET   = 3,
  NWM_ELEMENTID_CF_PARAMETER_SET   = 4,
  NWM_ELEMENTID_TIM                = 5,
  NWM_ELEMENTID_IBSS_PARAMETER_SET = 6,
  NWM_ELEMENTID_COUNTRY            = 7,
  NWM_ELEMENTID_HP_PARAMETERS      = 8,
  NWM_ELEMENTID_HP_TABLE           = 9,
  NWM_ELEMENTID_REQUEST            = 10,
  NWM_ELEMENTID_QBSS_LOAD          = 11,
  NWM_ELEMENTID_EDCA_PARAMETER_SET = 12,
  NWM_ELEMENTID_TSPEC              = 13,
  NWM_ELEMENTID_TRAFFIC_CLASS      = 14,
  NWM_ELEMENTID_SCHEDULE           = 15,
  NWM_ELEMENTID_CHALLENGE_TEXT     = 16,

  NWM_ELEMENTID_POWER_CONSTRAINT   = 32,
  NWM_ELEMENTID_POWER_CAPABILITY   = 33,
  NWM_ELEMENTID_TPC_REQUEST        = 34,
  NWM_ELEMENTID_TPC_REPORT         = 35,
  NWM_ELEMENTID_SUPPORTED_CHANNELS = 36,
  NWM_ELEMENTID_CH_SWITCH_ANNOUNCE = 37,
  NWM_ELEMENTID_MEASURE_REQUEST    = 38,
  NWM_ELEMENTID_MEASURE_REPORT     = 39,
  NWM_ELEMENTID_QUIET              = 40,
  NWM_ELEMENTID_IBSS_DFS           = 41,
  NWM_ELEMENTID_ERP_INFORMATION    = 42,
  NWM_ELEMENTID_TS_DELAY           = 43,
  NWM_ELEMENTID_TCLASS_PROCESSING  = 44,
  NWM_ELEMENTID_HT_CAPABILITY      = 45,
  NWM_ELEMENTID_QOS_CAPABILITY     = 46,
  NWM_ELEMENTID_RSN                = 48,
  NWM_ELEMENTID_EX_SUPPORTED_RATES = 50,
  NWM_ELEMENTID_HT_INFORMATION     = 61,
  
  NWM_ELEMENTID_VENDOR             = 221,
  NWM_ELEMENTID_NINTENDO           = 221
}
NWMElementID;

typedef enum NWMPowerMode {
  NWM_POWERMODE_ACTIVE,
  NWM_POWERMODE_STANDARD,
  NWM_POWERMODE_UAPSD
} NWMPowerMode;

typedef enum NWMAccessCategory {
  NWM_AC_BE,          /* Best effort */
  NWM_AC_BK,          /* Background */
  NWM_AC_VI,          /* Video */
  NWM_AC_VO,          /* Voice */
  NWM_AC_NUM
} NWMAccessCategory;

typedef enum NWMNwType
{
  NWM_NWTYPE_INFRA,
  NWM_NWTYPE_ADHOC,
  NWM_NWTYPE_WPS,
  NWM_NWTYPE_NUM
} NWMNwType;

typedef enum NWMFramePort
{
    NWM_PORT_IPV4_ARP, /* For TCP/IP */
    NWM_PORT_EAPOL,    /* For WPA supplicant */
    NWM_PORT_OTHERS,
    NWM_PORT_NUM
} NWMFramePort, NWMframePort;

typedef void (*NWMCallbackFunc) (void *arg);     // Callback type for the NWM API

/*---------------------------------------------------------------------------*
    Structure Definitions
 *---------------------------------------------------------------------------*/

typedef struct NWMBssDesc
{
    u16     length;                    // 2
    s16     rssi;                      // 4
    u8      bssid[NWM_SIZE_BSSID];     // 10
    u16     ssidLength;                // 12
    u8      ssid[NWM_SIZE_SSID];       // 44
    u16     capaInfo;                  // 46
    struct
    {
        u16     basic;                 // 48
        u16     support;               // 50
    }
    rateSet;
    u16     beaconPeriod;              // 52
    u16     dtimPeriod;                // 54
    u16     channel;                   // 56
    u16     cfpPeriod;                 // 58
    u16     cfpMaxDuration;            // 60
    u16     reserved;                  // 62 just for compatibility with WM (must be 0)
    u16     elementCount;              // 64
    u16     elements[1];
} NWMBssDesc, NWMbssDesc;


typedef struct NWMInfoElements
{
  u8  id;
  u8  length;
  u16 element[1];
}
NWMInfoElements;


//---------------------------------------
// NWM Scan parameter structure
typedef struct NWMScanParam
{
    NWMBssDesc *scanBuf;                   // Buffer that stores parent information
    u16         scanBufSize;               // Size of scanBuf
    u16         channelList;               // List of channels (more than one can be specified) to scan
    u16         channelDwellTime;          // Per-channel scan time (in ms)
    u16         scanType;                  // passive or active
    u8          bssid[NWM_SIZE_BSSID];     // MAC address to scan for (all parent devices will be targeted if this is 0xff)
    u16         ssidLength;                // Length of the SSID to scan for (all nodes will be targeted if this is 0)
    u8          ssid[NWM_SIZE_SSID];       // SSID to scan for
    u16         rsv[6];
} NWMScanParam, NWMscanParam;

// NWM Scan parameter structure
typedef struct NWMScanExParam
{
    NWMBssDesc *scanBuf;                   // Buffer that stores parent information
    u16         scanBufSize;               // Size of scanBuf
    u16         channelList;               // List of channels (more than one can be specified) to scan
    u16         channelDwellTime;          // Per-channel scan time (in ms)
    u16         scanType;                  // passive or active
    u8          bssid[NWM_SIZE_BSSID];     // MAC address to scan for (all parent devices will be targeted if this is 0xff)
    u16         ssidLength;                // Length of the SSID to scan for (all nodes will be targeted if this is 0)
    u8          ssid[NWM_SIZE_SSID];       // SSID to scan for
    u16         ssidMatchLength;           // Matching length of the SSID to scan for
    u16         rsv[5];
} NWMScanExParam, NWMscanExParam;


//---------------------------------------
// NWM  WPA parameter structure

typedef struct NWMWpaParam {
    u16   auth;     // NWMAuthMode (can use PSK only)
    u8    psk[NWM_WPA_PSK_LENGTH];
} NWMWpaParam;

// Buffer structure for receiving frames
typedef struct NWMRecvFrameHdr
{
  u8   da[NWM_SIZE_MACADDR];
  u8   sa[NWM_SIZE_MACADDR];
  u8   pid[2];
  u8   frame[2];
} NWMRecvFrameHdr;

//==========================================================================================

// Normal callback arguments
typedef struct NWMCallback
{
    u16     apiid;
    u16     retcode;

} NWMCallback;

// Callback arguments for the NWM_StartScan function
typedef struct NWMStartScanCallback
{
    u16     apiid;
    u16     retcode;
    u32     channelList;               // Scanned channel list, regardless of whether found
    u8      reserved[2];               // Padding
    u16     bssDescCount;              // Number of parents that were found
    u32     allBssDescSize;
    NWMBssDesc *bssDesc[NWM_SCAN_NODE_MAX];  // Beginning address of the parent information
    u16     linkLevel[NWM_SCAN_NODE_MAX];   // Reception signal strength

} NWMStartScanCallback, NWMstartScanCallback;

// Callback arguments for the NWM_Connect function
typedef struct NWMConnectCallback
{
    u16     apiid;
    u16     retcode;
    u16     channel;
    u8      bssid[NWM_SIZE_BSSID];
    s16     rssi;
    u16     aid;                       // Only when CONNECTED. AID assigned to self
    u16     reason;                    // reason when disconnecting. This is defined in NWMReasonCode
    u16     listenInterval;
    u8      networkType;
    u8      beaconIeLen;
    u8      assocReqLen;
    u8      assocRespLen;
    u8      assocInfo[2]; /* This field consists of beaconIe, assocReq, assocResp */
} NWMConnectCallback, NWMconnectCallback;

// Callback arguments for the NWM_Disconnect function
typedef struct NWMDisconnectCallback
{
    u16   apiid;
    u16   retcode;
    u16   reason;
    u16   rsv;
} NWMDisconnectCallback, NWMdisconnectCallback;

// Callback arguments for the NWM_SendFrame function
typedef struct NWMSendFrameCallback
{
    u16     apiid;
    u16     retcode;
    NWMCallbackFunc callback;
} NWMSendFrameCallback;

// Callback arguments for the NWM_SetReceivingFrameBuffer function
typedef struct NWMReceivingFrameCallback
{
  u16   apiid;
  u16   retcode;
  u16   port;
  s16   rssi;
  u32   length;
  NWMRecvFrameHdr *recvBuf;

} NWMReceivingFrameCallback;


/*===========================================================================
  NWM APIs
  ===========================================================================*/

/*---------------------------------------------------------------------------*
  Name:         NWM_Init

  Description:  Initializes the NWM library.
                Synchronous function that initializes only ARM9.

  Arguments:    sysBuf: Pointer to the buffer allocated by the caller.

                bufSize: Size of the buffer allocated by the caller

                dmaNo: DMA number used by the NWM library

  Returns:      NWMRetCode: Returns the processing result.
 *---------------------------------------------------------------------------*/
NWMRetCode NWM_Init(void* sysBuf, u32 bufSize, u8 dmaNo);

/*---------------------------------------------------------------------------*
  Name:         NWM_Reset

  Description:  Restarts the TWL wireless driver and resets the TWL wireless module.

  Arguments:    callback: Callback function that is called when the asynchronous process completes

  Returns:      NWMRetCode: Returns the processing result.
                            Returns NWM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
 *---------------------------------------------------------------------------*/
NWMRetCode NWM_Reset(NWMCallbackFunc callback);

/*---------------------------------------------------------------------------*
  Name:         NWM_LoadDevice

  Description:  Starts the TWL wireless module.
                There will be an internal state transition from NWM_STATE_INITIALIZED to NWM_STATE_LOADED.
                

  Arguments:    callback: Callback function that is called when the asynchronous process completes

  Returns:      NWMRetCode: Returns the processing result.
                            Returns NWM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
 *---------------------------------------------------------------------------*/
NWMRetCode NWM_LoadDevice(NWMCallbackFunc callback);

/*---------------------------------------------------------------------------*
  Name:         NWM_UnloadDevice

  Description:  Shuts down the TWL wireless module.
                There will be an internal state transition from NWM_STATE_LOADED to NWM_STATE_INITIALIZED.
                

  Arguments:    callback: Callback function that is called when the asynchronous process completes

  Returns:      NWMRetCode: Returns the processing result.
                            Returns NWM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
 *---------------------------------------------------------------------------*/
NWMRetCode NWM_UnloadDevice(NWMCallbackFunc callback);

/*---------------------------------------------------------------------------*
  Name:         NWM_Open

  Description:  Allows use of TWL wireless functionality.
                There will be an internal state transition from NWM_STATE_LOADED to NWM_STATE_DISCONNECTED.
                

  Arguments:    callback: Callback function that is called when the asynchronous process completes

  Returns:      NWMRetCode: Returns the processing result.
                            Returns NWM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
 *---------------------------------------------------------------------------*/
NWMRetCode NWM_Open(NWMCallbackFunc callback);

/*---------------------------------------------------------------------------*
  Name:         NWM_Close

  Description:  Disables TWL wireless functionality.
                There will be an internal state transition from NWM_STATE_DISCONNECTED to NWM_STATE_LOADED.
                

  Arguments:    callback: Callback function that is called when the asynchronous process completes

  Returns:      NWMRetCode: Returns the processing result.
                            Returns NWM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
 *---------------------------------------------------------------------------*/
NWMRetCode NWM_Close(NWMCallbackFunc callback);

/*---------------------------------------------------------------------------*
  Name:         NWM_End

  Description:  Shuts down the NWM library.
                This is a synchronous function that shuts down processing only on the ARM9.

  Arguments:    None.

  Returns:      NWMRetCode: Returns the processing result.
 *---------------------------------------------------------------------------*/
NWMRetCode NWM_End(void);

/*---------------------------------------------------------------------------*
  Name:         NWM_StartScan

  Description:  Starts scanning for an AP.
                A single function call can get parent information for multiple devices.
                There are no state transitions during or after a scan.

  Arguments:    callback: Callback function that is called when the asynchronous process completes.
                          Arguments will be returned as a NWMStartScanCallback structure.
                param: Pointer to a structure that shows information on scan settings.
                       The ARM7 directly writes scan result information to param->scanBuf, so it must match the cache line.
                                
                        See the NWMScanParam structure for individual parameters.

  Returns:      NWMRetCode: Returns the processing result.
                            Returns NWM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
 *---------------------------------------------------------------------------*/
NWMRetCode NWM_StartScan(NWMCallbackFunc callback, const NWMScanParam *param);

/*---------------------------------------------------------------------------*
  Name:         NWM_StartScanEx

  Description:  Starts scanning for an AP.
                A single function call can get parent information for multiple devices.
                There are no state transitions during or after a scan.
                You can run a partial SSID match on the scan results.

  Arguments:    callback: Callback function that is called when the asynchronous process completes.
                          Arguments will be returned as a NWMStartScanCallback structure.
                param: Pointer to a structure that shows information on scan settings.
                       The target SSID and the scan results from the wireless module are compared only for the length specified by ssidMatchLength and then only the relevant scan results are written out.
                       The ARM7 directly writes scan result information to param->scanBuf, so it must match the cache line.
                                
                                
                                
                        See the NWMScanExParam structure for individual parameters.

  Returns:      NWMRetCode: Returns the processing result.
                            Returns NWM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
 *---------------------------------------------------------------------------*/
NWMRetCode NWM_StartScanEx(NWMCallbackFunc callback, const NWMScanExParam *param);

/*---------------------------------------------------------------------------*
  Name:         NWM_Connect

  Description:  Connects to an AP. This cannot connect to DS parent devices.

  Arguments:    callback: Callback function that is called when the asynchronous process completes.
                          Arguments will be returned as a NWMConnectCallback structure.
                nwType: Takes the value defined by NWMNwType.
                        In general, use NWM_NWTYPE_INFRA.
                pBdesc: Information on the AP to connect to.
                        Specifies the structure obtained with NWM_StartScan.
                        Instances of this structure are forcibly stored in the cache.
                        See the NWMBssDesc structure for individual parameters.

  Returns:      NWMRetCode: Returns the processing result.
                            Returns NWM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
 *---------------------------------------------------------------------------*/
NWMRetCode NWM_Connect(NWMCallbackFunc callback, u8 nwType, const NWMBssDesc *pBdesc);

/*---------------------------------------------------------------------------*
  Name:         NWM_Disconnect

  Description:  Disconnects from an access point.

  Arguments:    callback: Callback function that is called when the asynchronous process completes.
                          Arguments will be returned as a NWMDisconnectCallback structure.

  Returns:      NWMRetCode: Returns the processing result.
                            Returns NWM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
 *---------------------------------------------------------------------------*/
NWMRetCode NWM_Disconnect(NWMCallbackFunc callback);

/*---------------------------------------------------------------------------*
  Name:         NWM_SetWEPKey

  Description:  Configures the WEP encryption mode and key.
                You must configure these before calling the NWM_Connect function.

  Arguments:    callback: Callback function that is called when the asynchronous process completes.
                wepmode: NWM_WEPMODE_OPEN: No encryption feature
                         NWM_WEPMODE_40BIT: RC4 (40bit) encryption mode
                         NWM_WEPMODE_104BIT: RC4 (104bit) encryption mode
                         NWM_WEPMODE_128BIT: RC4 (128bit) encryption mode.
                wepkeyid: Selects which of the 4 specified wepkeys to use for sending data.
                          Specify using 0-3.
                wepkey: Pointer to the encryption key data (80 bytes).
                        Key data consists of 4 pieces of data, each 20 bytes long.
                        Of each 20 bytes,
                        5 bytes in 40-bit mode
                        13 bytes in 104-bit mode
                        16 bytes in 128-bit mode
                        are used.
                        The data entity is forcibly stored in cache.
                authMode: Authentication mode when connecting.
                          NWM_AUTHMODE_OPEN: Open authentication
                          NWM_AUTHMODE_SHARED: Shared key authentication

  Returns:      NWMRetCode: Returns the processing result.
                            Returns NWM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
 *---------------------------------------------------------------------------*/
NWMRetCode NWM_SetWEPKey(NWMCallbackFunc callback, u16 wepmode, u16 wepkeyid, const u8 *wepkey, u16 authMode);

/*---------------------------------------------------------------------------*
  Name:         NWM_Passphrase2PSK

  Description:  Calculates the PSK from a WPA passphrase. Synchronous function.

  Arguments:    passphrase: Pointer to a buffer with the WPA passphrase.
                            This has a maximum size of 63 bytes.
                ssid: Pointer to a buffer with the SSID of the access point to connect to
                ssidlen: SSID size of the access point to connect to
                psk: Pointer to the buffer that will store the calculated PSK.
                     This is fixed to 32 bytes.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NWM_Passphrase2PSK(const u8 passphrase[NWM_WPA_PASSPHRASE_LENGTH_MAX], const u8 *ssid, u8 ssidlen, u8 psk[NWM_WPA_PSK_LENGTH]);

/*---------------------------------------------------------------------------*
  Name:         NWM_SetWPAPSK

  Description:  Configures the WPA encryption mode and key.
                You must configure these before calling the NWM_Connect function.

  Arguments:    callback: Callback function that is called when the asynchronous process completes
                param: Pointer to a structure that shows information on WPA settings.
                       See the NWMWpaParam structure for individual parameters.

  Returns:      NWMRetCode: Returns the processing result.
                            Returns NWM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
 *---------------------------------------------------------------------------*/
NWMRetCode NWM_SetWPAPSK(NWMCallbackFunc callback, NWMWpaParam *param);

/*---------------------------------------------------------------------------*
  Name:         NWM_SetReceivingFrameBuffer

  Description:  Configures the receive buffer.

  Arguments:    callback: Callback function that is invoked when asynchronous processing finishes and when data is sent.
                                
                          Arguments will be returned as a NWMReceivingFrameCallback structure.
                          This entails NWM_RETCODE_RECV_IND when sending data.
                                
                recvBuf: Pointer to the data receive buffer.
                         Pay attention to the cache because the ARM7 writes data out directly.
                recvBufSize: Size of data receive buffer. The receive buffer count is an integer quotient of this size divided by 1536 (0x600) bytes.
                             (Anything below the decimal point will be rounded off.)
                             To avoid losing received data, allocate at least 3072 (0xC00) bytes.
                protocol: Which protocol the receive buffer is for.
                          NWM_PORT_IPV4_ARP: IPv4 TCP/IP data and ARP data
                                             (You should normally use this.)
                          NWM_PORT_EAPOL: EAPOL data (for WPS)
                          NWM_PORT_OTHERS: Other protocols

  Returns:      NWMRetCode: Returns the processing result.
                            Returns NWM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
 *---------------------------------------------------------------------------*/
NWMRetCode NWM_SetReceivingFrameBuffer(NWMCallbackFunc callback, u8* recvBuf, u16 recvBufSize, u16 protocol);

/*---------------------------------------------------------------------------*
  Name:         NWM_SendFrame

  Description:  Sets the data for the wireless module to send.
                Note that this does not guarantee that the transmission has completed.

  Arguments:    callback: Callback function that is called when the asynchronous process completes.
                          Arguments will be returned as a NWMSendFrameCallback structure.
                destAddr: Pointer to a buffer with the BSSID to send to
                sendFrame: Pointer to the data to send.
                           Note that instances of the data to send are forced to be stored in the cache.
                                
                sendFrameSize: Size of the data to send

  Returns:      NWMRetCode: Returns the processing result.
                            Returns NWM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
 *---------------------------------------------------------------------------*/
NWMRetCode NWM_SendFrame(NWMCallbackFunc callback, const u8 *destAddr,
                         const u16 *sendFrame, u16 sendFrameSize);

/*---------------------------------------------------------------------------*
  Name:         NWM_UnsetReceivingFrameBuffer

  Description:  Releases the receive buffer.

  Arguments:    callback: The callback function that is invoked when asynchronous processing finishes and when data is sent
                                
                                
                protocol: Which protocol the receive buffer is for.
                          NWM_PORT_IPV4_ARP: IPv4 TCP/IP data and ARP data
                                             (You should normally use this.)
                          NWM_PORT_EAPOL: EAPOL data (for WPS)
                          NWM_PORT_OTHERS: Other protocols

  Returns:      NWMRetCode: Returns the processing result.
                            Returns NWM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
 *---------------------------------------------------------------------------*/
NWMRetCode NWM_UnsetReceivingFrameBuffer(NWMCallbackFunc callback, u16 protocol);

/*---------------------------------------------------------------------------*
  Name:         NWM_SetPowerSaveMode

  Description:  Changes the PowerSaveMode.

  Arguments:    callback: Callback function that is called when the asynchronous process completes
                powerSave: Power-save mode.
                           NWM_POWERMODE_ACTIVE: Power-save mode off
                           NWM_POWERMODE_STANDARD: Standard 802.11 power-save mode
                           NWM_POWERMODE_UAPSD: Enhanced power-save mode established in 802.11e
                                                        (Unscheduled Automatic Power Save Delivery)

  Returns:      NWMRetCode: Returns the processing result.
                            Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
 *---------------------------------------------------------------------------*/
NWMRetCode NWM_SetPowerSaveMode(NWMCallbackFunc callback, NWMPowerMode powerSave);

/*---------------------------------------------------------------------------*
  Name:         NWM_GetMacAddress

  Description:  Gets the MAC address for this device from the TWL wireless module.
                Synchronous function.

  Arguments:    macAddr: Pointer to a buffer to store this device's MAC address

  Returns:      NWMRetCode: Returns the processing result.
 *---------------------------------------------------------------------------*/
NWMRetCode NWM_GetMacAddress(u8* macAddr);

/*---------------------------------------------------------------------------*
  Name:         NWM_GetBssDesc

  Description:  Gets BSS data from the buffer with scan results from NWM_StartScan.
                Synchronous function.

  Arguments:    bssbuf: Pointer to the buffer with scan results
                bsssize: Size of the buffer with scan results
                index: Index to the BSS data to get

  Returns:      NWMBssDesc: Returns the BSS data at index.
                            Returns NULL if it could not be obtained.
 *---------------------------------------------------------------------------*/
NWMBssDesc* NWM_GetBssDesc(void* bssbuf, u32 bsssize, int index);

/*---------------------------------------------------------------------------*
  Name:         NWM_GetAllowedChannel

  Description:  Gets the channel that was permitted to use for the communication. Synchronous function.
                Call this from a state after NWM_STATE_DISCONNECTED (after NWM_Open has completed).

  Arguments:    None.

  Returns:      u16: Returns the bit field of the permitted channel.
                     The least significant bit indicates channel 1, and the most significant bit indicates channel 16.
                     A channel can be used if its corresponding bit is set to 1 and is prohibited from being used if its corresponding bit is set to 0.
                     Typically, a value is returned with several of the bits corresponding to channels 1-13 set to 1.
                     When 0x0000 is returned, no channels can be used and wireless features are themselves prohibited.
                     Also, in case the function failed, such as when it is not yet initialized, 0x8000 is returned.
                        
 *---------------------------------------------------------------------------*/
u16 NWM_GetAllowedChannel(void);

/*---------------------------------------------------------------------------*
  Name:         NWM_CalcLinkLevel

  Description:  Calculates the link level from the threshold value defined in nwm_common_private.h.

  Arguments:    s16: RSSI value sent with a notification from the Atheros driver

  Returns:      u16: The same link level as the WM library.
 *---------------------------------------------------------------------------*/
u16 NWM_CalcLinkLevel(s16 rssi);

/*---------------------------------------------------------------------------*
  Name:         NWM_GetDispersionScanPeriod

  Description:  Gets the time limit that should be set on searching for an AP or DS parent device as an STA.

  Arguments:    u16 scanType: Scan type, either NWM_SCANTYPE_PASSIVE or NWM_SCANTYPE_ACTIVE
  
  Returns:      u16: Search limit time that should be set (ms).
 *---------------------------------------------------------------------------*/
u16 NWM_GetDispersionScanPeriod(u16 scanType);

/*---------------------------------------------------------------------------*
  Name:         NWM_GetState

  Description:  Gets the NWM state.

  Arguments:    None.
  
  Returns:      u16: The NWM state. This is indicated by an NWMState enumerated type.
 *---------------------------------------------------------------------------*/
u16 NWM_GetState(void);

/*---------------------------------------------------------------------------*
  Name:         NWM_GetInfoElements

  Description:  Gets the specified information element (IE) from BSS data.

  Arguments:    bssDesc: BSS data to get
                elementID: ElementID of an information element, established in 802.11

  Returns:      NWMInfoElements: Returns a pointer to the specified information element if it exists.
                                 Returns NULL if it could not be obtained.
                                      
 *---------------------------------------------------------------------------*/
NWMInfoElements* NWM_GetInfoElements(NWMBssDesc *bssDesc, u8 elementID);

/*---------------------------------------------------------------------------*
  Name:         NWM_GetVenderInfoElements

  Description:  Gets the specified vendor information element (IE) from BSS data.
                This is used to get information elements for WPA and so on.

  Arguments:    bssDesc: BSS data to get
                elementID: ElementID of an information element, established in 802.11.
                           This must be specified as NWM_ELEMENTID_VENDOR.
                ouiType: Array of the OUI (3 bytes) and Type (1 byte).
                         For WPA, this is 0x00 0x50 0xf2 0x01.

  Returns:      NWMInfoElements: Returns a pointer to the specified vendor information element if it exists.
                                 Returns NULL if it could not be obtained.
                                      
 *---------------------------------------------------------------------------*/
NWMInfoElements* NWM_GetVenderInfoElements(NWMBssDesc *bssDesc, u8 elementID, const u8 ouiType[4]);


#ifdef  __cplusplus
}       /* extern "C" */
#endif

#endif /* LIBRARIES_NWM_ARM9_NWM_H__ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
