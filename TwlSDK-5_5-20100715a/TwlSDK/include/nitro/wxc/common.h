/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - include -
  File:     common.h

  Copyright 2005-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef	NITRO_WXC_COMMON_H_
#define	NITRO_WXC_COMMON_H_


#include <nitro.h>


/*****************************************************************************/
/* macro */

/*
 * Switches debug output (disabled by default)
 * You can set this symbol to 0 (disabled) or 1 (enabled) from the command line.
 * The WXC library must be rebuilt before the change takes effect.
 */
#if !defined(WXC_DEBUG_OUTPUT)
#define     WXC_DEBUG_OUTPUT    0
#endif

/* Planned to be deleted */
#define     WXC_PACKET_LOG      WXC_DEBUG_LOG
#define     WXC_DRIVER_LOG      WXC_DEBUG_LOG

/* Compile legitimacy check (used inside library) */
#if defined(SDK_COMPILER_ASSERT)
#define SDK_STATIC_ASSERT  SDK_COMPILER_ASSERT
#else
#define SDK_STATIC_ASSERT(expr) \
    extern void sdk_compiler_assert ## __LINE__ ( char is[(expr) ? +1 : -1] )
#endif


/*****************************************************************************/
/* Constants */

typedef enum
{
    /* Internal state obtainable by WXC_GetStatus function */
    WXC_STATE_END,                     /* End process completed by WXC_End function */
    WXC_STATE_ENDING,                  /* After WXC_End function and currently running end process */
    WXC_STATE_READY,                   /* After WXC_Init function and before WXC_Start function */
    WXC_STATE_ACTIVE,                  /* After WXC_Start function and wireless communication is enabled */

    /* Internal event notified to the system callback */
    WXC_STATE_CONNECTED,               /* Child connected (only for Parent side. argument is WMStartParentCallback) */
    WXC_STATE_EXCHANGE_DONE,           /* Data exchange completed (argument is the user specified receive buffer) */
    WXC_STATE_BEACON_RECV              /* Received beacon (argument is the WXCBeaconFoundCallback function) */
}
WXCStateCode;

/* Parent mode beacon setting */
#define WXC_BEACON_PERIOD            90 /* [ms] */
#define WXC_PARENT_BEACON_SEND_COUNT_OUT (900 / WXC_BEACON_PERIOD)

/* Library MP data packet size */
#define WXC_PACKET_SIZE_MIN          20
#define WXC_PACKET_SIZE_MAX          WM_SIZE_MP_DATA_MAX

/* GGID for shared chance encounter communications */
#define WXC_GGID_COMMON_BIT          0x80000000UL
#define WXC_GGID_COMMON_ANY          (u32)(WXC_GGID_COMMON_BIT | 0x00000000UL)
#define WXC_GGID_COMMON_PARENT       (u32)(WXC_GGID_COMMON_BIT | 0x00400120UL)


/*****************************************************************************/
/* Declaration */

/* WXC library callback function prototype */
typedef void (*WXCCallback) (WXCStateCode stat, void *arg);

/* User information per AID held by WXC (obtained with WXC_GetUserInfo()) */
typedef struct WXCUserInfo
{
    u16     aid;
    u8      macAddress[6];
}
WXCUserInfo;

/* Structure used for handling MP data packet */
typedef struct WXCPacketInfo
{
    /*
     * The send buffer or receive buffer.
     * When sending, the actual data is stored in this buffer and returned.
     */
    u8     *buffer;
    /*
     * The maximum size that can be sent or the receive data size.
     * When sending, the actual data size sent is stored here and returned.
     */
    u16     length;
    /*
     * A bitmap of the currently connected AIDs.
     * (A system can determine whether it is a parent or child from the presence of bit 0)
     * When sending, the actual send target is stored here and returned.
     */
    u16     bitmap;
}
WXCPacketInfo;

/* WXC_STATE_BEACON_RECV callback argument */
typedef struct WXCBeaconFoundCallback
{
    /*
     * TRUE is given if the format matches the specified protocol and FALSE is given otherwise.
     * This member is set to TRUE and returned when connecting to this beacon's sender.
     * 
     */
    BOOL    matched;
    /*
     * The beacon detected this time
     */
    const WMBssDesc *bssdesc;
}
WXCBeaconFoundCallback;


/*****************************************************************************/
/* Functions */

#ifdef  __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*
    WXC Utilities
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
  Name:         WXC_DEBUG_LOG

  Description:  WXC library internal debug output function.
                This function is modified with SDK_WEAK_SYMBOL and normally is equivalent to OS_TPrintf processing.
                You can override this with a function of the same name in an application-specific debugging system, for example.
                
                

  Arguments:    format: Document format string representing the debug output content
                ...: Variable argument

  Returns:      None.
 *---------------------------------------------------------------------------*/
#if (WXC_DEBUG_OUTPUT == 1)
void    WXC_DEBUG_LOG(const char *format, ...);
#else
#define     WXC_DEBUG_LOG(...) ((void)0)
#endif

/*---------------------------------------------------------------------------*
  Name:         WXC_GetWmApiName

  Description:  Gets WM function's name string.

  Arguments:    id: WMApiid enumeration value representing WM function type

  Returns:      WM function name string.
 *---------------------------------------------------------------------------*/
const char *WXC_GetWmApiName(WMApiid id);

/*---------------------------------------------------------------------------*
  Name:         WXC_GetWmErrorName

  Description:  Gets WM error code's name string.

  Arguments:    err: WMErrCode enumeration value representing WM error code

  Returns:      WM error code name string.
 *---------------------------------------------------------------------------*/
const char *WXC_GetWmErrorName(WMErrCode err);

/*---------------------------------------------------------------------------*
  Name:         WXC_CheckWmApiResult

  Description:  Determines the return value from a WM function call and sends details to debug output if it is an error.
                

  Arguments:    id: WM function types
                err: Error code returned from the function

  Returns:      TRUE if the return value is legitimate. FALSE if not.
 *---------------------------------------------------------------------------*/
BOOL    WXC_CheckWmApiResult(WMApiid id, WMErrCode err);

/*---------------------------------------------------------------------------*
  Name:         WXC_CheckWmCallbackResult

  Description:  Determines the return value from a WM callback and sends details to debug output if it is an error.
                

  Arguments:    arg: Argument returned to WM function callback

  Returns:      TRUE if the resulting value is legitimate. FALSE if not
 *---------------------------------------------------------------------------*/
BOOL    WXC_CheckWmCallbackResult(void *arg);

/*---------------------------------------------------------------------------*
  Name:         WXC_GetNextAllowedChannel

  Description:  Gets the next usable channel after the selected channel.
                Loops to the bottom channel after the top channel.
                Gets the bottom channel if 0 is selected.

  Arguments:    ch: Channel position this time (1-14 or 0)

  Returns:      Next usable channel (1-14).
 *---------------------------------------------------------------------------*/
u16     WXC_GetNextAllowedChannel(int ch);

/*---------------------------------------------------------------------------*
  Name:         WXC_IsCommonGgid

  Description:  Determines whether or not the specified GGID is a shared chance encounter communication protocol GGID.

  Arguments:    gid: GGID to be determined

  Returns:      TRUE if it is a shared chance encounter communication protocol.
 *---------------------------------------------------------------------------*/
SDK_INLINE
BOOL    WXC_IsCommonGgid(u32 ggid)
{
    return ((ggid & WXC_GGID_COMMON_BIT) != 0);
}


#ifdef  __cplusplus
}       /* extern "C" */
#endif


#endif /* NITRO_WXC_COMMON_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
