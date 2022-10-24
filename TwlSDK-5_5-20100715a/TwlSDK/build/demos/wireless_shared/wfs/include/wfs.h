 /*---------------------------------------------------------------------------*
  Project:  TwlSDK - wireless_shared - demos - wfs
  File:     wfs.h

  Copyright 2006-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef	NITRO_BUILD_DEMOS_WIRELESSSHARED_WFS_INCLUDE_WFS_H_
#define	NITRO_BUILD_DEMOS_WIRELESSSHARED_WFS_INCLUDE_WFS_H_


#include <nitro.h>

#ifdef __cplusplus
extern  "C" {
#endif


/*****************************************************************************/
/* Constants */


/*
 * This enumerated type indicates the internal WFS state.
 */
typedef enum
{
    WFS_STATE_STOP,
    WFS_STATE_IDLE,
    WFS_STATE_READY,
    WFS_STATE_ERROR
}
WFSState;


/*****************************************************************************/
/* Prototypes */

/*
 * This callback is invoked whenever the internal WFS state changes.
 * The FS will become usable automatically when WFS_STATE_READY is returned.
 */
typedef void (*WFSStateCallback) (void *);


/*
 * This is a dynamic memory allocation callback internal to WFS.
 * arg is a user-defined argument.
 * If ptr is NULL, return size bytes of memory.
 * If ptr is not NULL, deallocate ptr.
 */
typedef void *(*WFSAllocator) (void *arg, u32 size, void *ptr);


/*****************************************************************************/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         WFS_InitParent

  Description:  Initializes wireless file system (WFS) as parent.
                Because this function configures and automatically activates the port callback to the WM library, this must be called when WM is in a READY state or later.
                

  Arguments:    port             Port number used for internal MP communications.
                callback         Callback that receives various state notifications.
                allocator        Callback used for internal memory allocation.
                                 The amount of memory the WFS uses is decided each time application runs.
                                 
                allocator_arg    User-defined argument given to allocator.
                parent_packet    Default parent send size.
                                 This value must be WBT_PACKET_SIZE_MIN or greater, and equal to or smaller than parent maximum send size.
                                 
                                 A child device will be simply ignored.
                p_rom            The file pointer indicating the program that includes the FAT/FNT/OVT provided to the child device.
                                 
                                 Normally, this designates the child device program sent via wireless download.
                                 
                                 Seek position maintains the position before the call.
                                 When this argument is NULL, provides parent's own file system.
                                 
                use_parent_fs    When TRUE, provides parent's own FAT/FNT, not program specified with p-rom.
                                 
                                 With these configurations, an independent parent/child program can share a single file system.
                                 
                                 If p_rom is NULL, this setting is ignored, and always handled as shared.
                                 

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WFS_InitParent(int port, WFSStateCallback callback,
                       WFSAllocator allocator, void *allocator_arg, int parent_packet,
                       FSFile *p_rom, BOOL use_parent_fs);

/*---------------------------------------------------------------------------*
  Name:         WFS_InitChild

  Description:  Initializes wireless file system (WFS) as child.
                Because this function configures and automatically activates the port callback to the WM library, this must be called when WM is in a READY state or later.
                

  Arguments:    port             Port number used for internal MP communications.
                callback         Callback that receives various state notifications.
                allocator        Callback used for internal memory allocation.
                                 The amount of memory the WFS uses is decided each time application runs.
                                 
                allocator_arg    User-defined argument given to allocator.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WFS_InitChild(int port, WFSStateCallback callback,
                      WFSAllocator allocator, void *allocator_arg);

/*---------------------------------------------------------------------------*
  Name:         WFS_Init

  Description:  Initializes wireless file system (WFS).
                After initializing internal state as parent or child, this function configures and automatically activates the port callback to the WM library. This function must be called when WM is in a READY state or later.
                
                

  Arguments:    is_parent        If wireless parent, TRUE. If child, FALSE.
                                 For actual multiboot, this argument is given !MB_IsMultiBootChild().
                                 
                port             Port number used for internal MP communications.
                parent_packet    Default parent send size.
                                 This value must be WBT_PACKET_SIZE_MIN or greater, and equal to or smaller than parent maximum send size.
                                 
                                 A child device will be simply ignored.
                callback         Callback that receives various state notifications.
                allocator        Callback used for internal memory allocation.
                                 The amount of memory the WFS uses is decided each time application runs.
                                 
                allocator_arg    User-defined argument given to allocator.

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void WFS_Init(BOOL is_parent, int port, int parent_packet,
                         WFSStateCallback callback, WFSAllocator allocator, void *allocator_arg)
{
    if (is_parent)
        WFS_InitParent(port, callback, allocator, allocator_arg, parent_packet, NULL, TRUE);
    else
        WFS_InitChild(port, callback, allocator, allocator_arg);
}

/*---------------------------------------------------------------------------*
  Name:         WFS_Start

  Description:  For WFS, sends notification that wireless has entered MP state.
                After this notification, WFS uses WM_SetMPDataToPort() and runs automatically with "Priority: low (WM_PRIORITY_LOW)".
               
                For this reason, it must be called in MP_PARENT or MP_CHILD state.
                
                Normally, this function is called in the WM_StartMP() callback after WM_STATECODE_MP_START is passed to the parameter state.
                

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WFS_Start(void);

/*---------------------------------------------------------------------------*
  Name:         WFS_End

  Description:  Called when WFS is not needed.
                Deallocates all internally allocated memory and returns to state before initialization.
                Normally, this is called when all wireless communications have been disconnected.
                This function cannot be called from the interrupt handler.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WFS_End(void);

/*---------------------------------------------------------------------------*
  Name:         WFS_GetStatus

  Description:  Gets current internal state of WFS in WFSState type.

  Arguments:    None.

  Returns:      One of WFSState-type enumerated values.
 *---------------------------------------------------------------------------*/
WFSState WFS_GetStatus(void);

/*---------------------------------------------------------------------------*
  Name:         WFS_GetCurrentBitmap

  Description:  Gets the current child device group bitmap that WBT is aware of.
                This function can only be called by the parent.

  Arguments:    None.

  Returns:      Currently recognized child devices.
 *---------------------------------------------------------------------------*/
int     WFS_GetCurrentBitmap(void);

/*---------------------------------------------------------------------------*
  Name:         WFS_GetSyncBitmap

  Description:  Gets bitmap of child group with access sync specified.
                This function can only be called by the parent.

  Arguments:    None.

  Returns:      Child devices that are specified to synchronize access
 *---------------------------------------------------------------------------*/
int     WFS_GetSyncBitmap(void);

/*---------------------------------------------------------------------------*
  Name:         WFS_GetBusyBitmap

  Description:  Gets bitmap of child devices that are currently accessing parent.
                This function can only be called by the parent.

  Arguments:    None.

  Returns:      Child devices that are currently accessing parent.
 *---------------------------------------------------------------------------*/
int     WFS_GetBusyBitmap(void);

/*---------------------------------------------------------------------------*
  Name:         WFS_IsBusy

  Description:  Determines whether the child device of a designated AID is being accessed.
                This function can only be called by the parent.

  Arguments:    aid              AID of child to check.

  Returns:      If the designated child device is being accessed, TRUE. Otherwise, FALSE.
 *---------------------------------------------------------------------------*/
BOOL    WFS_IsBusy(int aid);

/*---------------------------------------------------------------------------*
  Name:         WFS_GetCurrentDownloadProgress

  Description:  Gets the progress status of the file that child is currently performing ReadFile with.
                The unit will be the WBT internal sequence number.
                This function can only be called by children.

  Arguments:    current_count    Address to the variable storing the number of currently received sequences.
                                 
                total_count      Address to the variable storing the total number of sequences that should be received.
                                 

  Returns:      If this is the current ReadFile state and the correct progress can be obtained, TRUE.
                Otherwise, returns FALSE.
 *---------------------------------------------------------------------------*/
BOOL    WFS_GetCurrentDownloadProgress(int *current_count, int *total_count);

/*---------------------------------------------------------------------------*
  Name:         WFS_GetPacketSize

  Description:  Gets MP communication packet size of parent set in WFS.

  Arguments:    None.

  Returns:      MP communication packet size of parent set in WFS.
 *---------------------------------------------------------------------------*/
int     WFS_GetPacketSize(void);

/*---------------------------------------------------------------------------*
  Name:         WFS_SetPacketSize

  Description:  Sets parent MP communication packet size.
                If this value is increased, it will improve transmission efficiency. If it is decreased, the single MP communication transmission will be bundled with separate port communications such as data sharing, and unilateral slowdowns due to the port priority can be avoided.
                
                
                This function can only be called by the parent.

  Arguments:    size             Parent send size to set.
                                 This value must be WBT_PACKET_SIZE_MIN or greater, and equal to or smaller than parent maximum send size.
                                 

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WFS_SetPacketSize(int size);

/*---------------------------------------------------------------------------*
  Name:         WFS_EnableSync

  Description:  Configures the settings of the child device group that takes access synchronization on the parent device side.
                This function achieves efficient transmission rates that utilities unique characteristics of the WBT library; this is done by synchronizing responses to child devices that are all guaranteed to access the same files in precisely the same order.
                
                
                However, be cautious of the fact that if the synchronization start timing is not logically safe, responses to child devices will become out-of-sync, causing deadlocks to occur.
                
                This function can only be called by the parent.

  Arguments:    sync_bitmap      The AID bitmap of the child device group that takes the access synchronization.
                                 The lowest bit 1 indicating the parent device itself is ignored.
                                 By assigning 0 for this value, synchronicity does not occur.
                                 This is the default state.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WFS_EnableSync(int sync_bitmap);

/*---------------------------------------------------------------------------*
  Name:         WFS_DisableSync

  Description:  The parent device releases access synchronization among child devices.
                This function is equivalent to specifying WFS_EnableSync() 0.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void WFS_DisableSync(void)
{
    WFS_EnableSync(0);
}

/*---------------------------------------------------------------------------*
  Name:         WFS_SetDebugMode

  Description:  Enables/disables WFS internal debug output.
                The default setting is FALSE.

  Arguments:    enable_debug     TRUE if debug output is enabled. FALSE if disabled.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WFS_SetDebugMode(BOOL enable_debug);


/*****************************************************************************/


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* NITRO_BUILD_DEMOS_WIRELESSSHARED_WFS_INCLUDE_WFS_H_ */
