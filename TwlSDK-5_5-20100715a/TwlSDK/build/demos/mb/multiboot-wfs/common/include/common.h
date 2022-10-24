/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - multiboot-wfs - common
  File:     common.h

  Copyright 2005-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8575 $
  $Author: nishimoto_takashi $
 *---------------------------------------------------------------------------*/
#ifndef __NITROSDK_DEMO_MB_MULTIBOOT_WFS_COMMON_H
#define __NITROSDK_DEMO_MB_MULTIBOOT_WFS_COMMON_H


#ifdef SDK_TWL
#include	<twl.h>
#else
#include	<nitro.h>
#endif
#include "util.h"


/******************************************************************************/
/* Constants */

/* Wireless communication parameters */
#define		WC_PARENT_DATA_SIZE_MAX		128
#define		WC_CHILD_DATA_SIZE_MAX		256

#if	!defined(SDK_MAKEGGID_SYSTEM)
#define SDK_MAKEGGID_SYSTEM(num)    (0x003FFF00 | (num))
#endif
#define GGID_WBT_FS                 SDK_MAKEGGID_SYSTEM(0x23)

extern const WMParentParam def_parent_param[1];


/* Communication settings for WFS */
#define	port_wbt            5
#define	parent_packet_max   WC_PARENT_DATA_SIZE_MAX


/* Screen transition mode */
enum
{
    MODE_SELECT,                       /* Select start options */
    MODE_ERROR,                        /* Stop due to error */
    MODE_BUSY,                         /* Startup processing under way */
    MODE_PARENT,                       /* Parent processing under way */
    MODE_CHILD,                        /* Child processing under way */
    MODE_MAX
};


/******************************************************************************/
/* Functions */

#if	defined(__cplusplus)
extern  "C"
{
#endif

/*---------------------------------------------------------------------------*
  Name:         StateCallbackForWFS

  Description:  WFS callback function.
                This is called when the state became WFS_STATE_READY.
                The WFS_GetStatus function can be used at any position to make a determination without receiving notification with this callback.
                

  Arguments:    arg       User-defined argument specified in the callback.

  Returns:      None.
 *---------------------------------------------------------------------------*/
    void    StateCallbackForWFS(void *arg);

/*---------------------------------------------------------------------------*
  Name:         AllocatorForWFS

  Description:  Dynamic allocation function for memory specified in WFS.

  Arguments:    arg       User-defined argument specified in the allocator.
                size      Required size if requesting memory allocation.
                ptr       If NULL, allocates memory. Otherwise, requests deallocation.

  Returns:      If ptr is NULL, the amount of memory in size is allocated and its pointer returned.
                If not, the ptr memory is released.
                If deallocation, the return value is simply ignored.
 *---------------------------------------------------------------------------*/
    void   *AllocatorForWFS(void *arg, u32 size, void *ptr);

/*---------------------------------------------------------------------------*
  Name:         WHCallbackForWFS

  Description:  Callback that receives status notification from the WC's wireless automatic drive.

  Arguments:    arg       Callback pointer for the notification source WM function.

  Returns:      None.
 *---------------------------------------------------------------------------*/
    void    WHCallbackForWFS(void *arg);

/*---------------------------------------------------------------------------*
  Name:         InitFrameLoop

  Description:  Initialization for game frame loop.

  Arguments:    p_key           Key information structure to initialize.

  Returns:      None.
 *---------------------------------------------------------------------------*/
    void    InitFrameLoop(KeyInfo * p_key);

/*---------------------------------------------------------------------------*
  Name:         WaitForNextFrame

  Description:  Wait until next rendering frame.

  Arguments:    p_key           Key information structure to update.

  Returns:      None.
 *---------------------------------------------------------------------------*/
    void    WaitForNextFrame(KeyInfo * p_key);

/*---------------------------------------------------------------------------*
  Name:         ModeInitialize

  Description:  Initializes the wireless module.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
    void    ModeInitialize(void);

/*---------------------------------------------------------------------------*
  Name:         ModeError

  Description:  Processing in error display screen.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
    void    ModeError(void);

/*---------------------------------------------------------------------------*
  Name:         ModeWorking

  Description:  Processing in busy screen.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
    void    ModeWorking(void);

/*---------------------------------------------------------------------------*
  Name:         ModeSelect

  Description:  Process in parent/child selection screen.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
    void    ModeSelect(void);

/*---------------------------------------------------------------------------*
  Name:         ModeMultiboot

  Description:  DS Single-Card play parent processing.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
    void    ModeMultiboot(void);

/*---------------------------------------------------------------------------*
  Name:         ModeParent

  Description:  Processing in parent communication screen.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
    void    ModeParent(void);

/*---------------------------------------------------------------------------*
  Name:         ModeChild

  Description:  Child communication screen.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
    void    ModeChild(void);


#if	defined(__cplusplus)
}                                      /* extern "C" */
#endif


/******************************************************************************/


#endif                                 /* __NITROSDK_DEMO_WBT_WBT_FS_COMMON_H */
