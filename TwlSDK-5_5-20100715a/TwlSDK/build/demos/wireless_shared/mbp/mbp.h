 /*---------------------------------------------------------------------------*
  Project:  TwlSDK - wireless_shared - demos - mbp
  File:     mbp.h

  Copyright 2006-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef NITROSDK_DEMO_WIRELESSSHARED_MBP_H_
#define NITROSDK_DEMO_WIRELESSSHARED_MBP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/mb.h>


/******************************************************************************/
/* Constants */

/*
 * Game group ID
 *
 * You can now set a GGID for individual files with the MBGameRegistry structure, so you may not need to specify values with the MB_Init function in the future.
 * 
 * 
 */

/*
 * Channel for the parent device to distribute
 *
 * The multiboot child cycles through all possible channels, so you may use one of the values permitted by the WM library (currently 3, 8, and 13).
 * 
 *
 * Make this variable to avoid communication congestion in user applications.
 * Applications will decide when to change channels, but it is possible that the user will be given the opportunity to press a Restart button, for example, to deal with poor responsiveness.
 * 
 * 
 */

/* Maximum number of connectable child devices */
#define MBP_CHILD_MAX       (15)

/* DMA number to allocate to the MB library */
#define MBP_DMA_NO          (2)

/*
 * Switch that uses the MB_StartParentFromIdle and MB_EndToIdle functions in the MB library.
 * You can run multiboot processing through the IDLE state.
 */
#define MBP_USING_MB_EX

/*
 * Switch to start sending data in advance to a child entry.
 * Because data is sent immediately after entry, you can decrease the time spent waiting for downloads if you start before the maximum number of children is reached.
 * 
 */
#define MBP_USING_PREVIOUS_DOWNLOAD


/* MB parent device state */
typedef struct
{
    u16     state;                     // Parent state
    u16     connectChildBmp;           // Flag for all connected children
    u16     requestChildBmp;           // Flag for children that are requesting entry
    u16     entryChildBmp;             // Flag for children that are waiting for data
    u16     downloadChildBmp;          // Flag for children that are downloading data
    u16     bootableChildBmp;          // Flag for children that have completed downloading
    u16     rebootChildBmp;            // Flag for children that sent boot
}
MBPState;


/* Connected child device state */
typedef struct
{
    MBUserInfo user;
    u8      macAddress[6];
    u16     playerNo;
}
MBPChildInfo;


/* The game sequence state value of this demo */
enum
{
    MBP_STATE_STOP,                    // Stop state
    MBP_STATE_IDLE,                    // Idle state (Init done)
    MBP_STATE_ENTRY,                   // Child entry accepting
    MBP_STATE_DATASENDING,             // MP data transmitting
    MBP_STATE_REBOOTING,               // Child rebooting
    MBP_STATE_COMPLETE,                // Child reboot completed
    MBP_STATE_CANCEL,                  // Cancelling multiboot
    MBP_STATE_ERROR,                   // Error occurred
    MBP_STATE_NUM
};

/* Type of connected state bitmap */
typedef enum
{
    MBP_BMPTYPE_CONNECT,               // Connected child information
    MBP_BMPTYPE_REQUEST,               // Information of the child requesting connection
    MBP_BMPTYPE_ENTRY,                 // Information of the child waiting for downloading after the entry
    MBP_BMPTYPE_DOWNLOADING,           // Information on the child that is downloading
    MBP_BMPTYPE_BOOTABLE,              // Bootable child
    MBP_BMPTYPE_REBOOT,                // Reboot completed child
    MBP_BMPTYPE_NUM
}
MBPBmpType;

/* Multiboot child device state */
typedef enum
{
    MBP_CHILDSTATE_NONE,               // No connection
    MBP_CHILDSTATE_CONNECTING,         // Connecting
    MBP_CHILDSTATE_REQUEST,            // Requesting connection
    MBP_CHILDSTATE_ENTRY,              // Undergoing entry
    MBP_CHILDSTATE_DOWNLOADING,        // Downloading
    MBP_CHILDSTATE_BOOTABLE,           // Boot waiting
    MBP_CHILDSTATE_REBOOT,             // Rebooting
    MBP_CHILDSTATE_NUM
}
MBPChildState;


/******************************************************************************/
/* Variables */

/* Parent device initialization */
void    MBP_Init(u32 ggid, u32 tgid);
void    MBP_Start(const MBGameRegistry *gameInfo, u16 channel);

/* Parent device main process in each single frame */
const MBPState *MBP_Main(void);

void    MBP_KickChild(u16 child_aid);
void    MBP_AcceptChild(u16 child_aid);
void    MBP_StartRebootAll(void);
void    MBP_StartDownload(u16 child_aid);
void    MBP_StartDownloadAll(void);
BOOL    MBP_IsBootableAll(void);

void    MBP_Cancel(void);

u16     MBP_GetState(void);
u16     MBP_GetChildBmp(MBPBmpType bmpType);
void    MBP_GetConnectState(MBPState * state);
const u8 *MBP_GetChildMacAddress(u16 aid);
MBPChildState MBP_GetChildState(u16 aid);
u16     MBP_GetPlayerNo(const u8 *macAddress);
const MBPChildInfo *MBP_GetChildInfo(u16 child_aid);


#ifdef __cplusplus
}/* extern "C" */
#endif

#endif // NITROSDK_DEMO_WIRELESSSHARED_MBP_H_
