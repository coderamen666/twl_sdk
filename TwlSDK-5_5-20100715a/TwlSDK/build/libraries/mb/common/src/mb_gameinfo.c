/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - libraries
  File:     mb_gameinfo.c

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

#include <nitro.h>
#include "mb_gameinfo.h"
#include "mb_common.h"
#include "mb_private.h"                // for MB_SCAN_TIME_NORMAL & MB_SCAN_TIME_LOKING & MB_OUTPUT, MB_DEBUG_OUTPUT


/*

   ********** PRECAUTIONS **********
   - During a scan lock, the scan time should be set to be greater than the parent's beacon interval. (200ms or greater)
     However, if this is done the scan time will change dynamically, so what will be done about values that have already been counted, and the maximum value of LifetimeCount and LockTimeCount?
    

*/


/*
 * The wmDEMOlib functions that are being used:
 *  - none
 *
 * The WM API that is being used:
 *  - WM_SetGameInfo (MBi_SendFixedBeacon, MBi_SendVolatBeacon)
 *
 *
 *
 *
 */

// define data---------------------------------------------

#define MB_GAME_INFO_LIFETIME_SEC       60      // Lifetime of the received game information (deleted if a beacon is not received in this time period (seconds))
#define MB_SCAN_LOCK_SEC                8       // Maximum time for locking a specific parent and scanning
#define MB_SAME_BEACON_RECV_MAX_COUNT   3       // The maximum number of times that a parent's beacon can be continuously received when the parent repeatedly sends out the same beacon.

#define MB_BEACON_DATA_SIZE         (WM_SIZE_USER_GAMEINFO - 8)
#define MB_BEACON_FIXED_DATA_SIZE   (MB_BEACON_DATA_SIZE   - 6) // The data size of the fixed region for the parent device's game information in the beacon.
#define MB_BEACON_VOLAT_DATA_SIZE   (MB_BEACON_DATA_SIZE   - 8) // The data size of the volatile region for the parent device's game information in the beacon.
#define MB_SEND_MEMBER_MAX_NUM      (MB_BEACON_VOLAT_DATA_SIZE / sizeof(MBUserInfo))    // The maximum amount of member information that can be send at one time in the beacon.

#define FIXED_NORMAL_SIZE            sizeof(MBGameInfoFixed)    // The normal size of the game information's fixed region.
#define FIXED_NO_ICON_SIZE          (sizeof(MBGameInfoFixed) - sizeof(MBIconInfo))      // The size of the game information's fixed region without icons.
#define FIXED_FRAGMENT_MAX(size)    ( ( size / MB_BEACON_FIXED_DATA_SIZE) + ( size % MB_BEACON_FIXED_DATA_SIZE ? 1 : 0 ) )
                                                                                    // The number of beacon fragments of the game information's fixed region.

#define MB_LIFETIME_MAX_COUNT       ( MB_GAME_INFO_LIFETIME_SEC * 1000 / MB_SCAN_TIME_NORMAL + 1 )
#define MB_LOCKTIME_MAX_COUNT       ( MB_SCAN_LOCK_SEC          * 1000 / MB_SCAN_TIME_NORMAL + 1 )
                                                                                    // This is the parent's game information lifetime converted into ScanTime units.
                                                                                    // This is the scan lock time converted into ScanTime units.
#define MB_SCAN_COUNT_UNIT_NORMAL   ( 1 )
#define MB_SCAN_COUNT_UNIT_LOCKING  ( MB_SCAN_TIME_LOCKING / MB_SCAN_TIME_NORMAL )



typedef enum MbBeaconState
{
    MB_BEACON_STATE_STOP = 0,
    MB_BEACON_STATE_READY,
    MB_BEACON_STATE_FIXED_START,
    MB_BEACON_STATE_FIXED,
    MB_BEACON_STATE_VOLAT_START,
    MB_BEACON_STATE_VOLAT,
    MB_BEACON_STATE_NEXT_GAME
}
MbBeaconState;


/* Beacon Format Structure */
typedef struct MbBeacon
{
    u32     ggid;                      // GGID
    u8      dataAttr:2;                // Data attributes (designated by MbBeaconDataAttr)
    u8      fileNo:6;                  // File Number
    u8      seqNoFixed;                // Fixed data sequence number (will be added if there was an update to the data contents)
    u8      seqNoVolat;                // Volat data sequence number (will be added if there was an update to the data contents)
    u8      beaconNo;                  // Beacon number (increments for each beacon transmission)
    /* Original MbBeaconData */
    union
    {
        struct
        {                              // When sending MbGameInfoFixed
            u16     sum;               // 16-bit checksum
            u8      fragmentNo;        // The current number of the fragmented data.
            u8      fragmentMaxNum;    // The maximum number of the fragmented data.
            u8      size;              // Data size.
            u8      rsv;
            u8      data[MB_BEACON_FIXED_DATA_SIZE];    // The data itself.
        }
        fixed;
        struct
        {                              // When sending MbGameInfoVolatile
            u16     sum;               // 16-bit checksum
            u8      nowPlayerNum;      // Current number of players
            u8      pad[1];
            u16     nowPlayerFlag;     // Shows in bits the player numbers of all the current players.
            u16     changePlayerFlag;  // Shows in bits the numbers of the player information that changed at this update. (Used for judgment only at the instant when the sequence number has changed.)
            MBUserInfo member[MB_SEND_MEMBER_MAX_NUM];  // Send the user information of each child device in increments of MB_SEND_MEMBER_MAX_NUM (end if the PlayerNo == 15 (parent device))
            u8      userVolatData[MB_USER_VOLAT_DATA_SIZE];     // The data that can be set by the user
        }
        volat;
    }
    data;

}
MbBeacon;

/* The parent-side structure for the beacon-transmission status */
typedef struct MbBeaconSendStatus
{
    MBGameInfo *gameInfoListTop;       // Pointer to the top of the game information list (a one-way list)
    MBGameInfo *nowGameInfop;          // Pointer to the game information that is currently being sent.
    u8     *srcp;                      // Pointer to the source of the game information that is currently being sent.
    u8      state;                     // The beacon transmission state. (Transmission of fixed data part, dynamic status part. The state changes when each kind of data is completely sent.)
    u8      seqNoFixed;                // Fixed region's sequence number
    u8      seqNoVolat;                // Volatile region's sequence number
    u8      fragmentNo;                //   @ Fragment number of a region (if a fixed region)
    u8      fragmentMaxNum;            //   @ Fragment count of a region (if a fixed region)
    u8      beaconNo;
    u8      pad[2];
}
MbBeaconSendStatus;



// function's prototype------------------------------------
static BOOL MBi_ReadIconInfo(const char *filePathp, MBIconInfo *iconp, BOOL char_flag);
static void MBi_ClearSendStatus(void);
static BOOL MBi_ReadyBeaconSendStatus(void);
static void MBi_InitSendFixedBeacon(void);
static void MBi_SendFixedBeacon(u32 ggid, u16 tgid, u8 attribute);
static void MBi_InitSendVolatBeacon(void);
static void MBi_SendVolatBeacon(u32 ggid, u16 tgid, u8 attribute);

static void MBi_SetSSIDToBssDesc(WMBssDesc *bssDescp, u32 ggid);
static int MBi_GetStoreElement(WMBssDesc *bssDescp, MBBeaconMsgCallback Callbackp);
static void MBi_CheckCompleteGameInfoFragments(int index, MBBeaconMsgCallback Callbackp);
static void MBi_AnalyzeBeacon(WMBssDesc *bssDescp, int index, u16 linkLevel);
static void MBi_CheckTGID(WMBssDesc *bssDescp, int inex);
static void MBi_CheckSeqNoFixed(int index);
static void MBi_CheckSeqNoVolat(int index);
static void MBi_InvalidateGameInfoBssID(u8 *bssidp);
static void MBi_RecvFixedBeacon(int index);
static void MBi_RecvVolatBeacon(int index);

static void MBi_LockScanTarget(int index);
static void MBi_UnlockScanTarget(void);
static int mystrlen(u16 *str);


// const data----------------------------------------------

// global variables----------------------------------------

static MbBeaconSendStatus mbss;        // Beacon send status.

static MbBeaconRecvStatus mbrs;        // Beacon receive status
static MbBeaconRecvStatus *mbrsp = NULL;        // Beacon receive status

// static variables----------------------------------------
static MbScanLockFunc sLockFunc = NULL; // Pointer to the function for setting a scan lock.
static MbScanUnlockFunc sUnlockFunc = NULL;     // Pointer to the function for releasing a scan lock.

static MbBeacon bsendBuff ATTRIBUTE_ALIGN(32);  // Beacon send buffer.
static MbBeacon *brecvBuffp;           // Beacon receive buffer.
static WMBssDesc bssDescbuf ATTRIBUTE_ALIGN(32);        // BssDesc temporary buffer.

static MBSendVolatCallbackFunc sSendVolatCallback = NULL;       // User data transmission callback
static u32 sSendVolatCallbackTiming;  // Timing at which the transmission callback is issued.

// function's description-----------------------------------------------


//=========================================================
// Operations for the work buffer that receives beacons
//=========================================================
// Obtains the work buffer that is set.
const MbBeaconRecvStatus *MB_GetBeaconRecvStatus(void)
{
    return mbrsp;
}

// Sets the work buffer from a static variable (for compatibility with older versions).
void MBi_SetBeaconRecvStatusBufferDefault(void)
{
    mbrsp = &mbrs;
}

// Sets the work buffer for receiving beacons.
void MBi_SetBeaconRecvStatusBuffer(MbBeaconRecvStatus * buf)
{
    mbrsp = buf;
}


// Sets the function for scan lock
void MBi_SetScanLockFunc(MbScanLockFunc lock, MbScanUnlockFunc unlock)
{
    sLockFunc = lock;
    sUnlockFunc = unlock;
}



//=========================================================
// Beacon transmission of game information by the parent device
//=========================================================

// Generate the parent device's game information to be sent by a beacon.
void MBi_MakeGameInfo(MBGameInfo *gameInfop,
                      const MBGameRegistry *mbGameRegp, const MBUserInfo *parent)
{
    BOOL    icon_disable;

    // Clear all for the time being.
    MI_CpuClear16(gameInfop, sizeof(MBGameInfo));

    // Register the icon data.
    gameInfop->dataAttr = MB_BEACON_DATA_ATTR_FIXED_NORMAL;
    icon_disable = !MBi_ReadIconInfo(mbGameRegp->iconCharPathp, &gameInfop->fixed.icon, TRUE);
    icon_disable |= !MBi_ReadIconInfo(mbGameRegp->iconPalettePathp, &gameInfop->fixed.icon, FALSE);

    if (icon_disable)
    {
        gameInfop->dataAttr = MB_BEACON_DATA_ATTR_FIXED_NO_ICON;
        MI_CpuClearFast(&gameInfop->fixed.icon, sizeof(MBIconInfo));
    }

    // GGID
    gameInfop->ggid = mbGameRegp->ggid;

    // Register the parent's user information.
    if (parent != NULL)
    {
        MI_CpuCopy16(parent, &gameInfop->fixed.parent, sizeof(MBUserInfo));
    }

    // Register the maximum number of players.
    gameInfop->fixed.maxPlayerNum = mbGameRegp->maxPlayerNum;

    // Register the game name and a description of the game content.
    {
        int     len;

#define COPY_GAME_INFO_STRING   MI_CpuCopy16

        len = mystrlen(mbGameRegp->gameNamep) << 1;
        COPY_GAME_INFO_STRING((u8 *)mbGameRegp->gameNamep, gameInfop->fixed.gameName, (u16)len);
        // Data after the NULL character is also sent so that data can be packed in after the game content explanation.
        len = MB_GAME_INTRO_LENGTH * 2;
        COPY_GAME_INFO_STRING((u8 *)mbGameRegp->gameIntroductionp,
                              gameInfop->fixed.gameIntroduction, (u16)len);
    }

    // Register the parent device's portion of the player information.
    gameInfop->volat.nowPlayerNum = 1;
    gameInfop->volat.nowPlayerFlag = 0x0001;    // The player number of the parent device is 0.
    gameInfop->broadcastedPlayerFlag = 0x0001;

    // NOTE: There is no registration at multiboot startup since there are no play members.
}


// Read icon data.
static BOOL MBi_ReadIconInfo(const char *filePathp, MBIconInfo *iconp, BOOL char_flag)
{
    FSFile  file;
    s32     size = char_flag ? MB_ICON_DATA_SIZE : MB_ICON_PALETTE_SIZE;
    u16    *dstp = char_flag ? iconp->data : iconp->palette;

    if (filePathp == NULL)
    {                                  // Returns FALSE if there is no file designation.
        return FALSE;
    }

    FS_InitFile(&file);

    if (FS_OpenFileEx(&file, filePathp, FS_FILEMODE_R) == FALSE)
    {                                  // Returns FALSE if the file could not be opened.
        MB_DEBUG_OUTPUT("\t%s : file open error.\n", filePathp);
        return FALSE;
    }
    else if ((u32)size != FS_GetFileLength(&file))
    {
        MB_DEBUG_OUTPUT("\t%s : different file size.\n", filePathp);
        (void)FS_CloseFile(&file);
        return FALSE;
    }

    (void)FS_ReadFile(&file, dstp, size);
    (void)FS_CloseFile(&file);
    return TRUE;
}


// Update the volatile portion of the game information.
void MB_UpdateGameInfoMember(MBGameInfo *gameInfop,
                             const MBUserInfo *member, u16 nowPlayerFlag, u16 changePlayerFlag)
{
    int     i;
    u8      playerNum = 1;

    MI_CpuCopy16(member, &gameInfop->volat.member[0], sizeof(MBUserInfo) * MB_MEMBER_MAX_NUM);
    /* Count the number of child devices */
    for (i = 0; i < MB_MEMBER_MAX_NUM; i++)
    {
        if (nowPlayerFlag & (0x0002 << i))
        {
            playerNum++;
        }
    }
    gameInfop->volat.nowPlayerNum = playerNum;
    gameInfop->volat.nowPlayerFlag = (u16)(nowPlayerFlag | 0x0001);
    gameInfop->volat.changePlayerFlag = changePlayerFlag;
    gameInfop->seqNoVolat++;
}


// Calculate the length of a character string.
static int mystrlen(u16 *str)
{
    int     len = 0;
    while (*str++)
        len++;
    return len;
}


// Adds the game information to the send list.
void MB_AddGameInfo(MBGameInfo *newGameInfop)
{
    MBGameInfo *gInfop = mbss.gameInfoListTop;

    if (!gInfop)
    {                                  // If the top of mbss is NULL, register at the top.
        mbss.gameInfoListTop = newGameInfop;
    }
    else
    {                                  // Otherwise, follow the list and add to the end.
        while (gInfop->nextp != NULL)
        {
            gInfop = gInfop->nextp;
        }
        gInfop->nextp = newGameInfop;
    }
    newGameInfop->nextp = NULL;        // 'next' of the added game information is the end.
}


// Deletes the game information from the send list.
BOOL MB_DeleteGameInfo(MBGameInfo *gameInfop)
{
    MBGameInfo *gInfop = mbss.gameInfoListTop;
    MBGameInfo *before;

    while (gInfop != NULL)
    {
        if (gInfop != gameInfop)
        {
            before = gInfop;
            gInfop = gInfop->nextp;
            continue;
        }

        // It matched, so it is deleted from the list.
        if ((u32)gInfop == (u32)mbss.gameInfoListTop)
        {                              // when deleting the top of the list
            mbss.gameInfoListTop = mbss.gameInfoListTop->nextp;
        }
        else
        {
            before->nextp = gInfop->nextp;
        }

        if ((u32)gameInfop == (u32)mbss.nowGameInfop)   // If the game information to delete is currently being transmitted, re-set that game information from the rebuilt list.
        {                              //  
            mbss.nowGameInfop = NULL;
            if (!MBi_ReadyBeaconSendStatus())
            {
                mbss.state = MB_BEACON_STATE_READY;     // If the game information has been completely deleted, the status shifts to READY.
            }
        }
        return TRUE;
    }

    // When there is no game information
    return FALSE;
}


// Initializes the game information send configuration.
void MB_InitSendGameInfoStatus(void)
{
    mbss.gameInfoListTop = NULL;       // Delete the entire game information list.
    mbss.nowGameInfop = NULL;          // Also delete the current transmitted game.
    mbss.state = MB_BEACON_STATE_READY;
    sSendVolatCallback = NULL;
    MBi_ClearSendStatus();             // Also clear the send status.
}


// Clears the send status.
static void MBi_ClearSendStatus(void)
{
    mbss.seqNoFixed = 0;
    mbss.seqNoVolat = 0;
    mbss.fragmentNo = 0;
    mbss.fragmentMaxNum = 0;
    mbss.beaconNo = 0;
}


// Sends a beacon.
void MB_SendGameInfoBeacon(u32 ggid, u16 tgid, u8 attribute)
{
    while (1)
    {
        switch (mbss.state)
        {
        case MB_BEACON_STATE_STOP:
        case MB_BEACON_STATE_READY:
            if (!MBi_ReadyBeaconSendStatus())
            {                          // Preparations for sending game information.
                return;
            }
            break;
        case MB_BEACON_STATE_FIXED_START:
            MBi_InitSendFixedBeacon();
            break;
        case MB_BEACON_STATE_FIXED:
            MBi_SendFixedBeacon(ggid, tgid, attribute);
            return;
        case MB_BEACON_STATE_VOLAT_START:
            MBi_InitSendVolatBeacon();
            break;
        case MB_BEACON_STATE_VOLAT:
            MBi_SendVolatBeacon(ggid, tgid, attribute);
            return;
        case MB_BEACON_STATE_NEXT_GAME:
            break;
        }
    }
}


// Prepare the beacon send status so that it is sendable.
static BOOL MBi_ReadyBeaconSendStatus(void)
{
    MBGameInfo *nextGameInfop;

    // Return an error if the game information has not been registered.
    if (!mbss.gameInfoListTop)
    {
        /* The MB flag and ENTRY flag are dropped here. */
        (void)WM_SetGameInfo(NULL, (u16 *)&bsendBuff, WM_SIZE_USER_GAMEINFO, MBi_GetGgid(),
                             MBi_GetTgid(), WM_ATTR_FLAG_CS);
        return FALSE;
    }

    // Select the GameInfo to send next.
    if (!mbss.nowGameInfop || !mbss.nowGameInfop->nextp)
    {
        // If there is no game information still being sent, or if at the end of the list, prepare to send the top of the list.
        nextGameInfop = mbss.gameInfoListTop;
    }
    else
    {
        // Otherwise, prepare to send the next game in the list.
        nextGameInfop = mbss.nowGameInfop->nextp;
    }

    mbss.nowGameInfop = nextGameInfop;

    MBi_ClearSendStatus();
    mbss.seqNoVolat = mbss.nowGameInfop->seqNoVolat;

    mbss.state = MB_BEACON_STATE_FIXED_START;

    return TRUE;
}


// Initialize transmission of the fixed data portion of the game information.
static void MBi_InitSendFixedBeacon(void)
{
    if (mbss.state != MB_BEACON_STATE_FIXED_START)
    {
        return;
    }

    if (mbss.nowGameInfop->dataAttr == MB_BEACON_DATA_ATTR_FIXED_NORMAL)
    {
        mbss.fragmentMaxNum = FIXED_FRAGMENT_MAX(FIXED_NORMAL_SIZE);
        mbss.srcp = (u8 *)&mbss.nowGameInfop->fixed;
    }
    else
    {
        mbss.fragmentMaxNum = FIXED_FRAGMENT_MAX(FIXED_NO_ICON_SIZE);
        mbss.srcp = (u8 *)&mbss.nowGameInfop->fixed.parent;
    }
    mbss.state = MB_BEACON_STATE_FIXED;
}


// Split up the fixed data portion of the game information and send a beacon.
static void MBi_SendFixedBeacon(u32 ggid, u16 tgid, u8 attribute)
{
    u32     lastAddr = (u32)mbss.nowGameInfop + sizeof(MBGameInfoFixed);

    if ((u32)mbss.srcp + MB_BEACON_FIXED_DATA_SIZE <= lastAddr)
    {
        bsendBuff.data.fixed.size = MB_BEACON_FIXED_DATA_SIZE;
    }
    else
    {
        bsendBuff.data.fixed.size = (u8)(lastAddr - (u32)mbss.srcp);
        MI_CpuClear16((void *)((u8 *)bsendBuff.data.fixed.data + bsendBuff.data.fixed.size),
                      (u32)(MB_BEACON_FIXED_DATA_SIZE - bsendBuff.data.fixed.size));
    }

    // Set to the beacon send buffer.
    MB_DEBUG_OUTPUT("send fragment= %2d  adr = 0x%x\n", mbss.fragmentNo, mbss.srcp);
    MI_CpuCopy16(mbss.srcp, bsendBuff.data.fixed.data, bsendBuff.data.fixed.size);
    bsendBuff.data.fixed.fragmentNo = mbss.fragmentNo;
    bsendBuff.data.fixed.fragmentMaxNum = mbss.fragmentMaxNum;
    bsendBuff.dataAttr = mbss.nowGameInfop->dataAttr;
    bsendBuff.seqNoFixed = mbss.nowGameInfop->seqNoFixed;
    bsendBuff.seqNoVolat = mbss.seqNoVolat;
    bsendBuff.ggid = mbss.nowGameInfop->ggid;
    bsendBuff.fileNo = mbss.nowGameInfop->fileNo;
    bsendBuff.beaconNo = mbss.beaconNo++;
    bsendBuff.data.fixed.sum = 0;
    bsendBuff.data.fixed.sum = MBi_calc_cksum((u16 *)&bsendBuff.data, MB_BEACON_DATA_SIZE);

    // Update the send status.
    mbss.fragmentNo++;
    if (mbss.fragmentNo < mbss.fragmentMaxNum)
    {
        mbss.srcp += MB_BEACON_FIXED_DATA_SIZE;
    }
    else
    {
        mbss.state = MB_BEACON_STATE_VOLAT_START;
    }

    /* Register the GGID of the parent program itself. */
    (void)WM_SetGameInfo(NULL, (u16 *)&bsendBuff, WM_SIZE_USER_GAMEINFO, ggid, tgid,
                         (u8)(attribute | WM_ATTR_FLAG_MB | WM_ATTR_FLAG_ENTRY));
    // The multiboot flag is activated here.
}


// Initialize transmission of the volatile data portion of the game information.
static void MBi_InitSendVolatBeacon(void)
{
    mbss.nowGameInfop->broadcastedPlayerFlag = 0x0001;
    mbss.seqNoVolat = mbss.nowGameInfop->seqNoVolat;
    mbss.state = MB_BEACON_STATE_VOLAT;
}


// Split up the volatile data portion of the game information and send a beacon.
static void MBi_SendVolatBeacon(u32 ggid, u16 tgid, u8 attribute)
{
    int     i;
    int     setPlayerNum;
    u16     remainPlayerFlag;

    if (mbss.seqNoVolat != mbss.nowGameInfop->seqNoVolat)
    {                                  // Resend if the data is updated during a send.
        MBi_InitSendVolatBeacon();
    }

    // Set the beacon information.
    bsendBuff.dataAttr = MB_BEACON_DATA_ATTR_VOLAT;
    bsendBuff.seqNoFixed = mbss.nowGameInfop->seqNoFixed;
    bsendBuff.seqNoVolat = mbss.seqNoVolat;
    bsendBuff.ggid = mbss.nowGameInfop->ggid;
    bsendBuff.fileNo = mbss.nowGameInfop->fileNo;
    bsendBuff.beaconNo = mbss.beaconNo++;

    // Set the state of the current player information.
    bsendBuff.data.volat.nowPlayerNum = mbss.nowGameInfop->volat.nowPlayerNum;
    bsendBuff.data.volat.nowPlayerFlag = mbss.nowGameInfop->volat.nowPlayerFlag;
    bsendBuff.data.volat.changePlayerFlag = mbss.nowGameInfop->volat.changePlayerFlag;

    // Set the application configuration data.
    if (sSendVolatCallbackTiming == MB_SEND_VOLAT_CALLBACK_TIMING_BEFORE
        && sSendVolatCallback != NULL)
    {
        sSendVolatCallback(mbss.nowGameInfop->ggid);
    }

    for (i = 0; i < MB_USER_VOLAT_DATA_SIZE; i++)
    {
        bsendBuff.data.volat.userVolatData[i] = mbss.nowGameInfop->volat.userVolatData[i];
    }

    MB_DEBUG_OUTPUT("send PlayerFlag = %x\n", mbss.nowGameInfop->volat.nowPlayerFlag);

    // Set the player information to send this time.
    MI_CpuClear16(&bsendBuff.data.volat.member[0], sizeof(MBUserInfo) * MB_SEND_MEMBER_MAX_NUM);
    setPlayerNum = 0;
    remainPlayerFlag =
        (u16)(mbss.nowGameInfop->broadcastedPlayerFlag ^ mbss.nowGameInfop->volat.nowPlayerFlag);
    for (i = 0; i < MB_MEMBER_MAX_NUM; i++)
    {
        if ((remainPlayerFlag & (0x0002 << i)) == 0)
        {
            continue;
        }

        MB_DEBUG_OUTPUT("  member %d set.\n", i);

        MI_CpuCopy16(&mbss.nowGameInfop->volat.member[i],
                     &bsendBuff.data.volat.member[setPlayerNum], sizeof(MBUserInfo));
        mbss.nowGameInfop->broadcastedPlayerFlag |= 0x0002 << i;
        if (++setPlayerNum == MB_SEND_MEMBER_MAX_NUM)
        {
            break;
        }
    }
    if (setPlayerNum < MB_SEND_MEMBER_MAX_NUM)
    {                                  // Terminate when the maximum number of sends was not reached.
        bsendBuff.data.volat.member[setPlayerNum].playerNo = 0;
    }

    // Set the checksum.
    bsendBuff.data.volat.sum = 0;
    bsendBuff.data.volat.sum = MBi_calc_cksum((u16 *)&bsendBuff.data, MB_BEACON_DATA_SIZE);


    // check for send end
    if (mbss.nowGameInfop->broadcastedPlayerFlag == mbss.nowGameInfop->volat.nowPlayerFlag)
    {
        mbss.state = MB_BEACON_STATE_READY;     // When sending of all information is complete, moves to a ready state in order to send the next game information.
    }

    /* Register the GGID of the parent program itself. */

    (void)WM_SetGameInfo(NULL, (u16 *)&bsendBuff, WM_SIZE_USER_GAMEINFO, ggid, tgid,
                         (u8)(attribute | WM_ATTR_FLAG_MB | WM_ATTR_FLAG_ENTRY));
    // The multiboot flag is activated here.

    if (sSendVolatCallbackTiming == MB_SEND_VOLAT_CALLBACK_TIMING_AFTER
        && sSendVolatCallback != NULL)
    {
        sSendVolatCallback(mbss.nowGameInfop->ggid);
    }

}


//=========================================================
// Beacon reception of the game information by the child device.
//=========================================================

// Initializes the game information receive status.
void MB_InitRecvGameInfoStatus(void)
{
    MI_CpuClearFast(mbrsp, sizeof(MbBeaconRecvStatus));

    mbrsp->scanCountUnit = MB_SCAN_COUNT_UNIT_NORMAL;
}


// Receives a beacon.
BOOL MB_RecvGameInfoBeacon(MBBeaconMsgCallback Callbackp, u16 linkLevel, WMBssDesc *bssDescp)
{
    int     index;

    // Determine whether or not the beacon obtained this time is a multiboot parent device.
    if (!MBi_CheckMBParent(bssDescp))
    {
        return FALSE;
    }

    /* Copy the obtained bssDesc to a temporary buffer */
    MI_CpuCopy16(bssDescp, &bssDescbuf, sizeof(WMBssDesc));

    /* Switch bssDescp to a local buffer. */
    bssDescp = &bssDescbuf;

    brecvBuffp = (MbBeacon *) bssDescp->gameInfo.userGameInfo;

    // Confirm checksum.
    if (MBi_calc_cksum((u16 *)&brecvBuffp->data, MB_BEACON_DATA_SIZE))
    {
        MB_DEBUG_OUTPUT("Beacon checksum error!\n");
        return FALSE;
    }

    // Determines the location for storing the parent's game information (if the same parent data has already been received, continue to store at that location)
    index = MBi_GetStoreElement(bssDescp, Callbackp);
    if (index < 0)
    {
        return FALSE;                  // Returns an error because there is no storage location
    }
    MB_DEBUG_OUTPUT("GameInfo Index:%6d\n", index);

    // Beacon analysis
    MBi_AnalyzeBeacon(bssDescp, index, linkLevel);

    // Determines whether all the parent's game data fragments are gathered together and makes a notification with a callback
    MBi_CheckCompleteGameInfoFragments(index, Callbackp);

    return TRUE;
}


// Determines whether the beacon that was obtained this time is a multiboot parent device.
BOOL MBi_CheckMBParent(WMBssDesc *bssDescp)
{
    // Determines whether it is a multiboot parent device.
    if ((bssDescp->gameInfo.magicNumber != WM_GAMEINFO_MAGIC_NUMBER)
        || !(bssDescp->gameInfo.attribute & WM_ATTR_FLAG_MB))
    {
        MB_DEBUG_OUTPUT("not MB parent : %x%x\n",
                        *(u16 *)(&bssDescp->bssid[4]), *(u32 *)bssDescp->bssid);
        return FALSE;
    }
    else
    {

        MB_DEBUG_OUTPUT("MB parent     : %x%x",
                        *(u16 *)(&bssDescp->bssid[4]), *(u32 *)bssDescp->bssid);
        return TRUE;
    }
}


// Sets an SSID to bssDesc.
static void MBi_SetSSIDToBssDesc(WMBssDesc *bssDescp, u32 ggid)
{
    /* 
       SSID settings 

       The SSID is generated from the download application specific GGID and the multiboot parent's TGID.
       
       The child reconnects with the parent application using this SSID.
     */
    bssDescp->ssidLength = 32;
    ((u16 *)bssDescp->ssid)[0] = (u16)(ggid & 0x0000ffff);
    ((u16 *)bssDescp->ssid)[1] = (u16)((ggid & 0xffff0000) >> 16);
    ((u16 *)bssDescp->ssid)[2] = bssDescp->gameInfo.tgid;
}


// Based on bssDesc, gets where in the receive list elements to perform a store.
static int MBi_GetStoreElement(WMBssDesc *bssDescp, MBBeaconMsgCallback Callbackp)
{
    int     i;

    // Determines whether the same game information has already been received from this parent (if "GGID", "BSSID" and "fileNo" all match, the game information is determined to be the same)
    for (i = 0; i < MB_GAME_INFO_RECV_LIST_NUM; i++)
    {
        MBGameInfoRecvList *info = &mbrsp->list[i];

        if ((mbrsp->usingGameInfoFlag & (0x01 << i)) == 0)
        {
            continue;
        }
        // Does GGID match?
        if (info->gameInfo.ggid != brecvBuffp->ggid)
        {
            continue;
        }
        // Does the MAC address match?
        if (!WM_IsBssidEqual(info->bssDesc.bssid, bssDescp->bssid))
        {
            continue;
        }
        // Does the file number match?
        if (mbrsp->list[i].gameInfo.fileNo != brecvBuffp->fileNo)
        {
            continue;
        }

        // =========================================
        // Determined that a reception location for this parent device information has already been allocated.
        // =========================================
        if (!(mbrsp->validGameInfoFlag & (0x01 << i)))
        {
            MBi_LockScanTarget(i);     // If information for the appropriate Parent Device is not ready, scan lock.
        }
        return i;
    }

    // It has not yet been received, so search for the NULL position in the list and make that the storage location.
    for (i = 0; i < MB_GAME_INFO_RECV_LIST_NUM; i++)
    {
        if (mbrsp->usingGameInfoFlag & (0x01 << i))
        {
            continue;
        }

        MI_CpuCopy16(bssDescp, &mbrsp->list[i].bssDesc, sizeof(WMBssDesc));
        // Copies BssDesc
        mbrsp->list[i].gameInfo.seqNoFixed = brecvBuffp->seqNoFixed;
        mbrsp->usingGameInfoFlag |= (u16)(0x01 << i);

        MB_DEBUG_OUTPUT("\n");
        // Only this parent device should be locked to the scan target.
        MBi_LockScanTarget(i);
        return i;
    }

    // If all the storage locations are full, an error is returned with a callback notification
    if (Callbackp != NULL)
    {
        Callbackp(MB_BC_MSG_GINFO_LIST_FULL, NULL, 0);
    }
    return -1;
}


// Determines whether all the parent's game data fragments are gathered together and makes a notification with a callback
static void MBi_CheckCompleteGameInfoFragments(int index, MBBeaconMsgCallback Callbackp)
{
    MBGameInfoRecvList *info = &mbrsp->list[index];

    /* In any case, notify the application that a parent was discovered. */
    if (Callbackp != NULL)
    {
        Callbackp(MB_BC_MSG_GINFO_BEACON, info, index);
    }
    if ((info->getFragmentFlag == info->allFragmentFlag) && (info->getFragmentFlag)     // If the new parent game information is complete
        && (info->gameInfo.volat.nowPlayerFlag)
        && (info->getPlayerFlag == info->gameInfo.volat.nowPlayerFlag))
    {
        if (mbrsp->validGameInfoFlag & (0x01 << index))
        {
            return;
        }
        mbrsp->validGameInfoFlag |= 0x01 << index;
        mbrsp->usefulGameInfoFlag |= 0x01 << index;
        MBi_UnlockScanTarget();        // Releases the lock on the scan target
        MB_DEBUG_OUTPUT("validated ParentInfo = %d\n", index);
        if (Callbackp != NULL)
        {
            Callbackp(MB_BC_MSG_GINFO_VALIDATED, info, index);
        }
    }
    else
    {                                  // If the parent's game information that was already obtained is updated and becomes incomplete
        if ((mbrsp->validGameInfoFlag & (0x01 << index)) == 0)
        {
            return;
        }
        mbrsp->validGameInfoFlag ^= (0x01 << index);
        MB_DEBUG_OUTPUT("Invaldated ParentInfo = %d\n", index);
        if (Callbackp != NULL)
        {
            Callbackp(MB_BC_MSG_GINFO_INVALIDATED, info, index);
        }
    }
}


// The parent's game information lifetime count (while doing so, also counts the scan lock time)
void MB_CountGameInfoLifetime(MBBeaconMsgCallback Callbackp, BOOL found_parent)
{
    int     i;
    BOOL    unlock = FALSE;

    // Determines the lifetime of the game information receiving list and confirms the presence of a new lock target.
    for (i = 0; i < MB_GAME_INFO_RECV_LIST_NUM; i++)
    {
        MBGameInfoRecvList *info = &mbrsp->list[i];
        u16     mask = (u16)(0x0001 << i);
        if ((mbrsp->usingGameInfoFlag & mask) == 0)
        {
            continue;
        }
        // Lifetime determination
        info->lifetimeCount -= mbrsp->scanCountUnit;
        if (info->lifetimeCount >= 0)
        {
            continue;
        }
        info->lifetimeCount = 0;
        if (mbrsp->validGameInfoFlag & mask)
        {
            if (Callbackp != NULL)
            {
                Callbackp(MB_BC_MSG_GINFO_LOST, info, i);
            }
        }
        if (mbrsp->nowScanTargetFlag & mask)
        {
            unlock = TRUE;
        }
        mbrsp->usingGameInfoFlag &= ~mask;
        MB_DeleteRecvGameInfo(i);      // Deletes game information after callback notification.
        MB_DEBUG_OUTPUT("gameInfo %2d : lifetime end.\n", i);
    }

    // Scan lock time count
    if (mbrsp->nowScanTargetFlag && mbrsp->nowLockTimeCount > 0)
    {
        mbrsp->nowLockTimeCount -= mbrsp->scanCountUnit;        // Releases the lock and searches for the next lock target if the scan lock times out.
        if (mbrsp->nowLockTimeCount < 0)
        {
            MB_DEBUG_OUTPUT("scan lock time up!\n");
            unlock = TRUE;
        }
        else if (!found_parent)
        {
            if (++mbrsp->notFoundLockTargetCount > 4)
            {
                MB_DEBUG_OUTPUT("scan lock target not found!\n");
                unlock = TRUE;
            }
        }
        else
        {
            mbrsp->notFoundLockTargetCount = 0;
        }
    }

    // Scan unlock processing.
    if (unlock)
    {
        mbrsp->nowLockTimeCount = 0;
        MBi_UnlockScanTarget();
    }
}


// Beacon analysis
static void MBi_AnalyzeBeacon(WMBssDesc *bssDescp, int index, u16 linkLevel)
{
    MBi_CheckTGID(bssDescp, index);    // Checks the TGID
    MBi_CheckSeqNoFixed(index);        // Checks seqNoFixed
    MBi_CheckSeqNoVolat(index);        // Checks seqNoVolat

    // Obtains data for the shared portion of the received beacon
    {
        MBGameInfoRecvList *info = &mbrsp->list[index];

        // Excludes parents that are in an abnormal state, repeatedly sending out the same beacon.
        if (info->beaconNo == brecvBuffp->beaconNo)
        {
            if (++info->sameBeaconRecvCount > MB_SAME_BEACON_RECV_MAX_COUNT)
            {
                info->lifetimeCount = 0;        // Sets the lifetime of the parent information to zero so it gets deleted.
                MB_OUTPUT("The parent broadcast same beacon.: %d\n", index);
                MBi_InvalidateGameInfoBssID(&info->bssDesc.bssid[0]);
                return;                // If this parent has distributed other game information, that is deleted as well.
            }
        }
        else
        {
            info->sameBeaconRecvCount = 0;
        }
        // Data reception for normal parents.
        info->beaconNo = brecvBuffp->beaconNo;
        info->lifetimeCount = MB_LIFETIME_MAX_COUNT;    // Extends the lifetime of the parent's information.
        info->gameInfo.ggid = brecvBuffp->ggid; // Gets ggid.
        info->gameInfo.fileNo = brecvBuffp->fileNo;     // Gets fileNo.
        info->linkLevel = linkLevel;   // Gets the signal strength.
        // Sets an SSID to bssDesc.
        MBi_SetSSIDToBssDesc(&info->bssDesc, info->gameInfo.ggid);
    }

    // Gets data for each data type in the received beacon.
    if (brecvBuffp->dataAttr == MB_BEACON_DATA_ATTR_VOLAT)
    {
        MBi_RecvVolatBeacon(index);
    }
    else
    {
        MBi_RecvFixedBeacon(index);
    }
}


// Checks tgid
static void MBi_CheckTGID(WMBssDesc *bssDescp, int index)
{
    if (mbrsp->list[index].bssDesc.gameInfo.tgid == bssDescp->gameInfo.tgid)
    {
        return;
    }

    // If tgid is updated, that parent device is assumed to be restarting; all data is cleared and re-obtained.
    MB_DEBUG_OUTPUT("\ntgid updated! : %x%x", *(u16 *)(&bssDescp->bssid[4]),
                    *(u32 *)bssDescp->bssid);
    MB_DeleteRecvGameInfoWithoutBssdesc(index);
    MI_CpuCopy16(bssDescp, &mbrsp->list[index].bssDesc, sizeof(WMBssDesc));
    // A new bssDesc is copied in the event of a tgid update.
    MBi_LockScanTarget(index);         // Scan-locks the appropriate parent.
}


// Checks the SeqNo of the fixed data
static void MBi_CheckSeqNoFixed(int index)
{
    // Handles the situation where the sequence number has been updated
    if (mbrsp->list[index].gameInfo.seqNoFixed == brecvBuffp->seqNoFixed)
    {
        return;
    }
    // Clears the data received thus far if the sequence number has been updated.
    MB_DEBUG_OUTPUT("\n seqNoFixed updated!");
    MB_DeleteRecvGameInfoWithoutBssdesc(index);
    MBi_LockScanTarget(index);         // Scan-locks the appropriate parent.
    mbrsp->list[index].gameInfo.seqNoFixed = brecvBuffp->seqNoFixed;
}


// Checks the SeqNo of the volatile data.
static void MBi_CheckSeqNoVolat(int index)
{
    MBGameInfoRecvList *grecvp = &mbrsp->list[index];

    // Handles the situation where the sequence number has been updated
    if (mbrsp->list[index].gameInfo.seqNoVolat != brecvBuffp->seqNoVolat)
    {
        MB_DEBUG_OUTPUT("\n seqNoVolat updated!");
        MBi_LockScanTarget(index);     // Checks whether a scan lock is possible, then does so.
    }
}


// Invalidates all the parent game information of the target BSSID.
static void MBi_InvalidateGameInfoBssID(u8 *bssidp)
{
    int     i;
    for (i = 0; i < MB_GAME_INFO_RECV_LIST_NUM; i++)
    {
        if ((mbrsp->usingGameInfoFlag & (0x01 << i)) == 0)
        {
            continue;
        }

        if (!WM_IsBssidEqual(bssidp, mbrsp->list[i].bssDesc.bssid))
        {
            continue;
        }

        // Determined to be data that is targeted for deletion.
        mbrsp->list[i].lifetimeCount = 0;       // Sets the lifetime of the parent information to zero so it gets deleted.
        MB_OUTPUT("The parent broadcast same beacon.: %d\n", i);
    }
}


// Splits up the fixed data portion of the game information and receives the beacon.
static void MBi_RecvFixedBeacon(int index)
{
    MBGameInfoRecvList *grecvp = &mbrsp->list[index];
    u32     lastAddr = (u32)&grecvp->gameInfo + sizeof(MBGameInfoFixed);
    u8     *dstp;

    // Do not receive if the sequence number has not been updated, and the beacon has already been obtained.
    if (grecvp->gameInfo.seqNoFixed == brecvBuffp->seqNoFixed)
    {
        if (grecvp->getFragmentFlag & (0x01 << brecvBuffp->data.fixed.fragmentNo))
        {
            return;
        }
    }

    // Checks whether the received beacon exceeds the receive buffer.
    if (brecvBuffp->dataAttr == MB_BEACON_DATA_ATTR_FIXED_NORMAL)
    {
        dstp = (u8 *)&grecvp->gameInfo.fixed;
    }
    else
    {
        dstp = (u8 *)&grecvp->gameInfo.fixed.parent;
    }
    dstp += MB_BEACON_FIXED_DATA_SIZE * brecvBuffp->data.fixed.fragmentNo;
    // Calculates the receiving address for the game information buffer.

    if ((u32)dstp + brecvBuffp->data.fixed.size > lastAddr)
    {
        MB_DEBUG_OUTPUT("recv beacon gInfoFixed Buffer over!\n");
        // Ignores beacon data that overflows the buffer.
        return;
    }

    // Sets the received beacon to the target game information buffer
    MB_DEBUG_OUTPUT("recv fragment= %2d  adr = 0x%x", brecvBuffp->data.fixed.fragmentNo, dstp);
    MI_CpuCopy16(brecvBuffp->data.fixed.data, dstp, brecvBuffp->data.fixed.size);
    grecvp->gameInfo.dataAttr = brecvBuffp->dataAttr;
    grecvp->getFragmentFlag |= 0x01 << brecvBuffp->data.fixed.fragmentNo;
    grecvp->allFragmentFlag = (u32)((0x01 << brecvBuffp->data.fixed.fragmentMaxNum) - 1);
    MB_DEBUG_OUTPUT("\t now fragment = 0x%x \t all fragment = 0x%x\n",
                    grecvp->getFragmentFlag, grecvp->allFragmentFlag);
}


// Splits up the volatile data portion of the game information and receives the beacon.
static void MBi_RecvVolatBeacon(int index)
{
    int     i;
    MBGameInfoRecvList *grecvp = &mbrsp->list[index];

    /* Always receive user-defined data. */
    for (i = 0; i < MB_USER_VOLAT_DATA_SIZE; i++)
    {
        grecvp->gameInfo.volat.userVolatData[i] = brecvBuffp->data.volat.userVolatData[i];
    }
    MI_CpuCopy16(brecvBuffp, &grecvp->bssDesc.gameInfo.userGameInfo, WM_SIZE_USER_GAMEINFO);

    // Processing when a member information update is detected.
    if (grecvp->gameInfo.seqNoVolat != brecvBuffp->seqNoVolat)
    {
        if ((u8)(grecvp->gameInfo.seqNoVolat + 1) == brecvBuffp->seqNoVolat)
        {                              // Transfer of non-updated member information if seqNoVolat is off by one
            for (i = 0; i < MB_MEMBER_MAX_NUM; i++)
            {
                if (brecvBuffp->data.volat.changePlayerFlag & (0x02 << i))
                {
                    MI_CpuClear16(&grecvp->gameInfo.volat.member[i], sizeof(MBUserInfo));
                }
            }
            grecvp->getPlayerFlag &= ~brecvBuffp->data.volat.changePlayerFlag;
            mbrsp->validGameInfoFlag &= ~(0x0001 << index);
        }
        else
        {                              // Clears all the member information received thus far if seqNoVolat is further off.
            MI_CpuClear16(&grecvp->gameInfo.volat.member[0],
                          sizeof(MBUserInfo) * MB_MEMBER_MAX_NUM);
            grecvp->getPlayerFlag = 0;
            mbrsp->validGameInfoFlag &= ~(0x0001 << index);
        }
        grecvp->gameInfo.seqNoVolat = brecvBuffp->seqNoVolat;
    }
    else if (grecvp->getPlayerFlag == brecvBuffp->data.volat.nowPlayerFlag)
    {
        return;                        // Do not receive if the sequence number has not been updated, and the beacon has already been obtained.
    }

    // Reads the player information
    grecvp->gameInfo.volat.nowPlayerNum = brecvBuffp->data.volat.nowPlayerNum;
    grecvp->gameInfo.volat.nowPlayerFlag = brecvBuffp->data.volat.nowPlayerFlag;
    grecvp->gameInfo.volat.changePlayerFlag = brecvBuffp->data.volat.changePlayerFlag;
    grecvp->getPlayerFlag |= 0x0001;

    // Reads the user information for each member
    for (i = 0; i < MB_SEND_MEMBER_MAX_NUM; i++)
    {
        int     playerNo = (int)brecvBuffp->data.volat.member[i].playerNo;
        if (playerNo == 0)
        {
            continue;
        }
        MB_DEBUG_OUTPUT("member %d recv.\n", playerNo);
        MI_CpuCopy16(&brecvBuffp->data.volat.member[i],
                     &grecvp->gameInfo.volat.member[playerNo - 1], sizeof(MBUserInfo));
        grecvp->getPlayerFlag |= 0x01 << playerNo;
    }
}


// Locks the scan target to a single parent
static void MBi_LockScanTarget(int index)
{
    /* Ignore if there is already a locked target */
    if (mbrsp->nowScanTargetFlag)
    {
        return;
    }

    if (sLockFunc != NULL)
    {
        sLockFunc(mbrsp->list[index].bssDesc.bssid);
    }
    mbrsp->scanCountUnit = MB_SCAN_COUNT_UNIT_LOCKING;

    mbrsp->nowScanTargetFlag = (u16)(0x01 << index);    // Shows new scan lock targets in bits.
    mbrsp->nowLockTimeCount = MB_LOCKTIME_MAX_COUNT;    // Initializes the scan lock time
    MB_DEBUG_OUTPUT("scan target locked. : %x %x %x %x %x %x\n",
                    mbrsp->list[index].bssDesc.bssid[0],
                    mbrsp->list[index].bssDesc.bssid[1],
                    mbrsp->list[index].bssDesc.bssid[2],
                    mbrsp->list[index].bssDesc.bssid[3],
                    mbrsp->list[index].bssDesc.bssid[4], mbrsp->list[index].bssDesc.bssid[5]);
}


// Releases the scan target lock.
static void MBi_UnlockScanTarget(void)
{
    if (mbrsp->nowScanTargetFlag == 0)
    {
        return;
    }

    if (sUnlockFunc != NULL)
    {
        sUnlockFunc();
    }
    mbrsp->scanCountUnit = MB_SCAN_COUNT_UNIT_NORMAL;
    mbrsp->nowScanTargetFlag = 0;
    mbrsp->notFoundLockTargetCount = 0;

    MB_DEBUG_OUTPUT(" unlock target\n");
}


// Completely deletes the received game information (also deletes the valid game information flag)
void MB_DeleteRecvGameInfo(int index)
{
    mbrsp->usefulGameInfoFlag &= ~(0x0001 << index);
    mbrsp->validGameInfoFlag &= ~(0x0001 << index);
    MI_CpuClear16(&mbrsp->list[index], sizeof(MBGameInfoRecvList));
}


// Deletes the received game information, except for bssDesc.
void MB_DeleteRecvGameInfoWithoutBssdesc(int index)
{
    mbrsp->usefulGameInfoFlag &= ~(0x0001 << index);
    mbrsp->validGameInfoFlag &= ~(0x0001 << index);
    mbrsp->list[index].getFragmentFlag = 0;
    mbrsp->list[index].allFragmentFlag = 0;
    mbrsp->list[index].getPlayerFlag = 0;
    mbrsp->list[index].linkLevel = 0;
    MI_CpuClear16(&(mbrsp->list[index].gameInfo), sizeof(MBGameInfo));
}

// Gets a pointer to a received parent information structure.
MBGameInfoRecvList *MB_GetGameInfoRecvList(int index)
{
    // Returns NULL if there is no valid data
    if ((mbrsp->usefulGameInfoFlag & (0x01 << index)) == 0)
    {
        return NULL;
    }

    return &mbrsp->list[index];
}


//=========================================================
// User volatile data settings
//=========================================================

/*---------------------------------------------------------------------------*
  Name:         MB_SetSendVolatileCallback

  Description:  Sets the multiboot beacon-transmission callback.
  
  Arguments:    callback:    The callback function for a completed transmission.
                            The callback is invoked whenever data is sent.
                timing:     The timing for issuing the callback.
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void MB_SetSendVolatCallback(MBSendVolatCallbackFunc callback, u32 timing)
{
    OSIntrMode enabled = OS_DisableInterrupts();

    sSendVolatCallback = callback;
    sSendVolatCallbackTiming = timing;

    (void)OS_RestoreInterrupts(enabled);
}



/*---------------------------------------------------------------------------*
  Name:         MB_SetUserVolatData

  Description:  Sets user data in an available region of a multiboot beacon.
  
  Arguments:    ggid:     Specifies the ggid of the program that was set by MB_RegisterFile, and attaches user data to this file's beacon.
                            
                userData:    Pointer to the user data being set.
                size:        Size of the user data being set. (8 bytes maximum)
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void MB_SetUserVolatData(u32 ggid, const u8 *userData, u32 size)
{
    MBGameInfo *gameInfo;

    SDK_ASSERT(size <= MB_USER_VOLAT_DATA_SIZE);
    SDK_NULL_ASSERT(userData);

    gameInfo = mbss.gameInfoListTop;
    if (gameInfo == NULL)
    {
        return;
    }

    while (gameInfo->ggid != ggid)
    {
        if (gameInfo == NULL)
        {
            return;
        }
        gameInfo = gameInfo->nextp;
    }

    {
        u32     i;

        OSIntrMode enabled = OS_DisableInterrupts();

        for (i = 0; i < size; i++)
        {
            gameInfo->volat.userVolatData[i] = userData[i];
        }

        (void)OS_RestoreInterrupts(enabled);
    }
}

/*---------------------------------------------------------------------------*
  Name:         MB_GetUserVolatData

  Description:  Gets the user data in an available region of a beacon.
  
  Arguments:    gameInfo:    Pointer to the gameInfo parameter that will be obtained at Scan time.
  
  Returns:      Pointer to the user data.
 *---------------------------------------------------------------------------*/
void   *MB_GetUserVolatData(const WMGameInfo *gameInfo)
{
    MbBeacon *beacon;

    SDK_NULL_ASSERT(gameInfo);

    if (!(gameInfo->attribute & WM_ATTR_FLAG_MB))
    {
        return NULL;
    }

    beacon = (MbBeacon *) (gameInfo->userGameInfo);

    if (beacon->dataAttr != MB_BEACON_DATA_ATTR_VOLAT)
    {
        return NULL;
    }

    return beacon->data.volat.userVolatData;
}

