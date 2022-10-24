/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - include -
  File:     wxc.h

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

#ifndef	NITRO_WXC_H_
#define	NITRO_WXC_H_


#include <nitro.h>

#include <nitro/wxc/common.h>
#include <nitro/wxc/protocol.h>
#include <nitro/wxc/driver.h>
#include <nitro/wxc/scheduler.h>

#include <nitro/wxc/wxc_protocol_impl_wxc.h>


/*****************************************************************************/
/* Constants */

/* Size of internal work memory necessary for the library */
#define WXC_WORK_SIZE              0xA000


/*****************************************************************************/
/* Functions */

#ifdef  __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*
  Name:         WXC_Init

  Description:  Initializes the library.

  Arguments:    work: Work memory used in the library.
                      The memory must be at least WXC_WORK_SIZE bytes and 32-byte aligned.
                              
                callback: Sends notifications for the various library system states
                          Pointer to the callback
                dma: DMA channel used in the library

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_Init(void *work, WXCCallback callback, u32 dma);

/*---------------------------------------------------------------------------*
  Name:         WXC_Start

  Description:  Starts the wireless operation of the library.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_Start(void);

/*---------------------------------------------------------------------------*
  Name:         WXC_Stop

  Description:  Stops the wireless operation of the library.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_Stop(void);

/*---------------------------------------------------------------------------*
  Name:         WXC_End

  Description:  Closes the library.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_End(void);

/*---------------------------------------------------------------------------*
  Name:         WXC_GetStateCode

  Description:  Gets the current status of the library.

  Arguments:    None.

  Returns:      WXCStateCode type that shows the current library status.
 *---------------------------------------------------------------------------*/
WXCStateCode WXC_GetStateCode(void);

/*---------------------------------------------------------------------------*
  Name:         WXC_IsParentMode

  Description:  Determines whether the current wireless state is parent.
                Only valid in the WXC_STATE_ACTIVE state.

  Arguments:    None.

  Returns:      If the wireless status is parent mode, TRUE. If it is child mode, FALSE.
 *---------------------------------------------------------------------------*/
BOOL    WXC_IsParentMode(void);

/*---------------------------------------------------------------------------*
  Name:         WXC_GetParentParam

  Description:  Refers to the WMParentParam structure for the current wireless state.
                This is only valid in the WXC_STATE_ACTIVE state when the WXC_IsParentMode function returns TRUE.
                

  Arguments:    None.

  Returns:      WMParentParam structure.
 *---------------------------------------------------------------------------*/
const WMParentParam *WXC_GetParentParam(void);

/*---------------------------------------------------------------------------*
  Name:         WXC_GetParentBssDesc

  Description:  Refers to the WMBssDesc structure for the current connection target.
                This is only valid in the WXC_STATE_ACTIVE state when the WXC_IsParentMode function returns FALSE.
                

  Arguments:    None.

  Returns:      WMBssDesc structure
 *---------------------------------------------------------------------------*/
const WMBssDesc *WXC_GetParentBssDesc(void);

/*---------------------------------------------------------------------------*
  Name:         WXC_GetUserBitmap

  Description:  Gets the currently connected user's status with a bitmap.
                If communications are not established, it returns 0.

  Arguments:    None.

  Returns:      WMBssDesc structure
 *---------------------------------------------------------------------------*/
u16     WXC_GetUserBitmap(void);

/*---------------------------------------------------------------------------*
  Name:         WXC_GetCurrentGgid

  Description:  Gets the selected GGID in the current connection.
                If communications are not established, it returns 0.
                The current communications status can be determined by the WXC_GetUserBitmap function's return value.

  Arguments:    None.

  Returns:      The selected GGID in the current connection.
 *---------------------------------------------------------------------------*/
u32     WXC_GetCurrentGgid(void);

/*---------------------------------------------------------------------------*
  Name:         WXC_GetOwnAid

  Description:  Gets its own AID in the current connection.
                If communications are not established, it returns an inconstant value.
                The current communications status can be determined by the WXC_GetUserBitmap function's return value.

  Arguments:    None.

  Returns:      Its own AID in the current connection.
 *---------------------------------------------------------------------------*/
u16     WXC_GetOwnAid(void);

/*---------------------------------------------------------------------------*
  Name:         WXC_GetUserInfo

  Description:  Gets the user information of the user currently connected.
                You must not change the data indicated by the returned pointer.

  Arguments:    aid: User AID that gets information

  Returns:      If the specified AID is valid, user information. Otherwise, NULL.
 *---------------------------------------------------------------------------*/
const WXCUserInfo *WXC_GetUserInfo(u16 aid);

/*---------------------------------------------------------------------------*
  Name:         WXC_SetChildMode

  Description:  Sets wireless to only run as a child.
                This function must be called before the WXC_Start function is called.

  Arguments:    enable: If it can only be run on the child side, TRUE

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_SetChildMode(BOOL enable);

/*---------------------------------------------------------------------------*
  Name:         WXC_SetParentParameter

  Description:  Specifies wireless connection settings
                The configuration contents are only used when in parent mode.
                When connected in child mode, the device follows the parent's settings.
                This function is optional. Call it before the WXC_Start function only when necessary.
                

  Arguments:    sendSize: Parent send size.
                          It must be at least WXC_PACKET_SIZE_MIN and no more than WXC_PACKET_SIZE_MAX.
                              
                recvSize: Parent receive size.
                          It must be at least WXC_PACKET_SIZE_MIN and no more than WXC_PACKET_SIZE_MAX.
                              
                maxEntry: Maximum number of connected players.
                          The current implementation only supports a specified value of 1.

  Returns:      If the settings are valid, they are applied internally, and TRUE is returned. Otherwise, a warning message is sent to debug output, and FALSE is returned.
                
 *---------------------------------------------------------------------------*/
BOOL    WXC_SetParentParameter(u16 sendSize, u16 recvSize, u16 maxEntry);

/*---------------------------------------------------------------------------*
  Name:         WXC_RegisterDataEx

  Description:  Registers data for exchange

  Arguments:    ggid: GGID of the registered data
                callback: Callback function to the user (invoked when data exchange is complete)
                send_ptr: Pointer to registered data
                send_size: Size of registered data
                recv_ptr: Pointer to the receive buffer
                recv_size: Receive buffer size
                type: String representing the protocol

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_RegisterDataEx(u32 ggid, WXCCallback callback, u8 *send_ptr, u32 send_size,
                           u8 *recv_ptr, u32 recv_size, const char *type);

#define WXC_RegisterData(...) SDK_OBSOLETE_FUNCTION("WXC_RegisterData() is obsolete. use WXC_RegisterCommonData()")

/*---------------------------------------------------------------------------*
  Name:         WXC_RegisterCommonData

  Description:  Registers the conversion data as a shared chance encounter communication specification.

  Arguments:    ggid: GGID of the registered data
                callback: Callback function to the user (invoked when data exchange is complete)
                send_ptr: Pointer to registered data
                send_size: Size of registered data
                recv_ptr: Pointer to the receive buffer
                recv_size: Receive buffer size

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void WXC_RegisterCommonData(u32 ggid, WXCCallback callback,
                                          u8 *send_ptr, u32 send_size,
                                          u8 *recv_ptr, u32 recv_size)
{
    WXC_RegisterDataEx((u32)(ggid | WXC_GGID_COMMON_BIT), callback, send_ptr,
                             send_size, recv_ptr, recv_size, "COMMON");
}

/*---------------------------------------------------------------------------*
  Name:         WXC_SetInitialData

  Description:  The data block used for every exchange is set by the first exchange.

  Arguments:    ggid: GGID of the registered data
                send_ptr: Pointer to registered data
                send_size: Size of registered data
                recv_ptr: Pointer to the receive buffer
                recv_size: Receive buffer size

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_SetInitialData(u32 ggid, u8 *send_ptr, u32 send_size, u8 *recv_ptr, u32 recv_size);

/*---------------------------------------------------------------------------*
  Name:         WXC_AddData

  Description:  Data is configured by adding it to a completed block data exchange.

  Arguments:    send_buf: Send buffer
                send_size: Send buffer size
                recv_buf: Receive buffer
                recv_max: Receive buffer size

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_AddData(const void *send_buf, u32 send_size, void *recv_buf, u32 recv_max);

/*---------------------------------------------------------------------------*
  Name:         WXC_UnregisterData

  Description:  Deletes data for exchange from registration

  Arguments:    ggid: GGID related to the deleted block

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_UnregisterData(u32 ggid);

/*---------------------------------------------------------------------------*
  Name:         WXCi_SetSsid

  Description:  Configures the SSID as a child when connecting.
                Internal function.

  Arguments:    buffer: SSID data to configure
                length: SSID data length
                        Must be less than WM_SIZE_CHILD_SSID.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXCi_SetSsid(const void *buffer, u32 length);


#ifdef  __cplusplus
}       /* extern "C" */
#endif


#endif /* NITRO_WXC_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
