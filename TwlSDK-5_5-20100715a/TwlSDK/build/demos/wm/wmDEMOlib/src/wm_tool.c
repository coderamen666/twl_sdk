/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WM - demos - wmDEMOlib
  File:     wm_tool.c

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif

#include <nitro/wm.h>
#include "wm_lib.h"

#define WM_BEACON_PERIOD_DEFAULT    (200)       // Default value for the Beacon interval (ms)


static WM_lib_param libParam ATTRIBUTE_ALIGN(32);       // WM_lib parameter structure

static u8 libParam_wmbuf[WM_SYSTEM_BUF_SIZE] ATTRIBUTE_ALIGN(32);
static u8 libParam_statusBuf[WM_STATUS_BUF_SIZE] ATTRIBUTE_ALIGN(32);
static u8 libParam_parentParam[WM_PARENT_PARAM_SIZE] ATTRIBUTE_ALIGN(32);

static u8 libParam_dcfBuf[WM_DCF_MAX_SIZE * 2] ATTRIBUTE_ALIGN(32);

static u8 libParam_parentInfoBuf[WM_BSS_DESC_SIZE] ATTRIBUTE_ALIGN(32);
static u8 my_parentInfo[WM_NUM_PARENT_INFORMATIONS][WM_BSS_DESC_SIZE] ATTRIBUTE_ALIGN(32);

static CHILD_INFO child_info[WM_NUM_CHILD_INFORMATIONS] ATTRIBUTE_ALIGN(32);
static childInfo ACinfo;               // Variable structure used with child mode

static WMkeySetBuf libParam_ksbuf ATTRIBUTE_ALIGN(32);  // Keysharing buffer

// For storing parent info (for 4 machines) used during connection
static u16 userGameInfoBuf[32] ATTRIBUTE_ALIGN(32);

static u8 user_name[WM_SIZE_USERNAME] ATTRIBUTE_ALIGN(4);
static u8 game_name[WM_SIZE_GAMENAME] ATTRIBUTE_ALIGN(4);

static int last_found_parent_no = -1;

int wm_lib_get_last_found_parent_no(void)
{
    return last_found_parent_no;
}

//--------------------------------------------------------------
// Store the parent information obtained with scan
void save_parentInfo(WMstartScanCallback *buf, WM_lib_param * param, childInfo * info)
{
    int     i = 0;

    // WM_lib_set_scan_channel_seek(FALSE);
    // wm_lib_set_scan_bssid(buf->macAddress[0],buf->macAddress[1], buf->macAddress[2]);

    while (i < info->found_parent_count)
    {
        if (buf->macAddress[0] == info->MacAdrList[i][0] &&
            buf->macAddress[1] == info->MacAdrList[i][1] &&
            buf->macAddress[2] == info->MacAdrList[i][2] &&
            buf->macAddress[3] == info->MacAdrList[i][3] &&
            buf->macAddress[4] == info->MacAdrList[i][4] &&
            buf->macAddress[5] == info->MacAdrList[i][5])
        {

            ++info->find_counter[i];   // Increment the number of times found

            info->channelList[i] = buf->channel;        // Update channel information
            info->gameInfoLength[i] = buf->gameInfoLength;
            info->gameInfoList[i] = buf->gameInfo;
            DC_InvalidateRange(info->parentInfo[i], WM_BSS_DESC_SIZE);  // Invalidate cache
            MI_DmaCopy16(WM_DMA_NO,    // DMA number
                         param->parentInfoBuf,  // src
                         info->parentInfo[i],   // dest
                         WM_BSS_DESC_SIZE       // Size of the transmission
                );
            last_found_parent_no = i;
            return;
        }
        i++;
    }

    if (i >= WM_NUM_PARENT_INFORMATIONS)
    {
        return;                        /* When the registration limitations have been exceeded */
    }

    // Check to see if it is the designated GGID
    if (param->ggidFilter != 0xffffffff)
    {
        if (buf->gameInfoLength == 0 || buf->gameInfo.ggid != param->ggidFilter)
            return;
    }

    info->found_parent_count = (u16)(i + 1);


    // Newly discovered parent, so store data
    info->MacAdrList[i][0] = buf->macAddress[0];
    info->MacAdrList[i][1] = buf->macAddress[1];
    info->MacAdrList[i][2] = buf->macAddress[2];
    info->MacAdrList[i][3] = buf->macAddress[3];
    info->MacAdrList[i][4] = buf->macAddress[4];
    info->MacAdrList[i][5] = buf->macAddress[5];
    info->channelList[i] = buf->channel;
    info->ssidLength[i] = buf->ssidLength;

    MI_CpuCopy8(buf->ssid, info->ssidList[i], 32);
    //    for(j = 0; j < 16; j++ )
    //        info->ssidList[i][j]       = buf->ssid[j];
    info->gameInfoLength[i] = buf->gameInfoLength;
    info->gameInfoList[i] = buf->gameInfo;

    DC_InvalidateRange(info->parentInfo[i], WM_BSS_DESC_SIZE);  // Invalidate cache

    MI_DmaCopy16(WM_DMA_NO,            // DMA number
                 param->parentInfoBuf, // src
                 info->parentInfo[i],  // dest
                 WM_BSS_DESC_SIZE      // Size of the transmission
        );
    last_found_parent_no = i;
}

//--------------------------------------------------------------
// Store the parent information obtained with scan
void save_parentInfoEx(WMstartScanExCallback *buf, WM_lib_param * param, childInfo * info)
{
    int     i, j, k;

    WMBssDesc *bssDesc;
    u16     bssDescLength;
    // WMOtherElements elems;   // To test the acquisition of otherElement

    // Invalidate cache for parent information storage buffer
    DC_InvalidateRange(param->parentInfoBuf, WM_SIZE_SCAN_EX_BUF);

    // Loop as many times as the number of parents that were found this time
    for (k = 0; k < buf->bssDescCount; ++k)
    {
        // Get parent information start address from callback
        bssDesc = buf->bssDesc[k];

        // Test the getting of otherElement
        //elems = WM_GetOtherElements( bssDesc );

        i = 0;
        while (1)
        {
            // bssDesc length is in 16-bit units. Convert to byte lengths.
            bssDescLength = (u16)(bssDesc->length * 2);

            // Check whether this is a parent that has already been found
            if (i < info->found_parent_count)
            {
                if (bssDesc->bssid[0] == info->MacAdrList[i][0] &&
                    bssDesc->bssid[1] == info->MacAdrList[i][1] &&
                    bssDesc->bssid[2] == info->MacAdrList[i][2] &&
                    bssDesc->bssid[3] == info->MacAdrList[i][3] &&
                    bssDesc->bssid[4] == info->MacAdrList[i][4] &&
                    bssDesc->bssid[5] == info->MacAdrList[i][5])
                {
                    ++info->find_counter[i];    // Increment the number of times found

                    info->channelList[i] = bssDesc->channel;    // Update channel information
                    info->gameInfoLength[i] = bssDesc->gameInfoLength;

                    if (info->gameInfoLength[i] != 0)
                    {
                        MI_CpuCopy16((u16 *)&(bssDesc->gameInfo),
                                     (u16 *)&(info->gameInfoList[i]), info->gameInfoLength[i]);
                    }
                    MI_DmaCopy16(WM_DMA_NO,     // DMA number
                                 bssDesc,       // src
                                 info->parentInfo[i],   // dest
                                 bssDescLength  // Size of the transmission
                        );

                    break;             // Move to next parent information check
                }
            }
            else
            {
                // Do nothing and exit if the parent information list is entirely full
                if (i == WM_NUM_PARENT_INFORMATIONS - 1)
                    break;

                // Check to see if it is the designated GGID
                if (param->ggidFilter != 0xffffffff)
                {
                    if (bssDesc->gameInfoLength == 0 || bssDesc->gameInfo.ggid != param->ggidFilter)
                        break;
                }

                if (i == info->found_parent_count)
                {
                    // This is a newly found parent. Therefore add to parent information list.
                    info->found_parent_count = (u16)(i + 1);

                    info->MacAdrList[i][0] = bssDesc->bssid[0];
                    info->MacAdrList[i][1] = bssDesc->bssid[1];
                    info->MacAdrList[i][2] = bssDesc->bssid[2];
                    info->MacAdrList[i][3] = bssDesc->bssid[3];
                    info->MacAdrList[i][4] = bssDesc->bssid[4];
                    info->MacAdrList[i][5] = bssDesc->bssid[5];

                    info->channelList[i] = bssDesc->channel;
                    info->ssidLength[i] = bssDesc->ssidLength;
                    for (j = 0; j < 16; j++)
                        info->ssidList[i][j] = bssDesc->ssid[j];
                    info->gameInfoLength[i] = bssDesc->gameInfoLength;

                    if (info->gameInfoLength[i] != 0)
                    {
                        MI_CpuCopy16((u16 *)&(bssDesc->gameInfo),
                                     (u16 *)&(info->gameInfoList[i]), info->gameInfoLength[i]);
                    }

                    MI_DmaCopy16(WM_DMA_NO,     // DMA number
                                 bssDesc,       // src
                                 info->parentInfo[i],   // dest
                                 bssDescLength  // Size of the transmission
                        );

                    break;             // Move to next parent information check
                }
            }
            ++i;
        }
    }
}

void wm_lib_comm_init(void)
{
    int     i;

    WM_lib_set_auto(TRUE);

    MI_DmaClear32(WM_DMA_NO, &libParam, sizeof(libParam));
    MI_DmaClear32(WM_DMA_NO, libParam_statusBuf, WM_STATUS_BUF_SIZE);
    MI_DmaClear32(WM_DMA_NO, libParam_wmbuf, WM_SYSTEM_BUF_SIZE);
    MI_DmaClear32(WM_DMA_NO, libParam_parentParam, WM_PARENT_PARAM_SIZE);
    MI_DmaClear32(WM_DMA_NO, libParam_parentInfoBuf, WM_BSS_DESC_SIZE);
    MI_DmaClear32(WM_DMA_NO, my_parentInfo, sizeof(my_parentInfo));
    MI_DmaClear32(WM_DMA_NO, child_info, sizeof(child_info));
    MI_DmaClear32(WM_DMA_NO, &ACinfo, sizeof(ACinfo));
    MI_DmaClear32(WM_DMA_NO, &libParam_ksbuf, sizeof(libParam_ksbuf));

    libParam.statusBuf = (WMstatus *)&libParam_statusBuf;
    libParam.wmBuf = (void *)&libParam_wmbuf;
    libParam.parentParam = (WMpparam *)&libParam_parentParam;
    libParam.parentInfoBuf = (WMbssDesc *)&libParam_parentInfoBuf;

    libParam.dcfBufSize = WM_DCF_MAX_SIZE;
    libParam.dcfBuf = (WMdcfRecvBuf *)libParam_dcfBuf;

    libParam.wepMode = WM_WEPMODE_NO;  /* WM_WEPMODE_40 */

    libParam.keySharing = 0;
    libParam.contSend = 0;             // 1 -> continuous send

    libParam.parentParam->KS_Flag = libParam.keySharing;
    libParam.parentParam->CS_Flag = libParam.contSend;


    /*
       OS_Printf("parentParam = %d %d\n",sizeof(WMpparam), WM_PARENT_PARAM_SIZE);
       OS_Printf("parentInfoBuf = %d %d\n",sizeof(WMbssDesc), WM_BSS_DESC_SIZE);
       OS_Printf("wmstatus = %d %d\n",sizeof(WMstatus), WM_STATUS_BUF_SIZE);
     */

    // Set the parent information (information obtained by a child when scanning)
    libParam.parentParam->ggid = 0x003fff12;
    libParam.parentParam->tgid = 123;  // Plan to use a random value
    libParam.parentParam->entryFlag = 1;        // Allow child entry
    libParam.parentParam->multiBootFlag = 0;    // Do not allow multiboot

    libParam.parentParam->beaconPeriod = WM_GetDispersionBeaconPeriod();        // Default value for the Beacon interval (ms)
    libParam.parentParam->maxEntry = 15;


//  userGameInfoBuf[0] = 0x1111;        // Test
//  userGameInfoBuf[1] = 0x2222;
//  userGameInfoBuf[2] = 0x3333;
//  userGameInfoBuf[3] = 0x4444;

    libParam.parentParam->userGameInfo = userGameInfoBuf;
    libParam.parentParam->userGameInfoLength = 32;

    libParam.sendBufSize = 0;
    libParam.recvBufSize = 0;

    for (i = 0; i < WM_NUM_CHILD_INFORMATIONS; i++)
    {
        child_info[i].valid = 0;
    }

    wm_lib_parent_found_count_reset();
    for (i = 0; i < WM_NUM_PARENT_INFORMATIONS; i++)
    {
        ACinfo.parentInfo[i] = (WMbssDesc *)&(my_parentInfo[i][0]);
        ACinfo.MacAdrList[i][0] = 0;
        ACinfo.MacAdrList[i][1] = 0;
        ACinfo.MacAdrList[i][2] = 0;
        ACinfo.MacAdrList[i][3] = 0;
        ACinfo.MacAdrList[i][4] = 0;
        ACinfo.MacAdrList[i][5] = 0;
    }
    ACinfo.my_aid = (u16)0;

}

void wm_lib_set_callback(WMcallbackFunc2 callback)
{
    libParam.callback = callback;
}


void wm_lib_set_parent_send_size(u16 size)
{
    libParam.parentParam->parentMaxSize = size; // Parent maximum transmission size (in bytes)
    //  libParam.sendBufSize =  size;                //  Buffer length
    libParam.sendBufSize = WM_CalcParentSendBufSize(libParam);

}

void wm_lib_set_parent_recv_size_per_child(u16 size)
{
    libParam.parentParam->childMaxSize = size;  // Child maximum transmission size (in bytes)
    libParam.recvBufSize = WM_CalcParentRecvBufSize(libParam);
}


void wm_lib_set_keySharing_mode(int flag)
{
    libParam.keySharing = (u16)flag;   // Will be deleted
    libParam.parentParam->KS_Flag = (u16)flag;
    libParam.ksBuf = &libParam_ksbuf;
}


void wm_lib_set_contSend_mode(int flag)
{
    libParam.contSend = (u16)flag;     // Will be deleted
    libParam.parentParam->CS_Flag = (u16)flag;
}


void wm_lib_set_multiboot_mode(int flag)
{
    libParam.parentParam->multiBootFlag = (u16)flag;
}

void wm_lib_set_parent_channel(u16 ch)
{
    libParam.parentParam->channel = ch;
}

void wm_lib_set_beacon_period(u16 period_ms)
{
    libParam.parentParam->beaconPeriod = period_ms;
}

#define WM_LIB_GGID_LO_MASK     (0x0000ffff)
#define WM_LIB_GGID_HI_MASK     (0xffff0000)
#define WM_LIB_GGID_HI_SHIFT    (16)

void wm_lib_set_ggid(u32 ggid)
{
    libParam.parentParam->ggid = ggid;
}

u32 wm_lib_get_ggid(void)
{
    return libParam.parentParam->ggid;
}

void wm_lib_set_tgid(u16 tgid)
{
    libParam.parentParam->tgid = tgid;
}

u16 wm_lib_get_tgid(void)
{
    return libParam.parentParam->tgid;
}

void wm_lib_set_scan_bssid(u16 bssid0, u16 bssid1, u16 bssid2)
{
    WM_lib_set_bssid(bssid2, bssid1, bssid0);
}

void wm_lib_set_gameinfo_username(char *name)
{
    int     i;
    char   *c = (char *)user_name;
    for (i = 0; i < WM_SIZE_USERNAME; i++)
    {
        if (*name == 0)
        {
            break;
        }
        *c++ = *name++;
    }
    libParam.uname = user_name;
}

void wm_lib_set_gameinfo_gamename(char *game)
{
    int     i;
    char   *c = (char *)game_name;
    for (i = 0; i < WM_SIZE_GAMENAME; i++)
    {
        if (*game == 0)
        {
            break;
        }
        *c++ = *game++;
    }
    libParam.gname = game_name;
}


void wm_lib_set_mode(u16 mode)
{
    libParam.mode = mode;
}

void wm_lib_set_sendbuffer(u8 *buf)
{
    libParam.sendBuf = (u16 *)buf;
}

void wm_lib_set_recvbuffer(u8 *buf)
{
    libParam.recvBuf = (u16 *)buf;
}

int wm_lib_get_recvbuffer_size(void)
{
    return (int)libParam.recvBufSize;
}

int wm_lib_get_sendbuffer_size(void)
{
    return (int)libParam.sendBufSize;
}

const char *wm_lib_get_wlib_version(void)
{
    return (const char *)(libParam.statusBuf->wlVersion);
}

void wm_lib_set_ggidFilter(u32 ggidFilter)
{
    libParam.ggidFilter = ggidFilter;
}


int wm_lib_start(void)
{
    int     errcode;

    errcode = WM_lib_Init(&libParam);  // WM_lib initialization

    return errcode;
}


BOOL wm_lib_get_own_macaddress(MACADDRESS * ma)
{
    int     i;
    (void)WM_ReadStatus(libParam.statusBuf);
    for (i = 0; i < 6; i++)
    {
        ma->addr[i] = libParam.statusBuf->MacAddress[i];
    }
    return TRUE;
}


/* Parent Functions */
BOOL wm_lib_is_child_valid(int aid)
{
    if (child_info[aid].valid)
        return TRUE;
    else
        return FALSE;
}


void wm_lib_add_child_list(WMstartParentCallback *arg)
{
    int     i;
    child_info[arg->aid].valid = 1;
    for (i = 0; i < 6; i++)
    {
        child_info[arg->aid].macaddr.addr[i] = arg->macAddress[i];
    }
}

void wm_lib_delete_child_list(WMstartParentCallback *arg)
{
    child_info[arg->aid].valid = 0;
}

BOOL wm_lib_get_child_macaddress(int aid, MACADDRESS * ma)
{
    int     i;
    for (i = 0; i < 6; i++)
    {
        ma->addr[i] = child_info[aid].macaddr.addr[i];
    }
    return TRUE;
}

int wm_lib_set_gameinfo(void)
{
    // Set user name and game name
    MI_DmaCopy16(WM_DMA_NO, libParam.uname,
                 &libParam.parentParam->userGameInfo[0], WM_SIZE_USERNAME);
    MI_DmaCopy16(WM_DMA_NO, libParam.gname,
                 &libParam.parentParam->userGameInfo[WM_SIZE_USERNAME / sizeof(u16)],
                 WM_SIZE_GAMENAME);
    return WM_lib_SetGameInfo(libParam.parentParam->userGameInfo,
                              libParam.parentParam->userGameInfoLength,
                              libParam.parentParam->ggid, libParam.parentParam->tgid);
}

/******************************/
/* Child Functions */

int wm_lib_get_my_aid(void)
{
    return ACinfo.my_aid;
}

void wm_lib_set_my_aid(int aid)
{
    ACinfo.my_aid = (u16)aid;
}

void wm_lib_add_parent_list(WMstartScanCallback *arg)
{
    save_parentInfo(arg, &libParam, &ACinfo);
}

void wm_lib_add_parent_listEx(WMStartScanExCallback *arg)
{
    save_parentInfoEx(arg, &libParam, &ACinfo);
}

void wm_lib_delete_parent_list(WMstartScanCallback *arg)
{
#pragma unused(arg)
    /* Called with WM_STATECODE_PARENT_NOT_FOUND */
    //wm_lib_set_scan_bssid(arg->macAddress[0],buf->macAddress[1], buf->macAddress[2]);  
    // WM_lib_set_scan_channel_seek(TRUE);
    // wm_lib_set_scan_bssid(0xffff,0xffff,0xffff);
}

int wm_lib_get_parent_gameinfo_channel(int id)
{
    return (int)(ACinfo.channelList[id]);
}

BOOL wm_lib_is_parent_gameinfo_valid(int id)
{
    if (ACinfo.gameInfoLength[id] != 0)
        return TRUE;
    else
        return FALSE;
}

int wm_lib_get_parent_gameinfo_parent_sendmaxsize(int id)
{
    return ACinfo.gameInfoList[id].parentMaxSize;
}

int wm_lib_get_parent_gameinfo_child_sendbufsize(int id)
{
    // return ACinfo.gameInfoList[id].childMaxSize;
    return WM_CalcChildSendBufSize((ACinfo.parentInfo[id]));
}

int wm_lib_get_parent_gameinfo_child_recvbufsize(int id)
{
    return WM_CalcChildRecvBufSize((ACinfo.parentInfo[id]));
}


const char *wm_lib_get_parent_gameinfo_username(int id)
{
    return (const char *)(ACinfo.gameInfoList[id].old_type.userName);
}

const char *wm_lib_get_parent_gameinfo_gamename(int id)
{
    return (const char *)(ACinfo.gameInfoList[id].old_type.gameName);
}


u32 wm_lib_get_parent_gameinfo_ggid(int id)
{
    return (ACinfo.gameInfoList[id].ggid);
}

u16 wm_lib_get_parent_gameinfo_tgid(int id)
{
    return (ACinfo.gameInfoList[id].tgid);
}

u16    *wm_lib_get_parent_gameinfo_usergameinfo(int id)
{
    return (ACinfo.gameInfoList[id].userGameInfo);
}

u16 wm_lib_get_parent_gameinfo_usergameinfosize(int id)
{
    return (ACinfo.gameInfoList[id].userGameInfoLength);
}


BOOL wm_lib_get_parent_macaddress(int id, MACADDRESS * ma)
{
    int     i;
    for (i = 0; i < 6; i++)
    {
        ma->addr[i] = ACinfo.MacAdrList[id][i];
    }
    return TRUE;
}

void wm_lib_parent_found_count_reset(void)
{
    ACinfo.found_parent_count = 0;
}

int wm_lib_get_parent_found_count(void)
{
    return ACinfo.found_parent_count;
}

int wm_lib_get_keyset(WMkeySet *keyset)
{
    return WM_GetKeySet(libParam.ksBuf, keyset);
}

int wm_lib_connect_parent(int parent_no)
{
    WMbssDesc *pInfo = (ACinfo.parentInfo[parent_no]);
    libParam.recvBufSize = (u16)wm_lib_get_parent_gameinfo_child_recvbufsize(parent_no);
    libParam.sendBufSize = (u16)wm_lib_get_parent_gameinfo_child_sendbufsize(parent_no);
    return WM_lib_ConnectToParent(pInfo);
}


//  Get pointer to WMbssDesc structure
WMbssDesc *wm_get_parent_bssdesc(int id, WMbssDesc *bssDescp)
{

    if (bssDescp != NULL)
    {
        MI_CpuCopy16(ACinfo.parentInfo[id], bssDescp, sizeof(WMbssDesc));
    }
    return ACinfo.parentInfo[id];

}

int wm_lib_connect_parent_via_bssdesc(WMbssDesc *bssDescp)
{
    if (bssDescp == NULL)
        return WM_ERRCODE_FAILED;

    libParam.recvBufSize = (u16)WM_CalcChildRecvBufSize(bssDescp);
    libParam.sendBufSize = (u16)WM_CalcChildSendBufSize(bssDescp);
    return WM_lib_ConnectToParent(bssDescp);
}

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
