/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - include
  File:     mb_fake_child.h

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_MB_MB_FAKE_CHILD_H_
#define NITRO_MB_MB_FAKE_CHILD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/types.h>
#include <nitro/mb/mb.h>

// 
// This is the size of the work buffer needed as a MB_FakeInit() argument.
#define     MB_FAKE_WORK_SIZE       (0x5f20)

enum
{
    MB_FAKESCAN_PARENT_FOUND,
    MB_FAKESCAN_PARENT_LOST,
    MB_FAKESCAN_API_ERROR,
    MB_FAKESCAN_END_SCAN,
    MB_FAKESCAN_PARENT_BEACON
};

/*
 * Notification callback for the scan parent
 */
typedef struct
{
    u16     index;                     // Index for managing obtained parent devices
    u16     padding;
    MBGameInfo *gameInfo;              // Parent game information
    WMBssDesc *bssDesc;                // Parent connection information
}
MBFakeScanCallback;

/*
 * Error notification callback
 */
typedef struct
{
    u16     apiid;                     // API code
    u16     errcode;                   // Error Code
}
MBFakeScanErrorCallback;

typedef void (*MBFakeScanCallbackFunc) (u16 type, void *arg);
typedef BOOL (*MBFakeCompareGGIDCallbackFunc) (WMStartScanCallback *arg, u32 defaultGGID);

void    MB_FakeInit(void *buf, const MBUserInfo *user);
void    MB_FakeEnd(void);
u32     MB_FakeGetWorkSize(void);
void    MB_FakeSetCStateCallback(MBCommCStateCallbackFunc callback);
void    MB_FakeStartScanParent(MBFakeScanCallbackFunc callback, u32 ggid);
void    MB_FakeEndScan(void);
BOOL    MB_FakeEntryToParent(u16 index);
BOOL    MB_FakeGetParentGameInfo(u16 index, MBGameInfo *pGameInfo);
BOOL    MB_FakeGetParentBssDesc(u16 index, WMBssDesc *pBssDesc);
BOOL    MB_FakeReadParentBssDesc(u16 index, WMBssDesc *pBssDesc, u16 parent_max_size,
                                 u16 child_max_size, BOOL ks_flag, BOOL cs_flag);
void    MB_FakeSetVerboseScanCallback(WMCallbackFunc callback);
void    MB_FakeSetCompareGGIDCallback(MBFakeCompareGGIDCallbackFunc callback);


#ifdef __cplusplus
}
#endif


#endif // NITRO_MB_MB_FAKE_CHILD_H_
