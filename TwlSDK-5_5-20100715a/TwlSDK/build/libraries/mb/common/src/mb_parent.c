/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - libraries
  File:     mb_parent.c

  Copyright 2007-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-11#$
  $Rev: 10742 $
  $Author: yosizaki $
 *---------------------------------------------------------------------------*/

#include "mb_private.h"

// ----------------------------------------------------------------------------
// Definitions

#define MB_SEND_THRESHOLD                       2       // Send up to 2 blocks ahead during a send

// ----------------------------------------------------------------------------
// Variables

/* Bit set indicating the file ID for which a transfer request was received from the child device in the latest MP */
static int any_recv_bitmap = 0;

// ----------------------------------------------------------------------------
// Static Function Prototypes

// --- For parent
static void MBi_CommChangeParentState(u16 child, int state, void *arg);
static void MBi_CommChangeParentStateCallbackOnly(u16 child, int state, void *arg);
static void MBi_CommParentRecvDataPerChild(void *arg, u16 child);
static void MBi_CommParentRecvData(void *arg);
static int MBi_CommParentSendMsg(u8 type, u16 pollbmp);
static int MBi_CommParentSendDLFileInfo(void);
static int MBi_CommParentSendBlock(void);
static int MBi_CommParentSendData(void);
static void MBi_calc_sendblock(u8 file_id);
static u16 MBi_calc_nextsendblock(u16 next_block, u16 next_block_req);

// --- Miscellany
static inline u16 max(u16 a, u16 b);
static BOOL IsChildAidValid(u16 child_aid);
static void MBi_CommCallParentError(u16 aid, u16 errcode);

/* ============================================================================

    Parent Functions

    ============================================================================*/

/*---------------------------------------------------------------------------*
  Name:         MB_CommSetParentStateCallback

  Description:  Sets the parent event callback.

  Arguments:    callback: Callback function to specify

  Returns:      None.
 *---------------------------------------------------------------------------*/

void MB_CommSetParentStateCallback(MBCommPStateCallback callback)
{
    OSIntrMode enabled;

    SDK_ASSERT(pPwork != 0);

    enabled = OS_DisableInterrupts();  /* Interrupts prohibited */

    pPwork->parent_callback = callback;

    (void)OS_RestoreInterrupts(enabled);        /* Remove interrupt prohibition */
}

/*---------------------------------------------------------------------------*
  Name:         MB_CommGetParentState

  Description:  Gets the parent device state for each child device.

  Arguments:    child_aid (handled between 1 and 15 to match the WM AID expression format)

  Returns:      
 *---------------------------------------------------------------------------*/

int MB_CommGetParentState(u16 child_aid)
{

    if (pPwork && IsChildAidValid(child_aid))
    {
        return pPwork->p_comm_state[child_aid - 1];
    }
    return 0;
}


/*---------------------------------------------------------------------------*
  Name:         MB_CommGetChildUser

  Description:  Gets the user information for a connected child device.

  Arguments:    child (handled between 1 and 15 to match the WM AID expression format)

  Returns:      MbUser structure.
 *---------------------------------------------------------------------------*/

const MBUserInfo *MB_CommGetChildUser(u16 child_aid)
{
    OSIntrMode enabled = OS_DisableInterrupts();        /* Interrupts prohibited */

    if (pPwork && IsChildAidValid(child_aid))
    {
        MI_CpuCopy8(&pPwork->childUser[child_aid - 1], &pPwork->childUserBuf, sizeof(MBUserInfo));
        (void)OS_RestoreInterrupts(enabled);    /* Remove interrupt prohibition */
        return &pPwork->childUserBuf;
    }
    (void)OS_RestoreInterrupts(enabled);        /* Remove interrupt prohibition */
    return NULL;
}

/*---------------------------------------------------------------------------*
  Name:         MB_CommGetChildrenNumber

  Description:  Gets the number of connected children.

  Arguments:    

  Returns:      Number of children.
 *---------------------------------------------------------------------------*/

u8 MB_CommGetChildrenNumber(void)
{
    if (pPwork)
    {
        return pPwork->child_num;
    }
    return 0;
}

/*---------------------------------------------------------------------------*
  Name:         MB_GetGameEntryBitmap

  Description:  Gets the AID bitmap entered for the GameRegistry being distributed.
                

  Arguments:    game_req: Pointer to the target GameRegistry

  Returns:      Bitmap of AIDs entered in the specified GameRegistry
                (Parent AID: 0, Child AID: 1-15)
                If a game is not in distribution, the return value is 0.
                (If a game is being distributed, parent AID:0 is always included in the entry members.)
                  
 *---------------------------------------------------------------------------*/

u16 MB_GetGameEntryBitmap(const MBGameRegistry *game_reg)
{
    int     i;
    for (i = 0; i < MB_MAX_FILE; i++)
    {
        if ((pPwork->fileinfo[i].active) && ((u32)pPwork->fileinfo[i].game_reg == (u32)game_reg))
        {
            return pPwork->fileinfo[i].gameinfo_child_bmp;
        }
    }
    return 0;
}


/*---------------------------------------------------------------------------*
  Name:         MB_CommIsBootable

  Description:  Determines whether a boot is possible.

  Arguments:    child_aid: AID of target child

  Returns:      Yes: TRUE; no: FALSE.
 *---------------------------------------------------------------------------*/

BOOL MB_CommIsBootable(u16 child_aid)
{
    SDK_ASSERT(pPwork != NULL);

    if (pPwork && IsChildAidValid(child_aid))
    {
        /* Return TRUE for a child that has completed transmission */
        if (pPwork->p_comm_state[child_aid - 1] == MB_COMM_PSTATE_SEND_COMPLETE)
        {
            return TRUE;
        }
    }
    return FALSE;
}


/*---------------------------------------------------------------------------*
  Name:         MB_CommResponseRequest

  Description:  Specifies a response to a connect request from a child.

  Arguments:    None.

  Returns:      Success = TRUE, failure = FALSE.
 *---------------------------------------------------------------------------*/

BOOL MB_CommResponseRequest(u16 child_aid, MBCommResponseRequestType ack)
{
    u16     req;
    int     state;
    OSIntrMode enabled;

    SDK_ASSERT(pPwork != NULL);

    enabled = OS_DisableInterrupts();  /* Interrupts prohibited */

    switch (ack)
    {
    case MB_COMM_RESPONSE_REQUEST_KICK:
        state = MB_COMM_PSTATE_REQUESTED;
        req = MB_COMM_USER_REQ_KICK;
        break;
    case MB_COMM_RESPONSE_REQUEST_ACCEPT:
        state = MB_COMM_PSTATE_REQUESTED;
        req = MB_COMM_USER_REQ_ACCEPT;
        break;

    case MB_COMM_RESPONSE_REQUEST_DOWNLOAD:
        state = MB_COMM_PSTATE_WAIT_TO_SEND;
        req = MB_COMM_USER_REQ_SEND_START;
        break;

    case MB_COMM_RESPONSE_REQUEST_BOOT:
        state = MB_COMM_PSTATE_SEND_COMPLETE;
        req = MB_COMM_USER_REQ_BOOT;
        break;
    default:
        (void)OS_RestoreInterrupts(enabled);    /* Remove interrupt prohibition */
        return FALSE;
    }

    if (pPwork && IsChildAidValid(child_aid))
    {
        if (pPwork->p_comm_state[child_aid - 1] == state)
        {
            pPwork->req2child[child_aid - 1] = req;
            (void)OS_RestoreInterrupts(enabled);        /* Remove interrupt prohibition */
            return TRUE;
        }
    }

    (void)OS_RestoreInterrupts(enabled);        /* Remove interrupt prohibition */

    return FALSE;
}


/*---------------------------------------------------------------------------*
  Name:         MBi_CommChangeParentState

  Description:  Changes the parent device state.

  Arguments:    child (handled between 1 and 15 to match the WM AID expression format; 0 indicates the parent device)
                state

  Returns:      
 *---------------------------------------------------------------------------*/

static void MBi_CommChangeParentState(u16 child, int state, void *arg)
{
    SDK_ASSERT(pPwork && child >= 0 && child <= WM_NUM_MAX_CHILD);

    /* If the child's number is specified, change its state */
    if (IsChildAidValid(child))
    {
        pPwork->p_comm_state[child - 1] = state;
    }

    MBi_CommChangeParentStateCallbackOnly(child, state, arg);

}

/*---------------------------------------------------------------------------*
  Name:         MBi_CommChangeParentStateCallbackOnly

  Description:  Perform child state notification with only a callback invocation.
                The internal state is not changed.

  Arguments:    

  Returns:      
 *---------------------------------------------------------------------------*/

static void MBi_CommChangeParentStateCallbackOnly(u16 child, int state, void *arg)
{
    if (pPwork->parent_callback)       // State-change callback
    {
        (*pPwork->parent_callback) (child, (u32)state, arg);
    }
}

/*---------------------------------------------------------------------------*
  Name:         MBi_CommParentCallback

  Description:  Main body of the parent callback.

  Arguments:    type: WM_TYPE event arg:callback argument

  Returns:      None.
 *---------------------------------------------------------------------------*/

void MBi_CommParentCallback(u16 type, void *arg)
{
    MB_COMM_WMEVENT_OUTPUT(type, arg);

    switch (type)
    {
    case MB_CALLBACK_INIT_COMPLETE:
        /* Initialization complete */
        /* Pass an arg of type WMStartParentCallback to the callback's arguments */
        MBi_CommChangeParentState(0, MB_COMM_PSTATE_INIT_COMPLETE, arg);
        break;

    case MB_CALLBACK_END_COMPLETE:
        /* End complete */
        // Execute at the end of the function

        break;

    case MB_CALLBACK_CHILD_CONNECTED:
        {
            WMstartParentCallback *sparg = (WMstartParentCallback *)arg;

            /* Ignore if the AID is 0 (parent device) */
            if (sparg->aid == 0)
                break;

            if (sparg->aid >= 16)
            {
                OS_TWarning("Connected Illegal AID No. ---> %2d\n", sparg->aid);
                break;
            }

            MB_DEBUG_OUTPUT("child %d connected to get bootimage!\n", sparg->aid);

            /* Pass an arg of type WMStartParentCallback to the callback's arguments */
            MBi_CommChangeParentState(sparg->aid, MB_COMM_PSTATE_CONNECTED, arg);
        }
        break;

    case MB_CALLBACK_CHILD_DISCONNECTED:
        {
            WMstartParentCallback *sparg = (WMstartParentCallback *)arg;

            /* Ignore if the AID is 0 (parent device) */
            if (sparg->aid == 0)
                break;

            if (sparg->aid >= 16)
            {
                OS_TWarning("Disconnected Illegal AID No. ---> %2d\n", sparg->aid);
                break;
            }

            MB_DEBUG_OUTPUT("child %d disconnected \n", sparg->aid);

            /* Delete child information.
               (If disconnected, delete child information relating to that AID) */
            pPwork->childversion[sparg->aid - 1] = 0;
            MI_CpuClear8(&pPwork->childggid[sparg->aid - 1], sizeof(u32));
            MI_CpuClear8(&pPwork->childUser[sparg->aid - 1], sizeof(MBUserInfo));

            /* Clear the receive buffer */
            MBi_ClearParentPieceBuffer(sparg->aid);

            pPwork->req2child[sparg->aid - 1] = MB_COMM_USER_REQ_NONE;

            /* When the requested FileID has been set. Clear the ID to -1 */
            if (pPwork->fileid_of_child[sparg->aid - 1] != -1)
            {
                u8      fileID = (u8)pPwork->fileid_of_child[sparg->aid - 1];
                u16     nowChildFlag = pPwork->fileinfo[fileID].gameinfo_child_bmp;
                u16     child = sparg->aid;

                pPwork->fileinfo[fileID].gameinfo_child_bmp &= ~(MB_GAMEINFO_CHILD_FLAG(child));
                pPwork->fileinfo[fileID].gameinfo_changed_bmp |= MB_GAMEINFO_CHILD_FLAG(child);
                pPwork->fileid_of_child[child - 1] = -1;
                pPwork->fileinfo[fileID].pollbmp &= ~(0x0001 << (child));

                MB_DEBUG_OUTPUT("Update Member (AID:%2d)\n", child);
            }

            /* Clear the connection information */
            if (pPwork->child_entry_bmp & (0x0001 << (sparg->aid)))
            {
                pPwork->child_num--;
                pPwork->child_entry_bmp &= ~(0x0001 << (sparg->aid));
            }

            /* If disconnected at MB_COMM_PSTATE_BOOT_REQUEST, it is assumed that communication ended because a boot was in progress. A notification of the MB_COMM_PSTATE_BOOT_STARTABLE status is sent
                */
            if (pPwork->p_comm_state[sparg->aid - 1] == MB_COMM_PSTATE_BOOT_REQUEST)
            {
                MBi_CommChangeParentState(sparg->aid, MB_COMM_PSTATE_BOOT_STARTABLE, NULL);
            }

            /* Return an arg of type WMStartParentCallback to the callback's arguments */
            MBi_CommChangeParentState(sparg->aid, MB_COMM_PSTATE_DISCONNECTED, arg);
            pPwork->p_comm_state[sparg->aid - 1] = MB_COMM_PSTATE_NONE;
        }
        break;

    case MB_CALLBACK_MP_PARENT_RECV:
        MBi_CommParentRecvData(arg);
        break;

    case MB_CALLBACK_MP_SEND_ENABLE:
        (void)MBi_CommParentSendData();
        break;

    case MB_CALLBACK_BEACON_SENT:
        {
            u8      i;
            /* For each game, update the GameInfo member information */
            for (i = 0; i < MB_MAX_FILE; i++)
            {
                /* When the registered file is active, and the change flags of the game's entry members are set, update the GameInfo member information
                   
                    */
                if (pPwork->fileinfo[i].active && pPwork->fileinfo[i].gameinfo_changed_bmp)
                {
                    MB_UpdateGameInfoMember(&pPwork->fileinfo[i].game_info,
                                            &pPwork->childUser[0],
                                            pPwork->fileinfo[i].gameinfo_child_bmp,
                                            pPwork->fileinfo[i].gameinfo_changed_bmp);
                    /* After the update, clear the part of the bmp that changed */
                    pPwork->fileinfo[i].gameinfo_changed_bmp = 0;
                }
            }
        }
        /* Place GameInfo in a beacon and send */
        MB_SendGameInfoBeacon(MBi_GetGgid(), MBi_GetTgid(), MBi_GetAttribute());
        break;

    case MB_CALLBACK_API_ERROR:
        /* Error values returned when WM API was called in ARM9 */
        {
            u16     apiid, errcode;

            apiid = ((u16 *)arg)[0];
            errcode = ((u16 *)arg)[1];

            switch (errcode)
            {
            case WM_ERRCODE_INVALID_PARAM:
            case WM_ERRCODE_FAILED:
            case WM_ERRCODE_WM_DISABLE:
            case WM_ERRCODE_NO_DATASET:
            case WM_ERRCODE_FIFO_ERROR:
            case WM_ERRCODE_TIMEOUT:
                MBi_CommCallParentError(0, MB_ERRCODE_FATAL);
                break;
            case WM_ERRCODE_OPERATING:
            case WM_ERRCODE_ILLEGAL_STATE:
            case WM_ERRCODE_NO_CHILD:
            case WM_ERRCODE_OVER_MAX_ENTRY:
            case WM_ERRCODE_NO_ENTRY:
            case WM_ERRCODE_INVALID_POLLBITMAP:
            case WM_ERRCODE_NO_DATA:
            case WM_ERRCODE_SEND_QUEUE_FULL:
            case WM_ERRCODE_SEND_FAILED:
            default:
                MBi_CommCallParentError(0, MB_ERRCODE_WM_FAILURE);
                break;
            }
        }
        break;
    case MB_CALLBACK_ERROR:
        {
            /* Errors in callbacks returned after a WM API call */
            WMCallback *pWmcb = (WMCallback *)arg;
            switch (pWmcb->apiid)
            {
            case WM_APIID_INITIALIZE:
            case WM_APIID_SET_LIFETIME:
            case WM_APIID_SET_P_PARAM:
            case WM_APIID_SET_BEACON_IND:
            case WM_APIID_START_PARENT:
            case WM_APIID_START_MP:
            case WM_APIID_SET_MP_DATA:
            case WM_APIID_START_DCF:
            case WM_APIID_SET_DCF_DATA:
            case WM_APIID_DISCONNECT:
            case WM_APIID_START_KS:
                /* The above errors are important to WM initialization */
                MBi_CommCallParentError(0, MB_ERRCODE_FATAL);
                break;
            case WM_APIID_RESET:
            case WM_APIID_END:
            default:
                /* Other errors are returned as callback errors */
                MBi_CommCallParentError(0, MB_ERRCODE_WM_FAILURE);
                break;
            }
        }
        break;

    }


#if ( CALLBACK_WM_STATE == 1 )
    MBi_CommChangeParentState(0, (u32)(MB_COMM_PSTATE_WM_EVENT | type), arg);
#endif

    if (type == MB_CALLBACK_END_COMPLETE)
    {
        MBCommPStateCallback tmpCb = pPwork->parent_callback;

        /* Deallocate working memory */
        MI_CpuClearFast(pPwork, sizeof(MB_CommPWork));
        pPwork = NULL;
        if (tmpCb)                     // State-change callback
        {
            (*tmpCb) (0, MB_COMM_PSTATE_END, NULL);
        }
    }
}


/*---------------------------------------------------------------------------*
  Name:         MBi_CommParentRecvDataPerChild

  Description:  Processes the received data for each child.

  Arguments:    arg: pointer to callback argument, child: child AID

  Returns:      None.
 *---------------------------------------------------------------------------*/

static void MBi_CommParentRecvDataPerChild(void *arg, u16 child)
{
    MBCommChildBlockHeader hd;
    int     state;
    u8     *p_data;

    // The range of a child is from 1 to 15
    if (child == 0 || child > WM_NUM_MAX_CHILD)
    {
        return;
    }

    p_data = MBi_SetRecvBufferFromChild((u8 *)&((WMMpRecvData *)arg)->cdata[0], &hd, child);

    state = pPwork->p_comm_state[child - 1];

    MB_DEBUG_OUTPUT("RECV (CHILD:%2d)", child);
    MB_COMM_TYPE_OUTPUT(hd.type);

    switch (hd.type)                   // Processing for each data type
    {
    case MB_COMM_TYPE_CHILD_FILEREQ:

        if (p_data != NULL)
        {
            // Only accept when MB_COMM_PSTATE_CONNECTED
            if (state == MB_COMM_PSTATE_CONNECTED)
            {
                MBCommRequestData req_data;
                // Do not move to the next state until all of the child's request data is present
                MI_CpuCopy8(p_data, &req_data, MB_COMM_REQ_DATA_SIZE);
                pPwork->childggid[child - 1] = req_data.ggid;
                pPwork->childversion[child - 1] = req_data.version;
                MB_DEBUG_OUTPUT("Child [%2d] MB_IPL_VERSION : %04x\n", child, req_data.version);
                MI_CpuCopy8(&req_data.userinfo, &pPwork->childUser[child - 1], sizeof(MBUserInfo));
                pPwork->childUser[child - 1].playerNo = child;
                /* Pass the received MBCommRequestData to the callback arguments */
                MBi_CommChangeParentState(child, MB_COMM_PSTATE_REQUESTED, &req_data.userinfo);
            }

            if (state == MB_COMM_PSTATE_REQUESTED)
            {
                u8      i, entry_num = 0;
                u8      fileid = ((MBCommRequestData *)p_data)->fileid;

                /* Kick if an inactive fileNo is requested, the fileid is invalid, or the GGID of the fileNo does not match the requested GGID
                   
                   */
                if (fileid >= MB_MAX_FILE
                    || pPwork->fileinfo[fileid].active == 0
                    || pPwork->childggid[child - 1] != pPwork->fileinfo[fileid].game_reg->ggid)
                {
                    pPwork->req2child[child - 1] = MB_COMM_USER_REQ_KICK;
                }
                else
                {
                    /* Kick if the set number of people was exceeded */
                    for (i = 0; i < WM_NUM_MAX_CHILD + 1; i++)
                    {
                        if (pPwork->fileinfo[fileid].gameinfo_child_bmp & (0x0001 << i))
                        {
                            entry_num++;
                        }
                    }

                    if (entry_num >= pPwork->fileinfo[fileid].game_reg->maxPlayerNum)
                    {
                        MB_DEBUG_OUTPUT("Member full (AID:%2d)\n", child);
                        /* Forcibly cancel the request */
                        pPwork->req2child[child - 1] = MB_COMM_USER_REQ_NONE;
                        MBi_CommChangeParentState(child, MB_COMM_PSTATE_MEMBER_FULL, NULL);
                        break;
                    }
                }

                switch (pPwork->req2child[child - 1])
                {
                case MB_COMM_USER_REQ_ACCEPT:
                    {

                        if (0 == (pPwork->child_entry_bmp & (0x0001 << (child))))
                        {
                            pPwork->child_num++;
                            pPwork->child_entry_bmp |= (0x0001 << (child));
                            pPwork->fileid_of_child[child - 1] = (s8)fileid;

                            pPwork->fileinfo[fileid].gameinfo_child_bmp |=
                                MB_GAMEINFO_CHILD_FLAG(child);
                            pPwork->fileinfo[fileid].gameinfo_changed_bmp |=
                                MB_GAMEINFO_CHILD_FLAG(child);
                            MB_DEBUG_OUTPUT("Update Member (AID:%2d)\n", child);
                            pPwork->req2child[child - 1] = MB_COMM_USER_REQ_NONE;

                            MBi_CommChangeParentState(child, MB_COMM_PSTATE_REQ_ACCEPTED, NULL);
                        }
                    }
                    break;

                case MB_COMM_USER_REQ_KICK:
                    MB_DEBUG_OUTPUT("Kick (AID:%2d)\n", child);
                    pPwork->req2child[child - 1] = MB_COMM_USER_REQ_NONE;
                    MBi_CommChangeParentState(child, MB_COMM_PSTATE_KICKED, NULL);
                    break;
                }
            }
        }
        break;

    case MB_COMM_TYPE_CHILD_ACCEPT_FILEINFO:

        /* When MB_COMM_PSTATE_REQ_ACCEPTED, only transition to MB_COMM_PSTATE_WAIT_TO_SEND */
        if (state == MB_COMM_PSTATE_REQ_ACCEPTED)
        {
            MBi_CommChangeParentState(child, MB_COMM_PSTATE_WAIT_TO_SEND, NULL);
        }

        /* When MB_COMM_PSTATE_WAIT_TO_SEND, add to pollbitmap for the requested file and shift to MB_COMM_PSTATE_SEND_PROCEED
           
            */
        else if (state == MB_COMM_PSTATE_WAIT_TO_SEND)
        {
            // If the SendStart trigger was on, transition to the block send state
            if (pPwork->req2child[child - 1] == MB_COMM_USER_REQ_SEND_START)
            {
                u8      fid = (u8)pPwork->fileid_of_child[child - 1];
                pPwork->fileinfo[fid].pollbmp |= (0x0001 << (child));
                pPwork->fileinfo[fid].currentb = 0;

                pPwork->req2child[child - 1] = MB_COMM_USER_REQ_NONE;
                MBi_CommChangeParentState(child, MB_COMM_PSTATE_SEND_PROCEED, NULL);
            }
        }
        break;

    case MB_COMM_TYPE_CHILD_CONTINUE:
        if (state == MB_COMM_PSTATE_SEND_PROCEED)
        {
            u8      fileid = (u8)pPwork->fileid_of_child[child - 1];

            if (fileid == (u8)-1)
                break;

            // Of the nextSend's that come from multiple children, choose the largest one as the send target
            SDK_ASSERT(fileid < MB_MAX_FILE);
            SDK_ASSERT(pPwork->fileinfo[fileid].pollbmp);

            pPwork->fileinfo[fileid].nextb =
                MBi_calc_nextsendblock(pPwork->fileinfo[fileid].nextb, hd.data.req);
            any_recv_bitmap |= (1 << fileid);
        }
        break;

    case MB_COMM_TYPE_CHILD_STOPREQ:
        if (state == MB_COMM_PSTATE_SEND_PROCEED)
        {
            u8      fileid = (u8)pPwork->fileid_of_child[child - 1];

            if (fileid == (u8)-1)
                break;

            SDK_ASSERT(fileid < MB_MAX_FILE);

            pPwork->fileinfo[fileid].pollbmp &= ~(0x0001 << (child));

            MBi_CommChangeParentState(child, MB_COMM_PSTATE_SEND_COMPLETE, NULL);       // Send completed
        }
        else if (state == MB_COMM_PSTATE_SEND_COMPLETE)
        {
            if (pPwork->req2child[child - 1] == MB_COMM_USER_REQ_BOOT)
            {
                pPwork->req2child[child - 1] = MB_COMM_USER_REQ_NONE;
                MBi_CommChangeParentState(child, MB_COMM_PSTATE_BOOT_REQUEST, NULL);
                break;
            }
        }
        break;

    case MB_COMM_TYPE_CHILD_BOOTREQ_ACCEPTED:
        if (state == MB_COMM_PSTATE_BOOT_REQUEST)
        {
            /* BOOTREQ_ACCEPTED from a child is not used for state transitions */
            break;
        }

        break;

    default:
        break;
    }

}

/*---------------------------------------------------------------------------*
  Name:         MBi_CommParentRecvData

  Description:  Receives parent data.

  Arguments:    arg: Pointer to callback argument

  Returns:      None.
 *---------------------------------------------------------------------------*/

static void MBi_CommParentRecvData(void *arg)
{
    // The argument arg contains a pointer to the receive buffer
    WMmpRecvHeader *mpHeader = (WMmpRecvHeader *)arg;

    u16     i;
    WMmpRecvData *datap;

    // Initialize at this point in order to evaluate the total with MBi_CommParentRecvDataPerChild
    for (i = 0; i < MB_MAX_FILE; i++)
    {
        if (pPwork->fileinfo[i].active)
            pPwork->fileinfo[i].nextb = 0;
    }
    any_recv_bitmap = 0;

    // Display the data received from each child
    for (i = 1; i <= WM_NUM_MAX_CHILD; ++i)
    {
        // Get the starting address of the data for the child with AID==i
        datap = WM_ReadMPData(mpHeader, (u16)i);
        // If data exists for the child with AID==i
        if (datap != NULL)
        {
            // Display the received data
            if (datap->length == 0xffff)
            {
            }
            else if (datap->length != 0)
            {
                // Processing for each child's data
                MBi_CommParentRecvDataPerChild(datap, i);
            }
        }
    }
}


/*---------------------------------------------------------------------------*
  Name:         MBi_CommParentSendMsg

  Description:  Sends a message from the parent.

  Arguments:    pollbmp

  Returns:      None.
 *---------------------------------------------------------------------------*/

static int MBi_CommParentSendMsg(u8 type, u16 pollbmp)
{
    MBCommParentBlockHeader hd;

    /* Notify that transmission has begun (without arguments) */
    hd.type = type;

    (void)MBi_MakeParentSendBuffer(&hd, (u8 *)pPwork->common.sendbuf);
    return MBi_BlockHeaderEnd(MB_COMM_PARENT_HEADER_SIZE, pollbmp, pPwork->common.sendbuf);
}

/*---------------------------------------------------------------------------*
  Name:         MBi_CommParentSendDLFileInfo

  Description:  Sends DownloadFileInfo from the parent.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/

static int MBi_CommParentSendDLFileInfo(void)
{
    MBCommParentBlockHeader hd;
    u8     *databuf = ((u8 *)pPwork->common.sendbuf) + MB_COMM_PARENT_HEADER_SIZE;
    u16     child;
    u8      i, fid;
    s8      send_candidate_fid = -1;
    static s8 prev_fid = -1;
    u8      file_req_num[MB_MAX_FILE];
    u16     pollbmp = 0;

    MI_CpuClear8(&file_req_num[0], sizeof(u8) * MB_MAX_FILE);

    // Aggregate the FileID's that children are requesting
    for (child = 1; child <= WM_NUM_MAX_CHILD; child++)
    {
        if (pPwork->p_comm_state[child - 1] == MB_COMM_PSTATE_REQ_ACCEPTED)
        {
            // Count only children that are MB_COMM_PSTATE_REQ_ACCEPTED targets
            ++(file_req_num[pPwork->fileid_of_child[child - 1]]);
        }
    }

    fid = (u8)prev_fid;

    for (i = 0; i < MB_MAX_FILE; i++)  // Determine the send file ID
    {
#if 1
        fid = (u8)((fid + 1) % MB_MAX_FILE);

        if (pPwork->fileinfo[fid].active && file_req_num[fid] > 0)
        {
            send_candidate_fid = (s8)fid;
            break;
        }

#else
        if (pPwork->fileinfo[i].active)
        {
            if (file_req_num[i] > 0)
            {

                /* 
                   Majority decision
                   (In this case, when multiple child devices are waiting for different files to start downloading, the transmission of DownloadFileInfo becomes locked to the file with more devices.
                   
                   Although there are no problems with multiboot operations, because the state of the child application will no longer advance from MB_COMM_CSTATE_REQ_ENABLE, after a download decision from the parent, entry processing for those children will continue (should this be dealt with?)
                   
                   
                   
                 */

                if (send_candidate_fid == -1 || file_req_num[i] > file_req_num[send_candidate_fid])
                {
                    send_candidate_fid = i;
                }

            }
        }
#endif

    }

    if (send_candidate_fid == -1)
        return MB_SENDFUNC_STATE_ERR;

    prev_fid = send_candidate_fid;

    // Poll bitmap settings (only children that are requesting the same file number as the one to be sent)
    for (child = 1; child <= WM_NUM_MAX_CHILD; child++)
    {
        if (pPwork->p_comm_state[child - 1] == MB_COMM_PSTATE_REQ_ACCEPTED
            && pPwork->fileid_of_child[child - 1] == send_candidate_fid)
        {
            pollbmp |= (1 << child);
        }
    }

    MB_DEBUG_OUTPUT("DLinfo No %2d Pollbmp %04x\n", send_candidate_fid, pollbmp);

    // Send the FileInfo of the last child that received the request
    hd.type = MB_COMM_TYPE_PARENT_DL_FILEINFO;
    hd.fid = send_candidate_fid;

    databuf = MBi_MakeParentSendBuffer(&hd, (u8 *)pPwork->common.sendbuf);
    if (databuf)
    {
        // Copy data to the send buffer
        MI_CpuCopy8(&pPwork->fileinfo[send_candidate_fid].dl_fileinfo,
                    databuf, sizeof(MBDownloadFileInfo));
    }

    return MBi_BlockHeaderEnd(sizeof(MBDownloadFileInfo) + MB_COMM_PARENT_HEADER_SIZE, pollbmp,
                              pPwork->common.sendbuf);
}

/*---------------------------------------------------------------------------*
  Name:         MBi_ReloadCache

  Description:  Reload data in the destroyed cache that is specified.

  Arguments:    p_task: Task that stores MBiCacheInfo in param[0]

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MBi_ReloadCache(MBiTaskInfo * p_task)
{
    MBiCacheInfo *const p_info = (MBiCacheInfo *) p_task->param[0];
    MBiCacheList *const p_list = (MBiCacheList *) p_task->param[1];
    FSFile  file[1];
    FSArchive   *arc;
    u32          length = p_info->len;
    arc = FS_FindArchive(p_list->arc_name, (int)p_list->arc_name_len);
    if (!arc)
    {
        arc = p_list->arc_pointer;
    }
#ifdef SDK_TWL
    // When running in TWL mode, avoid reading outside the ROM
    if (OS_IsRunOnTwl() && (arc == FS_FindArchive("rom", 3)))
    {
        const CARDRomHeaderTWL *header = (const CARDRomHeaderTWL *)CARD_GetOwnRomHeaderTWL();
        // Note that in some cases the distributed program may be maintained as a TWL-exclusive file
        const u32   boundary_ntr = header->digest_area_ntr.offset + header->digest_area_ntr.length;
        const u32   boundary_ltd = header->digest_area_ltd.offset + header->digest_area_ltd.length;
        if ((p_info->src < boundary_ntr) && (p_info->src + length > boundary_ntr))
        {
            length = boundary_ntr - p_info->src;
        }
        else if ((p_info->src < boundary_ltd) && (p_info->src + length > boundary_ltd))
        {
            length = boundary_ltd - p_info->src;
        }
    }
#endif

    FS_InitFile(file);

    /* Specify the target archive for the MB_ReadSegment function */
    if (FS_OpenFileDirect(file,
                          arc,
                          p_info->src, p_info->src + length, (u32)~0))
    {
        if (FS_ReadFile(file, p_info->ptr, (int)length) == length)
        {
            /* It is acceptable to set to READY here */
            p_info->state = MB_CACHE_STATE_READY;
        }
        (void)FS_CloseFile(file);
    }

    /* File loading failed, due to causes such as card removal */
    if (p_info->state != MB_CACHE_STATE_READY)
    {
        /*
         * Leaving this page in MB_CACHE_STATE_BUSY will result in a leak, decreasing the page count of the entire cache. Therefore, always keep this page in MB_CACHE_STATE_READY, with the same address settings as the ROM header (0x00000000-) at index[0].
         * 
         * 
         * 
         * Even if indeterminate content is included in the buffer, this will ensure that the content will never be referenced, and will become the target of the next page fault.
         * 
         * (However, in actuality, this should not be called twice when the card is removed)
         */
        p_info->src = 0;
        p_info->state = MB_CACHE_STATE_READY;
    }
}

/*---------------------------------------------------------------------------*
  Name:         MBi_CommParentSendBlock

  Description:  Sends Block data from the parent.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/

static int MBi_CommParentSendBlock(void)
{
    MBCommParentBlockHeader hd;
    u8     *databuf;
    u8      i;
    MB_BlockInfo bi;

    // Determine the file number to send
    if (pPwork->file_num == 0)
        return MB_SENDFUNC_STATE_ERR;

    // The block transfer main body starts from here

    // Determine the file that is the send target
    for (i = 0; i < MB_MAX_FILE; i++)
    {
        pPwork->cur_fileid = (u8)((pPwork->cur_fileid + 1) % MB_MAX_FILE);
        if (pPwork->fileinfo[pPwork->cur_fileid].active
            && pPwork->fileinfo[pPwork->cur_fileid].pollbmp)
        {
            MB_DEBUG_OUTPUT("Send File ID:%2d\n", pPwork->cur_fileid);
            break;
        }
    }
    if (i == MB_MAX_FILE)              // No file to send
    {
        return MB_SENDFUNC_STATE_ERR;
    }

    /* Calculate the number of blocks to send for the file being sent */
    MBi_calc_sendblock(pPwork->cur_fileid);

    // Get the block information
    if (!MBi_get_blockinfo(&bi,
                           &pPwork->fileinfo[pPwork->cur_fileid].blockinfo_table,
                           pPwork->fileinfo[pPwork->cur_fileid].currentb,
                           &pPwork->fileinfo[pPwork->cur_fileid].dl_fileinfo.header))
    {
        return MB_SENDFUNC_STATE_ERR;
    }
    /* Prepare the packets to send */
    hd.type = MB_COMM_TYPE_PARENT_DATA;
    hd.fid = pPwork->cur_fileid;
    hd.seqno = pPwork->fileinfo[pPwork->cur_fileid].currentb;
    databuf = MBi_MakeParentSendBuffer(&hd, (u8 *)pPwork->common.sendbuf);

    /* Access though the cache (will always hit if there is enough memory) */
    {
        /* Calculate the CARD address from the block offset */
        u32     card_addr = (u32)(bi.offset -
                                  pPwork->fileinfo[pPwork->cur_fileid].blockinfo_table.
                                  seg_src_offset[bi.segment_no] +
                                  pPwork->fileinfo[pPwork->cur_fileid].card_mapping[bi.segment_no]);
        /* Cache read for the specified CARD address */
        MBiCacheList *const pl = pPwork->fileinfo[pPwork->cur_fileid].cache_list;
        if (!MBi_ReadFromCache(pl, card_addr, databuf, bi.size))
        {
            /* If a cache miss, send a reload request to the task thread */
            MBiTaskInfo *const p_task = &pPwork->cur_task;
            if (!MBi_IsTaskBusy(p_task))
            {
                /* Lifetime value to avoid continuous page faults */
                if (pl->lifetime)
                {
                    --pl->lifetime;
                }
                else
                {
                    /*
                     * Destroy the cache with the newest address.
                     * We will continue to test the operation of this part later.
                     */
                    MBiCacheInfo *pi = pl->list;
                    MBiCacheInfo *trg = NULL;
                    int     i;
                    for (i = 0; i < MB_CACHE_INFO_MAX; ++i)
                    {
                        if (pi[i].state == MB_CACHE_STATE_READY)
                        {
                            if (!trg || (trg->src > pi[i].src))
                                trg = &pi[i];
                        }
                    }
                    if (!trg)
                    {
                        OS_TPanic("cache-list is invalid! (all the pages are locked)");
                    }
                    pl->lifetime = 2;
                    trg->state = MB_CACHE_STATE_BUSY;
                    trg->src = (card_addr & ~31);
                    p_task->param[0] = (u32)trg;        /* MBiCacheInfo* */
                    p_task->param[1] = (u32)pl; /* MBiCacheList* */
                    MBi_SetTask(p_task, MBi_ReloadCache, NULL, 4);
                }
            }
            return MB_SENDFUNC_STATE_ERR;
        }
    }

    return MBi_BlockHeaderEnd((int)(bi.size + MB_COMM_PARENT_HEADER_SIZE),
                              pPwork->fileinfo[pPwork->cur_fileid].pollbmp, pPwork->common.sendbuf);
}

/*---------------------------------------------------------------------------*
  Name:         MBi_CommParentSendData

  Description:  Sends parent data.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/

static int MBi_CommParentSendData(void)
{
    struct bitmap
    {
        u16     connected;
        u16     req;
        u16     kick;
        u16     boot;
        u16     mem_full;
    };
    struct bitmap bmp;
    u16     child;
    int     errcode;

    MI_CpuClear16(&bmp, sizeof(struct bitmap));

    // Evaluate the parent status of each child
    for (child = 1; child <= WM_NUM_MAX_CHILD; child++)
    {

        switch (pPwork->p_comm_state[child - 1])
        {
        case MB_COMM_PSTATE_CONNECTED:
            bmp.connected |= (1 << child);
            break;

        case MB_COMM_PSTATE_REQ_ACCEPTED:
            bmp.req |= (1 << child);
            break;

        case MB_COMM_PSTATE_KICKED:
            bmp.kick |= (1 << child);
            break;

        case MB_COMM_PSTATE_SEND_PROCEED:
            break;

        case MB_COMM_PSTATE_BOOT_REQUEST:
            bmp.boot |= (1 << child);
            break;

        case MB_COMM_PSTATE_MEMBER_FULL:
            bmp.mem_full |= (1 << child);
            break;

        default:
            break;                     // Does not count states other than those above
        }

    }
    /*
       Send in the priority order of Startmsg > DLFileInfo > Block
       
     */
    if (bmp.boot)
    {
        errcode = MBi_CommParentSendMsg(MB_COMM_TYPE_PARENT_BOOTREQ, bmp.boot);
    }
    else if (bmp.connected)            // Send entry request permitted message
    {
        errcode = MBi_CommParentSendMsg(MB_COMM_TYPE_PARENT_SENDSTART, bmp.connected);
    }
    else if (bmp.mem_full)             // Send member exceeded message
    {
        errcode = MBi_CommParentSendMsg(MB_COMM_TYPE_PARENT_MEMBER_FULL, bmp.mem_full);
    }
    else if (bmp.kick)                 // Send entry denied message
    {
        errcode = MBi_CommParentSendMsg(MB_COMM_TYPE_PARENT_KICKREQ, bmp.kick);
    }
    else if (bmp.req)                  // Send MbDownloadFileInfo
    {
        errcode = MBi_CommParentSendDLFileInfo();
    }
    else                               // Send Block data
    {
        errcode = MBi_CommParentSendBlock();
    }

    // MP transmission for keeping Connection
    if (MB_SENDFUNC_STATE_ERR == errcode)
    {
        errcode = MBi_CommParentSendMsg(MB_COMM_TYPE_DUMMY, 0xffff);
    }

    return errcode;

}


/*---------------------------------------------------------------------------*
  Name:         MBi_calc_sendblock

  Description:  Calculates the blocks to be sent.

  Arguments:    file_id: ID of the file to be sent

  Returns:      
 *---------------------------------------------------------------------------*/

static void MBi_calc_sendblock(u8 file_id)
{
    /* Do not update if a request for the block specified by the child device has not been received */
    if ((any_recv_bitmap & (1 << file_id)) == 0)
    {
        return;
    }

    if (pPwork->fileinfo[file_id].active && pPwork->fileinfo[file_id].pollbmp)
    {
        if (pPwork->fileinfo[file_id].nextb <= pPwork->fileinfo[file_id].currentb &&
            pPwork->fileinfo[file_id].currentb <=
            pPwork->fileinfo[file_id].nextb + MB_SEND_THRESHOLD)
        {
            pPwork->fileinfo[file_id].currentb++;
        }
        else
        {
            pPwork->fileinfo[file_id].currentb = pPwork->fileinfo[file_id].nextb;
        }
        MB_DEBUG_OUTPUT("**FILE %2d SendBlock %d\n", file_id, pPwork->fileinfo[file_id].currentb);
    }

}

/*---------------------------------------------------------------------------*
  Name:         MBi_calc_nextsendblock

  Description:  Returns the next block to be sent.

  Arguments:    

  Returns:      
 *---------------------------------------------------------------------------*/

static u16 MBi_calc_nextsendblock(u16 next_block, u16 next_block_req)
{
    return max(next_block_req, next_block);
}


/*  ============================================================================

    Miscellaneous functions

    ============================================================================*/

static inline u16 max(u16 a, u16 b)
{
    return (u16)((a > b) ? a : b);
}

static BOOL IsChildAidValid(u16 child_aid)
{
    return (child_aid >= 1 && child_aid <= WM_NUM_MAX_CHILD) ? TRUE : FALSE;
}

static void MBi_CommCallParentError(u16 aid, u16 errcode)
{
    MBErrorStatus e_stat;
    e_stat.errcode = errcode;

    MBi_CommChangeParentStateCallbackOnly(aid, MB_COMM_PSTATE_ERROR, &e_stat);
}
