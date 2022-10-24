  /*---------------------------------------------------------------------------*
  Project:  TwlSDK - wireless_shared - demos - mbp
  File:     mbp.c

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
#include <nitro.h>
#include <nitro/wm.h>
#include <nitro/mb.h>

#include "mbp.h"




//============================================================================
//  Prototype Declarations
//============================================================================

static BOOL MBP_RegistFile(const MBGameRegistry *gameInfo);
static inline void MBP_AddBitmap(u16 *pBitmap, u16 aid);
static inline void MBP_RemoveBitmap(u16 *pBitmap, u16 aid);
static inline void MBP_DisconnectChild(u16 aid);
static inline void MBP_DisconnectChildFromBmp(u16 aid);
static void ParentStateCallback(u16 child_aid, u32 status, void *arg);
static void MBP_ChangeState(u16 state);


// Callbacks are run by IRQs, so it is safer to set a slightly large IRQ stack size in the LCF file when running complex processes within a callback.
// 
//
// Because OS_Printf() in particular consumes a large amount of stack, try using the lighter version, OS_TPrintf(), in callbacks whenever possible.
// 

#define MBP_DEBUG
#if defined( MBP_DEBUG )
#define MBP_Printf          OS_TPrintf //
#else
#define MBP_Printf(...)     ((void)0)
#endif



//============================================================================
//  Variable Definitions
//============================================================================

// Child connection information
static MBPState mbpState;
static MBPChildInfo childInfo[MBP_CHILD_MAX];

/* The work region to be allocated to the MB library */
static u32 *sCWork = NULL;

/* Buffer for child device startup binary transmission */
static u8 *sFilebuf = NULL;

//============================================================================
//  Function definitions
//============================================================================

/*---------------------------------------------------------------------------*
  Name:         MBP_Init

  Description:  Initializes the multiboot parent information.

  Arguments:    ggid  Specifies the parent's GGID when distributing a game
                tgid  Specifies the parent's TGID when distributing a game

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MBP_Init(u32 ggid, u32 tgid)
{
    /* For the configuration of the parent device information to be displayed to the child device screen */
    MBUserInfo myUser;

    /* It is not precisely determined what type of value needs to be specified as the parent information to display on the MB child screen.
     * Here, user information registered with the IPL is set as the MB parent information, but the name established within the game may be set.
     * However, this must be set as 2-byte code.
     * 
     * */
    OSOwnerInfo info;

    OS_GetOwnerInfo(&info);
    myUser.favoriteColor = info.favoriteColor;
    myUser.nameLength = (u8)info.nickNameLength;
    MI_CpuCopy8(info.nickName, myUser.name, (u32)(info.nickNameLength * 2));


    myUser.playerNo = 0;               // The parent is number 0

    // Initialize the status information
    mbpState = (const MBPState)
    {
    MBP_STATE_STOP, 0, 0, 0, 0, 0, 0};

    /* Begin MB parent device control */
    // Allocate the MB work area
#if !defined(MBP_USING_MB_EX)
    sCWork = OS_Alloc(MB_SYSTEM_BUF_SIZE);
#else
    /* A small work size does not matter if WM initialization has been finished externally */
    sCWork = OS_Alloc(MB_SYSTEM_BUF_SIZE - WM_SYSTEM_BUF_SIZE);
#endif

    if (MB_Init(sCWork, &myUser, ggid, tgid, MBP_DMA_NO) != MB_SUCCESS)
    {
        OS_Panic("ERROR in MB_Init\n");
    }

    // Set the maximum number of children to be connected (set a value that excludes the parent)
    (void)MB_SetParentCommParam(MB_COMM_PARENT_SEND_MIN, MBP_CHILD_MAX);

    MB_CommSetParentStateCallback(ParentStateCallback);

    MBP_ChangeState(MBP_STATE_IDLE);
}


/*---------------------------------------------------------------------------*
  Name:         MBP_Start

  Description:  Starts multiboot parent.

  Arguments:    gameInfo Distribution binary information
                channel  Channel to be used

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MBP_Start(const MBGameRegistry *gameInfo, u16 channel)
{
    SDK_ASSERT(MBP_GetState() == MBP_STATE_IDLE);

    MBP_ChangeState(MBP_STATE_ENTRY);
#if !defined(MBP_USING_MB_EX)
    if (MB_StartParent(channel) != MB_SUCCESS)
#else
    /* Transfer the WM library to an IDLE state, and directly begin */
    if (MB_StartParentFromIdle(channel) != MB_SUCCESS)
#endif
    {
        MBP_Printf("MB_StartParent fail\n");
        MBP_ChangeState(MBP_STATE_ERROR);
        return;
    }

    /* --------------------------------------------------------------------- *
     * This will have been initialized when MB_StartParent() is called.
     * Use MB_RegisterFile() for registration after MB_StartParent().
     * --------------------------------------------------------------------- */

    /* Register the download program file information */
    if (!MBP_RegistFile(gameInfo))
    {
        OS_Panic("Illegal multiboot gameInfo\n");
    }
}


/*---------------------------------------------------------------------------*
  Name:         MBP_RegistFile

  Description:  Registers the multiboot binary.

  Arguments:    gameInfo Pointer to the multiboot binary information.
                         Of these members, a romFilePathp value of NULL will cause operations to be the same as when a clone boot has been specified.
                         

  Returns:      Returns TRUE if the file open was successful.
                If failed, returns FALSE.
 *---------------------------------------------------------------------------*/
static BOOL MBP_RegistFile(const MBGameRegistry *gameInfo)
{
    FSFile  file, *p_file;
    u32     bufferSize;
    BOOL    ret = FALSE;

    /* --------------------------------------------------------------------- *
     * This will already be initialized when MB_StartParent() is called, even if before that binary data was registered with MB_RegisterFile().
     * 
     * Use MB_RegisterFile() for registration after MB_StartParent().
     * --------------------------------------------------------------------- */

    /*
     * As given by this function's specifications, this will operate as a clone boot if romFilePathp is NULL.
     * 
     * If not, the specified file will be treated as a child program.
     */
    if (!gameInfo->romFilePathp)
    {
        p_file = NULL;
    }
    else
    {
        /* 
         * The program file must be read using FS_ReadFile().
         * Usually, programs are maintained as files in CARD-ROM and pose no problems. If you anticipate a special multi-boot system, however, use FSArchive to build an original archive to handle it.
         * 
         *  
         */
        FS_InitFile(&file);
        if (!FS_OpenFileEx(&file, gameInfo->romFilePathp, FS_FILEMODE_R))
        {
            /* Cannot open file */
            OS_Warning("Cannot Register file\n");
            return FALSE;
        }
        p_file = &file;
    }

    /*
     * Get the size of the segment information.
     * If this is not a valid download program, 0 will be returned for this size.
     * 
     */
    bufferSize = MB_GetSegmentLength(p_file);
    if (bufferSize == 0)
    {
        OS_Warning("specified file may be invalid format.\"%s\"\n",
                   gameInfo->romFilePathp ? gameInfo->romFilePathp : "(cloneboot)");
    }
    else
    {
        /*
         * Allocate memory for loading the download program's segment information.
         * If file registration succeeded, this region will be used until MB_End() is called.
         * This memory may be prepared statically as long as it is large enough.
         * 
         */
        sFilebuf = OS_Alloc(bufferSize);
        if (sFilebuf == NULL)
        {
            /* Failed to allocate buffer that stores segment information */
            OS_Warning("can't allocate Segment buffer size.\n");
        }
        else
        {
            /* 
             * Extract segment information from the file.
             * The extracted segment information must stay resident in main memory while the download program is being distributed.
             * 
             */
            if (!MB_ReadSegment(p_file, sFilebuf, bufferSize))
            {
                /* 
                 * Failed to extract segments because the file is invalid.
                 * If extraction fails here even though the size was obtained successfully, it may be because the file handle was changed in some way.
                 * (The file was closed, a seek to some location was run, and so on)
                 * 
                 */
                OS_Warning(" Can't Read Segment\n");
            }
            else
            {
                /*
                 * Register the download program in the MBGameRegistry with the extracted segment information.
                 * 
                 */
                if (!MB_RegisterFile(gameInfo, sFilebuf))
                {
                    /* Registration failed because of incorrect program information */
                    OS_Warning(" Illegal program info\n");
                }
                else
                {
                    /* Process completed correctly */
                    ret = TRUE;
                }
            }
            if (!ret)
                OS_Free(sFilebuf);
        }
    }

    /* Close file if not clone boot */
    if (p_file == &file)
    {
        (void)FS_CloseFile(&file);
    }

    return ret;
}


/*---------------------------------------------------------------------------*
  Name:         MBP_AcceptChild

  Description:  Accepts a request from a child.

  Arguments:    child_aid       AID of a connected child.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MBP_AcceptChild(u16 child_aid)
{
    if (!MB_CommResponseRequest(child_aid, MB_COMM_RESPONSE_REQUEST_ACCEPT))
    {
        // If the request fails, disconnect that child
        MBP_DisconnectChild(child_aid);
        return;
    }

    MBP_Printf("accept child %d\n", child_aid);
}


/*---------------------------------------------------------------------------*
  Name:         MBP_KickChild

  Description:  Kicks the connected child.

  Arguments:    child_aid       AID of a connected child

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MBP_KickChild(u16 child_aid)
{
    if (!MB_CommResponseRequest(child_aid, MB_COMM_RESPONSE_REQUEST_KICK))
    {
        // If the request fails, disconnect that child
        MBP_DisconnectChild(child_aid);
        return;
    }

    {
        OSIntrMode enabled = OS_DisableInterrupts();

        mbpState.requestChildBmp &= ~(1 << child_aid);
        mbpState.connectChildBmp &= ~(1 << child_aid);

        (void)OS_RestoreInterrupts(enabled);
    }
}


/*---------------------------------------------------------------------------*
  Name:         MBP_StartDownload

  Description:  Sends the download start signals to the child.

  Arguments:    child_aid      AID of a connected child

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MBP_StartDownload(u16 child_aid)
{
    if (!MB_CommStartSending(child_aid))
    {
        // If the request fails, disconnect that child
        MBP_DisconnectChild(child_aid);
        return;
    }

    {
        OSIntrMode enabled = OS_DisableInterrupts();

        mbpState.entryChildBmp &= ~(1 << child_aid);
        mbpState.downloadChildBmp |= 1 << child_aid;

        (void)OS_RestoreInterrupts(enabled);
    }
}

/*---------------------------------------------------------------------------*
  Name:         MBP_StartDownloadAll

  Description:  Starts the data transfer to all of the children currently being entered.
                After this, it does not accept entries from children.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MBP_StartDownloadAll(void)
{
    u16     i;

    // Stop accepting entries
    MBP_ChangeState(MBP_STATE_DATASENDING);

    for (i = 1; i < 16; i++)
    {
        if (!(mbpState.connectChildBmp & (1 << i)))
        {
            continue;
        }

#ifdef MBP_USING_PREVIOUS_DOWNLOAD
        /* When data transfer has started immediately after entry */
        // Disconnect children that are not being entered
        if (MBP_GetChildState(i) == MBP_CHILDSTATE_CONNECTING)
        {
            MBP_DisconnectChild(i);
            continue;
        }
#else
        /* When starting to send data to all child devices at once */
        if (mbpState.requestChildBmp & (1 << i))
        {
            // The child currently being entered will be processed after it becomes ready
            continue;
        }

        // Disconnect children that are not being entered
        if (!(mbpState.entryChildBmp & (1 << i)))
        {
            MBP_DisconnectChild(i);
            continue;
        }

        // Children that are being entered start downloading
        MBP_StartDownload(i);
#endif
    }
}

/*---------------------------------------------------------------------------*
  Name:         MBP_IsBootableAll

  Description:  Checks whether all the connected child devices are in a bootable state.
                

  Arguments:    None.

  Returns:      TRUE if all of the children are bootable.
                FALSE if there is at least one child that is not bootable.
 *---------------------------------------------------------------------------*/
BOOL MBP_IsBootableAll(void)
{
    u16     i;

    if (mbpState.connectChildBmp == 0)
    {
        return FALSE;
    }

    for (i = 1; i < 16; i++)
    {
        if (!(mbpState.connectChildBmp & (1 << i)))
        {
            continue;
        }

        if (!MB_CommIsBootable(i))
        {
            return FALSE;
        }
    }
    return TRUE;
}


/*---------------------------------------------------------------------------*
  Name:         MBP_StartRebootAll

  Description:  Boots all of the connected children that are bootable.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MBP_StartRebootAll(void)
{
    u16     i;
    u16     sentChild = 0;

    for (i = 1; i < 16; i++)
    {
        if (!(mbpState.bootableChildBmp & (1 << i)))
        {
            continue;
        }
        if (!MB_CommBootRequest(i))
        {
            // If the request fails, disconnect that child
            MBP_DisconnectChild(i);
            continue;
        }
        sentChild |= (1 << i);
    }

    // If the connected child becomes 0, terminate with an error
    if (sentChild == 0)
    {
        MBP_ChangeState(MBP_STATE_ERROR);
        return;
    }

    // Changes the state and stops accepting data transfer
    MBP_ChangeState(MBP_STATE_REBOOTING);
}


/*---------------------------------------------------------------------------*
  Name:         MBP_Cancel

  Description:  Cancels multiboot.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MBP_Cancel(void)
{
    MBP_ChangeState(MBP_STATE_CANCEL);
#if !defined(MBP_USING_MB_EX)
    MB_End();
#else
    MB_EndToIdle();
#endif
}

/*---------------------------------------------------------------------------*
  Name:         MBPi_CheckAllReboot

  Description:  Determines whether all connected children are in the MB_COMM_PSTATE_BOOT_STARTABLE state and starts to shut down the MB library if they are.
                

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MBPi_CheckAllReboot(void)
{
    if ((mbpState.state == MBP_STATE_REBOOTING) &&
        (mbpState.connectChildBmp == mbpState.rebootChildBmp))
    {
        MBP_Printf("call MB_End()\n");
#if !defined(MBP_USING_MB_EX)
        MB_End();
#else
        MB_EndToIdle();
#endif
    }
}

/*---------------------------------------------------------------------------*
  Name:         ParentStateCallback

  Description:  Parent state notification callback

  Arguments:    child_aid       AID of the child
                status          Callback status
                arg             Option argument
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ParentStateCallback(u16 child_aid, u32 status, void *arg)
{
#if defined( MBP_DEBUG )
    // For debug output
    static const char *MB_CALLBACK_TABLE[] = {
        "MB_COMM_PSTATE_NONE",
        "MB_COMM_PSTATE_INIT_COMPLETE",
        "MB_COMM_PSTATE_CONNECTED",
        "MB_COMM_PSTATE_DISCONNECTED",
        "MB_COMM_PSTATE_KICKED",
        "MB_COMM_PSTATE_REQ_ACCEPTED",
        "MB_COMM_PSTATE_SEND_PROCEED",
        "MB_COMM_PSTATE_SEND_COMPLETE",
        "MB_COMM_PSTATE_BOOT_REQUEST",
        "MB_COMM_PSTATE_BOOT_STARTABLE",
        "MB_COMM_PSTATE_REQUESTED",
        "MB_COMM_PSTATE_MEMBER_FULL",
        "MB_COMM_PSTATE_END",
        "MB_COMM_PSTATE_ERROR",
        "MB_COMM_PSTATE_WAIT_TO_SEND",
    };
#endif

    MBP_Printf("get callback %s %d\n", MB_CALLBACK_TABLE[status], child_aid);

    switch (status)
    {
        //-----------------------------------------------------------
        // Notification that the initialization process completed with the parent
    case MB_COMM_PSTATE_INIT_COMPLETE:
        // None.
        break;

        //-----------------------------------------------------------
        // Notification of the moment the child attempts to connect
    case MB_COMM_PSTATE_CONNECTED:
        {
            // Connection is not accepted if the parent is not in entry accepting state
            if (MBP_GetState() != MBP_STATE_ENTRY)
            {
                break;
            }

            MBP_AddBitmap(&mbpState.connectChildBmp, child_aid);
            // Saves a child's MacAddress
            WM_CopyBssid(((WMStartParentCallback *)arg)->macAddress,
                         childInfo[child_aid - 1].macAddress);

            childInfo[child_aid - 1].playerNo = child_aid;
        }
        break;

        //-----------------------------------------------------------
        // Notification when the child device has terminated connection
    case MB_COMM_PSTATE_DISCONNECTED:
        {
            // Deletes the entry if it was disconnected by the condition other than rebooting of a child
            if (MBP_GetChildState(child_aid) != MBP_CHILDSTATE_REBOOT)
            {
                MBP_DisconnectChildFromBmp(child_aid);
                // Based on that, it determines whether every child has booted
                MBPi_CheckAllReboot();
            }
        }
        break;

        //-----------------------------------------------------------
        // Notification of the moment the entry request came from a child
    case MB_COMM_PSTATE_REQUESTED:
        {
            const MBUserInfo *userInfo;

            userInfo = (MBUserInfo *)arg;

            OS_TPrintf("callback playerNo = %d\n", userInfo->playerNo);

            // Unequivocally kick child devices that requested entry when the parent device is not accepting entries
            // 
            if (MBP_GetState() != MBP_STATE_ENTRY)
            {
                MBP_KickChild(child_aid);
                break;
            }

            // Accepts child's entry
            mbpState.requestChildBmp |= 1 << child_aid;

            MBP_AcceptChild(child_aid);

            // UserInfo has still not been set during MB_COMM_PSTATE_CONNECTED, so it is meaningless to call MB_CommGetChildUser before the REQUESTED state
            // 
            userInfo = MB_CommGetChildUser(child_aid);
            if (userInfo != NULL)
            {
                MI_CpuCopy8(userInfo, &childInfo[child_aid - 1].user, sizeof(MBUserInfo));
            }
            MBP_Printf("playerNo = %d\n", userInfo->playerNo);
        }
        break;

        //-----------------------------------------------------------
        // Notify ACK for ACCEPT to a child
    case MB_COMM_PSTATE_REQ_ACCEPTED:
        // No processing needed
        break;
        //-----------------------------------------------------------
        // Notification when received the download request from a child
    case MB_COMM_PSTATE_WAIT_TO_SEND:
        {
            // Changes the status of a child from entered to download standby.
            // Because this is the process in the interrupt, change it without prohibiting interrupt.
            mbpState.requestChildBmp &= ~(1 << child_aid);
            mbpState.entryChildBmp |= 1 << child_aid;

            // Starts the data transmission when MBP_StartDownload() is called from main routine.
            // If it is already in data transmission state, it starts the data transmission to the child.
#ifdef MBP_USING_PREVIOUS_DOWNLOAD
            /* When starting to send data immediately after entry */
            MBP_StartDownload(child_aid);
#else
            /* When starting to send data to all child devices at once */
            if (MBP_GetState() == MBP_STATE_DATASENDING)
            {
                MBP_StartDownload(child_aid);
            }
#endif
        }
        break;

        //-----------------------------------------------------------
        // Notification when kicking a child
    case MB_COMM_PSTATE_KICKED:
        // None.
        break;

        //-----------------------------------------------------------
        // Notification when starting the binary transmission to a child
    case MB_COMM_PSTATE_SEND_PROCEED:
        // None.
        break;

        //-----------------------------------------------------------
        // Notification when the binary transmission to the child completed
    case MB_COMM_PSTATE_SEND_COMPLETE:
        {
            // Because this is the process in the interrupt, change it without prohibiting interrupt
            mbpState.downloadChildBmp &= ~(1 << child_aid);
            mbpState.bootableChildBmp |= 1 << child_aid;
        }
        break;

        //-----------------------------------------------------------
        // Notify of a child when boot had completed properly
    case MB_COMM_PSTATE_BOOT_STARTABLE:
        {
            // Because this is the process in the interrupt, change it without prohibiting interrupt
            mbpState.bootableChildBmp &= ~(1 << child_aid);
            mbpState.rebootChildBmp |= 1 << child_aid;

            // When all of the children completed booting, starts the reconnection process with the parent
            MBPi_CheckAllReboot();
        }
        break;

        //-----------------------------------------------------------
        // Notify when the boot request transmission to the target child has started.
        // It does not return as a callback.
    case MB_COMM_PSTATE_BOOT_REQUEST:
        // None.
        break;

        //-----------------------------------------------------------
        // Notification when the members are full
    case MB_COMM_PSTATE_MEMBER_FULL:
        // None.
        break;

        //-----------------------------------------------------------
        // Notify when ending multiboot
    case MB_COMM_PSTATE_END:
        {
            if (MBP_GetState() == MBP_STATE_REBOOTING)
                // When reboot process ends, it ends MB and reconnects with a child
            {
                MBP_ChangeState(MBP_STATE_COMPLETE);
            }
            else
                // Shut down completed, and restore the STOP state
            {
                MBP_ChangeState(MBP_STATE_STOP);
            }

            // Clears the buffer used for distributing games.
            // Work memory will have been released when the MB_COMM_PSTATE_END callback returns, so we can free memory.
            // 
            if (sFilebuf)
            {
                OS_Free(sFilebuf);
                sFilebuf = NULL;
            }
            if (sCWork)
            {
                OS_Free(sCWork);
                sCWork = NULL;
            }

            /* MB_UnregisterFile can be omitted because initialization will be caused by calling MB_End and releasing work memory
             *                                         */

        }
        break;

        //-----------------------------------------------------------
        // Notify that the error occurred
    case MB_COMM_PSTATE_ERROR:
        {
            MBErrorStatus *cb = (MBErrorStatus *)arg;

            switch (cb->errcode)
            {
                //------------------------------
                // Notification level error. No process is necessary.
            case MB_ERRCODE_WM_FAILURE:
                // None.
                break;
                //------------------------------
                // Error at the level that requires wireless to be reset
            case MB_ERRCODE_INVALID_PARAM:
            case MB_ERRCODE_INVALID_STATE:
            case MB_ERRCODE_FATAL:
                MBP_ChangeState(MBP_STATE_ERROR);
                break;
            }
        }
        break;

        //-----------------------------------------------------------
    default:
        OS_Panic("Get illegal parent state.\n");
    }
}


/*---------------------------------------------------------------------------*
  Name:         MBP_ChangeState

  Description:  Changes the parent state.

  Arguments:    state       State to change

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MBP_ChangeState(u16 state)
{
#if defined( MBP_DEBUG )
    // For debug output
    static const char *STATE_NAME[] = {
        "MBP_STATE_STOP",
        "MBP_STATE_IDLE",
        "MBP_STATE_ENTRY",
        "MBP_STATE_DATASENDING",
        "MBP_STATE_REBOOTING",
        "MBP_STATE_COMPLETE",
        "MBP_STATE_CANCEL",
        "MBP_STATE_ERROR"
    };
#endif

    SDK_ASSERT(state < MBP_STATE_NUM);

    MBP_Printf("%s -> %s\n", STATE_NAME[mbpState.state], STATE_NAME[state]);
    mbpState.state = state;
}


/*---------------------------------------------------------------------------*
  Name:         MBP_GetState

  Description:  Gets the parent state.

  Arguments:    None.

  Returns:      Parent state.
 *---------------------------------------------------------------------------*/
u16 MBP_GetState(void)
{
    return mbpState.state;
}

/*---------------------------------------------------------------------------*
  Name:         MBP_GetConnectState

  Description:  Gets the connection information.

  Arguments:    Buffer area for getting the current connection state

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MBP_GetConnectState(MBPState * state)
{
    MI_CpuCopy8(&mbpState, state, sizeof(MBPState));
}

/*---------------------------------------------------------------------------*
  Name:         MBP_GetChildBmp

  Description:  Gets the connection bitmap of a child.

  Arguments:    Type of the connection bitmap to get

  Returns:      Child state.
 *---------------------------------------------------------------------------*/
u16 MBP_GetChildBmp(MBPBmpType bmpType)
{
    u16     tmpBitmap;
    static const u16 *BITMAP_TABLE[MBP_BMPTYPE_NUM] = {
        &mbpState.connectChildBmp,
        &mbpState.requestChildBmp,
        &mbpState.entryChildBmp,
        &mbpState.downloadChildBmp,
        &mbpState.bootableChildBmp,
        &mbpState.rebootChildBmp,
    };

    SDK_ASSERT(bmpType < MBP_BMPTYPE_NUM);

    tmpBitmap = *(BITMAP_TABLE[bmpType]);

    return tmpBitmap;
}


/*---------------------------------------------------------------------------*
  Name:         MBP_GetChildState

  Description:  Gets the connection state of a child.

  Arguments:    aid     Number for the connected child

  Returns:      Child state.
 *---------------------------------------------------------------------------*/
MBPChildState MBP_GetChildState(u16 aid)
{
    MBPState tmpState;
    u16     bitmap = (u16)(1 << aid);

    // Because an interrupt may change the connection state unexpectedly, prohibit interrupts and get a snapshot of the state at that time
    // 
    OSIntrMode enabled = OS_DisableInterrupts();
    if ((mbpState.connectChildBmp & bitmap) == 0)
    {
        (void)OS_RestoreInterrupts(enabled);
        return MBP_CHILDSTATE_NONE;    // Not connected
    }
    MI_CpuCopy8(&mbpState, &tmpState, sizeof(MBPState));
    (void)OS_RestoreInterrupts(enabled);

    // Determining the connection status
    if (tmpState.requestChildBmp & bitmap)
    {
        return MBP_CHILDSTATE_REQUEST; // Requesting connection
    }
    if (tmpState.entryChildBmp & bitmap)
    {
        return MBP_CHILDSTATE_ENTRY;   // Undergoing entry
    }
    if (tmpState.downloadChildBmp & bitmap)
    {
        return MBP_CHILDSTATE_DOWNLOADING;      // Downloading
    }
    if (tmpState.bootableChildBmp & bitmap)
    {
        return MBP_CHILDSTATE_BOOTABLE; // Boot preparation completed
    }
    if (tmpState.rebootChildBmp & bitmap)
    {
        return MBP_CHILDSTATE_REBOOT;  // Rebooted
    }

    return MBP_CHILDSTATE_CONNECTING;  // Connecting
}


/*---------------------------------------------------------------------------*
  Name:         MBP_AddBitmap

  Description:  Adds a child to the connection state bitmap.

  Arguments:    pBitmap     Pointer to the bitmap to add
                aid         AID of the child to be added

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void MBP_AddBitmap(u16 *pBitmap, u16 aid)
{
    // Prohibit interrupts to prevent an unexpected interrupt from breaking the state while values are being changed
    // 
    OSIntrMode enabled = OS_DisableInterrupts();
    *pBitmap |= (1 << aid);
    (void)OS_RestoreInterrupts(enabled);
}


/*---------------------------------------------------------------------------*
  Name:         MBP_RemoveBitmap

  Description:  Delete a child from the connection state bitmap.

  Arguments:    pBitmap     Pointer to the bitmap to delete
                aid         AID of the child to be deleted

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void MBP_RemoveBitmap(u16 *pBitmap, u16 aid)
{
    // Prohibit interrupts to prevent an unexpected interrupt from breaking the state while values are being changed
    // 
    OSIntrMode enabled = OS_DisableInterrupts();
    *pBitmap &= ~(1 << aid);
    (void)OS_RestoreInterrupts(enabled);
}

/*---------------------------------------------------------------------------*
  Name:         MBP_DisconnectChildFromBmp

  Description:  Deletes the child device flag from all bitmaps when a child device is disconnected.
                

  Arguments:    aid     AID of the child that was disconnected

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void MBP_DisconnectChildFromBmp(u16 aid)
{
    u16     aidMask = (u16)~(1 << aid);

    OSIntrMode enabled = OS_DisableInterrupts();

    mbpState.connectChildBmp &= aidMask;
    mbpState.requestChildBmp &= aidMask;
    mbpState.entryChildBmp &= aidMask;
    mbpState.downloadChildBmp &= aidMask;
    mbpState.bootableChildBmp &= aidMask;
    mbpState.rebootChildBmp &= aidMask;

    (void)OS_RestoreInterrupts(enabled);
}

/*---------------------------------------------------------------------------*
  Name:         MBP_DisconnectChild

  Description:  Disconnects a child.

  Arguments:    aid     AID of the child to be disconnected

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void MBP_DisconnectChild(u16 aid)
{
    // MB_Disconnect may be provided in the future
    MBP_Printf(" WM_Disconnect %d\n", aid);
    MBP_DisconnectChildFromBmp(aid);
    MB_DisconnectChild(aid);
}

/*---------------------------------------------------------------------------*
  Name:         MBP_GetChildInfo

  Description:  Gets a pointer to the user information on a child.

  Arguments:    child_aid   AID of the child

  Returns:      A pointer to user information.
 *---------------------------------------------------------------------------*/
const MBPChildInfo *MBP_GetChildInfo(u16 child_aid)
{
    if (!(mbpState.connectChildBmp & (1 << child_aid)))
    {
        return NULL;
    }
    return &childInfo[child_aid - 1];
}


/*---------------------------------------------------------------------------*
  Name:         MBP_GetPlayerNo

  Description:  Allows you to search for and obtain the player number (AID) used by a child device when it was a multiboot child, based on its MAC address, after that child has been rebooted and reconnected.
                

  Arguments:    macAddress      MAC address of a child

  Returns:      Child AID at the time of multiboot connection.
 *---------------------------------------------------------------------------*/
u16 MBP_GetPlayerNo(const u8 *macAddress)
{
    u16     i;

    for (i = 1; i < MBP_CHILD_MAX + 1; i++)
    {
        if ((mbpState.connectChildBmp & (1 << i)) == 0)
        {
            continue;
        }
        if (WM_IsBssidEqual(macAddress, childInfo[i - 1].macAddress))
        {
            return childInfo[i - 1].playerNo;
        }
    }

    return 0;
}


/*---------------------------------------------------------------------------*
  Name:         MBP_GetChildMacAddress

  Description:  Gets MAC address of the multiboot child.

  Arguments:    aid     AID of the child

  Returns:      Pointer to MAC address.
 *---------------------------------------------------------------------------*/
const u8 *MBP_GetChildMacAddress(u16 aid)
{
    if (!(mbpState.connectChildBmp & (1 << aid)))
    {
        return NULL;
    }
    return childInfo[aid - 1].macAddress;
}
