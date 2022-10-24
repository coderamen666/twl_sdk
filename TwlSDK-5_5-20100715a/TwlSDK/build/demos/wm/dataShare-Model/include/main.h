/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WM - demos - dataShare-Model
  File:     main.h

  Copyright 2006-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef __MAIN_H__
#define __MAIN_H__

enum
{
    SYSMODE_SELECT_ROLE,               // Role (whether to start as a parent or a child) selection screen
    SYSMODE_OPTION,                    // Option screen
    SYSMODE_SELECT_PARENT,             // Parent selection screen (only for child)
    SYSMODE_LOBBY,                     // Lobby screen (only for parent)
    SYSMODE_LOBBYWAIT,                 // Lobby standby screen (only for child)
    SYSMODE_SELECT_CHANNEL,            // Channel selection screen
    SYSMODE_SCAN_PARENT,               // Parent search screen
    SYSMODE_ERROR,                     // Error report screen
    SYSMODE_PARENT,                    // Parent mode screen
    SYSMODE_CHILD,                     // Child mode screen
    SYSMODE_NUM
};

enum
{
    SHARECMD_NONE = 0,                 // Nothing specific (normal)
    SHARECMD_EXITLOBBY,                // Signal for ending the lobby screen
    SHARECMD_NUM
};

typedef struct ShareData_
{
    unsigned int command:3;            // Instruction (used for switching the game states at once, etc.)
    unsigned int level:2;              // Signal reception strength
    unsigned int data:3;               // For graphing
    unsigned int key:16;               // Key data
    unsigned int count:24;             // Frame count
}
ShareData;

typedef struct MyGameInfo_
{
    u8      nickName[10 * 2];
    u8      nickNameLength;
    u8      entryCount;
    u16     periodicalCount;
}
MyGameInfo;

struct PRScreen_;

extern BOOL isDataReceived(int index);
extern ShareData *getRecvData(int index);
extern ShareData *getSendData(void);
extern void changeSysMode(int s);
extern struct PRScreen_ *getInfoScreen(void);

#endif
