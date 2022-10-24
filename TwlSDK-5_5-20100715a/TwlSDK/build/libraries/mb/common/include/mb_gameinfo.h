/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - include
  File:     mb_gameinfo.h

  Copyright 2007-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

/*
 * This header file is used only by the internal implementation and child devices.
 * This is not required to create a normal multiboot parent.
 */


#ifndef MB_GAME_INFO_H_
#define MB_GAME_INFO_H_

#ifdef __cplusplus
extern "C" {
#endif


#include <nitro/types.h>
#include <nitro/wm.h>
#include <nitro/mb/mb_gameinfo.h>


//=============================================================================
// 
// Data type definitions
//
//=============================================================================

#define MB_GAMEINFO_PARENT_FLAG                     (0x0001)
#define MB_GAMEINFO_CHILD_FLAG( __child_aid__ )     ( 1 << ( __child_aid__ ) )

//---------------------------------------------------------
// Game information distributed from a parent device to a child device by a beacon
//---------------------------------------------------------

/*
 * List structure for receiving a child device's game information
 */
typedef struct MBGameInfoRecvList
{
    MBGameInfo gameInfo;               // Parent game information
    WMBssDesc bssDesc;                 // Information for a parent connection.
    u32     getFragmentFlag;           // Indicates in bits the fragmentary beacon that, at present, was received
    u32     allFragmentFlag;           // Value of fragmentMaxNum converted into bits
    u16     getPlayerFlag;             // Indicates in bits the player flags that, at present, have already been received
    s16     lifetimeCount;             // Lifetime counter for this information (if this parent's beacon is received, lifetime will be prolonged)
    u16     linkLevel;                 /* Value (expressed in four stages) that indicates the beacon reception strength from the parent */
    u8      beaconNo;                  // Number of the beacon that was last received
    u8      sameBeaconRecvCount;       // The number of times the same beacon number was consecutively received
}
MBGameInfoRecvList, MbGameInfoRecvList;


/*
 * Structure that shows the reception status of beacons on the child device
 */
typedef struct MbBeaconRecvStatus
{
    u16     usingGameInfoFlag;         // Shows in bits the gameInfo array elements being used to receive game information
    u16     usefulGameInfoFlag;        // Receives all beacons at once and shows game information for which validGameInfoFlag is on
    // (The validGameInfoFlag may temporarily go down when updating communication members and so on. For display and connection, use this flag to make a determination)
    u16     validGameInfoFlag;         // Shows in bits the gameInfo array elements that have completely received game information
    u16     nowScanTargetFlag;         // Shows in bits the current scan target
    s16     nowLockTimeCount;          // Remaining lock time of the current Scan target
    s16     notFoundLockTargetCount;   // Number of times the current ScanTarget was consecutively not found
    u16     scanCountUnit;             // Value of the current scan time converted into a count number
    u8      pad[2];
    MBGameInfoRecvList list[MB_GAME_INFO_RECV_LIST_NUM];        // List for receiving game information
}
MbBeaconRecvStatus;
/*
 * Note: The child device has finished getting parent information when getFragmentFlag == allFragmentFlag and getPlayerFlag == gameInfo.volat.nowPlayerFlag
 * 
 * 
 * 
 */


/*
 * The msg returned by the callback functions for MB_RecvGameInfoBeacon and MB_CountGameInfoLifetime
 * 
 */
typedef enum MbBeaconMsg
{
    MB_BC_MSG_GINFO_VALIDATED = 1,
    MB_BC_MSG_GINFO_INVALIDATED,
    MB_BC_MSG_GINFO_LOST,
    MB_BC_MSG_GINFO_LIST_FULL,
    MB_BC_MSG_GINFO_BEACON
}
MbBeaconMsg;

typedef void (*MBBeaconMsgCallback) (MbBeaconMsg msg, MBGameInfoRecvList * gInfop, int index);

typedef void (*MbScanLockFunc) (u8 *macAddress);
typedef void (*MbScanUnlockFunc) (void);

/******************************************************************************/
/* The following are used internally */


//------------------
// Parent-side functions
//------------------

    // Initializes the send status
void    MB_InitSendGameInfoStatus(void);

    // Creates an MbGameInfo from the MbGameRegistry
void    MBi_MakeGameInfo(MBGameInfo *gameInfop,
                         const MBGameRegistry *mbGameRegp, const MBUserInfo *parent);

    // Updates MBGameInfo's child device member information
void    MB_UpdateGameInfoMember(MBGameInfo *gameInfop,
                                const MBUserInfo *member, u16 nowPlayerFlag, u16 changePlayerFlag);

    // Adds a created MBGameInfo to the send list so that it will be transmitted by a beacon
void    MB_AddGameInfo(MBGameInfo *newGameInfop);

    // Deletes the MBGameInfo that has been added to the send list
BOOL    MB_DeleteGameInfo(MBGameInfo *gameInfop);

    // Puts the MBGameInfo that is registered with the send list on a beacon and transmits it
void    MB_SendGameInfoBeacon(u32 ggid, u16 tgid, u8 attribute);


//------------------
// Child-side functions
//------------------
    // Statically allocates the beacon receive status buffer
void    MBi_SetBeaconRecvStatusBufferDefault(void);
    // Sets the beacon receive status buffer
void    MBi_SetBeaconRecvStatusBuffer(MbBeaconRecvStatus * buf);

    // Determines whether the obtained beacon represents a multiboot parent device
BOOL    MBi_CheckMBParent(WMBssDesc *bssDescp);

    // Initializes the receive status
void    MB_InitRecvGameInfoStatus(void);

    // Extracts MBGameInfo from a received beacon
BOOL    MB_RecvGameInfoBeacon(MBBeaconMsgCallback Callbackp, u16 linkLevel, WMBssDesc *bssDescp);

    // Lifetime count for the parent information list
void    MB_CountGameInfoLifetime(MBBeaconMsgCallback Callbackp, BOOL found_parent);

    // Sets the scan lock function
void    MBi_SetScanLockFunc(MbScanLockFunc lockFunc, MbScanUnlockFunc unlockFunc);

    // Gets a pointer to a received parent information structure
MBGameInfoRecvList *MB_GetGameInfoRecvList(int index);

/* Gets the beacon reception status */
const MbBeaconRecvStatus *MB_GetBeaconRecvStatus(void);

/* Deletes the specified game information */
void    MB_DeleteRecvGameInfo(int index);
void    MB_DeleteRecvGameInfoWithoutBssdesc(int index);


#ifdef __cplusplus
}
#endif

#endif // MB_GAME_INFO_H_
