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

#ifndef NITRO_MB_MB_GAMEINFO_H_
#define NITRO_MB_MB_GAMEINFO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/types.h>
#include <nitro/mb/mb.h>

#define MB_USER_VOLAT_DATA_SIZE     8


typedef void (*MBSendVolatCallbackFunc) (u32 ggid);

/*
 * Unchangeable parent game information
 * (This is re-received when seqNo changes. (However, this normally stays fixed.))
 */
typedef struct MBGameInfoFixed
{
    MBIconInfo icon;                   // 544B     Icon data
    MBUserInfo parent;                 //  22B     Parent user information
    u8      maxPlayerNum;              //   1B     Maximum number of players
    u8      pad[1];
    u16     gameName[MB_GAME_NAME_LENGTH];      //  96B     Game title
    u16     gameIntroduction[MB_GAME_INTRO_LENGTH];     // 192B     Description of game content
}
MBGameInfoFixed, MbGameInfoFixed;


/*
 * Volatile parent game information
 *(This is re-received when seqNo changes)
 */
typedef struct MBGameInfoVolatile
{
    u8      nowPlayerNum;              //   1B:     Current number of players
    u8      pad[1];
    u16     nowPlayerFlag;             //   2B:     Indicates, in bits, the player numbers of all current players.
    u16     changePlayerFlag;          //   2B:     Indicates, with a flag, the player information number that was changed in latest change.
    MBUserInfo member[MB_MEMBER_MAX_NUM];       // 330B:     Member information
    u8      userVolatData[MB_USER_VOLAT_DATA_SIZE];     //   8B:     Data the user can set
}
MBGameInfoVolatile, MbGameInfoVolatile;


/* Beacon attributes */
typedef enum MbBeaconDataAttr
{
    MB_BEACON_DATA_ATTR_FIXED_NORMAL = 0,       /* Fixed data for when there is icon data */
    MB_BEACON_DATA_ATTR_FIXED_NO_ICON, /* Fixed data for when there is no icon data */
    MB_BEACON_DATA_ATTR_VOLAT          /* Member information and other volatile data */
}
MBBeaconDataAttr, MbBeaconDataAttr;

/* When dataAttr = FIXED_NORMAL, sends MbGameInfoFixed data from the beginning in chunked MB_BEACON_DATA_SIZE units.
    When dataAttr = FIXED_NO_ICON, sends data from MBUserInfo starting at the beginning in MB_BEACON_DATA_SIZE units (icon data skipped).
    When dataAttr = VOLAT, sends as "Current number of senders x user information"
*/

/*
 * Parent game information beacon
 */
typedef struct MBGameInfo
{
    MBGameInfoFixed fixed;             // Fixed data
    MBGameInfoVolatile volat;          // Volatile data
    u16     broadcastedPlayerFlag;     // Indicates in bits player information that has been broadcast in volatile data.
    u8      dataAttr;                  // Data attributes
    u8      seqNoFixed;                // Fixed region's sequence number
    u8      seqNoVolat;                // Volatile region's sequence number
    u8      fileNo;                    // File number
    u8      pad[2];
    u32     ggid;                      // GGID
    struct MBGameInfo *nextp;          // Pointer to next GameInfo (unidirectional list)
}
MBGameInfo, MbGameInfo;


enum
{
    MB_SEND_VOLAT_CALLBACK_TIMING_BEFORE,
    MB_SEND_VOLAT_CALLBACK_TIMING_AFTER,
    // compatibility for typo. (not recommended)
    MB_SEND_VOLAT_CALLBACK_TIMMING_BEFORE = MB_SEND_VOLAT_CALLBACK_TIMING_BEFORE,
    MB_SEND_VOLAT_CALLBACK_TIMMING_AFTER  = MB_SEND_VOLAT_CALLBACK_TIMING_AFTER
};

void    MB_SetSendVolatCallback(MBSendVolatCallbackFunc callback, u32 timing);
void    MB_SetUserVolatData(u32 ggid, const u8 *userData, u32 size);
void   *MB_GetUserVolatData(const WMGameInfo *gameInfo);



#ifdef __cplusplus
}
#endif


#endif // NITRO_MB_MB_GAMEINFO_H_
