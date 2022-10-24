/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - multiboot-PowerSave
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


#ifdef SDK_TWL
#include	<twl.h>
#else
#include	<nitro.h>
#endif

#include "common.h"
#include "dispfunc.h"


/*
 * Basic processing on the parent
 *
 * Because the MB library samples use multiboot functionality, multiple development units with the same communications environment (wired or wireless) are required
 * 
 * The mb_child_NITRO.srl and mb_child_TWL.srl sample programs in the $TwlSDK/bin/ARM9-TS/Rom/ directory provide the same features as a multiboot child for retail units. Load these binaries on other hardware as with any sample program, and run them together.
 * 
 * 
 * 
 * 
 */


/******************************************************************************/
/* Declaration */

static void ParentInit(void);
static void ParentDraw(void);
static void ParentUpdate(void);


/******************************************************************************/
/* Constants */

/* DMA number to allocate to the MB library */
#define MB_DMA_NO       2

/* Parent's own GGID */
#define SDK_MAKEGGID_SYSTEM(num)    (0x003FFF00 | (num))
#define MY_GGID         SDK_MAKEGGID_SYSTEM(0x20)

/* Parent's initial distribution channel */
#define PARENT_CHANNEL  13


/******************************************************************************/
/* Variables */

/* The work region to be allocated to the MB library */
static u8 cwork[MB_SYSTEM_BUF_SIZE];

static u16 parent_channel = PARENT_CHANNEL;

static BOOL mb_running = FALSE;

/* The array of program information that this demo downloads */
static MBGameRegistry mbGameList[] = {
    {
     "/em.srl",
     L"edgeDemo",
     L"edgemarking demo",
     "/data/icon.char",
     "/data/icon.plt",
     0 /* GGID */ ,
     16 /* player-num */ ,
     },
    {
     "/pol_toon.srl",
     L"PolToon",
     L"toon rendering",
     "/data/icon.char",
     "/data/icon.plt",
     0 /* GGID */ ,
     8 /* player-num */ ,
     },
};

enum
{ GAME_PROG_MAX = sizeof(mbGameList) / sizeof(*mbGameList) };

static u8 *p_segbuf[GAME_PROG_MAX] = { NULL, };


/******************************************************************************/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         changeChannel

  Description:  Cyclically changes channels.

  Arguments:    p_channel: AID of the child devices for notification
                status: Notification status
                arg: Callback argument

  Returns:      None.
 *---------------------------------------------------------------------------*/
static u16 changeChannel(u16 channel)
{
    const u16 channel_bak = channel;
    u16     channel_bmp, i;

    /* Get channel bitmap. */
    channel_bmp = WM_GetAllowedChannel();

    /* If there are no usable channels, call OS_Panic */
    if (channel_bmp == 0)
    {
        OS_Panic("No Available Parent channel\n");
    }
    /* If there is a channel that can be used, it searches for a different channel from last time */
    for (i = 0; i < 16; i++, channel = (u16)((channel == 16) ? 1 : channel + 1))
    {
        if (channel_bmp & (1 << (channel - 1)))
        {
            if (channel_bak != channel)
            {
                break;
            }
        }

    }
    return channel;
}

/*---------------------------------------------------------------------------*
  Name:         ParentStateCallback

  Description:  MB library status notification callback.

  Arguments:    aid: AID of the child devices for notification
                status: Notification status
                arg: Callback argument

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ParentStateCallback(u16 aid, u32 status, void *arg)
{

    switch (status)
    {
    case MB_COMM_PSTATE_INIT_COMPLETE:
        BgSetMessage(WHITE, "MB lib Init complete");
        break;

    case MB_COMM_PSTATE_CONNECTED:
        {
            WMStartParentCallback *p = (WMStartParentCallback *)arg;
            BgSetMessage(YELLOW, "Connected(%2d)%02X:%02X:%02X:%02X:%02X:%02X", aid,
                         p->macAddress[0], p->macAddress[1], p->macAddress[2],
                         p->macAddress[3], p->macAddress[4], p->macAddress[5]);
        }
        break;

    case MB_COMM_PSTATE_DISCONNECTED:
        BgSetMessage(RED, "Disconnected [%2d]", aid);
        break;

    case MB_COMM_PSTATE_KICKED:
        BgSetMessage(RED, "Entry Refused [%2d]", aid);
        break;

    case MB_COMM_PSTATE_REQ_ACCEPTED:
        BgSetMessage(YELLOW, "Download Request [%2d]", aid);
        break;

    case MB_COMM_PSTATE_SEND_PROCEED:
        BgSetMessage(CYAN, "Start Sending [%2d]", aid);
        break;

    case MB_COMM_PSTATE_SEND_COMPLETE:
        BgSetMessage(CYAN, "Send Completed [%2d]", aid);
        (void)MB_CommBootRequest(aid);
        BgSetMessage(WHITE, "-->Boot Request [%2d]", aid);
        break;

    case MB_COMM_PSTATE_BOOT_REQUEST:
        BgSetMessage(CYAN, "Send boot request [%2d]", aid);
        break;

    case MB_COMM_PSTATE_BOOT_STARTABLE:
        BgSetMessage(YELLOW, "Boot ready [%2d]", aid);
        break;

    case MB_COMM_PSTATE_REQUESTED:
        BgSetMessage(YELLOW, "Entry Requested [%2d]", aid);
        (void)MB_CommResponseRequest(aid, MB_COMM_RESPONSE_REQUEST_ACCEPT);
        BgSetMessage(WHITE, "-->Entry Accept [%2d]", aid);
        break;

    case MB_COMM_PSTATE_MEMBER_FULL:
        BgSetMessage(RED, "Entry Member full [%2d]", aid);
        break;

    case MB_COMM_PSTATE_END:
        BgSetMessage(WHITE, "MB lib End");
        mb_running = FALSE;
        break;

    case MB_COMM_PSTATE_WAIT_TO_SEND:
        BgSetMessage(CYAN, "Waiting to send [%2d]", aid);
        (void)MB_CommStartSending(aid);
        BgSetMessage(WHITE, "-->Start Sending [%2d]", aid);
        break;
    }

}

/*---------------------------------------------------------------------------*
  Name:         ParentInit

  Description:  Initializes the parent's status in Single-Card Play

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ParentInit(void)
{
    int     i;
    /* Initializes the parent in Single-Card Play */
    {
        MBUserInfo user;
        /* Deallocates the previous segment buffer */
        for (i = 0; i < GAME_PROG_MAX; ++i)
        {
            if (p_segbuf[i])
            {
                OS_Free(p_segbuf[i]), p_segbuf[i] = NULL;
            }
        }
        /* Selects the parent device's name in accordance with power-save mode */
        if (g_power_save_mode)
        {
            MI_CpuCopy8(L"Power:Save", user.name, 10 * sizeof(u16));
            user.favoriteColor = OS_FAVORITE_COLOR_BLUE;
        }
        else
        {
            MI_CpuCopy8(L"Power:Full", user.name, 10 * sizeof(u16));
            user.favoriteColor = OS_FAVORITE_COLOR_YELLOW;
        }
        user.nameLength = 10;
        /* Initializes MB */
        (void)MB_Init(cwork, &user, MY_GGID, MB_TGID_AUTO, MB_DMA_NO);
        MB_SetPowerSaveMode(g_power_save_mode);
        (void)MB_CommSetParentStateCallback(ParentStateCallback);
        // MB_StartParent calls WM_Initialize internally
        if(MB_StartParent(parent_channel) != MB_SUCCESS)
        {
            OS_Panic("MB_StartParent failed.");
        }
    }

    GX_DispOn();
    GXS_DispOn();

    BgClear();
    BgSetMessage(WHITE, "Initializing Parent.");
    BgSetMessage(WHITE, "** Parameters **");
    BgSetMessage(WHITE, "channel      : %2d", parent_channel);
    BgSetMessage(WHITE, "GGID:%08x TGID:%04x", MY_GGID, MB_GetTgid());

    /* Registers the download program file information */
    for (i = 0; i < GAME_PROG_MAX; ++i)
    {
        BOOL    succeeded = FALSE;
        FSFile  file[1];
        FS_InitFile(file);

        if (!FS_OpenFileEx(file, mbGameList[i].romFilePathp, FS_FILEMODE_R))
        {
            OS_TPrintf("ParentInit : file cannot open (mbGameList[i].romFilePathp=\"%s)\"\n",
                       mbGameList[i].romFilePathp ? mbGameList[i].romFilePathp : "(NULL)");
        }
        else if ((p_segbuf[i] = (u8 *)OS_Alloc(MB_SEGMENT_BUFFER_MIN)) == NULL)
        {
            OS_TPrintf("ParentInit : memory allocation failed. (%d BYTE)\n", MB_SEGMENT_BUFFER_MIN);
        }
        else if (!MB_ReadSegment(file, p_segbuf[i], MB_SEGMENT_BUFFER_MIN))
        {
            OS_TPrintf("ParentInit : failed to extract segment\n");
        }
        else if (!MB_RegisterFile(&mbGameList[i], p_segbuf[i]))
        {
            OS_TPrintf("ParentInit : failed to register file No %d\n", i);
        }
        else
        {
            BgSetMessage(LIGHT_GREEN, "Registered\"%s\"", mbGameList[i].romFilePathp);
            succeeded = TRUE;
        }

        if (FS_IsFile(file))
        {
            (void)FS_CloseFile(file);
        }
        if (!succeeded && (p_segbuf[i] != NULL))
        {
            OS_Free(p_segbuf[i]), p_segbuf[i] = NULL;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         ParentUpdate

  Description:  Updates the parent's status in Single-Card Play.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ParentUpdate(void)
{
    const u16 keyData = ReadKeySetTrigger();

    /* When SELECT is pressed, end and re-initialize */
    if ((keyData & PAD_BUTTON_SELECT) != 0)
    {
        /* Change channel */
        parent_channel = changeChannel(parent_channel);
        MB_End();
    }

}

/*---------------------------------------------------------------------------*
  Name:         ParentUpdate

  Description:  Draws the parent's status in Single-Card Play.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ParentDraw(void)
{
    static const char *(pstate_string[]) =
    {
    "NONE        ",
            "INIT OK     ",
            "CONNECTED   ",
            "DISCONNECTED",
            "KICKED      ",
            "ENTRY OK    ",
            "SENDING     ",
            "SEND END    ",
            "BOOT REQ    ",
            "BOOT READY  ",
            "ENTRY REQ   ", "MEMBER FULL ", "END         ", "ERROR       ", "WAIT TO SEND",};
    enum
    { PSTATE_NUM = sizeof(pstate_string) / sizeof(*pstate_string) };
    enum
    { DISP_OX = 2, DISP_OY = 3 };

    BgPrintf(DISP_OX, DISP_OY - 2, WHITE, "CH:%2d", parent_channel);
    BgPutString(DISP_OX, DISP_OY - 1, WHITE, "AID USERNAME STATE        ", 32);
    BgPutString(DISP_OX, DISP_OY + MB_MAX_CHILD + 1, WHITE, "SEL:restart", 32);

    /*
     * Update the display for the download management state.
     * All interrupts are blocked to prevent inconsistency in the individual states that are obtained.
     */
    {
        OSIntrMode enabled = OS_DisableInterrupts();
        int     i;

        /* Child list display */
        for (i = 0; i < 15; ++i)
        {
            const u16 aid = (u16)(i + 1);
            const MBUserInfo *p = MB_CommGetChildUser(aid);
            const int state = MB_CommGetParentState(aid);

            BgPrintf(DISP_OX + 0, DISP_OY + i, WHITE, "%2d                       ", aid);
            if (p)
            {
                char    name[MB_USER_NAME_LENGTH * 2 + 1] = { 0, };
                MI_CpuCopy8(p->name, name, (u32)(p->nameLength * 2));
                BgPutString(DISP_OX + 4, DISP_OY + i, p->favoriteColor, name, 8);
            }
            if (state < PSTATE_NUM)
            {
                BgPrintf(DISP_OX + 13, DISP_OY + i, WHITE, "%s", pstate_string[state]);
            }
        }

        (void)OS_RestoreInterrupts(enabled);
    }

}

/*---------------------------------------------------------------------------*
  Name:         ParentMode

  Description:  DS Single-Card play parent processing.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ParentMode(void)
{
    ParentInit();
    /* Main loop */
    mb_running = TRUE;
    while (mb_running)
    {
        ParentUpdate();
        ParentDraw();

        OS_WaitVBlankIntr();
    }
}
