/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - include -
  File:     driver.h

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

#ifndef	NITRO_WXC_DRIVER_H_
#define	NITRO_WXC_DRIVER_H_

#include    <nitro.h>


/*****************************************************************************/
/* Constants */

/* Basic wireless setting */
#define     WXC_DEFAULT_PORT             4
#define     WXC_DEFAULT_PORT_PRIO        2

/* Beacon interval was previously 150 (ms), but now 90 (ms) + 20 (ms) */
#define     WXC_SCAN_TIME_MAX            (WXC_BEACON_PERIOD + 20)

#define     WXC_MAX_RATIO                100

/* Event callback type from the wireless driver */
typedef enum
{
    /* End wireless communication (argument is always NULL) */
    WXC_DRIVER_EVENT_STATE_END,
    /* Transition to STOP state completed (argument is always NULL) */
    WXC_DRIVER_EVENT_STATE_IDLE,
    /* Transition to IDLE state completed (argument is always NULL) */
    WXC_DRIVER_EVENT_STATE_STOP,
    /* Transition to MP_PARENT state completed (argument is always NULL) */
    WXC_DRIVER_EVENT_STATE_PARENT,
    /* Transition to MP_CHILD state completed (argument is always NULL) */
    WXC_DRIVER_EVENT_STATE_CHILD,
    /* Beacon update timing (argument is WMParentParam pointer) */
    WXC_DRIVER_EVENT_BEACON_SEND,
    /* Beacon detection timing (argument is WMBssDesc pointer) */
    WXC_DRIVER_EVENT_BEACON_RECV,
    /* Send MP packet (argument is WXCPacketInfo pointer) */
    WXC_DRIVER_EVENT_DATA_SEND,
    /* Receive MP packet (argument is the const WXCPacketInfo pointer) */
    WXC_DRIVER_EVENT_DATA_RECV,
    /* Pre-connection notification (the argument is the WMBssDesc pointer) */
    WXC_DRIVER_EVENT_CONNECTING,
    /* Detect connection (argument) */
    WXC_DRIVER_EVENT_CONNECTED,
    /* Detect disconnection (argument is WMStartParentCallback pointer) */
    WXC_DRIVER_EVENT_DISCONNECTED,

    WXC_DRIVER_EVENT_MAX
}
WXCDriverEvent;

/* Driver internal state */
typedef enum WXCDriverState
{
    WXC_DRIVER_STATE_END,              /* Before initialization (driver = NULL) */
    WXC_DRIVER_STATE_BUSY,             /* State in transition */
    WXC_DRIVER_STATE_STOP,             /* Stable in STOP state */
    WXC_DRIVER_STATE_IDLE,             /* Stable in IDLE state */
    WXC_DRIVER_STATE_PARENT,           /* Stable in MP_PARENT state */
    WXC_DRIVER_STATE_CHILD,            /* Stable in MP_CHILD state */
    WXC_DRIVER_STATE_ERROR             /* Error without automatic recovery */
}
WXCDriverState;


/*****************************************************************************/
/* Declaration */

/*---------------------------------------------------------------------------*
  Name:         WXCDriverEventFunc

  Description:  Event callback prototype from the wireless driver.

  Arguments:    event: Notified event
                arg: Function assigned to each event

  Returns:      u32 event result value assigned to each event.
 *---------------------------------------------------------------------------*/
typedef u32 (*WXCDriverEventFunc) (WXCDriverEvent event, void *arg);


/* Wireless driver structure inside WXC library */
typedef struct WXCDriverWork
{

    /* WM internal work */
    u8      wm_work[WM_SYSTEM_BUF_SIZE] ATTRIBUTE_ALIGN(32);
    u8      mp_send_work[WM_SIZE_MP_PARENT_SEND_BUFFER(WM_SIZE_MP_DATA_MAX, FALSE)]
        ATTRIBUTE_ALIGN(32);
    u8     
        mp_recv_work[WM_SIZE_MP_PARENT_RECEIVE_BUFFER(WM_SIZE_MP_DATA_MAX, WM_NUM_MAX_CHILD, FALSE)]
        ATTRIBUTE_ALIGN(32);
    u8      current_send_buf[WM_SIZE_MP_DATA_MAX] ATTRIBUTE_ALIGN(32);
    u16     wm_dma;                    /* WM DMA channel */
    u16     current_channel;           /* Current channel (Measure/Start) */
    u16     own_aid;                   /* This system's AID */
    u16     peer_bitmap;               /* Bitmap of connection peers */
    u16     send_size_max;             /* MP send size */
    u16     recv_size_max;             /* MP send size */
    BOOL    send_busy;                 /* Waiting for previous MP to complete   */

    /* State transition information */
    WXCDriverState state;
    WXCDriverState target;
    WXCDriverEventFunc callback;

    /* Parent device control information */
    WMParentParam *parent_param;
    BOOL    need_measure_channel;
    int     measure_ratio_min;

    /* Child device control information */
    int     scan_found_num;
    u8      padding1[20];
    WMBssDesc target_bss[1] ATTRIBUTE_ALIGN(32);
    u8      scan_buf[WM_SIZE_SCAN_EX_BUF] ATTRIBUTE_ALIGN(32);
    WMScanExParam scan_param[1] ATTRIBUTE_ALIGN(32);
    u8      ssid[24];
    u8      padding2[4];

}
WXCDriverWork;


/*****************************************************************************/
/* Functions */

#ifdef  __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*
  Name:         WXC_InitDriver

  Description:  Initializes wireless and starts the transition to IDLE.

  Arguments:    driver: Used as an internal work memory buffer
                        Pointer to WXCDriverWork structure
                        This must be 32-byte aligned.
                pp: Parent parameters
                func: Event notification callback
                dma: DMA channel assigned to wireless

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_InitDriver(WXCDriverWork * driver, WMParentParam *pp, WXCDriverEventFunc func, u32 dma);

/*---------------------------------------------------------------------------*
  Name:         WXC_SetDriverTarget

  Description:  Starts transition of a special status to the target.

  Arguments:    driver: WXCDriverWork structure
                target: State of the transition target

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_SetDriverTarget(WXCDriverWork * driver, WXCDriverState target);

/*---------------------------------------------------------------------------*
  Name:         WXC_GetDriverState

  Description:  Gets the current transition state.

  Arguments:    None.

  Returns:      The current transition state.
 *---------------------------------------------------------------------------*/
static inline WXCDriverState WXC_GetDriverState(const WXCDriverWork * driver)
{
    return driver->state;
}

/*---------------------------------------------------------------------------*
  Name:         WXC_SetDriverSsid

  Description:  Configures the SSID at connection.

  Arguments:    driver: WXCDriverWork structure
                buffer: SSID data to configure
                length: SSID data length
                        Must be less than WM_SIZE_CHILD_SSID.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_SetDriverSsid(WXCDriverWork * driver, const void *buffer, u32 length);

/*---------------------------------------------------------------------------*
  Name:         WXC_GetDriverTarget

  Description:  Gets the target current transition target state.

  Arguments:    None.

  Returns:      Current target transition target state.
 *---------------------------------------------------------------------------*/
static inline WXCDriverState WXC_GetDriverTarget(const WXCDriverWork * driver)
{
    return driver->target;
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_IsParentMode

  Description:  Determines whether the current wireless state is parent.
                Only valid in the WXC_STATE_ACTIVE state.

  Arguments:    None.

  Returns:      If the wireless status is parent mode, TRUE. If it's child mode, FALSE.
 *---------------------------------------------------------------------------*/
static inline BOOL WXCi_IsParentMode(const WXCDriverWork * driver)
{
    return (driver->state == WXC_DRIVER_STATE_PARENT);
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_GetParentBssDesc

  Description:  Gets the WMBssDesc structure for the connection target (valid only when child).

  Arguments:    None.

  Returns:      Connection target's WMBssDesc structure.
 *---------------------------------------------------------------------------*/
static inline const WMBssDesc *WXCi_GetParentBssDesc(const WXCDriverWork * driver)
{
    return driver->target_bss;
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_Stop

  Description:  Start transition of wireless state to STOP.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void WXCi_Stop(WXCDriverWork * driver)
{
    WXC_SetDriverTarget(driver, WXC_DRIVER_STATE_STOP);
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_StartParent

  Description:  Starts transition of wireless state from IDLE to MP_PARENT.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void WXCi_StartParent(WXCDriverWork * driver)
{
    WXC_SetDriverTarget(driver, WXC_DRIVER_STATE_PARENT);
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_StartChild

  Description:  Starts transition of wireless state from IDLE to MP_CHILD.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void WXCi_StartChild(WXCDriverWork * driver)
{
    WXC_SetDriverTarget(driver, WXC_DRIVER_STATE_CHILD);
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_Reset

  Description:  Resets wireless state and start transition to IDLE.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void WXCi_Reset(WXCDriverWork * driver)
{
    WXC_SetDriverTarget(driver, WXC_DRIVER_STATE_IDLE);
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_End

  Description:  Ends wireless communication.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void WXCi_End(WXCDriverWork * driver)
{
    WXC_SetDriverTarget(driver, WXC_DRIVER_STATE_END);
}


/*===========================================================================*/

#ifdef  __cplusplus
}       /* extern "C" */
#endif


#endif /* NITRO_WXC_DRIVER_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
