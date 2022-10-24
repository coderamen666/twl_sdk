/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - multiboot
  File:     parent.c

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

#include "common.h"
#include "dispfunc.h"


/*
 * Basic processing on the parent.
 *
 * Because the MB library samples use the multiboot functionality, multiple development units with the same communications environment (wired or wireless) are required
 * 
 * The mb_child_NITRO.srl and mb_child_TWL.srl sample programs in the $TwlSDK/bin/ARM9-TS/Rom/ directory provides the same features as a multiboot child for final retail units. Load this binary on other hardware as with any sample program, and run them together.
 * 
 * 
 * 
 * 
 */


/******************************************************************************/
/* Variables */

/*
 * Memory for loading the download program's segment information.
 */

static u8 *p_segbuf[GAME_PROG_MAX];
static u8 cursor = 0;
static MBUserInfo myUser;
static u16 parent_channel = PARENT_CHANNEL;
static u16 parent_send_size = 256;
static u8 entry_max = 15;

/*
 * Switch for automatically controlling responses from the parent to a child in this sample.
 * Children wait for a response from the parent before sending requests. By timing this response to be the same for all children or individually different, you can configure the behavior of multi-boot applications.
 * For example, you could respond individually if you are only distributing a demo, and respond all at once when the entry number is reached during a multiplayer match.
 * 
 * 
 *
 * 
 * 
 */

static u8 auto_accept = 0;             /* Automate the connection permission */
static u8 auto_send = 0;               /* Automate the download permission */
static u8 auto_boot = 0;               /* Automate the boot permission */

/******************************************************************************/
/* Functions */

/* Initialize the file buffer pointer */
void InitFileBuffer(void)
{
    int     i;

    for (i = 0; i < GAME_PROG_MAX; i++)
    {
        p_segbuf[i] = NULL;
    }

}

/* Deallocate the allocated file buffers */
static void FreeFileBuffer(void)
{
    int     i;

    for (i = 0; i < GAME_PROG_MAX; i++)
    {
        if (p_segbuf[i])
        {
            OS_Free(p_segbuf[i]);
            p_segbuf[i] = NULL;
        }

    }

}

/* Generate MBUserInfo */
static void MakeUserInfo(MBUserInfo *user, u8 favoriteColor, u8 playerNo, u16 *name,
                         u16 name_length)
{
    SDK_ASSERT(user != NULL);
    user->favoriteColor = favoriteColor;
    user->nameLength = (u8)(name_length);
    MI_CpuCopy8((char *)name, (char *)user->name, (u32)(name_length * 2));
    user->playerNo = playerNo;
}

/* Cyclically change channels */
static BOOL changeChannel(u16 *p_channel)
{
    u16     channel_bmp, channel, i;

    /* Get channel bitmap. */
    channel_bmp = WM_GetAllowedChannel();

    /* If there are no usable channels, call OS_Panic */
    if (channel_bmp == 0)
    {
        OS_Panic("No Available Parent channel\n");
        return FALSE;
    }

    /* If usable channels exist. */
    for (i = 0, channel = *p_channel;
         i < 16; i++, channel = (u16)((channel == 16) ? 1 : channel + 1))
    {
        if (channel_bmp & (1 << (channel - 1)))
        {
            /* If the detected channel is the same as before, search for another channel.
               
               
             */
            if (*p_channel != channel)
            {
                *p_channel = channel;
                break;
            }
        }

    }

    return TRUE;

}

/* Parent device file download state notification callback */
static void ParentStateCallback(u16 aid, u32 status, void *arg)
{

    switch (status)
    {
    case MB_COMM_PSTATE_INIT_COMPLETE:
        /* Initialization process complete */
        {
            WMStartParentCallback *p = (WMStartParentCallback *)arg;
            BgSetMessage(WHITE, "MB lib Init complete");
        }
        break;

    case MB_COMM_PSTATE_CONNECTED:
        /* Notification the moment the child device becomes connected */
        {
            WMStartParentCallback *p = (WMStartParentCallback *)arg;

            BgSetMessage(YELLOW, "Connected [%2d]", aid);
            BgSetMessage(YELLOW, "MACAddress %04x %04x %04x",
                         p->macAddress[0], p->macAddress[1], p->macAddress[2]);
        }
        break;

    case MB_COMM_PSTATE_DISCONNECTED:
        /* Notification when the child device has terminated connection */
        {
            WMStartParentCallback *p = (WMStartParentCallback *)arg;

            BgSetMessage(RED, "Disconnected [%2d]", aid);
            /* ReasonCode display at the time of disconnection */
            //                  BgSetMessage( WHITE, "reason code : %2d", p->reason);
        }
        break;

    case MB_COMM_PSTATE_KICKED:
        /* Notification when a child is kicked */
        BgSetMessage(RED, "Entry Refused [%2d]", aid);
        break;

    case MB_COMM_PSTATE_REQ_ACCEPTED:
        /* Notification when received the download request from a child */
        BgSetMessage(YELLOW, "Download Request [%2d]", aid);
        break;

    case MB_COMM_PSTATE_SEND_PROCEED:
        /* Notification when starting the binary transmission to a child */
        BgSetMessage(CYAN, "Start Sending [%2d]", aid);
        break;

    case MB_COMM_PSTATE_SEND_COMPLETE:
        /* Notification when the binary transmission to the child completed */
        BgSetMessage(CYAN, "Send Completed [%2d]", aid);
        if (auto_boot == 1)
        {
            /* Automatic boot request */
            (void)MB_CommBootRequest(aid);
            BgSetMessage(WHITE, "-->Boot Request [%2d]", aid);
        }
        break;

    case MB_COMM_PSTATE_BOOT_REQUEST:
        /* Notification when the boot request is sent to the child device */
        BgSetMessage(CYAN, "Send boot request [%2d]", aid);
        break;

    case MB_COMM_PSTATE_BOOT_STARTABLE:
        /* Notification when a series of processes has ended, and an application that supports multiboot can be started up */
        BgSetMessage(YELLOW, "Boot ready [%2d]", aid);
        break;

    case MB_COMM_PSTATE_REQUESTED:
        /* Notification of the moment the entry request came from a child */
        BgSetMessage(YELLOW, "Entry Requested [%2d]", aid);

        if (auto_accept == 1)
        {
            /* Auto-accept entry */
            (void)MB_CommResponseRequest(aid, MB_COMM_RESPONSE_REQUEST_ACCEPT);
            BgSetMessage(WHITE, "-->Entry Accept [%2d]", aid);
        }
        break;

    case MB_COMM_PSTATE_MEMBER_FULL:
        /* Notification when a child device is attempting to connect to a game with the full number of players */
        BgSetMessage(RED, "Entry Member full [%2d]", aid);
        break;

    case MB_COMM_PSTATE_END:
        /* Notification when the parent device ends */
        BgSetMessage(WHITE, "MB lib End");
        prog_state = STATE_MENU;
        break;

    case MB_COMM_PSTATE_WAIT_TO_SEND:
        /* Notification when the send wait state to the child device has started */
        BgSetMessage(CYAN, "Waiting to send [%2d]", aid);
        if (auto_send == 1)
        {
            /* Start auto-send */
            (void)MB_CommStartSending(aid);
            BgSetMessage(WHITE, "-->Start Sending [%2d]", aid);
        }
        break;

    }
}



/* Parent device initialization */
void ParentInit(void)
{
    int     i;

    OSOwnerInfo MyInfo;

    OS_GetOwnerInfo(&MyInfo);
    MakeUserInfo(&myUser, MyInfo.favoriteColor, 0, (u16 *)MyInfo.nickName, MyInfo.nickNameLength);

    BgClear();
    BgSetMessage(WHITE, "Initializing Parent.");

    /*
     * Begin MB parent device control
     * Enters the wait state when everything is ready
     */

    /* During re-initialization, deallocate the heap that was used previously */
    FreeFileBuffer();

    /* Multiboot library initialization */
    (void)MB_Init(cwork, &myUser, MY_GGID, MB_TGID_AUTO, MB_DMA_NO);

    /* Specify the data size to send / receive */
    (void)MB_SetParentCommParam(parent_send_size, entry_max);
    (void)MB_CommSetParentStateCallback(ParentStateCallback);
    // MB_StartParent calls WM_Initialize internally
    if(MB_StartParent(parent_channel) != MB_SUCCESS)
    {
        OS_Panic("MB_StartParent() failed.");
    }

    BgSetMessage(WHITE, "** Parameters **");
    BgSetMessage(WHITE, "channel      : %2d", parent_channel);
    BgSetMessage(WHITE, "send size    : %3d", parent_send_size);
    BgSetMessage(WHITE, "max children : %2d", entry_max);
    BgSetMessage(WHITE, "GGID:%08x TGID:%04x", MY_GGID, MB_GetTgid());

    prog_state = STATE_MB_PARENT;
    cursor = 0;

    /*
     * Next, register information for the download program file.
     *
     * Of the entire program file, only the ARM9-static and ARM7-static segments are required when multi-booting.
     * 
     * (Everything else will be overlays, data files, and so on)
     *
     * To read only this information from the program, use the MB_ReadSegment function first.
     * 
     * This sample dynamically allocates memory using the size obtained by the MB_GetSegmentLength function.
     * 
     *
     * The program file must be read using FS_ReadFile.
     * Usually, programs are maintained as files in CARD-ROM and pose no problems. If you anticipate a special multi-boot system, however, use FSArchive to build an original archive to handle it.
     * 
     * 
     */
    for (i = 0; i < GAME_PROG_MAX; ++i)
    {
        FSFile  file[1];
        FS_InitFile(file);

        /*
         * Open the child program file.
         * This file is used by MB_ReadSegment below.
         */
        if (!FS_OpenFileEx(file, mbGameList[i].romFilePathp, FS_FILEMODE_R))
        {
            OS_TPrintf("ParentInit : file cannot open (mbGameList[i].romFilePathp=\"%s)\"\n",
                       mbGameList[i].romFilePathp ? mbGameList[i].romFilePathp : "(NULL)");
        }
        else
        {
            /*
             * Get the size of the segment information.
             * If this is not a valid download program, 0 will be returned for this size.
             * 
             */
            u32     length = MB_GetSegmentLength(file);
            if (length == 0)
            {
                OS_TPrintf
                    ("ParentInit : specified file may be invalid format. (mbGameList[i].romFilePathp=\"%s)\"\n",
                     mbGameList[i].romFilePathp ? mbGameList[i].romFilePathp : "(NULL)");
            }
            else
            {
                /*
                 * Allocate memory for the segment information.
                 * This memory may be prepared statically as long as it is large enough.
                 * 
                 */
                p_segbuf[i] = (u8 *)OS_Alloc(length);
                if (!p_segbuf[i])
                {
                    OS_TPrintf("ParentInit : memory allocation failed. (%d BYTE)\n", length);
                }
                else
                {
                    /*
                     * Extract segment information from the file.
                     * The extracted segment information must stay resident in main memory while the download program is being distributed.
                     * 
                     *
                     * If extraction fails here even though the size was obtained successfully, it may be because the file handle was changed in some way
                     * 
                     * 
                     * (The file was closed, a seek to some location was run, and so on)
                     */
                    if (!MB_ReadSegment(file, p_segbuf[i], length))
                    {
                        OS_TPrintf("ParentInit : failed to extract segment\n");
                    }
                    else
                    {
                        /*
                         * Register the download program in the MBGameRegistry with the extracted segment information.
                         * 
                         *
                         */

                        if (MB_RegisterFile(&mbGameList[i], p_segbuf[i]))
                        {
                            BgSetMessage(LIGHT_GREEN, "Registered\"%s\"",
                                         mbGameList[i].romFilePathp);
                        }
                        else
                        {
                            OS_TPrintf("ParentInit : failed to register file No %d\n", i);
                        }
                    }
                }
            }
        }
        if (FS_IsFile(file))
            (void)FS_CloseFile(file);
    }

}


/* Parent device main process in each single frame */
void ParentMain(void)
{
    char    userName[MB_USER_NAME_LENGTH * 2 + 1];
    const int children_num = MB_CommGetChildrenNumber();
    const u16 keyData = ReadKeySetTrigger(PAD_Read());
    enum
    { DISP_OX = 2, DISP_OY = 3 };
    u16     i;
    OSIntrMode enabled;
    enum
    { PSTATE_NUM = 16 };
    const char *pstate_string[] = {
        "NONE        ",                //      STATE_NONE
        "INIT OK     ",                //      STATE_INIT_COMPLETE
        "CONNECTED   ",                //      STATE_CONNECTED
        "DISCONNECTED",                //      STATE_DISCONNECTED
        "KICKED      ",                //      STATE_KICKED
        "ENTRY OK    ",                //      STATE_REQ_ACCEPTED
        "SENDING     ",                //      STATE_SEND_PROCEED
        "SEND END    ",                //      STATE_SEND_COMPLETE
        "BOOT REQ    ",                //      STATE_BOOT_REQUEST
        "BOOT READY  ",                //      STATE_BOOT_STARTABLE
        "ENTRY REQ   ",                //      STATE_REQUESTED
        "MEMBER FULL ",                //      STATE_MEMBER_FULL
        "END         ",                //      STATE_END
        "ERROR       ",                //      STATE_ERROR
        "WAIT TO SEND",                //      STATE_WAIT_TO_SEND
    };

    BgPrintf(DISP_OX, DISP_OY - 2, WHITE, "CH:%2d  Max Children:%2d", parent_channel, entry_max);
    BgPutString(DISP_OX, DISP_OY - 1, WHITE, "AID USERNAME STATE        ");
    BgPutString(DISP_OX, DISP_OY + MB_MAX_CHILD + 1, WHITE, "A:operate B:refuse SEL:restart");
    BgPutString(DISP_OX, DISP_OY + MB_MAX_CHILD + 2, WHITE, "START:operate all children");

    enabled = OS_DisableInterrupts();  /* Interrupts disabled */

    /* Child list display */
    for (i = 0; i < entry_max; ++i)
    {
        const u16 aid = (u16)(i + 1);
        const MBUserInfo *p_child = MB_CommGetChildUser(aid);
        int     p_state;

        BgPutString(DISP_OX, (s16)(DISP_OY + i), WHITE, "                         ");

        BgPrintf(DISP_OX + 0, (s16)(DISP_OY + i), WHITE, "%2d  ", aid);

        if (p_child && p_child->nameLength)
        {
            u8      palette = p_child->favoriteColor;
            MI_CpuCopy8(p_child->name, userName, (u32)(p_child->nameLength * 2));
            BgPutStringN((s16)(DISP_OX + 4), (s16)(DISP_OY + i), palette, userName, 8);
        }

        p_state = MB_CommGetParentState(aid);

        if (p_state < PSTATE_NUM)
        {
            BgPrintf((s16)(DISP_OX + 13), (s16)(DISP_OY + i), WHITE,
                     "%s  ", (char *)pstate_string[p_state]);
        }

        BgPutChar((s16)(DISP_OX - 2), (s16)(DISP_OY + i), RED, (char)((i == cursor) ? '*' : ' '));
    }

    (void)OS_RestoreInterrupts(enabled);        /* Cancel interrupt disabling */

    /* All child devices are targets */
    if (keyData & PAD_BUTTON_START)
    {
        BOOL    result = FALSE;

        if (!auto_accept)
        {
            for (i = 1; i <= entry_max; i++)
            {
                /* Entry permission */
                if (MB_COMM_PSTATE_REQUESTED == MB_CommGetParentState(i))
                {
                    result = MB_CommResponseRequest(i, MB_COMM_RESPONSE_REQUEST_ACCEPT);
                }
            }
            if (TRUE == result)
                return;
        }

        if (!auto_send)
        {
            result = MB_CommStartSendingAll();
            if (TRUE == result)
                return;
        }

        if (!auto_boot)
        {
            result = MB_CommBootRequestAll();
            if (TRUE == result)
                return;
        }
    }

    /* Individually send to designated child devices */
    if (keyData & PAD_BUTTON_A)
    {
        const u16 aid = (u16)(cursor + 1);

        BgSetMessage(WHITE, "AID%2d State:%s", aid, pstate_string[MB_CommGetParentState(aid)]);

        /* Entry permission */
        if (MB_COMM_PSTATE_REQUESTED == MB_CommGetParentState(aid))
        {
            if (!auto_accept)
            {
                (void)MB_CommResponseRequest(aid, MB_COMM_RESPONSE_REQUEST_ACCEPT);
                BgSetMessage(WHITE, "-->Entry Accept [%2d]", aid);
            }
        }
        /* Start send */
        else if (MB_COMM_PSTATE_WAIT_TO_SEND == MB_CommGetParentState(aid))
        {
            if (!auto_send)
            {
                (void)MB_CommStartSending(aid);
                BgSetMessage(WHITE, "-->Start Sending [%2d]", aid);
            }
        }
        else
            /* Boot request */
        if (TRUE == MB_CommIsBootable(aid))
        {
            if (!auto_boot)
            {
                (void)MB_CommBootRequest(aid);
                BgSetMessage(WHITE, "-->Boot Request [%2d]", aid);
            }
        }
    }
    else
        /* Deny entry to the designated child device */
    if (keyData & PAD_BUTTON_B)
    {
        const u16 aid = (u16)(cursor + 1);

        BgSetMessage(WHITE, "AID%2d State:%s", aid, pstate_string[MB_CommGetParentState(aid)]);
        if (MB_COMM_PSTATE_REQUESTED == MB_CommGetParentState(aid))
        {
            if (!auto_accept)
            {
                (void)MB_CommResponseRequest(aid, MB_COMM_RESPONSE_REQUEST_KICK);
                BgSetMessage(CYAN, "Entry Refuse [%2d]", aid);
            }
        }
    }

    /* When SELECT is pressed, end and re-initialize */
    if (keyData & PAD_BUTTON_SELECT)
    {
        /* Change channel */
        (void)changeChannel(&parent_channel);
        MB_End();
    }

    /* Change cursor number */
    if (keyData & PAD_KEY_DOWN)
    {
        (void)RotateU8(&cursor, 0, (u8)(entry_max - 1), 1);
    }
    else if (keyData & PAD_KEY_UP)
    {
        (void)RotateU8(&cursor, 0, (u8)(entry_max - 1), -1);
    }

}

/* Set parent device MP send size */
void SetParentSendSize(u16 p_size)
{
    parent_send_size = p_size;
}

/* Get parent device MP send size */
u16 GetParentSendSize(void)
{
    return parent_send_size;
}

/* Set maximum number of child device connections */
void SetMaxEntry(u8 num)
{
    entry_max = num;
}

/* Get the maximum number of child device connections */
u8 GetMaxEntry(void)
{
    return entry_max;
}

/* Configure parent device auto operations */
void SetAutoOperation(u8 fAccept, u8 fSend, u8 fBoot)
{
    auto_accept = fAccept;
    auto_send = fSend;
    auto_boot = fBoot;
}
