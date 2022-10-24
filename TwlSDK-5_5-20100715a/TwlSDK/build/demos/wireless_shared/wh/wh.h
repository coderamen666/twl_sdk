/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WM - demos.Twl
  File:     wh.c

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8557 $
  $Author: nishimoto_takashi $
 *---------------------------------------------------------------------------*/


#ifndef __WMHIGH_H__
#define __WMHIGH_H__

#include "wh_config.h"

/* added from NITRO-SDK3.1RC (prior to that is always 1) */
#if !defined(WH_MP_FREQUENCY)
#define WH_MP_FREQUENCY   1
#endif

// OFF if initializing by using WM_Initialize
// Set to ON if there is a need to individually use and precisely control WM_Init, WM_Enable, and WM_PowerOn.
// #define WH_USE_DETAILED_INITIALIZE

enum
{
    WH_SYSSTATE_STOP,                  // Initial state
    WH_SYSSTATE_IDLE,                  // Waiting
    WH_SYSSTATE_SCANNING,              // Scanning
    WH_SYSSTATE_BUSY,                  // Connecting
    WH_SYSSTATE_CONNECTED,             // Completed connection (Communications are possible in this state)
    WH_SYSSTATE_DATASHARING,           // Completed connection, with data-sharing enabled
    WH_SYSSTATE_KEYSHARING,            // Completed connection, with key-sharing enabled
    WH_SYSSTATE_MEASURECHANNEL,        // Check the radio wave usage rate of the channel
    WH_SYSSTATE_CONNECT_FAIL,          // Connection to the parent device failed
    WH_SYSSTATE_ERROR,                 // Error occurred
    WH_SYSSTATE_FATAL,                 // A FATAL error has occurred
    WH_SYSSTATE_NUM
};

enum
{
    WH_CONNECTMODE_MP_PARENT,          // Parent device MP connection mode
    WH_CONNECTMODE_MP_CHILD,           // Child device MP connection mode
    WH_CONNECTMODE_KS_PARENT,          // Parent device key-sharing connection mode
    WH_CONNECTMODE_KS_CHILD,           // Child device key-sharing connection mode
    WH_CONNECTMODE_DS_PARENT,          // Parent device data-sharing connection mode
    WH_CONNECTMODE_DS_CHILD,           // Child device data-sharing connection mode
    WH_CONNECTMODE_NUM
};

enum
{
    // Separate error codes
    WH_ERRCODE_DISCONNECTED = WM_ERRCODE_MAX,   // Disconnected from parent
    WH_ERRCODE_PARENT_NOT_FOUND,       // No parent
    WH_ERRCODE_NO_RADIO,               // Wireless cannot be used
    WH_ERRCODE_LOST_PARENT,            // Parent lost
    WH_ERRCODE_NOMORE_CHANNEL,         // Scan has finished on all channels
    WH_ERRCODE_MAX
};

typedef void (*WHStartScanCallbackFunc) (WMBssDesc *bssDesc);

/* Parent device receive buffer size */
#define WH_PARENT_RECV_BUFFER_SIZE  WM_SIZE_MP_PARENT_RECEIVE_BUFFER( WH_CHILD_MAX_SIZE, WH_CHILD_MAX, FALSE )
/* Parent device send buffer size */
#define WH_PARENT_SEND_BUFFER_SIZE  WM_SIZE_MP_PARENT_SEND_BUFFER( WH_PARENT_MAX_SIZE, FALSE )

/* Child device receive buffer size */
#define WH_CHILD_RECV_BUFFER_SIZE   WM_SIZE_MP_CHILD_RECEIVE_BUFFER( WH_PARENT_MAX_SIZE, FALSE )
/* Child device send buffer size */
#define WH_CHILD_SEND_BUFFER_SIZE   WM_SIZE_MP_CHILD_SEND_BUFFER( WH_CHILD_MAX_SIZE, FALSE )

/* Macro that defines the GGID reserved for the SDK sample demo */
#define SDK_MAKEGGID_SYSTEM(num)    (0x003FFF00 | (num))

/* Bitmap value indicating that either there is no connection or only a parent device */
#define WH_BITMAP_EMPTY           1

// Send callback type (for data-sharing model)
typedef void (*WHSendCallbackFunc) (BOOL result);

// Function type for making connection permission determinations (for multiboot model)
typedef BOOL (*WHJudgeAcceptFunc) (WMStartParentCallback *);

// Receive callback type
typedef void (*WHReceiverFunc) (u16 aid, u16 *data, u16 size);

// Function for generating a WEP key
typedef u16 (*WHParentWEPKeyGeneratorFunc) (u16 *wepkey, const WMParentParam *parentParam);
typedef u16 (*WHChildWEPKeyGeneratorFunc) (u16 *wepkey, const WMBssDesc *bssDesc);



/**************************************************************************
 * The following functions modify various WH configuration values.
 **************************************************************************/

/*---------------------------------------------------------------------------*
  Name:         WH_SetGgid

  Description:  Sets the game's group ID.
                Call before making a connection to the parent device.

  Arguments:    ggid    The game group ID to configure.

  Returns:      None.
 *---------------------------------------------------------------------------*/
extern void WH_SetGgid(u32 ggid);

/*---------------------------------------------------------------------------*
  Name:         WH_SetSsid

  Description:  Sets the SSID specified during child connection.
                Call before making a connection from the child device.

  Arguments:    ssid    Buffer in which the SSID to be set has been stored.
                length  Data length of the SSID to be set.
                        If less than WM_SIZE_CHILD_SSID (24-byte), fill the subsequent whitespace with 0s. If more than WM_SIZE_CHILD_SSID, truncate.
                        

  Returns:      None.
 *---------------------------------------------------------------------------*/
extern void WH_SetSsid(const void *ssid, u32 length);

/*---------------------------------------------------------------------------*
  Name:         WH_SetUserGameInfo

  Description:  Configures user-defined parent device information
                Call before making a connection to the parent device.

  Arguments:    userGameInfo   Pointer to user-defined parent device information
                length         Size of the user-defined parent device information

  Returns:      None.
 *---------------------------------------------------------------------------*/
extern void WH_SetUserGameInfo(u16 *userGameInfo, u16 length);

/*---------------------------------------------------------------------------*
  Name:         WH_SetDebugOutput

  Description:  Configures the function for outputting the debug character string.

  Arguments:    func    Function for the set debug character string output.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WH_SetDebugOutput(void (*func) (const char *, ...));

/*---------------------------------------------------------------------------*
  Name:         WH_SetParentWEPKeyGenerator

  Description:  Sets the function that generates WEP Key.
                If this function is called, WEP will be used for authentication during connection.
                Using a unique algorithm for each game application, it sets the parent and child to the same value before connection.
                
                This function is for parents.

  Arguments:    func    Pointer to the function that generates the WEP Key
                        If NULL is specified, WEP Key is not used

  Returns:      None.
 *---------------------------------------------------------------------------*/
extern void WH_SetParentWEPKeyGenerator(WHParentWEPKeyGeneratorFunc func);

/*---------------------------------------------------------------------------*
  Name:         WH_SetChildWEPKeyGenerator

  Description:  Sets the function that generates WEP Key.
                If this function is called, WEP will be used for authentication during connection.
                Using a unique algorithm for each game application, it sets the parent and child to the same value before connection.
                
                This function is for children.

  Arguments:    func    Pointer to the function that generates the WEP Key
                        If NULL is specified, WEP Key is not used

  Returns:      None.
 *---------------------------------------------------------------------------*/
extern void WH_SetChildWEPKeyGenerator(WHChildWEPKeyGeneratorFunc func);

/*---------------------------------------------------------------------------*
  Name:         WH_SetIndCallback

  Description:  Sets the callback function specified in the WM_SetIndCallback function called by WH_Initialize.
                
                This function should be called before WH_Initialize is called.
                If a callback function is not specified by this function, the default WH_IndicateHandler will be set as the callback.
                

  Arguments:    callback    Call back for the indication notification specified by WM_SetIndCallback.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WH_SetIndCallback(WMCallbackFunc callback);

/*---------------------------------------------------------------------------*
  Name:         WH_SetSessionUpdateCallback

  Description:  This function registers callbacks that are invoked when connecting, disconnecting, and making a new child connection.

  Arguments:    callback: Callback to register

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WH_SetSessionUpdateCallback(WMCallbackFunc callback);

/**************************************************************************
 * The following wrapper functions get the low-level WM library state.
 **************************************************************************/

/* ----------------------------------------------------------------------
  Name:        WH_GetLinkLevel
  Description: Gets the radio reception strength.
  Arguments:   None.
  Returns:     Returns the numeric value of WMLinkLevel.
  ---------------------------------------------------------------------- */
extern int WH_GetLinkLevel(void);

/* ----------------------------------------------------------------------
   Name:        WH_GetAllowedChannel
   Description: Gets the bit pattern of channels that can be used for connection.
   Arguments:   None.
   Returns:     channel pattern
   ---------------------------------------------------------------------- */
extern u16 WH_GetAllowedChannel(void);


/**************************************************************************
 * The following functions get the WH state.
 **************************************************************************/

/* ----------------------------------------------------------------------
   Name:        WH_GetBitmap
   Description: Gets the bit pattern for displaying the connection state.
   Arguments:   None.
   Returns:     bitmap pattern
   ---------------------------------------------------------------------- */
extern u16 WH_GetBitmap(void);

/* ----------------------------------------------------------------------
   Name:        WH_GetSystemState
   Description: Gets the WH internal state.
   Arguments:   None.
   Returns:     The internal state (WH_SYSSTATE_XXXX).
   ---------------------------------------------------------------------- */
extern int WH_GetSystemState(void);

/* ----------------------------------------------------------------------
   Name:        WH_GetConnectMode
   Description: Gets the connection information.
   Arguments:   None.
   Returns:     The connection information (WH_CONNECTMODE_XX_XXXX).
   ---------------------------------------------------------------------- */
extern int WH_GetConnectMode(void);

/* ----------------------------------------------------------------------
   Name:        WH_GetLastError
   Description: Gets the error code that most recently occurred.
   Arguments:   None.
   Returns:     The error code.
   ---------------------------------------------------------------------- */
extern int WH_GetLastError(void);

/*---------------------------------------------------------------------------*
  Name:         WH_PrintBssDesc

  Description:  Debug outputs the members of the WMBssDesc structure.

  Arguments:    info    The pointer to the BssDesc to be debug output.

  Returns:      None.
 *---------------------------------------------------------------------------*/
extern void WH_PrintBssDesc(WMBssDesc *info);


/**************************************************************************
 * The following functions perform channel-related processes.
 **************************************************************************/

/*---------------------------------------------------------------------------*
  Name:         WH_StartMeasureChannel

  Description:  Starts measuring the radio wave usage rate on every usable channel.
                When measurement is complete, internally calculates the channel with the lowest usage rate, and transitions it to the WH_SYSSTATE_MEASURECHANNEL state.
                

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
extern BOOL WH_StartMeasureChannel(void);

/*---------------------------------------------------------------------------*
  Name:         WH_GetMeasureChannel

  Description:  Returns the channel with the lowest usage rate from all the usable channels.
                The WH_MeasureChannel operations must be complete and it must be in a WH_SYSSTATE_MEASURECHANNEL state.
                
                When this function is called, the channel with the lowest use rate is returned and the state transitions to WH_SYSSTATE_IDLE.
                
                
  Arguments:    None.

  Returns:      The channel number of the usable channel with the lowest usage rate
 *---------------------------------------------------------------------------*/
extern u16 WH_GetMeasureChannel(void);


/**************************************************************************
 * The following functions initialize wireless communication and move it into an enabled state.
 **************************************************************************/

/* ----------------------------------------------------------------------
   Name:        WH_Initialize
   Description: Performs initialization operations, and starts the initialization sequence.
   Arguments:   None.
   Returns:     TRUE if the sequence start was a success.
   ---------------------------------------------------------------------- */
extern BOOL WH_Initialize(void);


/*---------------------------------------------------------------------------*
  Name:         WH_TurnOnPictoCatch

  Description:  Enables the Pictochat Search feature.
                This causes a callback function to be invoked when PictoChat is discovered while scanning with WH_StartScan.
                

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
extern void WH_TurnOnPictoCatch(void);

/*---------------------------------------------------------------------------*
  Name:         WH_TurnOffPictoCatch

  Description:  Disables the Pictochat Search feature.
                This makes it to be ignored, even if pictochat was found while scanning with WH_StartScan.
                

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
extern void WH_TurnOffPictoCatch(void);

/*---------------------------------------------------------------------------*
  Name:         WH_StartScan

  Description:  Function for fetching the parent beacon

  Arguments:    callback - Sets the callback returned when the parent device is found.
                
                macAddr  - Specifies the MAC address for the connected parent
                           If 0xFFFFFF, all parents are searched for.
                           
                channel - Designate the channels on which to search for the parent.
                           If 0, all channels are searched.

  Returns:      None.
 *---------------------------------------------------------------------------*/
extern BOOL WH_StartScan(WHStartScanCallbackFunc callback, const u8 *macAddr, u16 channel);

/*---------------------------------------------------------------------------*
  Name:         WH_EndScan

  Description:  Function for fetching the parent beacon

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
extern BOOL WH_EndScan(void);

/* ----------------------------------------------------------------------
  Name:        WH_ParentConnect
  Description: Starts the connection sequence.
  Arguments:   mode    - If WH_CONNECTMODE_MP_PARENT, MP starts as a parent.
                         If WH_CONNECTMODE_DS_PARENT, DataSharing starts as a parent.
                         If WH_CONNECTMODE_KS_PARENT, KeySharing starts as a parent.
               tgid    - The parent device communications tgid
               channel - The parent device communications channel
  Returns:     TRUE if the connection sequence is successful
  ---------------------------------------------------------------------- */
extern BOOL WH_ParentConnect(int mode, u16 tgid, u16 channel);

/* ----------------------------------------------------------------------
  Name:        WH_ChildConnect
  Description: Starts the connection sequence.
  Arguments:   mode    - Start MP as the child if WH_CONNECTMODE_MP_CHILD.
                         Start DataSharing as the child if WH_CONNECTMODE_DS_CHILD.
                         Start KeySharing as the child if WH_CONNECTMODE_KS_CHILD.
               bssDesc - The bssDesc of the parent device to connect to.
               
  Returns:     TRUE if the connection sequence is successful
  ---------------------------------------------------------------------- */
extern BOOL WH_ChildConnect(int mode, WMBssDesc *bssDesc);

/* ----------------------------------------------------------------------
   Name:        WH_ChildConnectAuto
   Description: Starts the child device connection sequence.
                However, each type of setting specified with WH_ParentConnect or WH_ChildConnect is left to automatic internal processing.
                
   Arguments:   mode    - Start MP as the child if WH_CONNECTMODE_MP_CHILD.
                          Start DataSharing as the child if WH_CONNECTMODE_DS_CHILD.
                          Start KeySharing as the child if WH_CONNECTMODE_KS_CHILD.

                macAddr  - Specifies the MAC address for the connected parent
                          If 0xFFFFFF, all parents are searched for.
                          
                channel - Designate the channels on which to search for the parent.
                          If 0, all channels are searched.

  Returns:     TRUE if the connection sequence is successful
   ---------------------------------------------------------------------- */
extern BOOL WH_ChildConnectAuto(int mode, const u8 *macAddr, u16 channel);

/*---------------------------------------------------------------------------*
  Name:         WH_SetJudgeAcceptFunc

  Description:  Sets the functions used to determine whether to accept the child device connection.

  Arguments:    Set the child device connection determination functions.

  Returns:      None.
 *---------------------------------------------------------------------------*/
extern void WH_SetJudgeAcceptFunc(WHJudgeAcceptFunc func);


/**************************************************************************
 * The following functions perform direct MP communication using WH_DATA_PORT.
 **************************************************************************/

/* ----------------------------------------------------------------------
   Name:        WH_SetReceiver
   Description: Configures the data reception callback in the WH_DATA_PORT port.
   Arguments:   proc - Data reception callback
   Returns:     None.
   ---------------------------------------------------------------------- */
extern void WH_SetReceiver(WHReceiverFunc proc);

/* ----------------------------------------------------------------------
   Name:        WH_SendData
   Description: Starts sending data to the WH_DATA_PORT port.
               (For MP communications. There is no need to call this function while data sharing, etc.)
   Arguments:   size - The data size
   Returns:     Returns TRUE if starting the send is successful.
   ---------------------------------------------------------------------- */
extern BOOL WH_SendData(const void *data, u16 datasize, WHSendCallbackFunc callback);


/**************************************************************************
 * The following functions control data sharing communication.
 **************************************************************************/

/* ----------------------------------------------------------------------
   Name:        WH_GetKeySet
   Description: Reads the common key data.
   Arguments:   keyset - Designates the data storage destination
   Returns:     If it succeeds returns TRUE.
   ---------------------------------------------------------------------- */
extern BOOL WH_GetKeySet(WMKeySet *keyset);

/* ----------------------------------------------------------------------
   Name:        WH_GetSharedDataAdr
  Description: Calculates and gets the data address obtained from the machine with the designated aid from the shared data address.
                
   Arguments:   aid - The machine designation
   Returns:     NULL on failure.
   ---------------------------------------------------------------------- */
extern u16 *WH_GetSharedDataAdr(u16 aid);

/* ----------------------------------------------------------------------
   Name:        WH_StepDS
   Description: Proceeds to the next synchronized data sharing process.
                If communication is performed every frame, this function must also be called every frame.
                
   Arguments:   data - The data to send
   Returns:     If it succeeds returns TRUE.
   ---------------------------------------------------------------------- */
extern BOOL WH_StepDS(const void *data);

/* ----------------------------------------------------------------------
   Name:        WH_GetBitmapDS
   Description: Gets a list of systems that have shared data.
   Arguments:   None.
   Returns:     bitmap
   ---------------------------------------------------------------------- */
extern u16 WH_GetBitmapDS(void);

/**************************************************************************
 * The following functions end communication and transition to the initialized state.
 **************************************************************************/

/* ----------------------------------------------------------------------
   Name:        WH_Reset
   Description: Starts the reset sequence.
                When this function is called, it resets regardless of the current state.
        It is used for forced recovery from errors.
   Arguments:   None.
   Returns:     TRUE if the process successfully starts
   ---------------------------------------------------------------------- */
extern void WH_Reset(void);

/* ----------------------------------------------------------------------
   Name:        WH_Finalize
   Description: Starts the post-processing / end sequence.
                When this function is called, the current state is referenced and an appropriate end sequence is executed.
                
                This function is used in the normal end process (not WH_Reset).
   Arguments:   None.
   Returns:     None.
   ---------------------------------------------------------------------- */
extern void WH_Finalize(void);

/*---------------------------------------------------------------------------*
  Name:         WH_End

  Description:  Ends wireless communication.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
extern BOOL WH_End(void);

/*---------------------------------------------------------------------------*
  Name:         WH_GetCurrentAid

  Description:  Gets its own current AID.
                Children may change when they connect or disconnect.

  Arguments:    None.

  Returns:      AID value
 *---------------------------------------------------------------------------*/
extern u16 WH_GetCurrentAid(void);

#endif
