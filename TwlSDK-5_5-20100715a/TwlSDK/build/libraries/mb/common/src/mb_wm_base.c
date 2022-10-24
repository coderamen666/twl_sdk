/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - libraries
  File:     mb_wm_base.c

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



#include "mb_private.h"


/*
 * Determines whether the capacity is correct for the size that is being requested.
 */
SDK_COMPILER_ASSERT(32 + sizeof(MBiParam) + 32 + sizeof(MB_CommPWork) + 32 + WM_SYSTEM_BUF_SIZE <=
                    MB_SYSTEM_BUF_SIZE);
SDK_COMPILER_ASSERT(32 + sizeof(MBiParam) + 32 + sizeof(MB_CommCWork) + 32 + WM_SYSTEM_BUF_SIZE <=
                    MB_CHILD_SYSTEM_BUF_SIZE);


/*
 * Macro to calculate send and receive buffer sizes for the parent and child.
 * Receive buffer:  parentMaxSize + WLHeader (48B) + WMHeader (2B)
 * Send buffer: (parentMaxSize + WMHeader (2B) + ( KeySet (36B) only when KS ) + 31) & 0xffffffe0 (Correct alignment)
 * Receive buffer: 15 devices' worth of (childMaxSize + WLHeader (8B) + WMHeader (2B) + ( KeyData (2B) (only when KeySharing) ) + total Header (8B) + 31) & 0xffffffe0 (Correct alignment)
 */
#define WL_HEADER_LENGTH_P  48         /* WL header length when the parent receives. */
#define WM_HEADER_LENGTH    2          /* WM Header length. */
#define WL_HEADER_LENGTH_T  8          /* Total WL header length. */
#define WL_HEADER_LENGTH_C  8          /* WL header length of each child's data. */


// Macro; computes the size of the child receive buffer.

#define WM_CalcChildSendBufSize(_pInfo_)        (u16)(WM_SIZE_MP_CHILD_SEND_BUFFER(((WMGameInfo *)(&(((WMBssDesc*)(_pInfo_))->gameInfo)))->childMaxSize, FALSE))
#define WM_CalcChildRecvBufSize(_pInfo_)        (u16)(WM_SIZE_MP_CHILD_RECEIVE_BUFFER(((WMGameInfo *)(&(((WMBssDesc*)(_pInfo_))->gameInfo)))->parentMaxSize, FALSE))

// Macro; computes the size of the parent send and receive buffers.
#define WM_CalcParentSendBufSize(_libParam_)    (u16)(WM_SIZE_MP_PARENT_SEND_BUFFER(((MBiParam*)(_libParam_))->parentParam.parentMaxSize, FALSE))
#define WM_CalcParentRecvBufSize(_libParam_)    (u16)(WM_SIZE_MP_PARENT_RECEIVE_BUFFER(((MBiParam*)(_libParam_))->parentParam.childMaxSize, WM_NUM_MAX_CHILD, FALSE))


/* Default value for the Beacon interval (ms). */
#define MB_BEACON_PERIOD_DEFAULT    (200)

/*
 * Debug switch for switching the LifeTime.
 */
#define  NO_LIFETIME    0
#if (NO_LIFETIME == 1)
#define FRAME_LIFE_TIME 0xFFFF
#define CAM_LIFE_TIME   0xFFFF
#define MP_LIFE_TIME    0xFFFF
#else
/* Measure that was performed during the period in which the WM library was unstable. */
/*
#define FRAME_LIFE_TIME 0xFFFF
*/
#define FRAME_LIFE_TIME 5
#define CAM_LIFE_TIME   40
#define MP_LIFE_TIME    40
#endif

#define TABLE_NO    0xFFFF


static u16 mbi_life_table_no = TABLE_NO;
static u16 mbi_life_frame = FRAME_LIFE_TIME;
static u16 mbi_life_cam = CAM_LIFE_TIME;
static u16 mbi_life_mp = MP_LIFE_TIME;
static BOOL mbi_power_save_mode = TRUE;

static BOOL isEnableReject    = FALSE;
static u32  rejectGgid        = 0xffffffff;
static u32  rejectGgidMask    = 0xffffffff;

//===========================================================
// Function Prototype Declarations
//===========================================================
static void MBi_ScanLock(u8 *macAddr); // Scan lock setting function.
static void MBi_ScanUnlock(void);      // Scan lock releasing function.

/*
 * Callback if there is an error when the WM result value is checked.
 */
static void MBi_CheckWmErrcode(u16 apiid, int errcode);
static void MBi_ParentCallback(void *arg);



/******************************************************************************/
/* Variables */

static MBiParam *p_mbi_param;
static u16 WM_DMA_NO;

/*
 * WM system buffer.
 * This was originally a member of MBiParam, but it was made independent for MB_StartParentFromIdle().
 * 
 */
static u8 *wmBuf;


MB_CommCommonWork *mbc = NULL;


/* Variables for child */
WMscanExParam mbiScanParam ATTRIBUTE_ALIGN(32);


/******************************************************************************/
/* Functions */


/* Cycle through ScanChannel and make changes. */
static BOOL changeScanChannel(WMscanExParam *p)
{
    u16     channel_bmp, channel, i;

    /* Get channel bitmap. */
    channel_bmp = WM_GetAllowedChannel();

    /* If there is no channel that can be used, return FALSE. */
    if (channel_bmp == 0)
    {
        OS_TWarning("No Available Scan channel\n");
        return FALSE;
    }

    /* If usable channels exist. */
    for (i = 0, channel = p->channelList;
         i < 16; i++, channel = (u16)((channel == 0) ? 1 : channel << 1))
    {
        if ((channel_bmp & channel ) == 0)
        {
            continue;
        }

        /* If the detected channel is the same one as before, search for another channel. */
        if (p->channelList != channel)
        {
            p->channelList =  channel;
            break;
        }
    }

    return TRUE;

}


/*---------------------------------------------------------------------------*
  Name:         MB_SetLifeTime

  Description:  Explicitly specifies the lifetime of the MB wireless driver.
                The default value is ( 0xFFFF, 40, 0xFFFF, 40 ).

  Arguments:    tgid            TGID that will be specified.

  Returns:      The argument as is, or a suitable TGID.
 *---------------------------------------------------------------------------*/
void MB_SetLifeTime(u16 tableNumber, u16 camLifeTime, u16 frameLifeTime, u16 mpLifeTime)
{
    mbi_life_table_no = tableNumber;
    mbi_life_cam = camLifeTime;
    mbi_life_frame = frameLifeTime;
    mbi_life_mp = mpLifeTime;
}

/*---------------------------------------------------------------------------*
  Name:         MB_SetPowerSaveMode

  Description:  Sets continuous power mode.
                This option is for stabilized driving in situations where power consumption can be ignored. It is disabled by default.
                
                This should not be used by typical game applications unless they are being operated in an environment in which connection to power is guaranteed.
                

  Arguments:    enable:           To enable, TRUE; To disable, FALSE.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MB_SetPowerSaveMode(BOOL enable)
{
    mbi_power_save_mode = enable;
}


/*
 * If we are currently processing, consider this to be success and return the converted WM result.
 */
static inline int conv_errcode(int errcode)
{
    return (errcode == WM_ERRCODE_OPERATING) ? WM_ERRCODE_SUCCESS : errcode;
}


/*
 * Check MP transmit permission
   
   Added the mpBusy flag, which is set when SetMP is executed, to the determination elements, so that MP will not be set again after SetMP and before the callback returns.
  
 
 */
static BOOL MBi_IsSendEnabled(void)
{
    return (p_mbi_param->mpStarted == 1) &&
        (p_mbi_param->mpBusy == 0) &&
        (p_mbi_param->endReq == 0) && (p_mbi_param->child_bitmap != 0);
}

static void MBi_OnInitializeDone(void)
{
    int     errcode;
    /* Issue API */
    errcode = WM_SetIndCallback(MBi_ParentCallback);
    MBi_CheckWmErrcode(WM_APIID_INDICATION, errcode);
    errcode = WM_SetLifeTime(MBi_ParentCallback, mbi_life_table_no,
                             mbi_life_cam, mbi_life_frame, mbi_life_mp);
    MBi_CheckWmErrcode(WM_APIID_SET_LIFETIME, errcode);
}


/* 
 * If the MB parent device is using ichneumon to operate wirelessly with VRAM, the parent's MP data settings will be processed too fast if there are not enough child units, and the IPL will not follow the parent's MP communications.
 * 
 * 
 * As a countermeasure to this, insert a fixed wait.
 */
static inline void MbWaitForWvr(u32 cycles)
{
    u32     child_cnt = 0;
    u32     i;

    for (i = 0; i < MB_MAX_CHILD; i++)
    {
        if (pPwork->p_comm_state[i] != MB_COMM_PSTATE_NONE)
        {
            if (++child_cnt >= 2)
                break;
        }
    }
    // If there is only one child, insert a wait process.
    if (child_cnt == 1)
    {
        OS_SpinWait(cycles);
    }
}

/*---------------------------------------------------------------------------*
  Name:         MBi_EndCommon

  Description:  Common MB end processing.

  Arguments:    arg:              End callback arguments to the user.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MBi_EndCommon(void *arg)
{
    p_mbi_param->mbIsStarted = 0;
    mbc->isMbInitialized = 0;
    if (p_mbi_param->callback)
    {
        p_mbi_param->callback(MB_CALLBACK_END_COMPLETE, arg);
    }
}

/*
 * Parent device callback
 */
static void MBi_ParentCallback(void *arg)
{
    int     errcode;
    WMCallback *buf = (WMCallback *)arg;

    switch (buf->apiid)
    {
    case WM_APIID_INITIALIZE:
        {
            if (buf->errcode != WM_ERRCODE_SUCCESS)
            {
                p_mbi_param->callback(MB_CALLBACK_ERROR, arg);
                return;
            }
            MBi_OnInitializeDone();
        }
        break;

    case WM_APIID_SET_LIFETIME:
        {
            if (buf->errcode != WM_ERRCODE_SUCCESS)
            {
                p_mbi_param->callback(MB_CALLBACK_ERROR, arg);
                return;
            }

            // Issue API
            errcode = WM_SetParentParameter(MBi_ParentCallback, &p_mbi_param->parentParam);
            MBi_CheckWmErrcode(WM_APIID_SET_P_PARAM, errcode);
        }
        break;

    case WM_APIID_SET_P_PARAM:
        {
            p_mbi_param->callback(MB_CALLBACK_INIT_COMPLETE, arg);
            // An error code does not get returned here.
            errcode = WM_SetBeaconIndication(MBi_ParentCallback, 1 /* 1:ON, 0:OFF */ );
            MBi_CheckWmErrcode(WM_APIID_SET_BEACON_IND, errcode);
        }
        break;

        // Check whether or not to cause Beacon Send/Recv Indication to be issued.
    case WM_APIID_SET_BEACON_IND:
        if (!p_mbi_param->endReq)
        {
            if (buf->errcode != WM_ERRCODE_SUCCESS)
            {
                p_mbi_param->callback(MB_CALLBACK_ERROR, arg);
                return;
            }

            errcode = WMi_StartParentEx(MBi_ParentCallback, mbi_power_save_mode);
            MBi_CheckWmErrcode(WM_APIID_START_PARENT, errcode);
        }
        else
        {
            // Disable beacon notifications and complete end processing
            if (buf->errcode != WM_ERRCODE_SUCCESS)
            {
                p_mbi_param->endReq = 0;
                p_mbi_param->callback(MB_CALLBACK_ERROR, arg);
            }
            else
            {
                MBi_EndCommon(arg);
            }
        }
        break;

    case WM_APIID_START_PARENT:
        {
            WMstartParentCallback *callback = (WMstartParentCallback *)arg;

            if (callback->errcode != WM_ERRCODE_SUCCESS)
            {
                p_mbi_param->callback(MB_CALLBACK_ERROR, arg);
                return;
            }

            switch (callback->state)
            {
            case WM_STATECODE_PARENT_START:
                p_mbi_param->child_bitmap = 0;
                p_mbi_param->mpStarted = 0;
                break;
            case WM_STATECODE_CHILD_CONNECTED:

                /* Forcibly break after a call to MB_End(). */
                if (p_mbi_param->endReq == 1)
                    break;

                p_mbi_param->child_bitmap |= (0x1 << callback->aid);
                p_mbi_param->callback(MB_CALLBACK_CHILD_CONNECTED, arg);

                // If MP has not been started, start MP.
                if ((p_mbi_param->mpStarted == 0) && (!mbc->start_mp_busy))
                {
                    mbc->start_mp_busy = TRUE;
                    errcode = WM_StartMPEx(MBi_ParentCallback, (u16 *)p_mbi_param->recvBuf, p_mbi_param->recvBufSize, (u16 *)p_mbi_param->sendBuf, p_mbi_param->sendBufSize, (u16)(p_mbi_param->contSend ? 0 : 1), 0,   /* defaultRetryCount */
                                           FALSE, FALSE, 1,     /* fixFreqMode */
                                           TRUE /* ignoreFatalError */
                        );
                    MBi_CheckWmErrcode(WM_APIID_START_MP, errcode);

                }
                else
                {
                    // MP send-enabled callback.
                    if (MBi_IsSendEnabled())
                    {
                        p_mbi_param->callback(MB_CALLBACK_MP_SEND_ENABLE, NULL);
                    }
                }
                break;

            case WM_STATECODE_DISCONNECTED:
                p_mbi_param->child_bitmap &= ~(0x1 << callback->aid);
                p_mbi_param->callback(MB_CALLBACK_CHILD_DISCONNECTED, arg);
                break;

            case WM_STATECODE_DISCONNECTED_FROM_MYSELF:
                // Do nothing if it disconnects itself.
                break;

            case WM_STATECODE_BEACON_SENT:
                /* Forcibly break after a call to MB_End(). */
                if (p_mbi_param->endReq == 1)
                    break;

                // Send a notification that beacon transmission completed.
                p_mbi_param->callback(MB_CALLBACK_BEACON_SENT, arg);
                break;

            default:
                p_mbi_param->callback(MB_CALLBACK_ERROR, arg);
                break;
            }
        }
        break;

    case WM_APIID_START_MP:
        {
            // An error code does not get returned here.
            WMstartMPCallback *callback = (WMstartMPCallback *)arg;
            mbc->start_mp_busy = FALSE;
            switch (callback->state)
            {
            case WM_STATECODE_MP_START:
                // Set the flag that indicates MP has started.
                p_mbi_param->mpStarted = 1;
                {
                    // MP send-enabled callback.
                    if (p_mbi_param->endReq == 0)
                    {
                        p_mbi_param->callback(MB_CALLBACK_MP_SEND_ENABLE, NULL);
                    }
                }
                break;

            case WM_STATECODE_MPEND_IND:
                // MP receive callback.
                p_mbi_param->callback(MB_CALLBACK_MP_PARENT_RECV, (void *)(callback->recvBuf));
                break;

            default:
                p_mbi_param->callback(MB_CALLBACK_ERROR, arg);
                break;
            }
            break;
        }
        break;

    case WM_APIID_SET_MP_DATA:
        {
            /* If MB parent is using ichneumon, enter a wait */
            if (pPwork->useWvrFlag)
            {
                MbWaitForWvr(13000);   // Make ARM9 wait for approximately 3 lines during an IRQ interrupt.
            }
            p_mbi_param->mpBusy = 0;
            if (buf->errcode == WM_ERRCODE_SUCCESS)
            {
                p_mbi_param->callback(MB_CALLBACK_MP_PARENT_SENT, arg);
                if (p_mbi_param->endReq == 0)
                {
                    p_mbi_param->callback(MB_CALLBACK_MP_SEND_ENABLE, NULL);        // Allow the next transmission.
                }
            }
            else if (buf->errcode == WM_ERRCODE_SEND_QUEUE_FULL)
            {
                p_mbi_param->callback(MB_CALLBACK_SEND_QUEUE_FULL_ERR, arg);
            }
            else
            {
                p_mbi_param->callback(MB_CALLBACK_MP_PARENT_SENT_ERR, arg);
                if (p_mbi_param->endReq == 0)
                {
                    p_mbi_param->callback(MB_CALLBACK_MP_SEND_ENABLE, NULL);        // Allow the next transmission.
                }
            }
        }
        break;

    case WM_APIID_RESET:
        if (!mbc->is_started_ex)
        {
            if (buf->errcode != WM_ERRCODE_SUCCESS)
            {
                p_mbi_param->endReq = 0;
                p_mbi_param->callback(MB_CALLBACK_ERROR, arg);
            }
            else
            {
                p_mbi_param->child_bitmap = 0;
                p_mbi_param->mpStarted = 0;
                errcode = WM_End(MBi_ParentCallback);
                MBi_CheckWmErrcode(WM_APIID_END, errcode);
            }
        }
        else
        {
            (void)WM_SetPortCallback(WM_PORT_BT, NULL, NULL);
            (void)WM_SetIndCallback(NULL);
            if (buf->errcode != WM_ERRCODE_SUCCESS)
            {
                p_mbi_param->endReq = 0;
                p_mbi_param->callback(MB_CALLBACK_ERROR, arg);
            }
            else
            {
                errcode = WM_SetBeaconIndication(MBi_ParentCallback, 0);
                MBi_CheckWmErrcode(WM_APIID_SET_BEACON_IND, errcode);
            }
        }
        break;
    case WM_APIID_END:
        {
            if (buf->errcode != WM_ERRCODE_SUCCESS)
            {
                p_mbi_param->endReq = 0;
                p_mbi_param->callback(MB_CALLBACK_ERROR, arg);
            }
            else
            {
                MBi_EndCommon(arg);
            }
        }
        break;

    case WM_APIID_DISCONNECT:
        {
            WMDisconnectCallback *callback = (WMDisconnectCallback *)arg;

            if (buf->errcode != WM_ERRCODE_SUCCESS)
            {
                return;
            }

            // Update the child information.
            p_mbi_param->child_bitmap &= ~(callback->disconnectedBitmap);
        }
        break;

    case WM_APIID_INDICATION:
        {
            WMindCallback *cb = (WMindCallback *)arg;
            switch (cb->state)
            {
            case WM_STATECODE_BEACON_RECV:     // Indicate beacon received
                p_mbi_param->callback(MB_CALLBACK_BEACON_RECV, arg);
                break;
            case WM_STATECODE_DISASSOCIATE:    // Indicate disconnection
                p_mbi_param->callback(MB_CALLBACK_DISASSOCIATE, arg);
                break;
            case WM_STATECODE_REASSOCIATE:     // Indicate reconnection
                p_mbi_param->callback(MB_CALLBACK_REASSOCIATE, arg);
                break;
            case WM_STATECODE_AUTHENTICATE:    // Indicate confirmed authentication
                p_mbi_param->callback(MB_CALLBACK_AUTHENTICATE, arg);
                break;

            case WM_STATECODE_FIFO_ERROR:
                OS_TPanic("FIFO Error\n");
                break;
            case WM_STATECODE_INFORMATION:
                // Does nothing.
                break;
            }
        }
        break;

    default:
        p_mbi_param->callback(MB_CALLBACK_ERROR, arg);
        break;
    }
}

/*
 * Child device callback
 */
static void MBi_ChildPortCallback(void *arg)
{
    WMPortRecvCallback *cb = (WMPortRecvCallback *)arg;

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        return;
    }

    switch (cb->state)
    {
    case WM_STATECODE_PORT_RECV:
        // Notify the program that data was received.
        p_mbi_param->callback(MB_CALLBACK_MP_CHILD_RECV, (void *)(arg));
        break;
    case WM_STATECODE_CONNECTED:
        // Connection notification
        break;
    case WM_STATECODE_PORT_INIT:
    case WM_STATECODE_DISCONNECTED:
    case WM_STATECODE_DISCONNECTED_FROM_MYSELF:
        break;
    }
}

static void MBi_ChildCallback(void *arg)
{
    WMCallback *buf = (WMCallback *)arg;
    MBiParam *const p_param = p_mbi_param;
    int     errcode;

    switch (buf->apiid)
    {
        //---------------------------------------------------------------
    case WM_APIID_INITIALIZE:
        {
            if (buf->errcode != WM_ERRCODE_SUCCESS)
            {
                p_param->callback(MB_CALLBACK_ERROR, arg);
                return;
            }

            p_param->callback(MB_CALLBACK_INIT_COMPLETE, arg);

            // Issue API
            errcode =
                WM_SetLifeTime(MBi_ChildCallback, mbi_life_table_no, mbi_life_cam, mbi_life_frame,
                               mbi_life_mp);
            MBi_CheckWmErrcode(WM_APIID_SET_LIFETIME, errcode);
        }
        break;

    case WM_APIID_SET_LIFETIME:
        {
            if (buf->errcode != WM_ERRCODE_SUCCESS)
            {
                p_param->callback(MB_CALLBACK_ERROR, arg);
                return;
            }

            // Issue WM_StartScan
            if (mbiScanParam.channelList == 0)
                mbiScanParam.channelList = 0x0001; /* 1ch */
            if (mbiScanParam.maxChannelTime == 0)
            {
                mbiScanParam.maxChannelTime = 200;
            }
            mbiScanParam.bssid[0] = 0xff;
            mbiScanParam.bssid[1] = 0xff;
            mbiScanParam.bssid[2] = 0xff;
            mbiScanParam.bssid[3] = 0xff;
            mbiScanParam.bssid[4] = 0xff;
            mbiScanParam.bssid[5] = 0xff;
            mbiScanParam.scanType = WM_SCANTYPE_PASSIVE;
            mbiScanParam.ssidLength = 0;
            p_param->scanning_flag = TRUE;
            p_param->scan_channel_flag = TRUE;
            errcode = WM_StartScanEx(MBi_ChildCallback, &mbiScanParam);
            MBi_CheckWmErrcode(WM_APIID_START_SCAN_EX, errcode);
        }
        break;

    case WM_APIID_START_SCAN_EX:
        {
            WMstartScanExCallback *callback = (WMstartScanExCallback *)arg;

            if (callback->errcode != WM_ERRCODE_SUCCESS)
            {
                p_param->callback(MB_CALLBACK_ERROR, arg);
                return;
            }

            switch (callback->state)
            {
            case WM_STATECODE_SCAN_START:
                break;

            case WM_STATECODE_PARENT_FOUND:
                // Store the parent information gotten with scan
                {
                    WMstartScanExCallback *tmpBuf = (WMstartScanExCallback *)arg;

                    u8 j;
                    u8 numInvalidBeacon =0;
                    for(j=0;j<tmpBuf->bssDescCount;j++)
                    {
                        WMBssDesc *buffer = tmpBuf->bssDesc[j];
                        ParentInfo *p = &p_param->parent_info[0];
                        int i;
                        
                        /* A problem was discovered where MP communication beacons that are not general AP or MB fill up the list, so beacons from other than MB parent devices are removed from the list.
                         *    */
                        if( !WM_IsValidGameBeacon( buffer ) || !MBi_CheckMBParent( buffer ) )
                        {
                            numInvalidBeacon++;
                            continue; //Anything other than the MB beacon is destroyed
                        }

                        /* In the TWL download play feature, removed from list due to Parental Control reasons. 
                         *                                               */
                        if( isEnableReject == TRUE && ((buffer->gameInfo.ggid & rejectGgidMask) == (rejectGgid & rejectGgidMask)) )
                        {
                            numInvalidBeacon++;
                            OS_TPrintf("GGID %08X is reject!\n", buffer->gameInfo.ggid);
                            continue; //Anything other than the MB beacon is destroyed
                        }
                        
                        for (i = 0; i < p_param->found_parent_count; ++i)
                        {

                            if (WM_IsBssidEqual(buffer->bssid, p[i].scan_data.macAddress))
                            {
                                p[i].scan_data.gameInfoLength = buffer->gameInfoLength;
                                p[i].scan_data.gameInfo = buffer->gameInfo;
                                DC_InvalidateRange(p_param->parent_info[i].parentInfo,
                                                   WM_BSS_DESC_SIZE);
                                MI_DmaCopy16(WM_DMA_NO, buffer,
                                             p_param->parent_info[i].parentInfo, WM_BSS_DESC_SIZE);
                                p_param->last_found_parent_no = i;
                                p_param->callback(MB_CALLBACK_PARENT_FOUND, &tmpBuf->linkLevel[j]);
                                goto scan_end; //In the meantime, do not receive multiple MB beacons
                            }
                        }

                        if (i < MB_NUM_PARENT_INFORMATIONS)
                        {
                            p_param->found_parent_count = (u16)(i + 1);
                            // This is a newly discovered parent device, so store the data (to maintain compatibility, store in StartScanCallback)
                            MI_CpuCopy8(buffer->bssid, p[i].scan_data.macAddress, WM_SIZE_BSSID );
                            p[i].scan_data.channel   = buffer->channel;
                            p[i].scan_data.linkLevel = tmpBuf->linkLevel[j];
                            MI_CpuCopy16(&buffer->gameInfo, &p[i].scan_data.gameInfo, WM_SIZE_GAMEINFO);
                            
                            DC_InvalidateRange(p_param->parent_info[i].parentInfo, WM_BSS_DESC_SIZE);
                            MI_DmaCopy16(WM_DMA_NO, buffer,
                                         p_param->parent_info[i].parentInfo, WM_BSS_DESC_SIZE);
                            p_param->last_found_parent_no = i;
                            p_param->callback(MB_CALLBACK_PARENT_FOUND, &tmpBuf->linkLevel[j]);
                            goto scan_end; //In the meantime, do not receive multiple MB beacons
                        }
                    }
                    if( tmpBuf->bssDescCount == numInvalidBeacon)
                    {
                        p_param->callback(MB_CALLBACK_PARENT_NOT_FOUND, arg);   // This callback to ParentInfoLifeTimeCount
                    }
                    
                scan_end:
                    if (!p_param->scanning_flag)
                    {
                        return;
                    }

                    if (p_param->scan_channel_flag)
                    {
                        if (FALSE == changeScanChannel(&mbiScanParam))
                        {
                            (void)MBi_CommEnd();
                        }
                    }
                    errcode = WM_StartScanEx(MBi_ChildCallback, &mbiScanParam);
                    MBi_CheckWmErrcode(WM_APIID_START_SCAN_EX, errcode);
                }
                break;

            case WM_STATECODE_PARENT_NOT_FOUND:
                p_param->callback(MB_CALLBACK_PARENT_NOT_FOUND, arg);   // This callback to ParentInfoLifeTimeCount
                if (!p_param->scanning_flag)
                {
                    return;
                }

                if (p_param->scan_channel_flag)
                {
                    if (FALSE == changeScanChannel(&mbiScanParam))
                    {
                        (void)MBi_CommEnd();
                    }
                }
                errcode = WM_StartScanEx(MBi_ChildCallback, &mbiScanParam);
                MBi_CheckWmErrcode(WM_APIID_START_SCAN_EX, errcode);
                break;

            default:
                p_param->callback(MB_CALLBACK_ERROR, arg);
                break;
            }
        }
        break;

    case WM_APIID_END_SCAN:
        {
            if (buf->errcode != WM_ERRCODE_SUCCESS)
            {
                p_param->callback(MB_CALLBACK_ERROR, arg);
                return;
            }

            errcode = WM_StartConnect(MBi_ChildCallback, p_param->pInfo, NULL);
            MBi_CheckWmErrcode(WM_APIID_START_CONNECT, errcode);
        }
        break;

    case WM_APIID_START_CONNECT:
        {
            WMstartConnectCallback *callback = (WMstartConnectCallback *)arg;

            if (callback->errcode != WM_ERRCODE_SUCCESS)
            {
                /* Reset the number of parents. */
                p_param->found_parent_count = 0;
                // Error notification by a callback.
                p_param->callback(MB_CALLBACK_CONNECT_FAILED, arg);
                return;
            }

            switch (callback->state)
            {
            case WM_STATECODE_CONNECT_START:
                p_param->child_bitmap = 0;
                p_param->mpStarted = 1;
                break;

            case WM_STATECODE_CONNECTED:
                p_param->my_aid = (u16)callback->aid;
                p_param->callback(MB_CALLBACK_CONNECTED_TO_PARENT, arg);
                p_param->child_bitmap = 1;

                errcode = WM_SetPortCallback(WM_PORT_BT, MBi_ChildPortCallback, NULL);

                if (errcode != WM_ERRCODE_SUCCESS)
                {
                    break;
                }

                errcode = WM_StartMPEx(MBi_ChildCallback, (u16 *)p_param->recvBuf, p_param->recvBufSize, (u16 *)p_param->sendBuf, p_param->sendBufSize, (u16)(p_param->contSend ? 0 : 1), 0,    /* defaultRetryCount */
                                       FALSE, FALSE, 1, /* fixFreqMode */
                                       TRUE     /* ignoreFatalError */
                    );
                MBi_CheckWmErrcode(WM_APIID_START_MP, errcode);
                break;

            case WM_STATECODE_DISCONNECTED:
                p_param->callback(MB_CALLBACK_DISCONNECTED_FROM_PARENT, arg);
                p_param->child_bitmap = 0;
                p_param->mpStarted = 0;
                break;

            case WM_STATECODE_DISCONNECTED_FROM_MYSELF:
                // Do nothing if it disconnects itself.
                break;

            default:
                p_param->callback(MB_CALLBACK_ERROR, arg);
                break;
            }
        }
        break;

    case WM_APIID_START_MP:
        {
            WMstartMPCallback *callback = (WMstartMPCallback *)arg;

            switch (callback->state)
            {
            case WM_STATECODE_MP_START:
                p_param->mpStarted = 1; // Set the flag that indicates MP has started.
                {
                    // MP send-enabled callback.
                    if (MBi_IsSendEnabled())
                    {
                        p_param->callback(MB_CALLBACK_MP_SEND_ENABLE, NULL);
                    }
                }
                break;

            case WM_STATECODE_MP_IND:
                if (callback->errcode == WM_ERRCODE_INVALID_POLLBITMAP)
                {
//                  p_param->callback( MB_CALLBACK_MP_CHILD_RECV, (void*)(callback->recvBuf) );
                }
                else
                {
//                    p_param->callback( MB_CALLBACK_MP_CHILD_RECV, (void*)(callback->recvBuf) );
                }
                break;

            case WM_STATECODE_MPACK_IND:
                {
                    //p_param->callback( MB_CALLBACK_MPACK_IND, NULL );
                }
                break;

            default:
                p_param->callback(MB_CALLBACK_ERROR, arg);
                break;
            }
        }
        break;

    case WM_APIID_SET_MP_DATA:
        {
            p_param->mpBusy = 0;
            if (buf->errcode == WM_ERRCODE_SUCCESS)
            {
                p_param->callback(MB_CALLBACK_MP_CHILD_SENT, arg);
            }
            else if (buf->errcode == WM_ERRCODE_TIMEOUT)
            {
                p_param->callback(MB_CALLBACK_MP_CHILD_SENT_TIMEOUT, arg);
            }
            else
            {
                p_param->callback(MB_CALLBACK_MP_CHILD_SENT_ERR, arg);
            }
            // Allow the next transmission.
            if (p_mbi_param->endReq == 0)
            {
                p_param->callback(MB_CALLBACK_MP_SEND_ENABLE, NULL);
            }
        }
        break;

        //---------------------------------------------------------------
    case WM_APIID_RESET:
        {
            if (buf->errcode != WM_ERRCODE_SUCCESS)
            {
                p_param->endReq = 0;
                p_param->callback(MB_CALLBACK_ERROR, arg);
                return;
            }
            p_mbi_param->child_bitmap = 0;
            p_mbi_param->mpStarted = 0;

            errcode = WM_End(MBi_ChildCallback);
            MBi_CheckWmErrcode(WM_APIID_END, errcode);
        }
        break;

        //---------------------------------------------------------------
    case WM_APIID_END:
        {
            if (buf->errcode != WM_ERRCODE_SUCCESS)
            {
                p_param->endReq = 0;
                p_param->callback(MB_CALLBACK_ERROR, arg);
                return;
            }
            MBi_EndCommon(arg);
        }
        break;

        //---------------------------------------------------------------
    case WM_APIID_START_KS:
        {
            // MP send-enabled callback.
            if (MBi_IsSendEnabled())
            {
                p_param->callback(MB_CALLBACK_MP_SEND_ENABLE, NULL);
            }
        }
        break;

        //---------------------------------------------------------------        
    case WM_APIID_INDICATION:
        {
            WMindCallback *cb = (WMindCallback *)arg;
            switch (cb->state)
            {
            case WM_STATECODE_FIFO_ERROR:
                OS_TPanic("FIFO Error\n");
                break;
            case WM_STATECODE_INFORMATION:
                // Does nothing.
                break;
            }
        }
        break;
    default:
        p_param->callback(MB_CALLBACK_ERROR, arg);
        break;
    }
}


/*---------------------------------------------------------------------------*
  Name:         MBi_GetBeaconPeriodDispersion

  Description:  Gets the value to be added to the beacon interval with appropriate variation for each device's MAC address.
                

  Arguments:    None.

  Returns:      u32 -   Returns a value between 0 and 19 that is varied as appropriate for each device.
 *---------------------------------------------------------------------------*/
static u32 MBi_GetBeaconPeriodDispersion(void)
{
    u8      mac[6];
    u32     ret;
    s32     i;

    OS_GetMacAddress(mac);
    for (i = 0, ret = 0; i < 6; i++)
    {
        ret += mac[i];
    }
    ret += OS_GetVBlankCount();
    ret *= 7;
    return (ret % 20);
}


/******************************************************************************/
/* API */

/* Initialization */
int MB_Init(void *work, const MBUserInfo *user, u32 ggid, u32 tgid, u32 dma)
{

    SDK_ASSERT(user != NULL);
    SDK_ASSERT(work != NULL);
    SDK_ASSERT((dma < 4));
    SDK_ASSERT(user->nameLength <= MB_USER_NAME_LENGTH);

    if (mbc && mbc->isMbInitialized)
    {
        return MB_ERRCODE_INVALID_STATE;
    }
    else
    {
        MBiParam *const p_parm = (MBiParam *) ((((u32)work) + 0x1F) & ~0x1F);
        MB_CommCommonWork *const p_com =
            (MB_CommCommonWork *) ((((u32)p_parm) + sizeof(*p_parm) + 0x1F) & ~0x1F);
        OSIntrMode enabled;

        /*
         * Automatic retrieval is performed if TGID is automatically specified.
         * (since RTC is used internally, retrieve before interrupts are disabled)
         */
        if (tgid == MB_TGID_AUTO)
        {
            tgid = WM_GetNextTgid();
        }

        enabled = OS_DisableInterrupts();

        /* Initialization of values for lifetime and power-save mode settings. */
        mbi_life_table_no = TABLE_NO;
        mbi_life_frame = FRAME_LIFE_TIME;
        mbi_life_cam = CAM_LIFE_TIME;
        mbi_life_mp = MP_LIFE_TIME;
        mbi_power_save_mode = TRUE;

        /* Initialize the DMA channel and work region. */
        WM_DMA_NO = (u16)dma;
        p_mbi_param = p_parm;
        mbc = p_com;
        MI_CpuClear32(p_parm, sizeof(*p_parm));
        MI_CpuClear16(p_com, sizeof(*p_com));

        {                              /* Save the user name and game name. */
            int     i;
            static const u16 *game = L"multiboot";
            u16    *c;
            c = (u16 *)p_parm->uname;
            for (i = 0; i < user->nameLength; ++i)
            {
                *c++ = user->name[i];
            }
            c = (u16 *)p_parm->gname;
            for (i = 0; i < WM_SIZE_GAMENAME; ++i)
            {
                if (*game == 0)
                {
                    break;
                }
                *c++ = *game++;
            }
            MI_CpuCopy8(user, &p_com->user, sizeof(MBUserInfo));
            if (user->nameLength < MB_USER_NAME_LENGTH)
            {
                p_com->user.name[user->nameLength] = 0;
            }
        }

        {
            p_parm->p_sendlen = MB_COMM_P_SENDLEN_DEFAULT;
            p_parm->p_recvlen = MB_COMM_P_RECVLEN_DEFAULT;

            /* Set the parent information (information obtained by a child when scanning). */
            p_parm->sendBufSize = 0;
            p_parm->recvBufSize = 0;

            /* Unnecessary? */
            p_parm->contSend = 1;

            p_parm->recvBuf = (WMmpRecvBuf *)p_com->recvbuf;

            {
                WMParentParam *const p_parent = &p_parm->parentParam;
                /*
                 * The beacon's entry and mb are disabled the first time.
                 * This is turned on with SetGameInfo in MbBeacon.
                 */
                p_parent->entryFlag = 0;
                p_parent->multiBootFlag = 0;
                p_parent->CS_Flag = 1;
                p_parent->KS_Flag = 0;
                /* Set this device's unique GGID and TGID. */
                p_parent->ggid = ggid;
                p_parent->tgid = (u16)tgid;
                p_parent->beaconPeriod =
                    (u16)(MB_BEACON_PERIOD_DEFAULT + MBi_GetBeaconPeriodDispersion());
                p_parent->maxEntry = 15;
            }
        }

        p_parm->mpBusy = 0;
        p_parm->mbIsStarted = 0;
        p_com->isMbInitialized = 1;

        p_com->start_mp_busy = FALSE;

        (void)OS_RestoreInterrupts(enabled);
    }

    return MB_ERRCODE_SUCCESS;
}

#define MP_USEC_TIME_LIMIT  (5600)

static BOOL MBi_IsCommSizeValid(u16 sendSize, u16 recvSize, u16 entry_num)
{
    int     usec;

    SDK_ASSERT(entry_num > 0 && entry_num <= 15);

    /* If sendSize is outside the defined range, judge it to be an invalid size setting. */
    if (sendSize > MB_COMM_P_SENDLEN_MAX || sendSize < MB_COMM_P_SENDLEN_MIN)
    {
        OS_TWarning("MB Parent send buffer size is out of the range.[%3d - %3d Bytes]\n",
                    MB_COMM_P_SENDLEN_MIN, MB_COMM_P_SENDLEN_MAX);
        return FALSE;
    }

    /* If recvSize is outside the defined range, judge it to be an invalid size setting. */
    if (recvSize > MB_COMM_P_RECVLEN_MAX || recvSize < MB_COMM_P_RECVLEN_MIN)
    {
        OS_TWarning
            ("MB Parent receive buffer size per child is out of the range.[%3d - %3d Bytes]\n",
             MB_COMM_P_RECVLEN_MIN, MB_COMM_P_RECVLEN_MAX);
        return FALSE;
    }

    /* Evaluation for the time taken to perform MP communications once. */
    usec = 330 + 4 * (sendSize + 38) + entry_num * (112 + 4 * (recvSize + 32));

    /* If needed time exceeds 5600 microseconds, judge it to be an invalid size setting. */
    if (usec >= MP_USEC_TIME_LIMIT)
    {
        OS_TWarning("These send receive sizes require lower than %4dusec\n"
                    "it exceeds %4d usec\n", MP_USEC_TIME_LIMIT, usec);
        return FALSE;
    }

    return TRUE;
}


BOOL MB_SetParentCommParam(u16 sendSize, u16 maxChildren)
{
    OSIntrMode enabled = OS_DisableInterrupts();

    /* Cannot change if WM has started. */
    if (p_mbi_param->mbIsStarted)
    {
        OS_TWarning("MB has Already started\n");

        (void)OS_RestoreInterrupts(enabled);
        return FALSE;
    }

    /* Judge whether the send/receive data sizes are correct */
    if (FALSE == MBi_IsCommSizeValid(sendSize, MB_COMM_P_RECVLEN_DEFAULT, maxChildren))
    {
        OS_TWarning("MP data sizes have not changed\n");
        // 
        (void)OS_RestoreInterrupts(enabled);
        return FALSE;
    }

    /* Set the maximum number of people that can connect. */
    p_mbi_param->parentParam.maxEntry = maxChildren;

    /* Set the size of the data being sent and received. */
    p_mbi_param->p_sendlen = sendSize;
    p_mbi_param->p_recvlen = MB_COMM_P_RECVLEN_DEFAULT;

    (void)OS_RestoreInterrupts(enabled);

    return TRUE;
}


BOOL MB_SetParentCommSize(u16 sendSize)
{
    OSIntrMode enabled = OS_DisableInterrupts();
    BOOL    ret;

    /* The maximum number of connected people inherits the current settings. */
    ret = MB_SetParentCommParam(sendSize, p_mbi_param->parentParam.maxEntry);

    (void)OS_RestoreInterrupts(enabled);

    return ret;
}

u16 MB_GetTgid(void)
{
    return p_mbi_param->parentParam.tgid;
}

/* Initialization (The part common to parent and child) */
static int MBi_StartCommon(void)
{
    int     errcode;

    p_mbi_param->mpStarted = 0;
    p_mbi_param->child_bitmap = 0;
    p_mbi_param->endReq = 0;
    p_mbi_param->currentTgid = 0;
    MBi_SetMaxScanTime(MB_SCAN_TIME_NORMAL);

    if (!mbc->is_started_ex)
    {
        do
        {
            errcode = WM_Initialize(wmBuf, p_mbi_param->callback_ptr, WM_DMA_NO);
        }
        while (errcode == WM_ERRCODE_WM_DISABLE);
        if (errcode != WM_ERRCODE_OPERATING)
        {
            OS_TWarning("WM_Initialize failed!\n");
            return MB_ERRCODE_WM_FAILURE;
        }
        else
        {
            (void)WM_SetIndCallback(p_mbi_param->callback_ptr);
            p_mbi_param->mbIsStarted = 1;
            return MB_ERRCODE_SUCCESS;
        }
    }
    else
    {
        (void)WM_SetIndCallback(p_mbi_param->callback_ptr);
        p_mbi_param->mbIsStarted = 1;
        MBi_OnInitializeDone();
        return MB_ERRCODE_SUCCESS;
    }
}


/* Shared processing for setting parent parameters and starting. */
static int MBi_StartParentCore(int channel)
{
    int     i, ret;
    MBCommPStateCallback cb_tmp;
    OSIntrMode enabled;

    enabled = OS_DisableInterrupts();

    p_mbi_param->parentParam.channel = (u16)channel;
    wmBuf = (u8 *)((((u32)mbc) + sizeof(MB_CommPWork) + 31) & ~31);

    /* Save temporarily for times when the callback is already set. */
    cb_tmp = pPwork->parent_callback;

    /* Clear the parent's unique work region */
    MI_CpuClear16((void *)((u32)pPwork + sizeof(MB_CommCommonWork)),
                  sizeof(MB_CommPWork) - sizeof(MB_CommCommonWork));
    MB_CommSetParentStateCallback(cb_tmp);

    /* Calculate parameters that depend on the send/receive size. */
    mbc->block_size_max = MB_COMM_CALC_BLOCK_SIZE(p_mbi_param->p_sendlen);

    MBi_SetChildMPMaxSize(p_mbi_param->p_recvlen);
    MBi_SetParentPieceBuffer(&pPwork->req_data_buf);

    for (i = 0; i < MB_MAX_CHILD; i++)
    {
        pPwork->p_comm_state[i] = MB_COMM_PSTATE_NONE;
        pPwork->fileid_of_child[i] = -1;        /* Initialize the requested FileID from the child. */
    }
    pPwork->file_num = 0;

    MI_CpuClear16(&pPwork->fileinfo[0], sizeof(pPwork->fileinfo));
    MI_CpuClear8(&pPwork->req2child[0], sizeof(pPwork->req2child));

    p_mbi_param->mode = MB_MODE_PARENT;
    p_mbi_param->callback = MBi_CommParentCallback;
    p_mbi_param->callback_ptr = MBi_ParentCallback;

    /* Parent maximum transmission size (Bytes). */
    p_mbi_param->parentParam.parentMaxSize = p_mbi_param->p_sendlen;
    p_mbi_param->sendBufSize = WM_CalcParentSendBufSize(p_mbi_param);
    /* Child maximum transmission size (Bytes). */
    p_mbi_param->parentParam.childMaxSize = p_mbi_param->p_recvlen;
    p_mbi_param->recvBufSize = WM_CalcParentRecvBufSize(p_mbi_param);

    OS_TPrintf("Parent sendSize : %4d\n", p_mbi_param->parentParam.parentMaxSize);
    OS_TPrintf("Parent recvSize : %4d\n", p_mbi_param->parentParam.childMaxSize);
    OS_TPrintf("Parent sendBufSize : %4d\n", p_mbi_param->sendBufSize);
    OS_TPrintf("Parent recvBufSize : %4d\n", p_mbi_param->recvBufSize);

    MB_InitSendGameInfoStatus();

    ret = MBi_StartCommon();

    (void)OS_RestoreInterrupts(enabled);

    pPwork->useWvrFlag = PXI_IsCallbackReady(PXI_FIFO_TAG_WVR, PXI_PROC_ARM7);

    return ret;
}

/* Set parent parameters & start (Initialized internally in WM) */
int MB_StartParent(int channel)
{
    mbc->is_started_ex = FALSE;
    return MBi_StartParentCore(channel);
}

/* Set parent parameters & start (Already started external to WM) */
int MB_StartParentFromIdle(int channel)
{
    mbc->is_started_ex = TRUE;
    return MBi_StartParentCore(channel);
}

/* Set child parameters and start. */
int MB_StartChild(void)
{
    int     ret;
    MBCommCStateCallbackFunc cb_tmp;
    OSIntrMode enabled;

    enabled = OS_DisableInterrupts();

    mbc->is_started_ex = FALSE;

    wmBuf = (u8 *)((((u32)mbc) + sizeof(MB_CommCWork) + 31) & ~31);

    /* Save temporarily for times when the callback is already set. */
    cb_tmp = pCwork->child_callback;

    /* Clear the child's unique work region */
    MI_CpuClear16((void *)((u32)pCwork + sizeof(MB_CommCommonWork)),
                  sizeof(MB_CommCWork) - sizeof(MB_CommCommonWork));
    MB_CommSetChildStateCallback(cb_tmp);

    pCwork->c_comm_state = MB_COMM_CSTATE_NONE;

    p_mbi_param->mode = MB_MODE_CHILD;
    p_mbi_param->callback = MBi_CommChildCallback;
    p_mbi_param->callback_ptr = MBi_ChildCallback;

    p_mbi_param->scanning_flag = FALSE;
    p_mbi_param->scan_channel_flag = TRUE;
    p_mbi_param->last_found_parent_no = -1;

    MBi_SetBeaconRecvStatusBufferDefault();
    MBi_SetScanLockFunc(MBi_ScanLock, MBi_ScanUnlock);
    MB_InitRecvGameInfoStatus();

    ret = MBi_StartCommon();

    (void)OS_RestoreInterrupts(enabled);

    return ret;
}


/* Process for ending wireless communications if the task thread has ended or if it never started. */
static int MBi_CallReset(void)
{
    int     errcode;
    errcode = WM_Reset(p_mbi_param->callback_ptr);
    MBi_CheckWmErrcode(WM_APIID_RESET, errcode);
    return conv_errcode(errcode);
}

static void MBi_OnReset(MBiTaskInfo * p_task)
{
    (void)p_task;
    (void)MBi_CallReset();
}

/* End communications */
int MBi_CommEnd(void)
{
    int     ret = WM_ERRCODE_FAILED;
    OSIntrMode enabled = OS_DisableInterrupts();

    /* If it hasn't been started yet, end immediately here */
    if (!p_mbi_param->mbIsStarted)
    {
        MBi_EndCommon(NULL);
    }
    else if (p_mbi_param->endReq == 0)
    {
        p_mbi_param->scanning_flag = FALSE;
        p_mbi_param->endReq = 1;
        /* If the task thread is running, reset after stopping this. */
        if (MBi_IsTaskAvailable())
        {
            MBi_EndTaskThread(MBi_OnReset);
            ret = WM_ERRCODE_SUCCESS;
        }
        /* If not, reset here. */
        else
        {
            ret = MBi_CallReset();
        }
    }

    (void)OS_RestoreInterrupts(enabled);

    return ret;
}

void MB_End(void)
{
    OSIntrMode enabled = OS_DisableInterrupts();

    if (mbc->is_started_ex)
    {
        OS_TPanic("MB_End called after MB_StartParentFromIdle! (please call MB_EndToIdle)");
    }

    (void)MBi_CommEnd();

    (void)OS_RestoreInterrupts(enabled);
}

void MB_EndToIdle(void)
{
    OSIntrMode enabled = OS_DisableInterrupts();

    if (!mbc->is_started_ex)
    {
        OS_TPanic("MB_EndToIdle called after MB_StartParent! (please call MB_End)");
    }

    (void)MBi_CommEnd();

    (void)OS_RestoreInterrupts(enabled);
}

//-------------------------------------
// Disconnect child
//-------------------------------------
void MB_DisconnectChild(u16 aid)
{
    SDK_NULL_ASSERT(pPwork);
    SDK_ASSERT(p_mbi_param->endReq != 1);

    if (WM_Disconnect(MBi_ParentCallback, aid) != WM_ERRCODE_OPERATING)
    {
        OS_TWarning("MB_DisconnectChild failed disconnect child %d\n", aid);
    }

    if (aid == 0 || aid >= 16)
    {
        OS_TWarning("Disconnected Illegal AID No. ---> %2d\n", aid);
        return;
    }

    /* Delete child information.
       (If disconnected, delete child information relating to that AID) */
    pPwork->childversion[aid - 1] = 0;
    MI_CpuClear8(&pPwork->childggid[aid - 1], sizeof(u32));
    MI_CpuClear8(&pPwork->childUser[aid - 1], sizeof(MBUserInfo));

    /* Clear the receive buffer. */
    MBi_ClearParentPieceBuffer(aid);

    pPwork->req2child[aid - 1] = MB_COMM_USER_REQ_NONE;

    /* When the requested FileID has been set. Clear the ID to -1. */
    if (pPwork->fileid_of_child[aid - 1] != -1)
    {
        u8      fileID = (u8)pPwork->fileid_of_child[aid - 1];
        u16     nowChildFlag = pPwork->fileinfo[fileID].gameinfo_child_bmp;
        u16     child = aid;

        pPwork->fileinfo[fileID].gameinfo_child_bmp &= ~(MB_GAMEINFO_CHILD_FLAG(child));
        pPwork->fileinfo[fileID].gameinfo_changed_bmp |= MB_GAMEINFO_CHILD_FLAG(child);
        pPwork->fileid_of_child[child - 1] = -1;
        pPwork->fileinfo[fileID].pollbmp &= ~(0x0001 << child);

        MB_DEBUG_OUTPUT("Update Member (AID:%2d)\n", child);
    }

    /* Clear the connection information. */
    if (pPwork->child_entry_bmp & (0x0001 << aid))
    {
        pPwork->child_num--;
        pPwork->child_entry_bmp &= ~(0x0001 << aid);
    }
    pPwork->p_comm_state[aid - 1] = MB_COMM_PSTATE_NONE;
}


/* Used only in mb_child.c ***********************************************/

int MBi_GetLastFountParent(void)
{
    return p_mbi_param->last_found_parent_no;
}

WMBssDesc *MBi_GetParentBssDesc(int id)
{
    return p_mbi_param->parent_info[id].parentInfo;
}

static void MBi_ScanLock(u8 *macAddr)
{
    mbiScanParam.maxChannelTime = MB_SCAN_TIME_LOCKING;
    p_mbi_param->scan_channel_flag = FALSE;
    WM_CopyBssid(macAddr, mbiScanParam.bssid);
}

static void MBi_ScanUnlock(void)
{
    static const u8 bss_fill[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

    mbiScanParam.maxChannelTime = MB_SCAN_TIME_NORMAL;
    p_mbi_param->scan_channel_flag = TRUE;
    WM_CopyBssid(bss_fill, mbiScanParam.bssid);
}

/* Used only in mb_comm *************************************************/

// Settings at scan time
void MBi_SetMaxScanTime(u16 time)
{
    mbiScanParam.maxChannelTime = time;
}

static int
MBi_SetMPData(WMCallbackFunc callback, const u16 *sendData, u16 sendDataSize, u16 tmptt,
              u16 pollbmp)
{
#pragma unused( tmptt )
    int     errcode;
    errcode =
        WM_SetMPDataToPort(callback, sendData, sendDataSize, pollbmp, WM_PORT_BT, WM_PRIORITY_LOW);
    MBi_CheckWmErrcode(WM_APIID_SET_MP_DATA, errcode);
    return errcode;
}


int MBi_SendMP(const void *buf, int len, int pollbmp)
{
    int     errcode;

    u16     size = (u16)len;
    u16     pollbitmap = (u16)pollbmp;

    // MP data cannot be sent before MP is started as well as after a request to end MP has been issued.
    if ((p_mbi_param->mpStarted == 0) || (p_mbi_param->endReq == 1))
    {
        return WM_ERRCODE_FAILED;
    }

    switch (p_mbi_param->mode)
    {
    case MB_MODE_PARENT:
        errcode = MBi_SetMPData(p_mbi_param->callback_ptr, (u16 *)buf, size,    // * If MBi_ParentCallback is used, ParentCallback will be linked in mb_child as well, so callback_ptr should be used instead.
                                (u16)((!p_mbi_param->contSend) ? 1000 : 0), pollbitmap);

        if (errcode == WM_ERRCODE_OPERATING)
        {
            p_mbi_param->mpBusy = 1;
        }

        return conv_errcode(errcode);

    case MB_MODE_CHILD:
        errcode = MBi_SetMPData(MBi_ChildCallback, (u16 *)buf, size, 0, pollbitmap);

        if (errcode == WM_ERRCODE_OPERATING)
        {
            p_mbi_param->mpBusy = 1;
        }

        return conv_errcode(errcode);

    default:
        return WM_ERRCODE_FAILED;
    }
}

int MBi_GetSendBufSize(void)
{
    return (int)p_mbi_param->sendBufSize;
}

int MBi_GetRecvBufSize(void)
{
    return (int)p_mbi_param->recvBufSize;
}

int MBi_CommConnectToParent(const WMBssDesc *bssDescp)
{
    WMgameInfo *gameInfo;
    int     errcode;
    SDK_ASSERT(bssDescp != 0);

    gameInfo = (WMgameInfo *)(&(bssDescp->gameInfo));
    p_mbi_param->p_sendlen = gameInfo->parentMaxSize;
    p_mbi_param->p_recvlen = gameInfo->childMaxSize;
    /* Calculate parameters that depend on the send/receive size. */
    mbc->block_size_max = MB_COMM_CALC_BLOCK_SIZE(p_mbi_param->p_sendlen);

    MBi_SetChildMPMaxSize(p_mbi_param->p_recvlen);

    OS_TPrintf("\trecv size : %d\n", p_mbi_param->p_sendlen);
    OS_TPrintf("\tsend size : %d\n", p_mbi_param->p_recvlen);
    OS_TPrintf("\tblock size: %d\n", mbc->block_size_max);

    p_mbi_param->recvBufSize = (u16)WM_CalcChildRecvBufSize(bssDescp);
    p_mbi_param->sendBufSize = (u16)WM_CalcChildSendBufSize(bssDescp);
    p_mbi_param->pInfo = bssDescp;
    p_mbi_param->currentTgid = ((WMGameInfo *)&(bssDescp->gameInfo))->tgid;
    p_mbi_param->scanning_flag = FALSE;

    errcode = WM_EndScan(p_mbi_param->callback_ptr);
    MBi_CheckWmErrcode(WM_APIID_END_SCAN, errcode);
    return conv_errcode(errcode);
}

u32 MBi_GetGgid(void)
{
    return p_mbi_param->parentParam.ggid;
}

u16 MBi_GetTgid(void)
{
    return (p_mbi_param->parentParam.tgid);
}

u8 MBi_GetAttribute(void)
{
    return ((u8)(((p_mbi_param->parentParam.entryFlag) ? WM_ATTR_FLAG_ENTRY : 0) |      // entryFlag lowest bit
                 ((p_mbi_param->parentParam.multiBootFlag) ? WM_ATTR_FLAG_MB : 0) |     // multiBootFlag second bit
                 ((p_mbi_param->parentParam.KS_Flag) ? WM_ATTR_FLAG_KS : 0) |   // KS_Flag third bit
                 ((p_mbi_param->parentParam.CS_Flag) ? WM_ATTR_FLAG_CS : 0)     // CS_Flag fourth bit
            ));
}


// Restart scanning
int MBi_RestartScan(void)
{
    int     errcode;

    p_mbi_param->scanning_flag = TRUE;

    if (p_mbi_param->scan_channel_flag)
    {
        if (FALSE == changeScanChannel(&mbiScanParam))
        {
            (void)MBi_CommEnd();
        }
    }

    errcode = WM_StartScanEx(MBi_ChildCallback, &mbiScanParam);
    MBi_CheckWmErrcode(WM_APIID_START_SCAN_EX, errcode);
    return conv_errcode(errcode);
}

void MB_SetRejectGgid(u32 ggid, u32 mask, BOOL enable)
{
    isEnableReject    = enable;
    rejectGgidMask    = mask;
    rejectGgid        = ggid;
}

// Get scan channel
int MBi_GetScanChannel(void)
{
    return mbiScanParam.channelList;
}

// Get one's own AID.
u16 MBi_GetAid(void)
{
    return p_mbi_param->my_aid;
}

BOOL MBi_IsStarted(void)
{
    return (p_mbi_param->mbIsStarted == 1) ? TRUE : FALSE;
}

// Check WM_API's returned value
static void MBi_CheckWmErrcode(u16 apiid, int errcode)
{
    u16     arg[2];

    if (errcode != WM_ERRCODE_OPERATING && errcode != WM_ERRCODE_SUCCESS)
    {
        arg[0] = apiid;
        arg[1] = (u16)errcode;
        p_mbi_param->callback(MB_CALLBACK_API_ERROR, arg);
    }
}
