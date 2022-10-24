/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WM - demos - wep-1
  File:     main.c

  Copyright 2006-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-08-28#$
  $Rev: 11026 $
  $Author: tominaga_masafumi $
 *---------------------------------------------------------------------------*/

/*
  wep-1 demo

  This demo is the dataShare-Model demo with WEP usage added during connection.
*/
#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif

#include <nitro/wm.h>
#include <string.h>

#include "main.h"
#include "font.h"
#include "print.h"
#include "key.h"
#include "graphics.h"
#include "wh.h"
#include "menu.h"

/* The GGID used in this demo */
#define WH_GGID           SDK_MAKEGGID_SYSTEM(0x14)
#define GRAPH_TOTAL_FRAME 60

typedef struct GraphData_
{
    u16     level;
    s16     data;
}
GraphData;

static u16 sForcedChannel = 0;
static u16 sTgid = 0;
static int sSysMode = 0;
static int sSysModeStep = 0;

static s32 sFrame = 0;
static u8 sSendBuf[512] ATTRIBUTE_ALIGN(32);
static u8 sRecvBuf[512] ATTRIBUTE_ALIGN(32);
static BOOL sRecvFlag[WM_NUM_MAX_CHILD + 1];

static WMKeySet sKeySet;

static GraphData sGraphData[1 + WM_NUM_MAX_CHILD][GRAPH_TOTAL_FRAME];
static PRScreen sInfoScreen ATTRIBUTE_ALIGN(32);
static PRScreen sDebugScreen ATTRIBUTE_ALIGN(32);

static int sBeaconCount = 0;
static int sNeedWait = 0;
static BOOL sGraphEnabled = 0;

static WMBssDesc sBssDesc[3];
static Window sRoleMenuWindow;
static Window sSelectChannelWindow;
static Window sSelectParentWindow;
static Window sLobbyWindow;
static Window sErrorWindow;
static Window sBusyWindow;
static Window sOptionWindow;
static Window sWaitWindow;

static MATHRandContext32 sRand;

extern const unsigned char wlicon_image[];
extern const unsigned short wlicon_palette[];

static u16 ParentWEPKeyGenerator(u16 *wepkey, const WMParentParam *parentParam);
static u16 ChildWEPKeyGenerator(u16 *wepkey, const WMBssDesc *bssDesc);

static void TraceWH(const char *fmt, ...)
{
    va_list vlist;
    va_start(vlist, fmt);
    PR_VPrintString(&sDebugScreen, fmt, vlist);
    va_end(vlist);
}

static void printString(const char *fmt, ...)
{
    va_list vlist;
    va_start(vlist, fmt);
    PR_VPrintString(&sInfoScreen, fmt, vlist);
    va_end(vlist);
}

BOOL isDataReceived(int index)
{
    return sRecvFlag[index];
}

ShareData *getRecvData(int index)
{
    return (ShareData *) (&(sRecvBuf[index * sizeof(ShareData)]));
}

ShareData *getSendData(void)
{
    return (ShareData *) sSendBuf;
}

PRScreen *getInfoScreen(void)
{
    return &sInfoScreen;
}

void changeSysMode(int s)
{
    if (sSysMode == s)
    {
        return;
    }

    OS_Printf("sysmode = %d\n", s);
    sSysMode = s;
    sSysModeStep = 0;
}

static void indicateCallback(void *arg)
{
    WMIndCallback *cb;
    cb = (WMIndCallback *)arg;
    if (cb->state == WM_STATECODE_BEACON_RECV)
    {
        sBeaconCount = 2;
    }
}

static void scanCallback(WMBssDesc *bssdesc)
{
    char    buf[ITEM_LENGTH_MAX];
    int     i;

    for (i = 0; i < sSelectParentWindow.itemnum; ++i)
    {
        if (memcmp(sBssDesc[i].bssid, bssdesc->bssid, 12) == 0)
        {
            // Parents with the same bssid are excluded on grounds of being an identical entry
            return;
        }
    }

    WH_PrintBssDesc(bssdesc);

    // Save the information
    sBssDesc[sSelectParentWindow.itemnum] = *bssdesc;

    // Create the menu item for parent selection
    (void)snprintf(buf,
                   ITEM_LENGTH_MAX,
                   "[%d]channel%d", sSelectParentWindow.itemnum + 1, bssdesc->channel);
    addItemToWindow(&sSelectParentWindow, buf);
    WH_PrintBssDesc(bssdesc);
}

static void forceSpinWait(void)
{
    // Process to create the process failure state at will using OS_SpinWait
    // 

    static int waitcycle = 0;

    if (getKeyInfo()->cnt & PAD_BUTTON_L)
    {
        waitcycle += 4000;
        // OS_Printf("wait = %d\n", waitcycle);

    }
    else if (getKeyInfo()->cnt & PAD_BUTTON_R)
    {
        waitcycle -= 4000;
        if (waitcycle < 0)
        {
            waitcycle = 0;
        }
        // OS_Printf("wait = %d\n", waitcycle);
    }

    OS_SpinWait((u32)waitcycle);
}

static void ModeSelectRole(void)
{
    static const char *menuitems[] = {
        "Start (Parent mode)",
        "Start (Child mode)",
        "Option",
        NULL
    };

    if (sSysModeStep == 0)
    {
        sRoleMenuWindow.selected = 0;
        setupWindow(&sRoleMenuWindow, 16, 16, WIN_FLAG_SELECTABLE, 24, 24, 16);
        if (sRoleMenuWindow.itemnum == 0)
        {
            int     i;
            for (i = 0; menuitems[i] != NULL; ++i)
            {
                addItemToWindow(&sRoleMenuWindow, menuitems[i]);
            }
        }
        openWindow(&sRoleMenuWindow);
    }

    if (sRoleMenuWindow.state == WIN_STATE_CLOSED)
    {
        if (sRoleMenuWindow.selected < 0)
        {
            openWindow(&sRoleMenuWindow);
            return;
        }

        switch (sRoleMenuWindow.selected)
        {
        case 0:
            // WH initialization
            if (!WH_Initialize())
            {
                OS_Panic("WH_Initiailze failed.");
            }
            else
            {
                // Wait for initialization to complete
                while(WH_GetSystemState()==WH_SYSSTATE_BUSY){}
            }

            if (sForcedChannel == 0)
            {
                // Get the best channel based on the signal usage rate and connect.
                (void)WH_StartMeasureChannel();

            }
            else
            {
                // Start the connection using the manually-selected channel
                static u32 wepSeed;
                wepSeed = MATH_Rand32(&sRand, 0);
                // Embed the Seed value used in WEP calculations into UserGameInfo
                WH_SetUserGameInfo((u16 *)&wepSeed, 4);
                sTgid++;
                (void)WH_ParentConnect(WH_CONNECTMODE_DS_PARENT, sTgid, sForcedChannel);
            }
            changeSysMode(SYSMODE_LOBBY);
            break;

        case 1:
            // WH initialization
            if (!WH_Initialize())
            {
                OS_Panic("WH_Initiailze failed.");
            }
            else
            {
                // Wait for initialization to complete
                while(WH_GetSystemState()==WH_SYSSTATE_BUSY){}
            }

            {
                // Search for a parent
                static const u8 ANY_PARENT[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
                enum
                { ALL_CHANNEL = 0 };

                initWindow(&sSelectParentWindow);
                setupWindow(&sSelectParentWindow, 16, 16, WIN_FLAG_SELECTABLE, 8, 8, 16);
                (void)WH_StartScan(scanCallback, ANY_PARENT, ALL_CHANNEL);
                changeSysMode(SYSMODE_SCAN_PARENT);
            }
            break;

        case 2:
            // To the option screen
            changeSysMode(SYSMODE_OPTION);
            break;

        default:
            break;
        }
    }
}

static void ModeOption(void)
{
    // Option screen process

    if (sSysModeStep == 0)
    {
        initWindow(&sOptionWindow);
        setupWindow(&sOptionWindow, 16, 16, WIN_FLAG_SELECTABLE, 8, 8, 16);
        addItemToWindow(&sOptionWindow, "Select channel (parent)");

        if (sGraphEnabled)
        {
            addItemToWindow(&sOptionWindow, "Disable beacon graph");
        }
        else
        {
            addItemToWindow(&sOptionWindow, "Enable beacon graph");
        }

        openWindow(&sOptionWindow);
        return;
    }

    if (sOptionWindow.state == WIN_STATE_CLOSED)
    {
        if (sOptionWindow.selected < 0)
        {
            // Cancelled
            changeSysMode(SYSMODE_SELECT_ROLE);
            return;
        }

        if (sOptionWindow.selected == 0)
        {
            // To the channel selection screen
            changeSysMode(SYSMODE_SELECT_CHANNEL);

        }
        else if (sOptionWindow.selected == 1)
        {
            sGraphEnabled = sGraphEnabled ? FALSE : TRUE;
            changeSysMode(SYSMODE_SELECT_ROLE);
        }
    }
}

static void ModeLobby(void)
{
    // Lobby screen process

    u16     bmap;
    int     i;

    if (sSysModeStep == 0)
    {
        initWindow(&sLobbyWindow);
        setupWindow(&sLobbyWindow, 16, 16, WIN_FLAG_NONE, 8, 8, 16);

        for (i = 0; i < WH_CHILD_MAX; ++i)
        {
            addItemToWindow(&sLobbyWindow, "");
        }

        addItemToWindow(&sLobbyWindow, "");
        addItemToWindow(&sLobbyWindow, "Push A to start");

        openWindow(&sLobbyWindow);
        return;
    }

    bmap = WH_GetBitmap();
    for (i = 0; i < WH_CHILD_MAX; ++i)
    {
        if (bmap & (1 << i))
        {
            (void)OS_SNPrintf(sLobbyWindow.item[i], ITEM_LENGTH_MAX, "[%02d] - entry", i);
        }
        else
        {
            (void)OS_SNPrintf(sLobbyWindow.item[i], ITEM_LENGTH_MAX, "[%02d] - waiting", i);
        }
    }

    if (sLobbyWindow.state == WIN_STATE_CLOSED)
    {
        getSendData()->command = SHARECMD_NONE;
        if (sLobbyWindow.selected < 0)
        {
            // WH shutdown
            WH_Finalize();
            while(WH_GetSystemState()==WH_SYSSTATE_BUSY){}
            (void)WH_End();
            while(WH_GetSystemState()==WH_SYSSTATE_BUSY){}

            changeSysMode(SYSMODE_SELECT_ROLE);
            return;
        }

        changeSysMode(SYSMODE_PARENT);
    }
}

static void ModeLobbyWait(void)
{
    // Process during child device lobby standby
    // Wait until a parent sends a start signal

    if (sSysModeStep == 0)
    {
        initWindow(&sWaitWindow);
        setupWindow(&sWaitWindow, 32, 56, WIN_FLAG_NOCONTROL, 8, 8, 8);
        addItemToWindow(&sWaitWindow, "\\2Accepted.");
        addItemToWindow(&sWaitWindow, "\\2Waiting for parent...");
        openWindow(&sWaitWindow);
        return;
    }

    if (getRecvData(0)->command == SHARECMD_EXITLOBBY)
    {
        closeWindow(&sWaitWindow);
        changeSysMode(SYSMODE_CHILD);
    }
}

static void ModeSelectChannel(void)
{
    static u16 channelList[15];        // Auto select + maximum 14 channels
    // Channel selection screen
    if (sSysModeStep == 0)
    {
        setupWindow(&sSelectChannelWindow, 16, 16, WIN_FLAG_SELECTABLE, 16, 12, 16);

        if (sSelectChannelWindow.itemnum == 0)
        {
            u16     pattern;
            int     i, j;
            for (i = 0; i < 14; i++)
            {
                channelList[i] = 0;
            }

            // WH initialization
            if (!WH_Initialize())
            {
                OS_Panic("WH_Initiailze failed.");
            }
            else
            {
                // Wait for initialization to complete
                while(WH_GetSystemState()==WH_SYSSTATE_BUSY){}
            }

            pattern = WH_GetAllowedChannel();

            // WH shutdown
            WH_Finalize();
            while(WH_GetSystemState()==WH_SYSSTATE_BUSY){}
            (void)WH_End();
            while(WH_GetSystemState()==WH_SYSSTATE_BUSY){}

            addItemToWindow(&sSelectChannelWindow, "Auto select");
            for (i = 1, j = 1; i <= 14; ++i)
            {
                if (pattern & (1 << (i - 1)))
                {
                    char    buf[ITEM_LENGTH_MAX];
                    (void)OS_SNPrintf(buf, ITEM_LENGTH_MAX, "Channel %d", i);
                    channelList[j] = (u16)i;
                    ++j;
                    addItemToWindow(&sSelectChannelWindow, buf);
                }
            }
        }

        openWindow(&sSelectChannelWindow);
    }

    if (sSelectChannelWindow.state == WIN_STATE_CLOSED)
    {
        if (sSelectChannelWindow.selected >= 0)
        {
            sForcedChannel = channelList[sSelectChannelWindow.selected];
        }

        // Return to the role selection screen
        changeSysMode(SYSMODE_SELECT_ROLE);
    }
}

static void ModeSelectParent(void)
{
    // Display parent on the list and select it.
    // In this state, WH is scanning a parent, and if a new parent is found during scanning, it is immediately added/reflected to the menu.
    // 

    if (WH_GetSystemState() == WH_SYSSTATE_CONNECT_FAIL)
    {
        // Because the internal WM state is invalid if the WM_StartConnect function fails, WM_Reset must be called once to reset the state to IDLE
        // 
        WH_Reset();
        return;
    }

    if (sSysModeStep == 0)
    {
        openWindow(&sSelectParentWindow);
    }

    // Did the user close the parent search screen?
    if ((sSelectParentWindow.state == WIN_STATE_CLOSED))
    {
        if (WH_GetSystemState() == WH_SYSSTATE_SCANNING)
        {
            // If the parent is currently scanning, scan is finished for a moment
            (void)WH_EndScan();
            return;
        }

        if (WH_GetSystemState() == WH_SYSSTATE_IDLE)
        {
            if (sSelectParentWindow.selected < 0)
            {
                // WH shutdown
                WH_Finalize();
                while(WH_GetSystemState()==WH_SYSSTATE_BUSY){}
                (void)WH_End();
                while(WH_GetSystemState()==WH_SYSSTATE_BUSY){}

                changeSysMode(SYSMODE_SELECT_ROLE);
                return;
            }

            // If not scanning, and if the user has selected a parent, data sharing starts
            (void)WH_ChildConnect(WH_CONNECTMODE_DS_CHILD,
                                  &(sBssDesc[sSelectParentWindow.selected]));
            changeSysMode(SYSMODE_LOBBYWAIT);
        }
    }
}

static void ModeError(void)
{
    // Error state
    if (sSysModeStep == 0)
    {
        initWindow(&sErrorWindow);
        setupWindow(&sErrorWindow, 16, 16, WIN_FLAG_NONE, 8, 8, 16);

        addItemToWindow(&sErrorWindow, "\\1Error has occured!");

        if (WH_GetLastError() == WM_ERRCODE_OVER_MAX_ENTRY)
        {
            addItemToWindow(&sErrorWindow, "\\4Rejected\n");
        }

        if (WH_GetLastError() == WH_ERRCODE_DISCONNECTED)
        {
            addItemToWindow(&sErrorWindow, "\\4Disconnected by parent\n");
        }

        if (WH_GetLastError() == WH_ERRCODE_PARENT_NOT_FOUND)
        {
            addItemToWindow(&sErrorWindow, "\\4Parent not found\n");
        }

        if (WH_GetLastError() == WH_ERRCODE_LOST_PARENT)
        {
            addItemToWindow(&sErrorWindow, "\\4Lost parent\n");
        }

        addItemToWindow(&sErrorWindow, "");
        addItemToWindow(&sErrorWindow, "\\fPush A to reset");

        closeAllWindow();
        openWindow(&sErrorWindow);
    }

    if (sErrorWindow.state == WIN_STATE_CLOSED)
    {
        // WH shutdown
        WH_Finalize();
        while(WH_GetSystemState()==WH_SYSSTATE_BUSY){}
        (void)WH_End();
        while(WH_GetSystemState()==WH_SYSSTATE_BUSY){}

        getRecvData(0)->command = SHARECMD_NONE;
        changeSysMode(SYSMODE_SELECT_ROLE);
    }
}

static void printShareData(void)
{
    s32     i;
    ShareData *sd;

    sd = getSendData();
    printString("\\2Send:     0x%04x 0x%04x\n", sd->key, sd->count & 0xffff);

    printString("\\4Receive:\n");
    for (i = 1; i < (WM_NUM_MAX_CHILD + 1); i++)
    {
        if (sRecvFlag[i])
        {
            sd = getRecvData(i);
            printString("\\4Child%02d:  0x%04x 0x%04x\n", i, sd->key, sd->count & 0xffff);

        }
        else
        {
            printString("No child\n");
        }
    }
}

static void ModeParent(void)
{
    printString("\n  \\fParent mode\n\n");
    printShareData();
}

static void ModeChild(void)
{
    ShareData *sd;

    printString("\n  \\fChild mode\n\n");
    printShareData();

    sd = (ShareData *) getRecvData(0);
    printString("\\4Parent:   0x%04x 0x%04x\n", sd->key, sd->count & 0xffff);
}

static void VBlankIntr(void)
{
    /*
       Some other samples use StepDataSharing here (the VBlankIntr function).
       This sample also performed StepDataSharing here before.
       

       It is this way because WM_StepDataSharing (and WM_GetKeySet) have to be called before the start of MP communications in a frame for communications to be stable.
       Because the V-Count when the SDK prepares for the default MP communication is 240 for a child and 260 for a parent, setting it immediately after a V-Blank starts presents the fewest problems. (Although not implemented at the time this document was written, it is planned to make it possible to specify, during initialization, the V-Count of the MP communications startup.)
       
       For 30-fps (and other) games, code is normally required to limit the number of StepDS calls to one every two frames.
       In that case, wait only a single frame (instead of two) when WM_ERRCODE_NO_DATASET is returned (if processing was lost).
       Otherwise, when there is one frame difference between the parent and child, it cannot be corrected.
       

       
       
       
       
       
     */

    updateKeys();
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

static void initAllocSystem(void)
{
    void   *tempLo;
    OSHeapHandle hh;

    tempLo = OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1);
    OS_SetArenaLo(OS_ARENA_MAIN, tempLo);
    hh = OS_CreateHeap(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi());
    if (hh < 0)
    {
        OS_Panic("ARM9: Fail to create heap...\n");
    }
    hh = OS_SetCurrentHeap(OS_ARENA_MAIN, hh);
}

static void drawPowerGraph(void)
{
    static const GXRgb linecolor[4] = {
        GX_RGB(15, 0, 0),              // Dark red (dead)
        GX_RGB(31, 31, 0),             // Yellow
        GX_RGB(0, 31, 0),              // Green
        GX_RGB(20, 31, 20),            // Light green
    };

    int     midx, ringidx;

    ringidx = (sFrame % GRAPH_TOTAL_FRAME);

    for (midx = 0; midx < WM_NUM_MAX_CHILD + 1; ++midx)
    {
        sGraphData[midx][ringidx].data = (s16)getRecvData(midx)->data;
        sGraphData[midx][ringidx].level = (u16)getRecvData(midx)->level;
    }

    G3_PolygonAttr(GX_LIGHTMASK_NONE,
                   GX_POLYGONMODE_MODULATE, GX_CULL_NONE, 0, 31, GX_POLYGON_ATTR_MISC_DISP_1DOT);
    G3_TexImageParam(GX_TEXFMT_NONE,
                     GX_TEXGEN_NONE,
                     GX_TEXSIZE_S16,
                     GX_TEXSIZE_T16, GX_TEXREPEAT_NONE, GX_TEXFLIP_NONE, GX_TEXPLTTCOLOR0_USE, 0);

    G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);
    G3_PushMtx();
    G3_Identity();
    G3_Translate(0, 0, -FX16_ONE * 4);

    G3_Begin(GX_BEGIN_TRIANGLES);

    for (midx = 1; midx < WM_NUM_MAX_CHILD + 1; ++midx)
    {
        int     basey, ys, ye, gi, x, level;
        basey = ((WM_NUM_MAX_CHILD / 2 - midx) * 9 + 6) * FX16_ONE / 64;

        for (gi = 0; gi < GRAPH_TOTAL_FRAME; ++gi)
        {
            int     ri;
            ri = (ringidx - gi);
            if (ri < 0)
            {
                ri += GRAPH_TOTAL_FRAME;
            }

            ys = sGraphData[midx][ri].data;
            level = sGraphData[midx][ri].level;

            ++ri;
            if (ri >= GRAPH_TOTAL_FRAME)
            {
                ri -= GRAPH_TOTAL_FRAME;
            }

            ye = sGraphData[midx][ri].data;

            x = -(gi - GRAPH_TOTAL_FRAME / 2) * 3;
            x *= FX16_ONE / 64;
            ys = ys * FX16_ONE / 64 + basey;
            ye = ye * FX16_ONE / 64 + basey;

            G3_Color(linecolor[level]);

            G3_Vtx((fx16)x, (fx16)ys, 0);
            G3_Vtx((fx16)(x + FX16_ONE / 64 / 2), (fx16)(ys + FX16_ONE / 64), 0);
            G3_Vtx((fx16)(x + 3 * FX16_ONE / 64), (fx16)ye, 0);
        }
    }

    G3_End();
    G3_PopMtx(1);
}

static void drawPowerIcon(void)
{
    // GUIDELINE
    // Display the signal reception strength icon.
    setupPseudo2DCamera();

    G3_PolygonAttr(GX_LIGHTMASK_NONE, GX_POLYGONMODE_DECAL, GX_CULL_NONE, 0, 31, 0);

    G3_TexImageParam(GX_TEXFMT_PLTT16,
                     GX_TEXGEN_TEXCOORD,
                     GX_TEXSIZE_S16,
                     GX_TEXSIZE_T16,
                     GX_TEXREPEAT_NONE,
                     GX_TEXFLIP_NONE,
                     GX_TEXPLTTCOLOR0_USE, (u32)(0x2000 + WM_GetLinkLevel() * 16 * 16 / 2));
    G3_TexPlttBase(0x2000, GX_TEXFMT_PLTT16);
    G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);
    drawPseudo2DTexQuad(224, 160, 16, 16, 16, 16);
}

static void drawRadioIcon(void)
{
    // GUIDELINE
    // Display the sending-a-signal icon.
    int     i;

    G3_PolygonAttr(GX_LIGHTMASK_NONE, GX_POLYGONMODE_DECAL, GX_CULL_NONE, 0, 31, 0);

    G3_TexPlttBase(0x2000, GX_TEXFMT_PLTT16);
    G3_TexImageParam(GX_TEXFMT_PLTT16,
                     GX_TEXGEN_TEXCOORD,
                     GX_TEXSIZE_S16,
                     GX_TEXSIZE_T16,
                     GX_TEXREPEAT_NONE,
                     GX_TEXFLIP_NONE, GX_TEXPLTTCOLOR0_USE, 0x2000 + 4 * 16 * 16 / 2);

    for (i = 0; i < 2; ++i)
    {
        drawPseudo2DTexQuad(16,
                            12 + i * 24 + ((i == sRoleMenuWindow.selected) ? (sFrame / 15 & 1) : 0),
                            16, 16, 16, 16);
    }
}

static void updateShareData(void)
{
    if (WH_GetSystemState() == WH_SYSSTATE_DATASHARING)
    {
        if (WH_StepDS(sSendBuf))
        {
            u16     i;
            for (i = 0; i < WM_NUM_MAX_CHILD + 1; ++i)
            {
                u8     *adr;
                ShareData *sd;

                adr = (u8 *)WH_GetSharedDataAdr(i);
                sd = (ShareData *) & (sRecvBuf[i * sizeof(ShareData)]);

                if (adr != NULL)
                {
                    MI_CpuCopy8(adr, sd, sizeof(ShareData));
                    sRecvFlag[i] = TRUE;

                }
                else
                {
                    sd->level = 0;
                    sd->data = 0;
                    sRecvFlag[i] = FALSE;
                }
            }

            sNeedWait = FALSE;

        }
        else
        {
            u16     i;
            for (i = 0; i < WM_NUM_MAX_CHILD + 1; ++i)
            {
                sRecvFlag[i] = FALSE;
            }

            sNeedWait = TRUE;
        }

    }
    else
    {
        u16     i;
        for (i = 0; i < WM_NUM_MAX_CHILD + 1; ++i)
        {
            sRecvFlag[i] = FALSE;
        }

        sNeedWait = FALSE;
    }
}

static void packSendData(void)
{
    ShareData *senddata;

    if (sNeedWait)
    {
        return;
    }

    senddata = getSendData();
    senddata->command = (sSysMode == SYSMODE_LOBBY) ? SHARECMD_NONE : SHARECMD_EXITLOBBY;

    senddata->level = (u16)WM_GetLinkLevel();

    senddata->data = 0;
    senddata->key = getKeyInfo()->cnt;
    senddata->count = sFrame & 0xffff;

    if (sBeaconCount != 0)
    {
        senddata->data += sBeaconCount * senddata->level;

        if (sBeaconCount > 0)
        {
            sBeaconCount = -sBeaconCount + 1;
        }
        else
        {
            sBeaconCount = -sBeaconCount - 1;
        }
    }
}

static void mainLoop(void)
{
    int retry = 0;
    enum {
        MAX_RETRY = 5
    };

    for (sFrame = 0; TRUE; sFrame++)
    {
        int     whstate;
        int     prevmode;

        whstate = WH_GetSystemState();
        prevmode = sSysMode;

        switch (whstate)
        {
        case WH_SYSSTATE_CONNECT_FAIL:
            // Retry several times when a connection fails
            if (sSysMode == SYSMODE_LOBBYWAIT && retry < MAX_RETRY)
            {
                changeSysMode(SYSMODE_SELECT_PARENT);
                sSysModeStep = 1; // Make it so that 'window' will not open
                retry++;
                break;
            }
            // When a retry fails, transition unchanged to the error state

        case WH_SYSSTATE_ERROR:
            // WH state has the priority if an error occurred
            changeSysMode(SYSMODE_ERROR);
            break;

        case WH_SYSSTATE_MEASURECHANNEL:
            {
                u16     channel = WH_GetMeasureChannel();
                static u32 wepSeed;
                wepSeed = MATH_Rand32(&sRand, 0);
                // Embed the Seed value used in WEP calculations into UserGameInfo
                WH_SetUserGameInfo((u16 *)&wepSeed, 4);
                sTgid++;
                (void)WH_ParentConnect(WH_CONNECTMODE_DS_PARENT, sTgid, channel);
            }
            break;

        default:
            break;
        }

        PR_ClearScreen(&sInfoScreen);

        // Load test
        forceSpinWait();

        switch (sSysMode)
        {
        case SYSMODE_SELECT_ROLE:
            // Role (parent or child) selection screen
            ModeSelectRole();
            retry = 0;
            break;

        case SYSMODE_SELECT_CHANNEL:
            // Channel selection screen
            ModeSelectChannel();
            break;

        case SYSMODE_LOBBY:
            // Lobby screen
            ModeLobby();
            break;

        case SYSMODE_LOBBYWAIT:
            // Lobby standby screen
            ModeLobbyWait();
            break;

        case SYSMODE_OPTION:
            // Option screen
            ModeOption();
            break;

        case SYSMODE_SCAN_PARENT:
        case SYSMODE_SELECT_PARENT:
            // Parent selection screen
            ModeSelectParent();
            break;

        case SYSMODE_ERROR:
            // Error report screen
            ModeError();
            break;

        case SYSMODE_PARENT:
            // Parent mode screen
            ModeParent();
            break;

        case SYSMODE_CHILD:
            // Child mode screen
            ModeChild();
            break;

        default:
            break;
        }

        if ((whstate == WH_SYSSTATE_BUSY)
            || ((whstate == WH_SYSSTATE_SCANNING) && (sSelectParentWindow.itemnum == 0)))
        {
            sBusyWindow.state = WIN_STATE_OPENED;

        }
        else
        {
            sBusyWindow.state = WIN_STATE_CLOSED;
        }

        updateAllWindow();
        drawAllWindow();

        if( (sSysMode != SYSMODE_SELECT_ROLE) && (sSysMode != SYSMODE_OPTION) && (sSysMode != SYSMODE_SELECT_CHANNEL) )
        {
            drawPowerIcon();
        }

        if ((sSysMode == SYSMODE_PARENT) || (sSysMode == SYSMODE_CHILD))
        {
            if (sGraphEnabled)
            {
                drawPowerGraph();
            }
        }

        if ((sSysMode == SYSMODE_SELECT_ROLE) && (sRoleMenuWindow.state == WIN_STATE_OPENED))
        {
            drawRadioIcon();
        }

        G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);

        if (!sNeedWait && ((sSysMode == SYSMODE_PARENT) || (sSysMode == SYSMODE_CHILD)))
        {
            // ... In an actual game, the update processing for characters and so on is performed here
            // 
        }

        DC_FlushRange(sInfoScreen.screen, sizeof(u16) * PR_SCREEN_SIZE);
        /* I/O register is accessed using DMA operation, so cache wait is not needed */

        // DC_WaitWriteBufferEmpty();
        GX_LoadBG2Scr(sInfoScreen.screen, 0, sizeof(u16) * PR_SCREEN_SIZE);
        DC_FlushRange(sDebugScreen.screen, sizeof(u16) * PR_SCREEN_SIZE);
        /* I/O register is accessed using DMA operation, so cache wait is not needed */

        // DC_WaitWriteBufferEmpty();
        GXS_LoadBG2Scr(sDebugScreen.screen, 0, sizeof(u16) * PR_SCREEN_SIZE);

        OS_WaitVBlankIntr();

        packSendData();
        updateShareData();

        if (prevmode == sSysMode)
        {
            ++sSysModeStep;
        }
    }
}

#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    OS_Init();
    FX_Init();
    RTC_Init();

    initGraphics();

    GX_SetBankForBG(GX_VRAM_BG_256_AB);
    GX_SetBankForBGExtPltt(GX_VRAM_BGEXTPLTT_01_F);
    G2_SetBG2ControlText(GX_BG_SCRSIZE_TEXT_256x256,
                         GX_BG_COLORMODE_16, GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000);

    GX_BeginLoadTex();
    GX_LoadTex(wlicon_image, 0x2000, 16 * 16 * 5);
    GX_EndLoadTex();

    GX_BeginLoadTexPltt();
    GX_LoadTexPltt(wlicon_palette, 0x2000, 32);
    GX_EndLoadTexPltt();

    GX_LoadBG2Char(d_CharData, 0, sizeof(d_CharData));
    GX_LoadBGPltt(d_PaletteData, 0, sizeof(d_PaletteData));

    GX_SetBankForSubBG(GX_VRAM_SUB_BG_128_C);
    GX_SetBankForSubBGExtPltt(GX_VRAM_SUB_BGEXTPLTT_0123_H);
    G2S_SetBG2ControlText(GX_BG_SCRSIZE_TEXT_256x256,
                          GX_BG_COLORMODE_16, GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x00000);

    GXS_SetGraphicsMode(GX_BGMODE_0);
    GXS_SetVisiblePlane(GX_PLANEMASK_BG2);
    G2S_SetBG2Priority(0);
    G2S_BG2Mosaic(FALSE);

    GXS_LoadBG2Char(d_CharData, 0, sizeof(d_CharData));
    GXS_LoadBGPltt(d_PaletteData, 0, sizeof(d_PaletteData));

    initAllocSystem();

    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    sInfoScreen.scroll = FALSE;
    sDebugScreen.scroll = TRUE;

    // Initialize wireless
    // Set callback specified in the WM_SetIndCallback called within wh
    WH_SetIndCallback(indicateCallback);

    // WH initialization settings
    WH_SetGgid(WH_GGID);
    WH_SetDebugOutput(TraceWH);

    // WEP Key Generator configuration
    WH_SetParentWEPKeyGenerator(ParentWEPKeyGenerator);
    WH_SetChildWEPKeyGenerator(ChildWEPKeyGenerator);

    // Initialize random number generator for WEP Key seed
    {
        u64     randSeed = 0;
        RTCDate date;
        RTCTime time;
        if (RTC_GetDateTime(&date, &time) == RTC_RESULT_SUCCESS)
        {
            randSeed =
                (((((((u64)date.year * 16ULL + date.month) * 32ULL) + date.day) * 32ULL +
                   time.hour) * 64ULL + time.minute) * 64ULL + time.second);
        }
        MATH_InitRand32(&sRand, randSeed);
    }

    GX_DispOn();
    GXS_DispOn();

    initWindow(&sRoleMenuWindow);
    initWindow(&sSelectChannelWindow);
    initWindow(&sSelectParentWindow);
    initWindow(&sLobbyWindow);
    initWindow(&sErrorWindow);
    initWindow(&sOptionWindow);
    initWindow(&sWaitWindow);
    initWindow(&sBusyWindow);

    setupWindow(&sBusyWindow, 64, 80, WIN_FLAG_NOCONTROL, 8, 8, 16);
    addItemToWindow(&sBusyWindow, "\\2Working...");

    // Countermeasure for pressing A Button in IPL
    updateKeys();

    // initCharacter();
    changeSysMode(SYSMODE_SELECT_ROLE);
    mainLoop();
}

// Common key used to generate WEP key (parent and child use the same key)
// It should be unique for each application
// Does not have to be an ASCII char string; can be binary data of any length
static char *sSecretKey = "this is a secret key for HMAC";
static u32 sSecretKeyLength = 30;      // Ideally, the key should be more than 20 bytes

// Calculation routine for parent's WEP key
u16 ParentWEPKeyGenerator(u16 *wepkey, const WMParentParam *parentParam)
{
    u16     data[20 / sizeof(u16)];
    u16     tmpWep[20 / sizeof(u16)];
    u8      macAddress[WM_SIZE_MACADDR];

    OS_GetMacAddress(macAddress);
    MI_CpuClear16(data, sizeof(data));

    // Right before the WH_ParentConnect function, apply the Seed value set earlier in UserGameInfo to the WEP Key, using the WH_SetUserGameInfo function
    // 
    MI_CpuCopy16(parentParam->userGameInfo, data, 4);
    // To allow the same algorithm to be used in different applications, also reflect the GGID in the WEP key
    // 
    MI_CpuCopy16(&parentParam->ggid, data + 2, sizeof(u32));
    *(u16 *)(data + 4) = parentParam->tgid;
    // Also reflect the MAC Address in the WEP Key so that different parents will have different WEP keys
    MI_CpuCopy8(macAddress, (u8 *)(data + 5), WM_SIZE_MACADDR);

    // A keyed hash value is taken with HMAC-SHA-1 and is reduced to 128 bits to create the WEP key.
    // The key used with HMAC should be unique for each application.
    // (For details on HMAC, see RFC2104)
    MATH_CalcHMACSHA1(tmpWep, data, sizeof(data), sSecretKey, sSecretKeyLength);
    MI_CpuCopy8(tmpWep, wepkey, 16);
    OS_TPrintf("wepkey: %04x%04x %04x%04x %04x%04x %04x%04x\n", wepkey[0], wepkey[1],
               wepkey[2], wepkey[3], wepkey[4], wepkey[5], wepkey[6], wepkey[7]);

    return WM_WEPMODE_128BIT;
}

// Calculation routine for child's WEP key
u16 ChildWEPKeyGenerator(u16 *wepkey, const WMBssDesc *bssDesc)
{
    u16     data[20 / sizeof(u16)];
    u16     tmpWep[20 / sizeof(u16)];

    MI_CpuClear16(data, sizeof(data));

    // Calculate WEP key using the seed value embedded in UserGameInfo
    MI_CpuCopy16(bssDesc->gameInfo.userGameInfo, data, 4);
    // Include consideration of the GGID, TGID, and the MAC Address of the parent
    MI_CpuCopy16(&bssDesc->gameInfo.ggid, data + 2, sizeof(u32));
    *(u16 *)(data + 4) = bssDesc->gameInfo.tgid;
    MI_CpuCopy8(bssDesc->bssid, (u8 *)(data + 5), WM_SIZE_MACADDR);

    // A keyed hash value is taken with HMAC-SHA-1 and is reduced to 128 bits to create the WEP key.
    // The key used with HMAC should be unique for each application.
    // (For details on HMAC, see RFC2104)
    MATH_CalcHMACSHA1(tmpWep, data, sizeof(data), sSecretKey, sSecretKeyLength);
    MI_CpuCopy8(tmpWep, wepkey, 16);
    OS_TPrintf("wepkey: %04x%04x %04x%04x %04x%04x %04x%04x\n", wepkey[0], wepkey[1],
               wepkey[2], wepkey[3], wepkey[4], wepkey[5], wepkey[6], wepkey[7]);

    return WM_WEPMODE_128BIT;
}
