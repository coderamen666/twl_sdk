/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WM - demos.Twl
  File:     wh.c

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-10-20#$
  $Rev: 9005 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include <nitro.h>
#include <nitro/wm.h>
/* TODO: Remove the commenting out once the CHT library migration is complete.*/
// #include <nitro/cht.h>

#include "wh.h"

/*
  wh.c : Wireless Manager related Wrapper

  -Internal description for users

  The WM API implements the following three communication formats, in order of degree of specialization.

  - General MP communication
  - Data sharing
  - Key sharing

   
  These are not mutually exclusive. Key sharing, for example, is a method that uses data sharing, and although data sharing is just a means of sharing data using MP communication, we broke things down into these three levels for the sake of explanation.
  
  
  

  "General MP communication" refers to a communications mode at the multi-poll protocol level, and can be briefly summarized as performing the following steps for a single communication cycle.
  

  (1) Broadcasting of data by the parent to all the children
  (2) Replying from each child
  (3) The parent broadcasts notification that the communication cycle has ended.

   

  Data sharing is a mode where parent devices get data from all connected children (step 2, above) and then, after including their own portion, distribute that as shared data to all children (step 1, above).
  
  
  

  Key sharing is where the data shared in data sharing is an array of key data for each device.
  


  1) Overview of communication processing

  When features are implemented using wireless LAN, it is the ARM7 processor that does the actual work.
  
  All send, receive, and other requests from the ARM9 must therefore pass through the ARM7. As a result, nearly all WM functions on the ARM9 are asynchronous and basic processing proceeds approximately as follows.
  

  1. A function for writing ARM7 requests to the FIFO is called
  2. The system that obtained the results returned by the ARM7 from the FIFO will invoke the callback function (specified in step 1)
  

   

  In this sample's implementation, the two functions needed are considered as a set that expresses one of the wireless LAN functionality states. The communication process is carried out by stringing together chains of these states. (When we refer to "WH states," hereafter, it will indicate these wireless LAN states.)
  
  
  
  

  The names of functions in this source correspond to their respective WH states, as shown below.

  WH_StateInXXXXXX   - Start function for the WH state XXXXXX.
  WH_StateOutXXXXXX  - Callback function that receives completion notifications for the WH state XXXXXX.

   


  2) Transitions between WH states

  The callback functions of WH states that should automatically proceed to the next WH state will call a function to start transitioning to that state when their own state's processing has finished.
   
  (In actual processing, the system transitions through these states in basically the same top-to-bottom order they appear in the source code.)

  All the WH state names, and their mutual sequence transition relationships, are as follows.
  

  - Initialization sequence
  (WH_Initialize function)
  
  Initialize

  - Parent communication preparation sequence
  (WH_Connect function)
  
  SetParentParam
  
  StartParent
  
  StartParentMP
  
  (**)
  
  StartParentKeyShare

  Note: If MP connection mode (or DS connection mode) is specified in WH_Connect, end at (**).
  

  - Parent end sequence
  (WH_Finalize function)
  
  (EndParentKeyShare)
  
  EndParentMP
  
  EndParent

  - Child communication preparation sequence
  (WH_Connect function)
  
  StartScan
  
  EndScan
  
  StartChild
  
  StartChildMP
  
  (**)
  
  StartChildKeyShare

  Note: If MP connection mode (or DS connection mode) is specified in WH_Connect, end at (**).
  

  - Child end sequence
  (WH_Finalize function)
  
  (EndChildKeyShare)
  
  EndChildMP
  
  EndChild

  Be careful here, as the callback function for each WH state will not necessarily be called only when that WH state ends.
  

  For example, the WH_StateOutStartParentMP function, which is the callback for the WH state StartParentMP, is called not only when StartMP processing ends but also when an entire MP sequence ends and in other similar situations (for further details, refer to comments in the function body).
  
  


  3) Actual sending and receiving operations

  In this source code, three communications methods are implemented: general MP communications, data sharing, and key sharing. Depending on which of these communications methods is used, the way in which data is sent and received will change entirely.
  
  

  For general MP communication, a callback needs to be set in the functions below at initialization time.
  

  WH_SetReceiver(WHReceiverFunc proc);

  Data is passed to the configured callback function as the data content is received.
  
  Data transmission is executed with WH_SendData.
  Make sure the transmission data memory is not overwritten until the callback set with the WH_SendData function is called.
  

  With data sharing, internal updating is carried out at the same time the the WH_StepDS function is called and data is passed, and data is fetched with WH_GetSharedDataAdr.
  

  With key sharing communication, key data is fetched with the WH_GetKeySet function.
  (Processing corresponding to WH_StepDS is carried out internally. Key data that must be transmitted is sent automatically by the library, so nothing special needs to be done.)
   


  4) Errors and recovering

  This implementation is formulated as follows.

  - High-level functions use return values to determine whether there is an error. TRUE is determined to be success, and FALSE to be failure.
    
  - More detailed error codes are stored in sErrCode.
  - If a state is reached where processing cannot continue (such as when an operation is required), sSysState will change to WH_SYSSTATE_ERROR, and that will be detected.
     

   
  Depending on the application, it would seem that a callback function or OS_SendMessage needs to be used to report the error, but this is not particularly supported in this implementation. (If you are using this source code elsewhere, it is probably adequate to rewrite the WH_SetError function.)
  
  
  

  Also, for "well-behaved" disconnection (recovery) processing, we recommended that you explicitly call WM_Disconnect or a similar function whenever possible after terminating the state.
  
   In this implementation, the WH_Finalize function has been prepared for performing "well-behaved termination".
   This function will branch off to various shutdown processes (at least whenever possible) depending on the current state.
   


  5) Guideline compliance

  Comments are attached to code that is required for guideline compliance.
  (Note that processing to handle guideline rules may also be located outside of this file. 
  Relevant locations are accompanied by a comment with the string 'GUIDELINE'; use these for reference.)
   
 */

/* ======================================================================
   Static contents
   ====================================================================== */

static WMParentParam sParentParam ATTRIBUTE_ALIGN(32);

/*  The values below are configured throughout the code.
static WMParentParam sParentParam ATTRIBUTE_ALIGN(32) =
{
    Specify user definition data here if to be used in the beacon.
    // To display a parent device's nickname on the child device's parent selection screen (or for some other situation), set the information here and the parent will convey it to the child.
    // 
    NULL,                         // userGameInfo
    0,                            // userGameInfoLength
    0,                            // padding
    // The ggid is used in identifying the game group.
    // A value allocated by the game is used here.
    0,
    // The tgid is used to identify the temporary group.
    // A different value is needed each time activity is started as the parent device.
    // In this source, it is configured in the WH_Initialize function.
    0x0001,                       // tgid
    //  Designate whether or not to accept the new connection from the child device.
    1,                            // entryFlag
    // Designate the maximum number of connectable child devices
    WH_CHILD_MAX,                 // maxEntry
    //  Set to zero when not performing multiboot.
    0,                            // multiBootFlag
    // If KS_Flag is set to 1, key sharing correction will automatically be placed in parentMaxSize and childMaxSize when a connection is made.
    // 
    // Even when key sharing is in use, a value of 0 is acceptable if parentMaxSize and childMaxSize are accurately calculated and specified.
    // 
    0,                            // KS_Flag
    // In order to maintain compatibility with past versions, this is currently ignored in the prepared fields.
    0,                            // CS_Flag
    The interval of the beacon broadcast by the parent. [ms]
    // Values that have been appropriately scattered by WM_GetDispersionBeaconPeriod() are used during actual startup, but they  cluster around 200[ms].
    // 
    200,                          // beaconPeriod (millisec)
    // This is a reserved region
    {0},                           // rsv1
    {0},                           // rsv2
    // This is the wireless channel where the parent device waits for the child device.
    WH_DEFAULT_CHANNEL ,          // channel
    // Appropriate values have been set as maximum values in the members below
    // parentMaxSize - the maximum size of the data the parent device sends
    WH_PARENT_MAX_SIZE,
    // childMaxSize - the maximum data size the child device sends
    WH_CHILD_MAX_SIZE,
};
*/

// The wh state (This is different from the WH state. It can only be referenced externally.)
static int sSysState = WH_SYSSTATE_STOP;

// Connection mode (set externally, not touched in this source)
static int sConnectMode;

// The data receiving user function in MP communication mode
static WHReceiverFunc sReceiverFunc;

// User function for determining connection permission
static WHJudgeAcceptFunc sJudgeAcceptFunc;

// Own aid enters (the child may change during disconnect or reconnect)
static u16 sMyAid;

// Stores the bitmap showing the connection state
static u16 sConnectBitmap;

// For storing the error code
static int sErrCode;

// For random numbers
static u32 sRand;

/* For determining the communications usage rate (used in WH_StartMeasureChannel and WH_GetMeasureChannel) */
static u16 sChannel;
static u16 sChannelBusyRatio;
static u16 sChannelBitmap;

/* System work region buffer for WM */
static u8 sWmBuffer[WM_SYSTEM_BUF_SIZE] ATTRIBUTE_ALIGN(32);
// Send and receive buffer used internally and its size
static u8 sSendBuffer[MATH_MAX(WH_PARENT_SEND_BUFFER_SIZE, WH_CHILD_SEND_BUFFER_SIZE)]
ATTRIBUTE_ALIGN(32);
static u8 sRecvBuffer[MATH_MAX(WH_PARENT_RECV_BUFFER_SIZE, WH_CHILD_RECV_BUFFER_SIZE)]
ATTRIBUTE_ALIGN(32);
static s32 sSendBufferSize;
static s32 sRecvBufferSize;


/* For parent device search */
static WMBssDesc sBssDesc ATTRIBUTE_ALIGN(32);
static WMScanParam sScanParam ATTRIBUTE_ALIGN(32);
static WHStartScanCallbackFunc sScanCallback;
static u16 sChannelIndex;
static u16 sAutoConnectFlag;

static BOOL sPictoCatchFlag = FALSE;

/* Settings used for the parent connection */
static u8 sConnectionSsid[WM_SIZE_CHILD_SSID];

/* For WEP settings */
static WHParentWEPKeyGeneratorFunc sParentWEPKeyGenerator = NULL;
static WHChildWEPKeyGeneratorFunc sChildWEPKeyGenerator = NULL;
static u16 sWEPKey[20 / sizeof(u16)] ATTRIBUTE_ALIGN(32);

/* For data sharing */
static WMDataSharingInfo sDSInfo ATTRIBUTE_ALIGN(32);
static WMDataSet sDataSet ATTRIBUTE_ALIGN(32);
static WMKeySetBuf sWMKeySetBuf ATTRIBUTE_ALIGN(32);

/* WH initialization flag */
static u16 whInitialized = 0;

/* Callback for Indication notification of WM */
static WMCallbackFunc sIndicationCallback = NULL;

/* debug output hook function */
static void (*wh_trace) (const char *, ...) =
#if !defined(SDK_FINALROM)
        OS_TPrintf;
#else
        NULL;
#endif


/* ======================================================================
   State functions
   ====================================================================== */

// Random number macro
#define RAND()  ( sRand = sRand * 69069UL + 12345 )
#define RAND_INIT(x) ( sRand = (u32)(x) )

#define WH_MAX(a, b) ( ((a) > (b)) ? (a) : (b) )

/* Function that receives irregular notifications */
static void WH_IndicateHandler(void *arg);

/* (Stateless) -> READY -> STOP -> IDLE */
static BOOL WH_StateInInitialize(void);

#ifndef WH_USE_DETAILED_INITIALIZE
static void WH_StateOutInitialize(void *arg);
#else
static BOOL WH_StateInEnable(void);
static void WH_StateOutEnable(void *arg);
static BOOL WH_StateInPowerOn(void);
static void WH_StateOutPowerOn(void *arg);
#endif

/* IDLE -> MEASURECHANNEL -> IDLE */
static u16 WH_StateInMeasureChannel(u16 channel);
static void WH_StateOutMeasureChannel(void *arg);
static WMErrCode WHi_MeasureChannel(WMCallbackFunc func, u16 channel);
static s16 SelectChannel(u16 bitmap);


/* IDLE -> PARENT -> MP-PARENT */
static BOOL WH_StateInSetParentParam(void);
static void WH_StateOutSetParentParam(void *arg);
static BOOL WH_StateInSetParentWEPKey(void);
static void WH_StateOutSetParentWEPKey(void *arg);
static BOOL WH_StateInStartParent(void);
static void WH_StateOutStartParent(void *arg);
static BOOL WH_StateInStartParentMP(void);
static void WH_StateOutStartParentMP(void *arg);

/* IDLE -> SCAN */
static BOOL WH_StateInStartScan(void);
static void WH_StateOutStartScan(void *arg);

/* SCAN -> IDLE */
static BOOL WH_StateInEndScan(void);
static void WH_StateOutEndScan(void *arg);

/* IDLE -> CHILD -> MP-CHILD */
static BOOL WH_StateInSetChildWEPKey(void);
static void WH_StateOutSetChildWEPKey(void *arg);
static BOOL WH_StateInStartChild(void);
static void WH_StateOutStartChild(void *arg);
static BOOL WH_StateInStartChildMP(void);
static void WH_StateOutStartChildMP(void *arg);

/* MP + key sharing */
static BOOL WH_StateInStartParentKeyShare(void);
static BOOL WH_StateInEndParentKeyShare(void);
static BOOL WH_StateInStartChildKeyShare(void);
static BOOL WH_StateInEndChildKeyShare(void);

/* MP + SetMPData */
static BOOL WH_StateInSetMPData(void *data, u16 datasize, WHSendCallbackFunc callback);
static void WH_StateOutSetMPData(void *arg);
static void WH_PortReceiveCallback(void *arg);

/* MP-PARENT -> IDLE */
static BOOL WH_StateInEndParentMP(void);
static void WH_StateOutEndParentMP(void *arg);
static BOOL WH_StateInEndParent(void);
static void WH_StateOutEndParent(void *arg);
static BOOL WH_StateInDisconnectChildren(u16 bitmap);
static void WH_StateOutDisconnectChildren(void *arg);

/* MP-CHILD -> IDLE */
static BOOL WH_StateInEndChildMP(void);
static void WH_StateOutEndChildMP(void *arg);
static BOOL WH_StateInEndChild(void);
static void WH_StateOutEndChild(void *arg);

/* IDLE -> STOP -> READY -> (stateless) */
static BOOL WH_StateInPowerOff(void);
static void WH_StateOutPowerOff(void *arg);
static BOOL WH_StateInDisable(void);
static void WH_StateOutDisable(void *arg);
static void WH_StateOutEnd(void *arg);

/* X -> IDLE */
static BOOL WH_StateInReset(void);
static void WH_StateOutReset(void *arg);


/* ======================================================================
   Debug codes
   ====================================================================== */
#define WMHIGH_DEBUG

#if defined(WMHIGH_DEBUG)

#define WH_TRACE    if(wh_trace) wh_trace

#define WH_TRACE_STATE OS_TPrintf("%s sSysState = %d\n", __func__, sSysState)

#define WH_REPORT_FAILURE(result)                \
    do{ OS_TPrintf("Failed in %s, %s = %s\n",      \
                  __func__,                      \
                  #result,                       \
                  WH_GetWMErrCodeName(result));  \
        WH_SetError(result); }while(0)

#define WH_ASSERT(exp) \
    (void) ((exp) || (OSi_Panic(__FILE__, __LINE__, "Failed assertion " #exp), 0))

#else  /* defined(WMHIGH_DEBUG) */

#define WH_TRACE_STATE                 /* */
#define WH_TRACE(...)               ((void)0)
#define WH_REPORT_FAILURE(result)   WH_SetError(result)
#define WH_ASSERT(exp)              ((void) 0)

#endif



#if defined(WMHIGH_DEBUG)
static const char *sStateNames[] = {
    "WM_STATE_READY",
    "WM_STATE_STOP",
    "WM_STATE_IDLE",
    "WM_STATE_CLASS1",
    "WM_STATE_TESTMODE",
    "WM_STATE_SCAN",
    "WM_STATE_CONNECT",
    "WM_STATE_PARENT",
    "WM_STATE_CHILD",
    "WM_STATE_MP_PARENT",
    "WM_STATE_MP_CHILD",
    "WM_STATE_DCF_CHILD",
};

static const char *WH_GetWMErrCodeName(int result)
{
    static const char *errnames[] = {
        "WM_ERRCODE_SUCCESS",
        "WM_ERRCODE_FAILED",
        "WM_ERRCODE_OPERATING",
        "WM_ERRCODE_ILLEGAL_STATE",
        "WM_ERRCODE_WM_DISABLE",
        "WM_ERRCODE_NO_DATASET",
        "WM_ERRCODE_INVALID_PARAM",
        "WM_ERRCODE_NO_CHILD",
        "WM_ERRCODE_FIFO_ERROR",
        "WM_ERRCODE_TIMEOUT",
        "WM_ERRCODE_SEND_QUEUE_FULL",
        "WM_ERRCODE_NO_ENTRY",
        "WM_ERRCODE_OVER_MAX_ENTRY",
        "WM_ERRCODE_INVALID_POLLBITMAP",
        "WM_ERRCODE_NO_DATA",
        "WM_ERRCODE_SEND_FAILED",
        "WM_ERRCODE_DCF_TEST",
        "WM_ERRCODE_WL_INVALID_PARAM",
        "WM_ERRCODE_WL_LENGTH_ERR",
        "WM_ERRCODE_FLASH_ERROR",

        "WH_ERRCODE_DISCONNECTED",
        "WH_ERRCODE_PARENT_NOT_FOUND",
        "WH_ERRCODE_NO_RADIO",
    };

    if (0 <= result && result < sizeof(errnames) / sizeof(char *))
    {
        return errnames[result];
    }
    else
    {
        return "N/A";
    }
}

static const char *WH_GetWMStateCodeName(u16 code)
{
    static const char *statenames[] = {
        "WM_STATECODE_PARENT_START",
        "N/A",
        "WM_STATECODE_BEACON_SENT",
        "WM_STATECODE_SCAN_START",
        "WM_STATECODE_PARENT_NOT_FOUND",
        "WM_STATECODE_PARENT_FOUND",
        "WM_STATECODE_CONNECT_START",
        "WM_STATECODE_CONNECTED",
        "WM_STATECODE_BEACON_LOST",
        "WM_STATECODE_DISCONNECTED",
        "WM_STATECODE_MP_START",
        "WM_STATECODE_MPEND_IND",
        "WM_STATECODE_MP_IND",
        "WM_STATECODE_MPACK_IND",
        "WM_STATECODE_DCF_START",
        "WM_STATECODE_DCF_IND",
        "WM_STATECODE_BEACON_RECV",
        "WM_STATECODE_DISASSOCIATE",
        "WM_STATECODE_REASSOCIATE",
        "WM_STATECODE_AUTHENTICATE",
        "WM_STATECODE_PORT_SEND",
        "WM_STATECODE_PORT_RECV",
        "WM_STATECODE_FIFO_ERROR",
        "WM_STATECODE_INFORMATION",
        "WM_STATECODE_UNKNOWN",
        "WM_STATECODE_PORT_INIT",
        "WM_STATECODE_DISCONNECTED_FROM_MYSELF",
    };

    if (0 <= code && code < sizeof(statenames) / sizeof(char *))
    {
        return statenames[code];
    }
    else
    {
        return "N/A";
    }
}

static const char *WH_GetWMApiidName(u16 apiid)
{
    static const char *apinames[] = {
        "WM_APIID_INITIALIZE",
        "WM_APIID_RESET",
        "WM_APIID_END",

        "WM_APIID_ENABLE",
        "WM_APIID_DISABLE",
        "WM_APIID_POWER_ON",
        "WM_APIID_POWER_OFF",

        "WM_APIID_SET_P_PARAM",
        "WM_APIID_START_PARENT",
        "WM_APIID_END_PARENT",
        "WM_APIID_START_SCAN",
        "WM_APIID_END_SCAN",
        "WM_APIID_START_CONNECT",
        "WM_APIID_DISCONNECT",
        "WM_APIID_START_MP",
        "WM_APIID_SET_MP_DATA",
        "WM_APIID_END_MP",
        "WM_APIID_START_DCF",
        "WM_APIID_SET_DCF_DATA",
        "WM_APIID_END_DCF",
        "WM_APIID_SET_WEPKEY",
        "WM_APIID_START_KS",
        "WM_APIID_END_KS",
        "WM_APIID_GET_KEYSET",
        "WM_APIID_SET_GAMEINFO",
        "WM_APIID_SET_BEACON_IND",
        "WM_APIID_START_TESTMODE",
        "WM_APIID_STOP_TESTMODE",
        "WM_APIID_VALARM_MP",
        "WM_APIID_SET_LIFETIME",
        "WM_APIID_MEASURE_CHANNEL",
        "WM_APIID_INIT_W_COUNTER",
        "WM_APIID_GET_W_COUNTER",
        "WM_APIID_SET_ENTRY",
        "WM_APIID_AUTO_DEAUTH",
        "WM_APIID_SET_MP_PARAMETER",
        "WM_APIID_SET_BEACON_PERIOD",
        "WM_APIID_AUTO_DISCONNECT",

        "WM_APIID_KICK_MP_PARENT",
        "WM_APIID_KICK_MP_CHILD",
        "WM_APIID_KICK_MP_RESUME",
    };
    static const char *apinames_indicates[] = {
        "WM_APIID_INDICATION",
        "WM_APIID_PORT_SEND",
        "WM_APIID_PORT_RECV",
        "WM_APIID_READ_STATUS"
    };
    static const char *apiname_unknown = "WM_APIID_UNKNOWN";

    if (0 <= apiid && apiid < sizeof(apinames) / sizeof(char *))
    {
        return apinames[apiid];
    }
    else if (WM_APIID_INDICATION <= apiid &&
             apiid < WM_APIID_INDICATION + sizeof(apinames_indicates) / sizeof(char *))
    {
        return apinames_indicates[apiid - WM_APIID_INDICATION];
    }
    else if (apiid == WM_APIID_UNKNOWN)
    {
        return apiname_unknown;
    }
    else
    {
        return "N/A";
    }
}

static void WH_OutputWMState(void)
{
    WMStatus s;

    if (WM_ReadStatus(&s) == WM_ERRCODE_SUCCESS)
    {
        WH_TRACE("state = %s\n", WH_GetWMStateCodeName(s.state));
        WH_TRACE("apiid = %s\n", WH_GetWMApiidName(s.BusyApiid));
    }
    else
    {
        WH_TRACE("WM not Initialized\n");
    }
}

static void WH_OutputBitmap(u16 bitmap)
{
    u16     i;
    for (i = 15; i >= 0; --i)
    {
        if ((bitmap >> i) & 0x01)
        {
            OS_TPrintf("o");
        }
        else
        {
            OS_TPrintf("-");
        }
    }

    OS_TPrintf("\n");
}

static void WH_ChangeSysState(int state)
{
    static const char *statenames[] = {
        "WH_SYSSTATE_STOP",
        "WH_SYSSTATE_IDLE",
        "WH_SYSSTATE_SCANNING",
        "WH_SYSSTATE_BUSY",
        "WH_SYSSTATE_CONNECTED",
        "WH_SYSSTATE_DATASHARING",
        "WH_SYSSTATE_KEYSHARING",
        "WH_SYSSTATE_MEASURECHANNEL",
        "WH_SYSSTATE_CONNECT_FAIL",
        "WH_SYSSTATE_ERROR",
    };

    WH_TRACE("%s -> ", statenames[sSysState]);
    sSysState = state;
    WH_TRACE("%s\n", statenames[sSysState]);
}

#else

#define WH_GetWMErrCodeName(result)    ("")
#define WH_GetWMStateCodeName(result)  ("")
#define WH_GetWMApiidName(result)      ("")

static void WH_OutputWMState(void)
{;
}
static void WH_OutputBitmap(void)
{;
}

static void WH_ChangeSysState(int state)
{
    sSysState = state;
}

#endif

static void WH_SetError(int code)
{
    // Does not overwrite if the system is already in an ERROR state.
    if (sSysState == WH_SYSSTATE_ERROR || sSysState == WH_SYSSTATE_FATAL)
    {
        return;
    }

    sErrCode = code;
}



/* ======================================================================
   State functions
   ====================================================================== */

/* ----------------------------------------------------------------------
   State: setparentparam
  ---------------------------------------------------------------------- */
static BOOL WH_StateInSetParentParam(void)
{
    // In this state, the game information held by the parent is passed to the ARM7
    WMErrCode result;
    WH_TRACE_STATE;

    WH_ChangeSysState(WH_SYSSTATE_BUSY);

    result = WM_SetParentParameter(WH_StateOutSetParentParam, &sParentParam);
    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
        return FALSE;
    }

    return TRUE;
}

static void WH_StateOutSetParentParam(void *arg)
{
    WMCallback *cb = (WMCallback *)arg;
    WH_TRACE_STATE;

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(cb->errcode);
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
        return;
    }

    if (sParentWEPKeyGenerator != NULL)
    {
        // If the WEP Key Generator is configured, set the WEP Key
        if (!WH_StateInSetParentWEPKey())
        {
            WH_ChangeSysState(WH_SYSSTATE_ERROR);
        }
    }
    else
    {
        // If progress is normal, moves to the StartParent state
        if (!WH_StateInStartParent())
        {
            WH_ChangeSysState(WH_SYSSTATE_ERROR);
        }
    }
}

static BOOL WH_StateInSetParentWEPKey(void)
{
    u16     wepmode;
    WMErrCode result;
    WH_TRACE_STATE;

    WH_ChangeSysState(WH_SYSSTATE_BUSY);

    wepmode = (*sParentWEPKeyGenerator) (sWEPKey, &sParentParam);
    result = WM_SetWEPKey(WH_StateOutSetParentWEPKey, wepmode, sWEPKey);
    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
        return FALSE;
    }

    return TRUE;
}

static void WH_StateOutSetParentWEPKey(void *arg)
{
    WMCallback *cb = (WMCallback *)arg;
    WH_TRACE_STATE;

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(cb->errcode);
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
        return;
    }

    // If progress is normal, moves to the StartParent state
    if (!WH_StateInStartParent())
    {
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
    }
}


/* ----------------------------------------------------------------------
   State: StartParent
  ---------------------------------------------------------------------- */
static BOOL WH_StateInStartParent(void)
{
    // In this state, the StartParent function is called, and the settings are started for a parent

    WMErrCode result;
    WH_TRACE_STATE;

    if ((sSysState == WH_SYSSTATE_CONNECTED)
        || (sSysState == WH_SYSSTATE_KEYSHARING) || (sSysState == WH_SYSSTATE_DATASHARING))
    {
        // For the above case, the parent settings should have already been set
        return TRUE;
    }

    result = WM_StartParent(WH_StateOutStartParent);

    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        return FALSE;
    }

    sMyAid = 0;
    sConnectBitmap = WH_BITMAP_EMPTY;

    return TRUE;
}

static void WH_StateOutStartParent(void *arg)
{
    // The callback function set by StartParent will be called for a number of cases, as follows, so they need to be differentiated from each other.
    // 1) A beacon is sent
    // 2) A new connection is made to a child
    // 3) A StartParent ends
    // 4) A child disconnection is detected
    //  

    WMStartParentCallback *cb = (WMStartParentCallback *)arg;
    const u16 target_bitmap = (u16)(1 << cb->aid);

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(cb->errcode);
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
        return;
    }

    switch (cb->state)
    {
        //-----------------------------------
        // Beacon transmission notification
    case WM_STATECODE_BEACON_SENT:
        // Ignores the beacon transmission notification
        break;

        //-----------------------------------
        // Child connection notification
    case WM_STATECODE_CONNECTED:
        {
            // The connected child device MAC address is in cb->macAddress.
            // cb->ssid is the data the child device set in the ssid argument of WM_StartConnect.
            WH_TRACE("StartParent - new child (aid 0x%x) connected\n", cb->aid);
            // Checks whether the newly connected child satisfies the conditions for connection permission
            if (sJudgeAcceptFunc != NULL)
            {
                if (!sJudgeAcceptFunc(cb))
                {
                    WMErrCode result;
                    // Terminates the connection
                    result = WM_Disconnect(NULL, cb->aid);
                    if (result != WM_ERRCODE_OPERATING)
                    {
                        WH_REPORT_FAILURE(result);
                        WH_ChangeSysState(WH_SYSSTATE_ERROR);
                    }
                    break;
                }
            }
            sConnectBitmap |= target_bitmap;
        }
        break;

        //-----------------------------------
        // Notifies of a child disconnection
    case WM_STATECODE_DISCONNECTED:
        {
            WH_TRACE("StartParent - child (aid 0x%x) disconnected\n", cb->aid);
            // The disconnected child device MAC address is in cb->macAddress
            sConnectBitmap &= ~target_bitmap;
        }
        break;

        //-----------------------------------
        // Child has disconnected from itself
    case WM_STATECODE_DISCONNECTED_FROM_MYSELF:
        {
            WH_TRACE("StartParent - child (aid 0x%x) disconnected from myself\n", cb->aid);
            // Does nothing if the unit disconnects from itself
            // The disconnected child device MAC address is in cb->macAddress
        }
        break;

        //-----------------------------------
        // StartParent processing ends
    case WM_STATECODE_PARENT_START:
        {
            // Changes to the MP communication state
            if (!WH_StateInStartParentMP())
            {
                WH_ChangeSysState(WH_SYSSTATE_ERROR);
            }
        }
        break;

        //-----------------------------------
    default:
        WH_TRACE("unknown indicate, state = %d\n", cb->state);
    }
}

/* ----------------------------------------------------------------------
   State: StartParentMP
  ---------------------------------------------------------------------- */
static BOOL WH_StateInStartParentMP(void)
{
    // Calls the WM_Start function and starts a connection using the MP communication protocol

    WMErrCode result;
    WH_TRACE_STATE;

    if ((sSysState == WH_SYSSTATE_CONNECTED)
        || (sSysState == WH_SYSSTATE_KEYSHARING) || (sSysState == WH_SYSSTATE_DATASHARING))
    {
        return TRUE;
    }

    WH_ChangeSysState(WH_SYSSTATE_CONNECTED);
    result = WM_StartMP(WH_StateOutStartParentMP,
                        (u16 *)sRecvBuffer,
                        (u16)sRecvBufferSize, (u16 *)sSendBuffer, (u16)sSendBufferSize,
                        WH_MP_FREQUENCY);

    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        return FALSE;
    }

    return TRUE;
}

static void WH_StateOutStartParentMP(void *arg)
{
    // The callback function specified by StartMP will be called in the following four cases, so they need to be differentiated from each other.
    // 1) When a mode starts up due to StartMP
    // 2) When the MP sequence ends (parent only)
    // 3) During MP reception (child only)
    // 4) When MP sequence completion notification (ACK receiving) is detected (child only)
    //  

    WMstartMPCallback *cb = (WMstartMPCallback *)arg;
    // WH_TRACE_STATE;

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(cb->errcode);
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
        return;
    }

    switch (cb->state)
    {
    case WM_STATECODE_MP_START:
        // Notification that StartMP has ended normally.
        // Hereafter, send and receive are enabled.

        if (sConnectMode == WH_CONNECTMODE_KS_PARENT)
        {
            // Transition to StartParentKeyShare if key-sharing was specified
            //  
            if (sSysState == WH_SYSSTATE_CONNECTED)
            {
                // Normal MP connection
                if (!WH_StateInStartParentKeyShare())
                {
                    WH_TRACE("WH_StateInStartParentKeyShare failed\n");
                    WH_ChangeSysState(WH_SYSSTATE_ERROR);
                }
                return;

            }
            else if (sSysState == WH_SYSSTATE_KEYSHARING)
            {
                // Appears that the key sharing state is already set
                return;
            }
        }
        else if (sConnectMode == WH_CONNECTMODE_DS_PARENT)
        {
            // StartDataSharing is called if data sharing is specified.
            // This function is a synchronous operating function, so a switch to WH state is not made.
            WMErrCode result;
            u16     aidBitmap;

            aidBitmap = (u16)((1 << (WH_CHILD_MAX + 1)) - 1);   // Bitmap with the lower WH_CHILD_MAX+1 bit==1
            result = WM_StartDataSharing(&sDSInfo, WH_DS_PORT, aidBitmap, WH_DS_DATA_SIZE, TRUE);

            if (result != WM_ERRCODE_SUCCESS)
            {
                WH_REPORT_FAILURE(result);
                WH_ChangeSysState(WH_SYSSTATE_ERROR);
                return;
            }
            // WH_TRACE("WM_StartDataSharing OK\n");
            WH_ChangeSysState(WH_SYSSTATE_DATASHARING);
            return;
        }

        WH_ChangeSysState(WH_SYSSTATE_CONNECTED);
        break;

    case WM_STATECODE_MPEND_IND:
        // Parent receiving compete notification

        // cb->recvBuf accepts the data frame content received from the child device, but for normal data reception set a callback with WM_SetPortCallback.
        //  
        // When using data sharing or key sharing, data is received internally, so WM_SetPortCallback does not need to be used.
        //  
        break;

    case WM_STATECODE_MP_IND:
    case WM_STATECODE_MPACK_IND:
        // The parent should not come here

    default:
        WH_TRACE("unknown indicate, state = %d\n", cb->state);
        break;
    }
}

/* ----------------------------------------------------------------------
  State: StartParentKeyShare
  ---------------------------------------------------------------------- */
static BOOL WH_StateInStartParentKeyShare(void)
{
    WMErrCode result;
    WH_TRACE_STATE;
    WH_ChangeSysState(WH_SYSSTATE_KEYSHARING);

    // Calls the WM_StartKeySharing function and starts key sharing
    result = WM_StartKeySharing(&sWMKeySetBuf, WH_DS_PORT);

    if (result != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(result);
        return FALSE;
    }

    return TRUE;
}

/* ----------------------------------------------------------------------
   State: EndParentKeyShare
  ---------------------------------------------------------------------- */
static BOOL WH_StateInEndParentKeyShare(void)
{
    // Calls WM_EndKeySharing and ends key sharing
    WMErrCode result;
    WH_TRACE_STATE;

    result = WM_EndKeySharing(&sWMKeySetBuf);

    if (result != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(result);
        return FALSE;
    }

    // Automatically starts MP communication termination processing
    if (!WH_StateInEndParentMP())
    {
        WH_TRACE("WH_StateInEndParentMP failed\n");
        WH_Reset();
        return FALSE;
    }

    return TRUE;
}

/* ----------------------------------------------------------------------
   State: EndParentMP
  ---------------------------------------------------------------------- */
static BOOL WH_StateInEndParentMP(void)
{
    WMErrCode result;
    WH_TRACE_STATE;

    // Hereafter, send and receive are disabled
    WH_ChangeSysState(WH_SYSSTATE_BUSY);

    result = WM_EndMP(WH_StateOutEndParentMP);
    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        return FALSE;
    }

    return TRUE;
}

static void WH_StateOutEndParentMP(void *arg)
{
    WMCallback *cb = (WMCallback *)arg;
    WH_TRACE_STATE;

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(cb->errcode);
        WH_Reset();
        return;
    }

    // Automatically starts termination processing
    if (!WH_StateInEndParent())
    {
        WH_TRACE("WH_StateInEndParent failed\n");
        WH_Reset();
        return;
    }
}

/* ----------------------------------------------------------------------
   State: EndParent
   ---------------------------------------------------------------------- */
static BOOL WH_StateInEndParent(void)
{
    WMErrCode result;
    WH_TRACE_STATE;

    // Operation as a parent ends here.
    // If there are connected children, parent operations end after the parent severs authentication for each child individually.
    //  
    result = WM_EndParent(WH_StateOutEndParent);
    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        return FALSE;
    }

    return TRUE;
}

static void WH_StateOutEndParent(void *arg)
{
    WMCallback *cb = (WMCallback *)arg;
    WH_TRACE_STATE;

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(cb->errcode);
        return;
    }

    // At this point, the parent disconnection processing ends, and the state returns to idle (standby)
    WH_ChangeSysState(WH_SYSSTATE_IDLE);
}

/* ----------------------------------------------------------------------
   Name:        WH_ChildConnectAuto
   Description: Starts the child device connection sequence.
                However, each type of setting specified with WH_ParentConnect or WH_ChildConnect is left to automatic internal processing.
                 
   Arguments:   mode:     Start MP as the child if WH_CONNECTMODE_MP_CHILD.
                          Start DataSharing as the child if WH_CONNECTMODE_DS_CHILD.
                          Start KeySharing as the child if WH_CONNECTMODE_KS_CHILD.

                macAddr:  MAC address for the connected parent
                          If 0xFFFFFF, detection is run for all parents.
                          
                channel:  Designate the channels on which to search for the parent.
                          If 0, all channels are searched.
                          
   ---------------------------------------------------------------------- */
BOOL WH_ChildConnectAuto(int mode, const u8 *macAddr, u16 channel)
{
    WH_TRACE_STATE;

    // Calculation of send and receive buffer size for WM_StartMP()
    // Use the WM_SIZE_MP_* function macros to statically allocate a receive buffer in advance. If the buffer can be allocated dynamically, use the WM_GetReceiveBufferSize function after the parent and child have been connected and immediately before calling the WM_StartMP function.
    //  
    //  
    // Similarly, use the WM_SIZE_MP_* function macros to statically allocate a send buffer in advance. If the buffer can be allocated dynamically, use the WM_GetSendBufferSize function after the parent and child have been connected and immediately before calling the WM_StartMP function.
    // 
    // 
    sRecvBufferSize = WH_CHILD_RECV_BUFFER_SIZE;
    sSendBufferSize = WH_CHILD_SEND_BUFFER_SIZE;

    WH_TRACE("recv buffer size = %d\n", sRecvBufferSize);
    WH_TRACE("send buffer size = %d\n", sSendBufferSize);

    WH_ChangeSysState(WH_SYSSTATE_SCANNING);

    // Detection starts in child mode
    sBssDesc.channel = 1;
    *(u16 *)(&sScanParam.bssid[4]) = *(u16 *)(macAddr + 4);
    *(u16 *)(&sScanParam.bssid[2]) = *(u16 *)(macAddr + 2);
    *(u16 *)(&sScanParam.bssid[0]) = *(u16 *)(macAddr + 0);

    sConnectMode = mode;

    sScanCallback = NULL;
    sChannelIndex = channel;
    sScanParam.channel = 0;
    sAutoConnectFlag = TRUE;

    if (!WH_StateInStartScan())
    {
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
        return FALSE;
    }

    return TRUE;
}


/*---------------------------------------------------------------------------*
  Name:         WH_TurnOnPictoCatch

  Description:  Enables the Pictochat Search feature.
                When enabled, the callback function is called even if Pictochat was found while scanning with WH_StartScan.
                

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WH_TurnOnPictoCatch(void)
{
    sPictoCatchFlag = TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         WH_TurnOffPictoCatch

  Description:  Disables the Pictochat Search feature.
                This makes it to be ignored, even if Pictochat was found while scanning with WH_StartScan.
                

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WH_TurnOffPictoCatch(void)
{
    sPictoCatchFlag = FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         WH_StartScan

  Description:  Function for fetching the parent beacon

  Arguments:    callback:  Sets the callback returned when the parent device is found
                
                macAddr:   MAC address for the connected parent
                           If 0xFFFFFF, detection is run for all parents.
                           
                channel:   Designate the channels on which to search for the parent.
                           If 0, all channels are searched.

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL WH_StartScan(WHStartScanCallbackFunc callback, const u8 *macAddr, u16 channel)
{
    WH_TRACE_STATE;
    WH_ASSERT(sSysState == WH_SYSSTATE_IDLE);

    WH_ChangeSysState(WH_SYSSTATE_SCANNING);

    sScanCallback = callback;
    sChannelIndex = channel;
    sScanParam.channel = 0;
    sAutoConnectFlag = FALSE;          // Does not automatically connect

    // Sets the conditions for the MAC address to be searched for
    *(u16 *)(&sScanParam.bssid[4]) = *(u16 *)(macAddr + 4);
    *(u16 *)(&sScanParam.bssid[2]) = *(u16 *)(macAddr + 2);
    *(u16 *)(&sScanParam.bssid[0]) = *(u16 *)(macAddr);

    if (!WH_StateInStartScan())
    {
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
        return FALSE;
    }

    return TRUE;
}

/* ----------------------------------------------------------------------
  State: StartScan
  ---------------------------------------------------------------------- */
static BOOL WH_StateInStartScan(void)
{
    // When in this state, looks for a parent
    WMErrCode result;
    u16     chanpat;

    WH_ASSERT(sSysState == WH_SYSSTATE_SCANNING);

    chanpat = WM_GetAllowedChannel();

    // Checks if wireless is usable
    if (chanpat == 0x8000)
    {
        // If 0x8000 is returned, it indicates that wireless is not initialized or there is some other abnormality with the wireless library. Therefore, set to an error.
        //  
        WH_REPORT_FAILURE(WM_ERRCODE_ILLEGAL_STATE);
        return FALSE;
    }
    if (chanpat == 0)
    {
        // In this state, wireless cannot be used
        WH_REPORT_FAILURE(WH_ERRCODE_NO_RADIO);
        return FALSE;
    }

    if (sChannelIndex == 0)
    {
        /* Search possible channels in ascending order from the current designation */
        while (TRUE)
        {
            sScanParam.channel++;
            if (sScanParam.channel > 16)
            {
                sScanParam.channel = 1;
            }

            if (chanpat & (0x0001 << (sScanParam.channel - 1)))
            {
                break;
            }
        }
    }
    else
    {
        sScanParam.channel = (u16)sChannelIndex;
    }

    sScanParam.maxChannelTime = WM_GetDispersionScanPeriod();
    sScanParam.scanBuf = &sBssDesc;
    result = WM_StartScan(WH_StateOutStartScan, &sScanParam);

    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        return FALSE;
    }
    return TRUE;
}


static void WH_StateOutStartScan(void *arg)
{
    WMstartScanCallback *cb = (WMstartScanCallback *)arg;

    // If the scan command fails
    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(cb->errcode);
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
        return;
    }

    if (sSysState != WH_SYSSTATE_SCANNING)
    {
        // End scan if the state has been changed
        sAutoConnectFlag = FALSE;      // Cancel the automatic connection
        if (!WH_StateInEndScan())
        {
            WH_ChangeSysState(WH_SYSSTATE_ERROR);
        }
        return;
    }

    switch (cb->state)
    {
    case WM_STATECODE_SCAN_START:
        return;

    case WM_STATECODE_PARENT_NOT_FOUND:
        break;

    case WM_STATECODE_PARENT_FOUND:
        // If a parent is found
        // GUIDELINE: Guideline compliance items (6.3.5)
        // Compares ggids and fails if different.
        // First, you must confirm WMBssDesc.gameInfoLength, and check to see if ggid has a valid value.
        //  

        WH_TRACE("WH_StateOutStartScan : MAC=%02x%02x%02x%02x%02x%02x ",
                 cb->macAddress[0],
                 cb->macAddress[1],
                 cb->macAddress[2], cb->macAddress[3], cb->macAddress[4], cb->macAddress[5]);

        // Discards the BssDesc cache that is set in the buffer because the BssDesc information is written from the ARM7
        //  
        DC_InvalidateRange(&sBssDesc, sizeof(WMbssDesc));

        // Determines if the parent is using Pictochat
        if (sPictoCatchFlag)
        {
            /* TODO: Remove the commenting out once the CHT library migration is complete */
/*            if (CHT_IsPictochatParent(&sBssDesc))
            {
                // If the parent is using Pictochat
                WH_TRACE("pictochat parent find\n");
                if (sScanCallback != NULL)
                {
                    sScanCallback(&sBssDesc);
                }
                break;
            }
*/
        }

        if ((!WM_IsValidGameInfo(&cb->gameInfo, cb->gameInfoLength))
            || cb->gameInfo.ggid != sParentParam.ggid)
        {
            // If GGIDs are different, this is ignored
            WH_TRACE("not my parent ggid \n");
            break;
        }

        // If the entry flag is not up, the child is not receiving, so this is ignored
        // Also, if the multiboot flag is enabled, it is the DS download parent device, so ignore.
        if ((cb->gameInfo.gameNameCount_attribute & (WM_ATTR_FLAG_ENTRY | WM_ATTR_FLAG_MB))
            != WM_ATTR_FLAG_ENTRY)
        {
            WH_TRACE("not recieve entry\n");
            break;
        }

        WH_TRACE("parent find\n");

        // Call if callback is necessary
        if (sScanCallback != NULL)
        {
            sScanCallback(&sBssDesc);
        }

        // End scan for automatic connection to found parent
        if (sAutoConnectFlag)
        {
            if (!WH_StateInEndScan())
            {
                WH_ChangeSysState(WH_SYSSTATE_ERROR);
            }
            return;
        }
        break;
    }

    // Changes the channel and starts another scan
    if (!WH_StateInStartScan())
    {
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
    }
}

/* ----------------------------------------------------------------------
   State : EndScan
  ---------------------------------------------------------------------- */

/*---------------------------------------------------------------------------*
  Name:         WH_EndScan

  Description:  Fetches the parent beacon.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL WH_EndScan(void)
{
    if (sSysState != WH_SYSSTATE_SCANNING)
    {
        return FALSE;
    }

    sAutoConnectFlag = FALSE;
    WH_ChangeSysState(WH_SYSSTATE_BUSY);
    return TRUE;
}


static BOOL WH_StateInEndScan(void)
{
    WMErrCode result;
    WH_TRACE_STATE;

    // In this state, scan end processing is carried out
    result = WM_EndScan(WH_StateOutEndScan);
    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        return FALSE;
    }

    return TRUE;
}

static void WH_StateOutEndScan(void *arg)
{
    WMCallback *cb = (WMCallback *)arg;
    WH_TRACE_STATE;

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(cb->errcode);
        return;
    }

    WH_ChangeSysState(WH_SYSSTATE_IDLE);

    if (!sAutoConnectFlag)
    {
        return;
    }

    if (sChildWEPKeyGenerator != NULL)
    {
        // If the WEP Key Generator is configured, set the WEP Key
        if (!WH_StateInSetChildWEPKey())
        {
            WH_ChangeSysState(WH_SYSSTATE_ERROR);
        }
    }
    else
    {
        // The end of scan processing has finished, so begin child operations unchanged
        //  
        if (!WH_StateInStartChild())
        {
            WH_TRACE("WH_StateOutEndScan : startchild failed\n");
            WH_ChangeSysState(WH_SYSSTATE_ERROR);
        }
    }
}

static BOOL WH_StateInSetChildWEPKey(void)
{
    u16     wepmode;
    WMErrCode result;
    WH_TRACE_STATE;

    WH_ChangeSysState(WH_SYSSTATE_BUSY);

    wepmode = (*sChildWEPKeyGenerator) (sWEPKey, &sBssDesc);
    result = WM_SetWEPKey(WH_StateOutSetChildWEPKey, wepmode, sWEPKey);
    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
        return FALSE;
    }

    return TRUE;
}

static void WH_StateOutSetChildWEPKey(void *arg)
{
    WMCallback *cb = (WMCallback *)arg;
    WH_TRACE_STATE;

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(cb->errcode);
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
        return;
    }

    // Connect to parent as a child
    if (!WH_StateInStartChild())
    {
        WH_TRACE("WH_StateOutSetChildWEPKey : startchild failed\n");
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
    }
}


/* ----------------------------------------------------------------------
   State: StartChild
  ---------------------------------------------------------------------- */
static BOOL WH_StateInStartChild(void)
{
    WMErrCode result;
    WH_TRACE_STATE;

    if ((sSysState == WH_SYSSTATE_CONNECTED)
        || (sSysState == WH_SYSSTATE_KEYSHARING) || (sSysState == WH_SYSSTATE_DATASHARING))
    {
        // Already connected
        WH_TRACE("WH_StateInStartChild : already connected?\n");
        return TRUE;
    }

    WH_ChangeSysState(WH_SYSSTATE_BUSY);

    result = WM_StartConnectEx(WH_StateOutStartChild, &sBssDesc, sConnectionSsid, TRUE,
                               (u16)((sChildWEPKeyGenerator !=
                                      NULL) ? WM_AUTHMODE_SHARED_KEY : WM_AUTHMODE_OPEN_SYSTEM));
    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        return FALSE;
    }

    return TRUE;
}

static void WH_StateOutStartChild(void *arg)
{
    // The callback set by StartConnect will be invoked in several cases, as follows, so a distinction needs to be made between them.
    // 1) When the connection processing begins
    // 2) When authentication ends
    // 3) When the parent disconnects after a connection is complete
    // 4) When more than the maximum number of devices try to connect
    //  
    // For this function, advancing to the next item is possible only with 2).

    WMStartConnectCallback *cb = (WMStartConnectCallback *)arg;
    WH_TRACE_STATE;

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(cb->errcode);

        if (cb->errcode == WM_ERRCODE_OVER_MAX_ENTRY)
        {
            // GUIDELINE: Guideline compliance items (6.3.7)
            // If there are more children than the parent can handle.
            // Here, this qualifies as a fatal error.
            // At this time, something needs to be displayed by main.
            // The WH_GetLastError function can be used to check whether the error code was WM_ERRCODE_OVER_MAX_ENTRY.
            //  
            WH_ChangeSysState(WH_SYSSTATE_ERROR);
            return;
        }
        else if (cb->errcode == WM_ERRCODE_NO_ENTRY)
        {
            // The case wherein the local device (a child) tried to connect to the parent, but the parent was not accepting entries.
            //  
            // Here, this qualifies as a fatal error.
            // At this time, something needs to be displayed by main.
            // The WH_GetLastError function can be used to check whether the error code was WM_ERRCODE_NO_ENTRY.
            //  
            WH_ChangeSysState(WH_SYSSTATE_ERROR);
            return;
        }
        else if (cb->errcode == WM_ERRCODE_FAILED)
        {
            // The case wherein the connection request timed out, for example because the parent to which the local device (a child) tried to connect no longer exists.
            //  
            // Here, this qualifies as a fatal error.
            // At this time, either something needs to be displayed or the connection needs to be retried after a reset is performed by the main process.
            //  
            // The WH_GetLastError function can be used to check whether the error code was WM_ERRCODE_FAILED.
            //  
            WH_ChangeSysState(WH_SYSSTATE_CONNECT_FAIL);
            return;
        }
        else
        {
            // If the parent's channel setting is invalid, WM_ERRCODE_INVALID_PARAM may be returned
            //  
            WH_ChangeSysState(WH_SYSSTATE_ERROR);
        }
        return;
    }

    if (cb->state == WM_STATECODE_BEACON_LOST)
    {
        // Beacon reception failed after 16 attempts from the parent being connected.
        // If the beacon is lost, VBlank synchronization may be lost and the local host may not be able to notice when the parent has started the next session (that is, when the TGID has changed).
        //  
        //  

        // Nothing is executed in the demo.
        return;
    }

    if (cb->state == WM_STATECODE_CONNECTED)
    {
        // When authentication is complete.
        // Self-allocated AID goes in cb->aid
        WH_TRACE("Connect to Parent\n");
        WH_ChangeSysState(WH_SYSSTATE_CONNECTED);
        if (!WH_StateInStartChildMP())
        {
            /* FIXME : Is it OK to put this here while BUSY? */
            WH_TRACE("WH_StateInStartChildMP failed\n");
            WH_ChangeSysState(WH_SYSSTATE_BUSY);
            return;
        }

        // Saves own aid
        sMyAid = cb->aid;
        return;

    }
    else if (cb->state == WM_STATECODE_CONNECT_START)
    {
        // When the connection processing begins.
        // The following values are returned in cb->errcode: WM_ERRCODE_FAILED if the parent is no longer present (and in similar situations); WM_ERRCODE_NO_ENTRY if the parent is not accepting entries; and WM_ERRCODE_OVER_MAX_ENTRY if the parent has reached its connection capacity.
        //  
        //  
        //  
        // The process does nothing and waits for authentication to end.
        return;

    }
    else if (cb->state == WM_STATECODE_DISCONNECTED)
    {
        // GUIDELINE: Guideline compliance items (6.3.1)
        // When disconnected by the parent.
        // Processing like that above (6.3.7) is needed (to display something).
        // The error code here is WH_ERRCODE_DISCONNECTED
        // (Note that it is not 'WM_'.)
        // This situation (parent-initiated disconnection) may happen in the course of an ordinary game, but for now this is treated as an error to be handled by the main process.
        //  
        //  

        WH_TRACE("Disconnected from Parent\n");
        WH_SetError(WH_ERRCODE_DISCONNECTED);
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
        return;
    }
    else if (cb->state == WM_STATECODE_DISCONNECTED_FROM_MYSELF)
    {
        // Does nothing if the unit disconnects from itself
        return;
    }

    WH_TRACE("unknown state %d, %s\n", cb->state, WH_GetWMStateCodeName(cb->state));
    WH_ChangeSysState(WH_SYSSTATE_ERROR);
}

/* ----------------------------------------------------------------------
   State: StartChildMP
   ---------------------------------------------------------------------- */
static BOOL WH_StateInStartChildMP(void)
{
    WMErrCode result;
    WH_TRACE_STATE;

    result = WM_StartMP(WH_StateOutStartChildMP,
                        (u16 *)sRecvBuffer,
                        (u16)sRecvBufferSize, (u16 *)sSendBuffer, (u16)sSendBufferSize,
                        WH_MP_FREQUENCY);

    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        return FALSE;
    }

    return TRUE;
}

static void WH_StateOutStartChildMP(void *arg)
{
    WMstartMPCallback *cb = (WMstartMPCallback *)arg;
    // WH_TRACE_STATE;

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {

        if (cb->errcode == WM_ERRCODE_SEND_FAILED)
        {
            // The case wherein the parent had a receive error and sent notification via the MPACK frame of WM_STATECODE_MPACK_IND.
            //  
            // Since retransmission and other processing is handled in a separate location, nothing in particular needs to be done here. 
            //  This is for debugging.
            return;

        }
        else if (cb->errcode == WM_ERRCODE_TIMEOUT)
        {
            // In this case, the MPACK frame could not be received even after waiting a fixed period of time after receiving the MP frame.
            //  (cb->state == WM_STATECODE_MPACK_IND)
            // Since retransmission and other processing is handled in a separate location, nothing in particular needs to be done here.
            //  This is for debugging.
            return;

        }
        else if (cb->errcode == WM_ERRCODE_INVALID_POLLBITMAP)
        {
            // The case wherein a frame sent to a device other than the local host was received by the local host during WM_STATECODE_MP_IND or WM_STATECODE_MPACK_IND.
            //  
            // This occurs often during communications with 3 or more devices, so it cannot be treated as a fatal error.
            //  
            // Nothing in particular needs to be done here. This is for debugging.
            return;
        }

        WH_REPORT_FAILURE(cb->errcode);
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
        return;
    }

    switch (cb->state)
    {
    case WM_STATECODE_MP_START:
        // Notification that the StartMP has ended normally.
        // Hereafter, send and receive are enabled.

        if (sConnectMode == WH_CONNECTMODE_KS_CHILD)
        {
            // If key sharing was specified.
            if (sSysState == WH_SYSSTATE_KEYSHARING)
            {
                // The key sharing state is already set, so nothing happens.
                return;
            }

            if (sSysState == WH_SYSSTATE_CONNECTED)
            {
                // Switches to StartChildKeyShare.
                if (!WH_StateInStartChildKeyShare())
                {
                    WH_TRACE("WH_StateInStartChildKeyShare failed\n");
                    (void)WH_Finalize();
                }
                return;
            }

        }
        else if (sConnectMode == WH_CONNECTMODE_DS_CHILD)
        {
            // Call WM_StartDataSharing if data sharing is specified.
            //  This function is synchronous, so no switch has been made to a WH state.
            WMErrCode result;
            u16     aidBitmap;

            aidBitmap = (u16)((1 << (WH_CHILD_MAX + 1)) - 1);   // Bitmap with the lower WH_CHILD_MAX+1 bit==1
            result = WM_StartDataSharing(&sDSInfo, WH_DS_PORT, aidBitmap, WH_DS_DATA_SIZE, TRUE);
            if (result != WM_ERRCODE_SUCCESS)
            {
                WH_REPORT_FAILURE(result);
                (void)WH_Finalize();
                return;
            }

            WH_TRACE("WH_StateOutStartChildMP : WM_StartDataSharing OK\n");
            WH_ChangeSysState(WH_SYSSTATE_DATASHARING);
            return;
        }

        WH_ChangeSysState(WH_SYSSTATE_CONNECTED);
        break;

    case WM_STATECODE_MP_IND:
        // The child receiving is complete.

        // The local device can accept the data frame content received from the parent device via cb->recvBuf, but for normal data reception, set a callback with WM_SetPortCallback.
        //  
        // When using data sharing or key sharing, data is received internally, so WM_SetPortCallback does not need to be used.
        //  

        break;

    case WM_STATECODE_MPACK_IND:
        // Notification of the MPACK frame. This is for debugging.
        break;

    case WM_STATECODE_MPEND_IND:
        // The child should not come here.

    default:
        WH_TRACE("unknown indicate, state = %d\n", cb->state);
        break;
    }
}

/* ----------------------------------------------------------------------
   State: StartChildKeyShare
  ---------------------------------------------------------------------- */
static BOOL WH_StateInStartChildKeyShare(void)
{
    WMErrCode result;
    WH_TRACE_STATE;

    if (sSysState == WH_SYSSTATE_KEYSHARING)
    {
        // Already key sharing
        return TRUE;
    }

    if (sSysState != WH_SYSSTATE_CONNECTED)
    {
        // Not connected
        return FALSE;
    }

    WH_ChangeSysState(WH_SYSSTATE_KEYSHARING);
    result = WM_StartKeySharing(&sWMKeySetBuf, WH_DS_PORT);

    if (result != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(result);
        return FALSE;
    }

    return TRUE;
}

/* ----------------------------------------------------------------------
   State: EndChildKeyShare
   ---------------------------------------------------------------------- */
static BOOL WH_StateInEndChildKeyShare(void)
{
    // Ends key sharing
    WMErrCode result;
    WH_TRACE_STATE;

    if (sSysState != WH_SYSSTATE_KEYSHARING)
    {
        return FALSE;
    }

    WH_ChangeSysState(WH_SYSSTATE_BUSY);
    result = WM_EndKeySharing(&sWMKeySetBuf);

    if (result != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(result);
        return FALSE;
    }

    if (!WH_StateInEndChildMP())
    {
        return FALSE;
    }

    return TRUE;
}

/* ----------------------------------------------------------------------
   State: EndChildMP
  ---------------------------------------------------------------------- */
static BOOL WH_StateInEndChildMP(void)
{
    // Ends MP communication
    WMErrCode result;
    WH_TRACE_STATE;

    WH_ChangeSysState(WH_SYSSTATE_BUSY);

    result = WM_EndMP(WH_StateOutEndChildMP);
    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        return FALSE;
    }
    return TRUE;
}

static void WH_StateOutEndChildMP(void *arg)
{
    WMCallback *cb = (WMCallback *)arg;
    WH_TRACE_STATE;

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(cb->errcode);
        (void)WH_Finalize();
        return;
    }

    if (!WH_StateInEndChild())
    {
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
    }
}

/* ----------------------------------------------------------------------
  State: EndChild
  ---------------------------------------------------------------------- */
static BOOL WH_StateInEndChild(void)
{
    WMErrCode result;
    WH_TRACE_STATE;

    WH_ChangeSysState(WH_SYSSTATE_BUSY);

    // Terminates the connection with the parent
    result = WM_Disconnect(WH_StateOutEndChild, 0);
    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        WH_Reset();
        return FALSE;
    }

    return TRUE;
}

static void WH_StateOutEndChild(void *arg)
{
    WMCallback *cb = (WMCallback *)arg;
    WH_TRACE_STATE;
    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(cb->errcode);
        return;
    }
    // At this point, the child disconnection processing ends, and the state returns to idle (standby)
    WH_ChangeSysState(WH_SYSSTATE_IDLE);
}

/* ----------------------------------------------------------------------
  State: Reset
  ---------------------------------------------------------------------- */
static BOOL WH_StateInReset(void)
{
    // This state is common to parents and children.
    // This restores the system to its initial state.
    WMErrCode result;
    WH_TRACE_STATE;

    WH_ChangeSysState(WH_SYSSTATE_BUSY);
    result = WM_Reset(WH_StateOutReset);
    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        return FALSE;
    }
    return TRUE;
}

static void WH_StateOutReset(void *arg)
{
    WMCallback *cb = (WMCallback *)arg;
    WH_TRACE_STATE;
    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
        WH_REPORT_FAILURE(cb->errcode);
        return;
    }
    // Reset sets to the idling (standby) state without starting the next state
    WH_ChangeSysState(WH_SYSSTATE_IDLE);
}

/* ----------------------------------------------------------------------
   Disconnect
  ---------------------------------------------------------------------- */
static BOOL WH_StateInDisconnectChildren(u16 bitmap)
{
    // In this state, the connection with the child set with the argument is cut
    WMErrCode result;
    WH_TRACE_STATE;

    result = WM_DisconnectChildren(WH_StateOutDisconnectChildren, bitmap);

    if (result == WM_ERRCODE_NO_CHILD)
    {
        return FALSE;
    }

    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        return FALSE;
    }

    {
        OSIntrMode enabled = OS_DisableInterrupts();
        sConnectBitmap &= ~bitmap;
        (void)OS_RestoreInterrupts(enabled);
    }
    return TRUE;
}

static void WH_StateOutDisconnectChildren(void *arg)
{
    WMCallback *cb = (WMCallback *)arg;
    WH_TRACE_STATE;
    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(cb->errcode);
        return;
    }
}

/* ----------------------------------------------------------------------
   Power off
  ---------------------------------------------------------------------- */
static BOOL WH_StateInPowerOff(void)
{
    // Ends the power supply to the wireless hardware.
    // This state is common to parents and children.
    WMErrCode result;
    WH_TRACE_STATE;

    result = WM_PowerOff(WH_StateOutPowerOff);
    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        return FALSE;
    }
    return TRUE;
}

static void WH_StateOutPowerOff(void *arg)
{
    // This state is after power disconnect
    WMCallback *cb = (WMCallback *)arg;
    WH_TRACE_STATE;

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(cb->errcode);
        return;
    }

    if (!WH_StateInDisable())
    {
        WH_ChangeSysState(WH_SYSSTATE_FATAL);
    }
}

/* ----------------------------------------------------------------------
   Disable
  ---------------------------------------------------------------------- */
static BOOL WH_StateInDisable(void)
{
    // Communicates that the wireless hardware use has ended.
    // This state is common to parents and children.
    WMErrCode result;
    WH_TRACE_STATE;

    result = WM_Disable(WH_StateOutDisable);
    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        return FALSE;
    }
    return TRUE;
}

static void WH_StateOutDisable(void *arg)
{
    // Everything has ended
    WMCallback *cb = (WMCallback *)arg;
    WH_TRACE_STATE;

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(cb->errcode);
    }
}

/* ----------------------------------------------------------------------
  State: SetMPData
  ---------------------------------------------------------------------- */
static BOOL WH_StateInSetMPData(void *data, u16 datasize, WHSendCallbackFunc callback)
{
    // This state is common to parents and children.
    // Sets and sends the data.
    WMErrCode result;
    // WH_TRACE_STATE;

    DC_FlushRange(sSendBuffer, (u32)sSendBufferSize);
    /* The I/O register is accessed using PXI operation, so cache wait is not needed */
    // DC_WaitWriteBufferEmpty();
    result = WM_SetMPDataToPortEx(WH_StateOutSetMPData,
                                  (void *)callback,
                                  data, datasize, 0xffff, WH_DATA_PORT, WH_DATA_PRIO);
    if (result != WM_ERRCODE_OPERATING)
    {
        WH_TRACE("WH_StateInSetMPData failed - %s\n", WH_GetWMErrCodeName(result));
        return FALSE;
    }
    return TRUE;
}

static void WH_StateOutSetMPData(void *arg)
{
    WMPortSendCallback *cb = (WMPortSendCallback *)arg;
    // WH_TRACE_STATE;

    // You must not overwrite the memory region for the transmission data specified with SetMPDataToPort until this callback is invoked.
    //  

    // WM_ERRCODE_SEND_FAILED is returned if ports 0-7 are used and the transmission fails.
    //  
    // WM_ERRCODE_SEND_QUEUE_FULL is returned if the transmission queue was full.
    //  

    if (cb->errcode != WM_ERRCODE_SUCCESS && cb->errcode != WM_ERRCODE_SEND_FAILED)
    {
        WH_REPORT_FAILURE(cb->errcode);
        return;
    }

    if (cb->arg != NULL)
    {
        WHSendCallbackFunc callback = (WHSendCallbackFunc) cb->arg;
        // Change the WHSendCallbackFunc type as necessary to cause cb->aid, cb->data, or other information to be received.
        //  
        (*callback) ((cb->errcode == WM_ERRCODE_SUCCESS));
    }
}

static void WH_PortReceiveCallback(void *arg)
{
    WMPortRecvCallback *cb = (WMPortRecvCallback *)arg;

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(cb->errcode);
    }
    else if (sReceiverFunc != NULL)
    {
        if (cb->state == WM_STATECODE_PORT_INIT)
        {
            // Does nothing during initialization.
            // The AIDs of the other parties connected at time of initialization are set in cb->aidBitmap.
            // 
        }
        if (cb->state == WM_STATECODE_PORT_RECV)
        {
            // Data was received, so the callback is called.
            (*sReceiverFunc) (cb->aid, cb->data, cb->length);
        }
        else if (cb->state == WM_STATECODE_DISCONNECTED)
        {
            // The fact that there was a disconnection is communicated with NULL transmission.
            (*sReceiverFunc) (cb->aid, NULL, 0);
        }
        else if (cb->state == WM_STATECODE_DISCONNECTED_FROM_MYSELF)
        {
            // For backward compatibility, no notification is made regarding self-disconnection.
        }
        else if (cb->state == WM_STATECODE_CONNECTED)
        {
            // Do nothing when connected.
            // The AIDs of newly connected peers are set in cb->aid and the MAC addresses of peers are set in cb->macAddress.
            // 
            // 
        }
    }
}


/* ----------------------------------------------------------------------
  State: End
  ---------------------------------------------------------------------- */

/* ----------------------------------------------------------------------
  State: WM_End
  ---------------------------------------------------------------------- */
static void WH_StateOutEnd(void *arg)
{
    WMCallback *cb = (WMCallback *)arg;
    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_ChangeSysState(WH_SYSSTATE_FATAL);
        return;
    }
    WH_ChangeSysState(WH_SYSSTATE_STOP);
}


/* ======================================================================
   Public Interfaces
   ====================================================================== */


/**************************************************************************
 * The functions below alter each type of WH configuration values
 **************************************************************************/

/*---------------------------------------------------------------------------*
  Name:         WH_SetGgid

  Description:  Configures the game group ID.
                Call before making a connection to the parent device.

  Arguments:    ggid:   The game group ID to configure

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WH_SetGgid(u32 ggid)
{
    sParentParam.ggid = ggid;
}

/*---------------------------------------------------------------------------*
  Name:         WH_SetSsid

  Description:  Sets the SSID specified during child connection.
                Call before making a connection from the child device.

  Arguments:    ssid:   Buffer in which the SSID to be set has been stored
                length: Data length of the SSID to be set.
                        If less than WM_SIZE_CHILD_SSID (24-byte), fill the subsequent whitespace with 0s. If more than WM_SIZE_CHILD_SSID, truncate.
                        

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WH_SetSsid(const void *ssid, u32 length)
{
    length = (u32)MATH_MIN(length, WM_SIZE_CHILD_SSID);
    MI_CpuCopy8(ssid, sConnectionSsid, length);
    MI_CpuClear8(sConnectionSsid + length, (u32)(WM_SIZE_CHILD_SSID - length));
}

/*---------------------------------------------------------------------------*
  Name:         WH_SetUserGameInfo

  Description:  Configures user-defined parent device information
                Call before making a connection to the parent device.

  Arguments:    userGameInfo:  Pointer to user-defined parent device information
                length:        Size of the user-defined parent device information

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WH_SetUserGameInfo(u16 *userGameInfo, u16 length)
{
    SDK_ASSERT(length <= WM_SIZE_USER_GAMEINFO);
    SDK_ASSERT((userGameInfo != NULL) || (length > 0));
    SDK_ASSERT(sSysState == WH_SYSSTATE_IDLE);

    // Specify user definition data here if to be used in the beacon.
    // To display a parent device's nickname on the child device's parent selection screen (or for some other situation), set the information here and the parent will convey it to the child.
    //  
    sParentParam.userGameInfo = userGameInfo;
    sParentParam.userGameInfoLength = length;
}

/*---------------------------------------------------------------------------*
  Name:         WH_SetDebugOutput

  Description:  Configures the function for outputting the debug string.

  Arguments:    func:   Function for the set debug string output

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WH_SetDebugOutput(void (*func) (const char *, ...))
{
    OSIntrMode enabled = OS_DisableInterrupts();
    wh_trace = func;
    (void)OS_RestoreInterrupts(enabled);
}


/**************************************************************************
 * Wrapper functions for fetching the WM library state are given below
 **************************************************************************/

/* ----------------------------------------------------------------------
  Name:        WH_GetLinkLevel
  Description: Gets the radio reception strength.
  Arguments:   None.
  Returns:     Numeric value of WMLinkLevel.
  ---------------------------------------------------------------------- */
int WH_GetLinkLevel(void)
{
    return (int)WM_GetLinkLevel();
}

/* ----------------------------------------------------------------------
   Name:        WH_GetAllowedChannel
   Description: Gets the bit pattern of channels that can be used for connection.
   Arguments:   None.
   Returns:     Channel pattern
   ---------------------------------------------------------------------- */
u16 WH_GetAllowedChannel(void)
{
    return WM_GetAllowedChannel();
}


/**************************************************************************
 * The following functions get the WH state
 **************************************************************************/

/* ----------------------------------------------------------------------
   Name:        WH_GetBitmap
   Description: Gets the bit pattern for displaying the connection state.
   Arguments:   None.
   Returns:     Bitmap pattern
   ---------------------------------------------------------------------- */
u16 WH_GetBitmap(void)
{
    return sConnectBitmap;
}

/* ----------------------------------------------------------------------
   Name:        WH_GetSystemState
   Description: Gets the WH internal state.
   Arguments:   None.
   Returns:     The internal state (WH_SYSSTATE_XXXX).
   ---------------------------------------------------------------------- */
int WH_GetSystemState(void)
{
    return sSysState;
}

/* ----------------------------------------------------------------------
   Name:        WH_GetConnectMode
   Description: Gets the connection information.
   Arguments:   None.
   Returns:     The connection information (WH_CONNECTMODE_XX_XXXX).
   ---------------------------------------------------------------------- */
int WH_GetConnectMode(void)
{
    return sConnectMode;
}

/* ----------------------------------------------------------------------
   Name:        WH_GetLastError
   Description: Gets the error code that most recently occurred.
   Arguments:   None.
   Returns:     The error code.
   ---------------------------------------------------------------------- */
int WH_GetLastError(void)
{
    return sErrCode;
}

/*---------------------------------------------------------------------------*
  Name:         WH_PrintBssDesc

  Description:  Debug outputs the members of the WMBssDesc structure.

  Arguments:    info:   Pointer to the BssDesc to be debug output

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WH_PrintBssDesc(WMBssDesc *info)
{
#pragma unused( info )
    u16     i;

    OS_TPrintf("length = 0x%04x\n", info->length);
    OS_TPrintf("rssi   = 0x%04x\n", info->rssi);
    OS_TPrintf("bssid = %02x%02x%02x%02x%02x%02x\n", info->bssid[0], info->bssid[1], info->bssid[2],
               info->bssid[3], info->bssid[4], info->bssid[5]);
    OS_TPrintf("ssidLength = 0x%04x\n", info->ssidLength);
    OS_TPrintf("ssid = ");
    for (i = 0; i < 32; i++)
    {
        OS_TPrintf("0x%02x", info->ssid[i]);
    }
    OS_TPrintf("\n");
    OS_TPrintf("capaInfo        = 0x%04x\n", info->capaInfo);
    OS_TPrintf("rateSet.basic   = 0x%04x\n", info->rateSet.basic);
    OS_TPrintf("rateSet.support = 0x%04x\n", info->rateSet.support);
    OS_TPrintf("beaconPeriod    = 0x%04x\n", info->beaconPeriod);
    OS_TPrintf("dtimPeriod      = 0x%04x\n", info->dtimPeriod);
    OS_TPrintf("channel         = 0x%04x\n", info->channel);
    OS_TPrintf("cfpPeriod       = 0x%04x\n", info->cfpPeriod);
    OS_TPrintf("cfpMaxDuration  = 0x%04x\n", info->cfpMaxDuration);
    OS_TPrintf("gameInfoLength  = 0x%04x\n", info->gameInfoLength);
    OS_TPrintf("gameInfo.magicNumber = 0x%04x\n", info->gameInfo.magicNumber);
    OS_TPrintf("gameInfo.ver    = 0x%02x\n", info->gameInfo.ver);
    OS_TPrintf("gameInfo.ggid   = 0x%08x\n", info->gameInfo.ggid);
    OS_TPrintf("gameInfo.tgid   = 0x%04x\n", info->gameInfo.tgid);
    OS_TPrintf("gameInfo.userGameInfoLength = 0x%02x\n", info->gameInfo.userGameInfoLength);
    OS_TPrintf("gameInfo.gameNameCount_attribute = 0x%02x\n",
               info->gameInfo.gameNameCount_attribute);
    OS_TPrintf("gameInfo.parentMaxSize   = 0x%04x\n", info->gameInfo.parentMaxSize);
    OS_TPrintf("gameInfo.childMaxSize    = 0x%04x\n", info->gameInfo.childMaxSize);
}


/**************************************************************************
 * The following functions perform channel-related processes
 **************************************************************************/

/*---------------------------------------------------------------------------*
  Name:         WH_StartMeasureChannel

  Description:  Starts an examination of the channel wireless signal activity rate.

  Arguments:    None.

  Returns:     TRUE if the sequence start was a success.
 *---------------------------------------------------------------------------*/
BOOL WH_StartMeasureChannel(void)
{
#define MAX_RATIO 100                  // The channel use rate is between 0 and 100
    u16     result;
    u8      macAddr[6];

    OS_GetMacAddress(macAddr);
    RAND_INIT(OS_GetVBlankCount() + *(u16 *)&macAddr[0] + *(u16 *)&macAddr[2] + *(u16 *)&macAddr[4]);   // Random number initialization
    RAND();

    sChannel = 0;
    sChannelBusyRatio = MAX_RATIO + 1;

    WH_ChangeSysState(WH_SYSSTATE_BUSY);

    result = WH_StateInMeasureChannel(1);

    if (result == WH_ERRCODE_NOMORE_CHANNEL)
    {
        // There are not any channels available
        // An error is returned here because this is, after all, a special state wherein wireless signals cannot be used.
        //  
        // (Generally, something should be displayed at this point.)
        WH_REPORT_FAILURE(WH_ERRCODE_NOMORE_CHANNEL);
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
        return FALSE;
    }

    if (result != WM_ERRCODE_OPERATING)
    {
        // Error complete
        WH_REPORT_FAILURE(result);
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
        return FALSE;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         WH_StateInMeasureChannel

  Arguments:    channel:    The channel number to start the search at

  Returns:      WM_ERRCODE_SUCCESS        - Searching
                WM_ERRCODE_NOMORE_CHANNEL - There are no more channels to search
                WM_ERRCODE_API_ERR        - WM_MeasureChannel API call failure
 *---------------------------------------------------------------------------*/
static u16 WH_StateInMeasureChannel(u16 channel)
{
    u16     allowedChannel;
    u16     result;

    allowedChannel = WM_GetAllowedChannel();

    if (allowedChannel == 0x8000)
    {
        // If 0x8000 is returned, it indicates that wireless is not initialized or there is some other abnormality with the wireless library. Therefore, set to an error.
        //  
        WH_REPORT_FAILURE(WM_ERRCODE_ILLEGAL_STATE);
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
        return WM_ERRCODE_ILLEGAL_STATE;
    }
    if (allowedChannel == 0)
    {
        // If 0 has been returned, this is a special state wherein wireless signals cannot be used. Therefore, return a value indicating that there are no available channels.
        //  
        WH_REPORT_FAILURE(WH_ERRCODE_NO_RADIO);
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
        return WH_ERRCODE_NOMORE_CHANNEL;
    }

    while (((1 << (channel - 1)) & allowedChannel) == 0)
    {
        channel++;
        if (channel > 16)
        {
            /* When finished searching all allowed channels */
            return WH_ERRCODE_NOMORE_CHANNEL;
        }
    }

    result = WHi_MeasureChannel(WH_StateOutMeasureChannel, channel);
    if (result != WM_ERRCODE_OPERATING)
    {
        return result;
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         WH_StateOutMeasureChannel

  Arguments:    arg:    The WMMeasureChannelCallback structure for notification of the search results

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WH_StateOutMeasureChannel(void *arg)
{
    u16     result;
    u16     channel;
    WMMeasureChannelCallback *cb = (WMMeasureChannelCallback *)arg;
    WH_TRACE_STATE;

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        // If there is anything wrong
        // It can be assumed that wireless signals are unusable if MeasureChannel fails, so an error state will be the result.
        //  
        WH_REPORT_FAILURE(cb->errcode);
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
        return;
    }

    WH_TRACE("channel %d bratio = 0x%x\n", cb->channel, cb->ccaBusyRatio);

    channel = cb->channel;

    /* Get a channel with a lower usage rate (initial value is 101%, so be sure to select the top) */
    if (sChannelBusyRatio > cb->ccaBusyRatio)
    {
        sChannelBusyRatio = cb->ccaBusyRatio;
        sChannelBitmap = (u16)(1 << (channel - 1));
    }
    else if (sChannelBusyRatio == cb->ccaBusyRatio)
    {
        sChannelBitmap |= 1 << (channel - 1);
    }

    result = WH_StateInMeasureChannel(++channel);

    if (result == WH_ERRCODE_NOMORE_CHANNEL)
    {
        // The channel search ends
        WH_ChangeSysState(WH_SYSSTATE_MEASURECHANNEL);
        return;
    }

    if (result != WM_ERRCODE_OPERATING)
    {
        // Error complete
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
        return;
    }
}

/* ----------------------------------------------------------------------
  Checks the wireless signal activity rate
  ---------------------------------------------------------------------- */
static WMErrCode WHi_MeasureChannel(WMCallbackFunc func, u16 channel)
{
#define WH_MEASURE_TIME         30     // The time interval in ms for picking up the signal for making one communication for one frame
#define WH_MEASURE_CS_OR_ED     3      // The logical OR of the carrier sense and the ED value
#define WH_MEASURE_ED_THRESHOLD 17     // The recommended ED threshold value that has been empirically shown to be effective

    /*
     * A value considered to be empirically valid is used as a parameter for getting the wireless activity ratio
     * 
     */
    return WM_MeasureChannel(func,     /* Callback settings */
                             WH_MEASURE_CS_OR_ED,       /* CS or ED */
                             WH_MEASURE_ED_THRESHOLD,   /* Invalid only when the second argument is carrier sense */
                             channel,  /* Current search channel */
                             WH_MEASURE_TIME);  /* Search time (in ms) of each channel */
}


/*---------------------------------------------------------------------------*
  Name:         WH_GetMeasureChannel

  Description:  Returns the channel with the lowest usage rate from all the usable channels.
                The WH_MeasureChannel operations must be complete and it must be in a WH_SYSSTATE_MEASURECHANNEL state.
                
                When this function is called, the channel with the lowest use rate is returned and the state transitions to WH_SYSSTATE_IDLE.
                
                
  Arguments:    None.

  Returns:      The channel number of the usable channel with the lowest usage rate
 *---------------------------------------------------------------------------*/
u16 WH_GetMeasureChannel(void)
{
    WH_ASSERT(sSysState == WH_SYSSTATE_MEASURECHANNEL);

    WH_ChangeSysState(WH_SYSSTATE_IDLE);
    sChannel = (u16)SelectChannel(sChannelBitmap);
    WH_TRACE("decided channel = %d\n", sChannel);
    return sChannel;
}


/*---------------------------------------------------------------------------*
  Name:         SelectChannel

  Description:  Gets the channel with the lowest wireless signal activity ratio.
                If there are multiple channels with the lowest wireless signal activity, a random number is used.
                
                
  Arguments:    Channel bitmap

  Returns:      The channel number of the usable channel with the lowest usage rate
 *---------------------------------------------------------------------------*/
static s16 SelectChannel(u16 bitmap)
{
    s16     i;
    s16     channel = 0;
    u16     num = 0;
    u16     select;

    for (i = 0; i < 16; i++)
    {
        if (bitmap & (1 << i))
        {
            channel = (s16)(i + 1);
            num++;
        }
    }

    if (num <= 1)
    {
        return channel;
    }

    // If there are multiple channels of the same wireless signal activity rate
    select = (u16)(((RAND() & 0xFF) * num) / 0x100);

    channel = 1;

    for (i = 0; i < 16; i++)
    {
        if (bitmap & 1)
        {
            if (select == 0)
            {
                return (s16)(i + 1);
            }
            select--;
        }
        bitmap >>= 1;
    }

    return 0;
}


/**************************************************************************
 * Functions initializing wireless and switching to a communication enabled state are given below
 **************************************************************************/

/* ----------------------------------------------------------------------
   Name:        WH_Initialize
   Description: Performs initialization operations and starts the initialization sequence.
   Arguments:   None.
   Returns:     TRUE if the sequence start was a success.
   ---------------------------------------------------------------------- */
BOOL WH_Initialize(void)
{
    if (whInitialized)
    {
        return FALSE;
    }

    if (sIndicationCallback == NULL)
    {
        sIndicationCallback = WH_IndicateHandler;
    }

    sRecvBufferSize = 0;
    sSendBufferSize = 0;

    sReceiverFunc = NULL;
    sMyAid = 0;
    sConnectBitmap = WH_BITMAP_EMPTY;
    sErrCode = WM_ERRCODE_SUCCESS;

    sParentParam.userGameInfo = NULL;
    sParentParam.userGameInfoLength = 0;

    MI_CpuClear8(sConnectionSsid, sizeof(sConnectionSsid));
    // Sets the connected child user determination function to NULL (multiboot)
    sJudgeAcceptFunc = NULL;

    // Starts the initialization sequence
    if (!WH_StateInInitialize())
    {
        return FALSE;
    }

    whInitialized = 1;

    return TRUE;
}

/* ----------------------------------------------------------------------
   Indicate handler
   ---------------------------------------------------------------------- */
static void WH_IndicateHandler(void *arg)
{
    WMindCallback *cb = (WMindCallback *)arg;

    if (cb->errcode == WM_ERRCODE_FIFO_ERROR)
    {
        // The case wherein an unrecoverable error occurred.
        // Display a screen or otherwise notify the player of this, and stop wireless communications.
        //  
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
        OS_TPanic("Wireless FATAL error occured.\n");
    }
}

/* ----------------------------------------------------------------------
   State: Initialize
   ---------------------------------------------------------------------- */
static BOOL WH_StateInInitialize(void)
{
    // Starts the initialization sequence
    WMErrCode result;
    WH_TRACE_STATE;

#ifndef WH_USE_DETAILED_INITIALIZE
    WH_ChangeSysState(WH_SYSSTATE_BUSY);
    result = WM_Initialize(&sWmBuffer, WH_StateOutInitialize, WH_DMA_NO);
    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        WH_ChangeSysState(WH_SYSSTATE_FATAL);
        return FALSE;
    }

#else
    // WM_Init is a synchronous function
    result = WM_Init(&sWmBuffer, WH_DMA_NO);
    if (result != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(result);
        return FALSE;
    }

    WH_ChangeSysState(WH_SYSSTATE_BUSY);

    if (!WH_StateInEnable())
    {
        WH_ChangeSysState(WH_SYSSTATE_FATAL);
        return FALSE;
    }
#endif

    return TRUE;
}


#ifndef WH_USE_DETAILED_INITIALIZE

static void WH_StateOutInitialize(void *arg)
{
    // State after power-on
    WMErrCode result;
    WMCallback *cb = (WMCallback *)arg;
    WH_TRACE_STATE;

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(cb->errcode);
        WH_ChangeSysState(WH_SYSSTATE_FATAL);
        return;
    }
    // Sets the callback function to receive notifications that are generated irregularly
    result = WM_SetIndCallback(sIndicationCallback);
    if (result != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(result);
        WH_ChangeSysState(WH_SYSSTATE_FATAL);
        return;
    }

    // Changes the system state to idling (waiting)
    WH_ChangeSysState(WH_SYSSTATE_IDLE);

    // Does not set the next state, so the sequence ends here.
    // If WH_Connect is called while in this state, it changes to the connection operation.
}

#else

/* ----------------------------------------------------------------------
   Enable
  ---------------------------------------------------------------------- */
static BOOL WH_StateInEnable(void)
{
    // Makes the hardware usable. (Gets the permission to use.)
    WMErrCode result;
    WH_TRACE_STATE;

    result = WM_Enable(WH_StateOutEnable);
    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        WH_ChangeSysState(WH_SYSSTATE_FATAL);
        return FALSE;
    }
    return TRUE;
}

static void WH_StateOutEnable(void *arg)
{
    // When the hardware use is permitted, it changes to the power-on state
    WMCallback *cb = (WMCallback *)arg;
    WH_TRACE_STATE;

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(cb->errcode);
        WH_ChangeSysState(WH_SYSSTATE_FATAL);
        return;
    }

    if (!WH_StateInPowerOn())
    {
        WH_ChangeSysState(WH_SYSSTATE_FATAL);
        return;
    }
}

/* ----------------------------------------------------------------------
   Power on
  ---------------------------------------------------------------------- */
static BOOL WH_StateInPowerOn(void)
{
    // Because the wireless hardware became usable, start supplying power
    WMErrCode result;
    WH_TRACE_STATE;

    result = WM_PowerOn(WH_StateOutPowerOn);
    if (result != WM_ERRCODE_OPERATING)
    {
        WH_REPORT_FAILURE(result);
        WH_ChangeSysState(WH_SYSSTATE_FATAL);
        return FALSE;
    }

    return TRUE;
}

static void WH_StateOutPowerOn(void *arg)
{
    // State after power-on
    WMErrCode result;
    WMCallback *cb = (WMCallback *)arg;
    WH_TRACE_STATE;

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(cb->errcode);
        WH_ChangeSysState(WH_SYSSTATE_FATAL);
        return;
    }

    // Sets the callback function to receive notifications that are generated irregularly
    result = WM_SetIndCallback(sIndicationCallback);
    if (result != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(result);
        WH_ChangeSysState(WH_SYSSTATE_FATAL);
        return;
    }

    // Changes the system state to idling (waiting)
    WH_ChangeSysState(WH_SYSSTATE_IDLE);

    // Does not set the next state, so the sequence ends here.
    // If WH_Connect is called while in this state, it changes to the connection operation.
}

#endif // #ifdef WH_USE_DETAILED_INITIALIZE


/* ----------------------------------------------------------------------
  Name:        WH_ParentConnect
  Description: Starts the connection sequence.
  Arguments:   mode:     If WH_CONNECTMODE_MP_PARENT, MP starts as a parent.
                         If WH_CONNECTMODE_DS_PARENT, DataSharing starts as a parent.
                         If WH_CONNECTMODE_KS_PARENT, KeySharing starts as a parent.
               tgid:     The parent device communications tgid
               channel:  The parent device communications channel
  Returns:     TRUE if the connection sequence is successful
  ---------------------------------------------------------------------- */
BOOL WH_ParentConnect(int mode, u16 tgid, u16 channel)
{
    // If not in a standby state, the connection sequence cannot be started
    WH_ASSERT(sSysState == WH_SYSSTATE_IDLE);

    // Calculation of send and receive buffer size for WM_StartMP()
    // Use the WM_SIZE_MP_* function macros to statically allocate a receive buffer in advance. If the buffer can be allocated dynamically, use the WM_GetReceiveBufferSize function after the parent and child have been connected and immediately before calling the WM_StartMP function.
    //  
    //  
    // Similarly, use the WM_SIZE_MP_* function macros to statically allocate a send buffer in advance. If the buffer can be allocated dynamically, use the WM_GetSendBufferSize function after the parent and child have been connected and immediately before calling the WM_StartMP function.
    // 
    // 
    sRecvBufferSize = WH_PARENT_RECV_BUFFER_SIZE;
    sSendBufferSize = WH_PARENT_SEND_BUFFER_SIZE;

    WH_TRACE("recv buffer size = %d\n", sRecvBufferSize);
    WH_TRACE("send buffer size = %d\n", sSendBufferSize);

    sConnectMode = mode;
    WH_ChangeSysState(WH_SYSSTATE_BUSY);

    sParentParam.tgid = tgid;
    sParentParam.channel = channel;
    sParentParam.beaconPeriod = WM_GetDispersionBeaconPeriod();
    sParentParam.parentMaxSize = WH_PARENT_MAX_SIZE;
    sParentParam.childMaxSize = WH_CHILD_MAX_SIZE;
    sParentParam.maxEntry = WH_CHILD_MAX;
    sParentParam.CS_Flag = 0;
    sParentParam.multiBootFlag = 0;
    sParentParam.entryFlag = 1;
    sParentParam.KS_Flag = (u16)((mode == WH_CONNECTMODE_KS_PARENT) ? 1 : 0);

    switch (mode)
    {
    case WH_CONNECTMODE_MP_PARENT:
    case WH_CONNECTMODE_KS_PARENT:
    case WH_CONNECTMODE_DS_PARENT:
        // Starts the connection in child mode
        return WH_StateInSetParentParam();
    default:
        break;
    }

    WH_TRACE("unknown connect mode %d\n", mode);
    return FALSE;
}


/* ----------------------------------------------------------------------
  Name:        WH_ChildConnect
  Description: Starts the connection sequence.
  Arguments:   mode:     Start MP as the child if WH_CONNECTMODE_MP_CHILD.
                         Start DataSharing as the child if WH_CONNECTMODE_DS_CHILD.
                         Start KeySharing as the child if WH_CONNECTMODE_KS_CHILD.
               bssDesc:  The bssDesc of the parent device to connect to.
                
  Returns:     TRUE if the connection sequence is successful
  ---------------------------------------------------------------------- */
BOOL WH_ChildConnect(int mode, WMBssDesc *bssDesc)
{
    // If not in a standby state, the connection sequence cannot be started
    WH_ASSERT(sSysState == WH_SYSSTATE_IDLE);

    // Calculation of send and receive buffer size for WM_StartMP()
    // Use the WM_SIZE_MP_* function macros to statically allocate a receive buffer in advance. If the buffer can be allocated dynamically, use the WM_GetReceiveBufferSize function after the parent and child have been connected and immediately before calling the WM_StartMP function.
    //  
    //  
    // Similarly, use the WM_SIZE_MP_* function macros to statically allocate a send buffer in advance. If the buffer can be allocated dynamically, use the WM_GetSendBufferSize function after the parent and child have been connected and immediately before calling the WM_StartMP function.
    // 
    // 
    sRecvBufferSize = WH_CHILD_RECV_BUFFER_SIZE;
    sSendBufferSize = WH_CHILD_SEND_BUFFER_SIZE;

    WH_TRACE("recv buffer size = %d\n", sRecvBufferSize);
    WH_TRACE("send buffer size = %d\n", sSendBufferSize);

    sConnectMode = mode;
    WH_ChangeSysState(WH_SYSSTATE_BUSY);

    switch (mode)
    {
    case WH_CONNECTMODE_MP_CHILD:
    case WH_CONNECTMODE_KS_CHILD:
    case WH_CONNECTMODE_DS_CHILD:
        // Starts the connection in child mode.
        // Uses the saved BssDesc for the parent and connects without scanning.
        MI_CpuCopy8(bssDesc, &sBssDesc, sizeof(sBssDesc));
        DC_FlushRange(&sBssDesc, sizeof(sBssDesc));
        DC_WaitWriteBufferEmpty();
        if (sChildWEPKeyGenerator != NULL)
        {
            // If the WEP Key Generator is configured, set the WEP Key
            return WH_StateInSetChildWEPKey();
        }
        else
        {
            return WH_StateInStartChild();
        }
    default:
        break;
    }

    WH_TRACE("unknown connect mode %d\n", mode);
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         WH_SetJudgeAcceptFunc

  Description:  Sets the functions used to determine whether to accept the child device connection.

  Arguments:    Set the child device connection determination functions.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WH_SetJudgeAcceptFunc(WHJudgeAcceptFunc func)
{
    sJudgeAcceptFunc = func;
}


/**************************************************************************
 * The functions below are for direct MP communication that uses the WH_DATA_PORT port
 **************************************************************************/

/* ----------------------------------------------------------------------
   Name:        WH_SetReceiver
   Description: Configures the data receipt callback in the WH_DATA_PORT port.
   Arguments:   proc: The data receipt callback
   Returns:     None.
   ---------------------------------------------------------------------- */
void WH_SetReceiver(WHReceiverFunc proc)
{
    sReceiverFunc = proc;
    if (WM_SetPortCallback(WH_DATA_PORT, WH_PortReceiveCallback, NULL) != WM_ERRCODE_SUCCESS)
    {
        WH_ChangeSysState(WH_SYSSTATE_ERROR);
        WH_TRACE("WM not Initialized\n");
    }
}

/* ----------------------------------------------------------------------
   Name:        WH_SendData
   Description: Starts sending data to the WH_DATA_PORT port.
               (For MP communications. There is no need to call this function while data sharing, etc.)
   Arguments:   size:  The data size
   Returns:     Returns TRUE if starting the send is successful.
   ---------------------------------------------------------------------- */
BOOL WH_SendData(void *data, u16 size, WHSendCallbackFunc callback)
{
    return WH_StateInSetMPData(data, size, callback);
}


/**************************************************************************
 * The functions below control data sharing communication
 **************************************************************************/

/* ----------------------------------------------------------------------
   Name:        WH_GetKeySet
   Description: Reads the common key data.
   Arguments:   keyset: Data storage destination
   Returns:     If it succeeds, returns TRUE.
   ---------------------------------------------------------------------- */
BOOL WH_GetKeySet(WMKeySet *keyset)
{
    WMErrCode result;

    if (sSysState != WH_SYSSTATE_KEYSHARING)
    {
        WH_TRACE("WH_GetKeySet failed (invalid system state)\n");
        return FALSE;
    }

    if ((sConnectMode != WH_CONNECTMODE_KS_CHILD) && (sConnectMode != WH_CONNECTMODE_KS_PARENT))
    {
        WH_TRACE("WH_GetKeySet failed (invalid connect mode)\n");
        return FALSE;
    }

    result = WM_GetKeySet(&sWMKeySetBuf, keyset);
    if (result != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(result);
        return FALSE;
    }

    return TRUE;
}

/* ----------------------------------------------------------------------
   Name:        WH_GetSharedDataAdr
  Description: Calculates and gets the data address obtained from the machine with the designated aid from the shared data address.
                
   Arguments:   aid:  The machine designation
   Returns:     NULL on failure.
   ---------------------------------------------------------------------- */
u16    *WH_GetSharedDataAdr(u16 aid)
{
    return WM_GetSharedDataAddress(&sDSInfo, &sDataSet, aid);
}

/* ----------------------------------------------------------------------
   Name:        WH_StepDS
   Description: Proceeds to the next synchronized data sharing process.
                If communication is performed for each frame, this function must also be called for each frame.
                
   Arguments:   data:  The data to send
   Returns:     If it succeeds, returns TRUE.
   ---------------------------------------------------------------------- */
BOOL WH_StepDS(void *data)
{
    WMErrCode result;

    result = WM_StepDataSharing(&sDSInfo, (u16 *)data, &sDataSet);

    if (result == WM_ERRCODE_NO_CHILD)
    {
        // A parent with no children (this can be made an error if desired)
        return TRUE;
    }

    if (result == WM_ERRCODE_NO_DATASET)
    {
        WH_TRACE("WH_StepDataSharing - Warning No DataSet\n");
        WH_SetError(result);
        return FALSE;
    }

    if (result != WM_ERRCODE_SUCCESS)
    {
        WH_REPORT_FAILURE(result);
        return FALSE;
    }

    return TRUE;
}


/**************************************************************************
 * The functions below end communication and transition to initialization states
 **************************************************************************/

/* ----------------------------------------------------------------------
   Name:        WH_Reset
   Description: Starts the reset sequence.
                When this function is called, it resets regardless of the current state.
        It is used for forced recovery from errors.
   Arguments:   None.
   Returns:     None.
   ---------------------------------------------------------------------- */
void WH_Reset(void)
{
    WMErrCode result;

    if (sSysState == WH_SYSSTATE_DATASHARING)
    {
        result = WM_EndDataSharing(&sDSInfo);
        if (result != WM_ERRCODE_SUCCESS)
        {
            WH_REPORT_FAILURE(result);
        }
    }

    if (!WH_StateInReset())
    {
        WH_ChangeSysState(WH_SYSSTATE_FATAL);
    }
}

/* ----------------------------------------------------------------------
   Name:        WH_Finalize
   Description: Starts the post-processing / end sequence.
                When this function is called, the current state is referenced and an appropriate end sequence is executed.
                
                This function is used in the normal end process (not WH_Reset).
   Arguments:   None.
   Returns:     None.
   ---------------------------------------------------------------------- */
void WH_Finalize(void)
{
    if (sSysState == WH_SYSSTATE_IDLE)
    {
        WH_TRACE("already WH_SYSSTATE_IDLE\n");
        return;
    }

    WH_TRACE("WH_Finalize, state = %d\n", sSysState);

    if (sSysState == WH_SYSSTATE_SCANNING)
    {
        if (!WH_EndScan())
        {
            WH_Reset();
        }
        return;
    }


    if ((sSysState != WH_SYSSTATE_KEYSHARING) &&
        (sSysState != WH_SYSSTATE_DATASHARING) && (sSysState != WH_SYSSTATE_CONNECTED))
    {
        // Resets if there is no connection, there is an error, and so on
        WH_ChangeSysState(WH_SYSSTATE_BUSY);
        WH_Reset();
        return;
    }

    WH_ChangeSysState(WH_SYSSTATE_BUSY);

    switch (sConnectMode)
    {
    case WH_CONNECTMODE_KS_CHILD:
        if (!WH_StateInEndChildKeyShare())
        {
            WH_Reset();
        }
        break;

    case WH_CONNECTMODE_DS_CHILD:
        if (WM_EndDataSharing(&sDSInfo) != WM_ERRCODE_SUCCESS)
        {
            WH_Reset();
            break;
        }
        // Don't break

    case WH_CONNECTMODE_MP_CHILD:
        if (!WH_StateInEndChildMP())
        {
            WH_Reset();
        }
        break;

    case WH_CONNECTMODE_KS_PARENT:
        if (!WH_StateInEndParentKeyShare())
        {
            WH_Reset();
        }
        break;

    case WH_CONNECTMODE_DS_PARENT:
        if (WM_EndDataSharing(&sDSInfo) != WM_ERRCODE_SUCCESS)
        {
            WH_Reset();
            break;
        }
        // Don't break

    case WH_CONNECTMODE_MP_PARENT:
        if (!WH_StateInEndParentMP())
        {
            WH_Reset();
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WH_End

  Description:  Ends wireless communications.

  Arguments:    None.

  Returns:      If it succeeds, returns TRUE.
 *---------------------------------------------------------------------------*/
BOOL WH_End(void)
{
    WH_ASSERT(sSysState == WH_SYSSTATE_IDLE);

    WH_ChangeSysState(WH_SYSSTATE_BUSY);
    if (WM_End(WH_StateOutEnd) != WM_ERRCODE_OPERATING)
    {
        WH_ChangeSysState(WH_SYSSTATE_ERROR);

        return FALSE;
    }

    whInitialized = 0;

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         WH_GetCurrentAid

  Description:  Gets its own current AID.
                Children may change when they connect or disconnect.

  Arguments:    None.

  Returns:      AID value
 *---------------------------------------------------------------------------*/
u16 WH_GetCurrentAid(void)
{
    return sMyAid;
}

/*---------------------------------------------------------------------------*
  Name:         WH_SetParentWEPKeyGenerator

  Description:  Sets the function that generates WEP Key.
                If this function is called, WEP will be used for authentication during connection.
                Using a unique algorithm for each game application, it sets the parent and child to the same value before connection.
                
                This function is for parents.

  Arguments:    func:   Pointer to the function that generates the WEP Key
                        If NULL is specified, WEP Key is not used

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WH_SetParentWEPKeyGenerator(WHParentWEPKeyGeneratorFunc func)
{
    sParentWEPKeyGenerator = func;
}

/*---------------------------------------------------------------------------*
  Name:         WH_SetChildWEPKeyGenerator

  Description:  Sets the function that generates WEP Key.
                If this function is called, WEP will be used for authentication during connection.
                Using a unique algorithm for each game application, it sets the parent and child to the same value before connection.
                
                This function is for children.

  Arguments:    func:   Pointer to the function that generates the WEP Key
                        If NULL is specified, WEP Key is not used

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WH_SetChildWEPKeyGenerator(WHChildWEPKeyGeneratorFunc func)
{
    sChildWEPKeyGenerator = func;
}

/*---------------------------------------------------------------------------*
  Name:         WH_SetIndCallback

  Description:  Sets the callback function specified in the WM_SetIndCallback function called by WH_Initialize.
                
                This function should be called before WH_Initialize is called.
                If a callback function is not specified by this function, the default WH_IndicateHandler will be set as the callback.
                

  Arguments:    callback:   Callback for the indication notification specified by WM_SetIndCallback

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WH_SetIndCallback(WMCallbackFunc callback)
{
    sIndicationCallback = callback;
}
