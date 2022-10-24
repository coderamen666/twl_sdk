/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WM - demos - listenOnly
  File:     main.c

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

/*---------------------------------------------------------------------------*
    This is a simple functional sample that initializes the WM library in limited receive mode and performs scanning.
 *---------------------------------------------------------------------------*/

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif
#include    <nitro/wm.h>

#include    "font.h"

#define USE_WM_INITIALIZE // Use the WM_InitializeForListening function

/*---------------------------------------------------------------------------*
    Constant Definitions
 *---------------------------------------------------------------------------*/
#define     BEACON_INFO_MAX     256    // Maximum amount of AP information that can be accumulated through scanning

#define     WIRELESS_DMA_NO     2      // NOTE: Do not confuse this with the GX DMA number

#define     KEY_REPEAT_START    25     // Number of frames until key repeat starts
#define     KEY_REPEAT_SPAN     10     // Number of frames between key repeats

enum {
    STATE_NONE = 0,
    STATE_INITIALIZING,
    STATE_IDLE,
    STATE_SCANNING,
    STATE_TERMINATING_SCAN,
    STATE_FATAL
};

/*---------------------------------------------------------------------------*
    Structure Definitions
 *---------------------------------------------------------------------------*/
// Key input data
typedef struct KeyInfo
{
    u16     cnt;                       // Unprocessed input value
    u16     trg;                       // Push trigger input
    u16     up;                        // Release trigger input
    u16     rep;                       // Press and hold repeat input

}
KeyInfo;

// AP data
typedef struct BeaconInfo
{
    u32 timeStamp_found;
    u32 timeStamp_updated;
    BOOL isDS;
    u8 bssid[WM_SIZE_BSSID];
    u8 ssid[WM_SIZE_SSID];
    u8 padding[2];
    u32 channel;
}
BeaconInfo;

/*---------------------------------------------------------------------------*
    Internal Function Definitions
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void);          // V-Blank interrupt handler

static void SetState(u32 state);
static u32 GetState(void);

static BOOL InitializeWMForListeningAndStartScan(void);
#ifdef USE_WM_INITIALIZE
static void InitializeCb(void *arg);
#else
static void EnableCb(void *arg);
static void PowerOnCb(void *arg);
#endif
static BOOL StartScan(void);
static void StartScanExCb(void *arg);
static void EndScanCb(void* arg);

// General purpose subroutines
static void KeyRead(KeyInfo * pKey);
static void ClearString(void);
static void PrintString(s16 x, s16 y, u8 palette, char *text, ...);
static void ColorString(s16 x, s16 y, s16 length, u8 palette);
static void InitializeAllocateSystem(void);


/*---------------------------------------------------------------------------*
    Internal Variable Definitions
 *---------------------------------------------------------------------------*/
static u32 sState = STATE_NONE;
static BeaconInfo sBeaconInfo[BEACON_INFO_MAX];

/* System work region buffer for WM */
static u8 sWmBuffer[WM_SYSTEM_BUF_SIZE] ATTRIBUTE_ALIGN(32);
static u8 sScanBuf[WM_SIZE_SCAN_EX_BUF] ATTRIBUTE_ALIGN(32);
static WMScanExParam sScanExParam ATTRIBUTE_ALIGN(32);

static u16 sScreen[32 * 32];           // Virtual screen
static KeyInfo sKey;                   // Key input


/*---------------------------------------------------------------------------*
  Name:         NitroMain / TwlMain

  Description:  Initialization and main loop.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    // Various types of initialization
    OS_Init();
    FX_Init();
    GX_Init();
    GX_DispOff();
    GXS_DispOff();

    // Initializes display settings
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
    MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);
    (void)GX_DisableBankForLCDC();
    MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);
    MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);
    MI_CpuFillFast((void *)HW_DB_OAM, 192, HW_DB_OAM_SIZE);
    MI_CpuClearFast((void *)HW_DB_PLTT, HW_DB_PLTT_SIZE);

    // 2D display settings for text string display
    GX_SetBankForBG(GX_VRAM_BG_128_A);
    G2_SetBG0Control(GX_BG_SCRSIZE_TEXT_256x256, GX_BG_COLORMODE_16, GX_BG_SCRBASE_0xf800,      // SCR base block 31
                     GX_BG_CHARBASE_0x00000,    // CHR base block 0
                     GX_BG_EXTPLTT_01);
    G2_SetBG0Priority(0);
    G2_BG0Mosaic(FALSE);
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D);
    GX_SetVisiblePlane(GX_PLANEMASK_BG0);
    GX_LoadBG0Char(d_CharData, 0, sizeof(d_CharData));
    GX_LoadBGPltt(d_PaletteData, 0, sizeof(d_PaletteData));
    MI_CpuFillFast((void *)sScreen, 0, sizeof(sScreen));
    DC_FlushRange(sScreen, sizeof(sScreen));
    /* I/O register is accessed using DMA operation, so cache wait is not needed */
    // DC_WaitWriteBufferEmpty();
    GX_LoadBG0Scr(sScreen, 0, sizeof(sScreen));

    // Interrupt settings
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    // Memory allocation
    InitializeAllocateSystem();

    MI_CpuClear8(sBeaconInfo, sizeof(sBeaconInfo));

    // Initialize wireless
    if (! InitializeWMForListeningAndStartScan() )
    {
        OS_Panic("Cannot initialize WM");
    }

    // LCD display start
    GX_DispOn();
    GXS_DispOn();

    // Debug string output
    OS_Printf("ARM9: WM listenOnly demo started.\n");

    // Empty call for getting key input data (strategy for pressing A Button in the IPL)
    KeyRead(&sKey);

    // Main loop
    while (GetState() != STATE_FATAL)
    {
        // Get key input data
        KeyRead(&sKey);

        // Clear the screen
        ClearString();

        // Displays the current state
        switch (GetState())
        {
        case STATE_IDLE:
            PrintString( 14, 0, 6, "IDLE" );
            break;

        case STATE_SCANNING:
            PrintString( 14, 0, 5, "SCAN" );
            break;

        case STATE_FATAL:
            PrintString( 14, 0, 1, "ERROR" );
            break;

        default:
            PrintString( 13, 0, 7, "Busy..." );
            break;

        }

        // Displays the state of each channel
        {
            int i;
            u32 beaconCountAP[13];
            u32 beaconCountDS[13];
            BOOL isNewAP[13];
            BOOL isNewDS[13];
            u32 currentVCount = OS_GetVBlankCount();
            OSIntrMode e;

            MI_CpuClear8(beaconCountAP, sizeof(beaconCountAP));
            MI_CpuClear8(beaconCountDS, sizeof(beaconCountDS));
            MI_CpuClear8(isNewAP, sizeof(isNewAP));
            MI_CpuClear8(isNewDS, sizeof(isNewDS));

            // Use exclusion because data will be touched that is updated in interrupts
            e = OS_DisableInterrupts();
            for (i = 0; i < BEACON_INFO_MAX; i++)
            {
                if (sBeaconInfo[i].timeStamp_found != 0)
                {
                    if (1 <= sBeaconInfo[i].channel && sBeaconInfo[i].channel <= 13)
                    {
                        if ((u32)(currentVCount - sBeaconInfo[i].timeStamp_updated) > 60 * 60)
                        {
                            // Delete entries that were not updated for one minute
                            MI_CpuClear8(&sBeaconInfo[i], sizeof(sBeaconInfo[i]));
                        }
                        else
                        {
                            if (sBeaconInfo[i].isDS)
                            {
                                beaconCountDS[sBeaconInfo[i].channel-1]++;
                            }
                            else
                            {
                                beaconCountAP[sBeaconInfo[i].channel-1]++;
                            }
                            // Emphasize the display if any are found within a second
                            if ((u32)(currentVCount - sBeaconInfo[i].timeStamp_found) < 60 * 1)
                            {
                                if (sBeaconInfo[i].isDS)
                                {
                                    isNewDS[sBeaconInfo[i].channel-1] = TRUE;
                                }
                                else
                                {
                                    isNewAP[sBeaconInfo[i].channel-1] = TRUE;
                                }
                            }
                        }
                    }
                }
            }
            (void)OS_RestoreInterrupts(e);

            for (i = 0; i < 13; i++)
            {
                s16 y = (s16)(5 + i);
                u8 color;
                PrintString( 5, y, 4, "Ch%2d: ", i+1 );
                color = (u8)((beaconCountAP[i] > 0) ? ((isNewAP[i]) ? 1 : 15) : 13);
                PrintString( 5 + 7, y, color, "%3d APs", beaconCountAP[i]);
                color = (u8)((beaconCountDS[i] > 0) ? ((isNewDS[i]) ? 1 : 15) : 13);
                PrintString( 5 + 7 + 8, y, color, "%3d DSs", beaconCountDS[i]);
            }
        }

        // Waiting for the V-Blank
        OS_WaitVBlankIntr();
    }

    OS_Terminate();
}

static void SetState(u32 state)
{
    if (sState != STATE_FATAL)
    {
        sState = state;
    }
}

static inline u32 GetState(void)
{
    return sState;
}

static BOOL FindBeaconInfo(const u8* bssid, BeaconInfo** beaconInfo)
{
    int i, j;
    BeaconInfo* emptyEntry = NULL;

    for (i = 0; i < BEACON_INFO_MAX; i++)
    {
        if (sBeaconInfo[i].timeStamp_found != 0)
        {
            for (j = 0; j < WM_SIZE_BSSID; j++)
            {
                if (bssid[j] != sBeaconInfo[i].bssid[j])
                {
                    break;
                }
            }
            if (j == WM_SIZE_BSSID)
            {
                if (beaconInfo != NULL)
                {
                    *beaconInfo = &sBeaconInfo[i];
                }
                return TRUE;
            }
        }
        else
        {
            emptyEntry = &sBeaconInfo[i];
        }
    }
    if (beaconInfo != NULL)
    {
        *beaconInfo = emptyEntry;
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         InitializeWMForListeningAndStartScan

  Description:  Initializes WM in receive-only mode and begins scanning.

  Arguments:    None.

  Returns:      TRUE if initialization succeeded.
 *---------------------------------------------------------------------------*/
static BOOL InitializeWMForListeningAndStartScan(void)
{
    WMErrCode result;

    SetState( STATE_INITIALIZING );

#ifdef USE_WM_INITIALIZE
    // When using the WM_InitializeForListening function
    result = WM_InitializeForListening(&sWmBuffer, InitializeCb, WIRELESS_DMA_NO,
                                       FALSE /* Do not flash the LED */);
    if (result != WM_ERRCODE_OPERATING)
    {
        OS_TPrintf("WM_Initialize failed: %d\n", result);
        SetState( STATE_FATAL );
        return FALSE;
    }
#else
    // When calling the WM_Init, WM_EnableForListening, and WM_PowerOn functions separately
    result = WM_Init(&sWmBuffer, WIRELESS_DMA_NO);
    if (result != WM_ERRCODE_SUCCESS)
    {
        OS_TPrintf("WM_Init failed: %d\n", result);
        SetState( STATE_FATAL );
        return FALSE;
    }
    result = WM_EnableForListening(EnableCb, FALSE /* Do not flash the LED */);
    if (result != WM_ERRCODE_OPERATING)
    {
        OS_TPrintf("WM_EnableForListening failed: %d\n", result);
        SetState( STATE_FATAL );
        return FALSE;
    }
#endif

    return TRUE;
}

#ifdef USE_WM_INITIALIZE
static void InitializeCb(void *arg)
{
    // State after power-on
    WMCallback *cb = (WMCallback *)arg;

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        OS_TPrintf("WM_Initialize failed: %d\n", cb->errcode);
        SetState( STATE_FATAL );
        return;
    }

    SetState( STATE_IDLE );

    (void)StartScan();

    return;
}
#else
static void EnableCb(void *arg)
{
    // STOP state
    WMCallback *cb = (WMCallback *)arg;
    WMErrCode result;

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        OS_TPrintf("WM_Initialize failed: %d\n", cb->errcode);
        SetState( STATE_FATAL );
        return;
    }

    result = WM_PowerOn(PowerOnCb);
    if (result != WM_ERRCODE_OPERATING)
    {
        OS_TPrintf("WM_PowerOn failed: %d\n", result);
        SetState( STATE_FATAL );
        return;
    }

    return;
}

static void PowerOnCb(void *arg)
{
    // State after power-on
    WMCallback *cb = (WMCallback *)arg;

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        OS_TPrintf("WM_PowerOn failed: %d\n", cb->errcode);
        SetState( STATE_FATAL );
        return;
    }

    SetState( STATE_IDLE );

    (void)StartScan();

    return;
}
#endif // USE_WM_INITIALIZE

static BOOL StartScan(void)
{
    WMErrCode result;
    u16     chanpat;
    static u32 channel = 0;

    chanpat = WM_GetAllowedChannel();

    // Checks if wireless is usable
    if (chanpat == 0x8000)
    {
        // If 0x8000 is returned, it indicates that wireless is not initialized or there is some other abnormality with the wireless library. Therefore, set to an error.
        //  
        OS_TPrintf("WM_GetAllowedChannel returns %08x\n", chanpat);
        SetState( STATE_FATAL );
        return FALSE;
    }
    if (chanpat == 0)
    {
        // In this state, wireless cannot be used
        OS_TPrintf("WM_GetAllowedChannel returns %08x\n", chanpat);
        SetState( STATE_FATAL );
        return FALSE;
    }

    {
        /* Search possible channels in ascending order from the current designation */
        while (TRUE)
        {
            channel++;
            if (channel > 16)
            {
                channel = 1;
            }

            if (chanpat & (0x0001 << (channel - 1)))
            {
                break;
            }
        }
        sScanExParam.channelList = (u16)(1 << (channel - 1));
    }

    sScanExParam.maxChannelTime = WM_GetDispersionScanPeriod();
    sScanExParam.scanBuf = (WMbssDesc *)sScanBuf;
    sScanExParam.scanBufSize = WM_SIZE_SCAN_EX_BUF;
    sScanExParam.scanType = WM_SCANTYPE_PASSIVE;
    sScanExParam.ssidLength = 0;
    MI_CpuFill8(sScanExParam.ssid, 0xFF, WM_SIZE_SSID);
    MI_CpuFill8(sScanExParam.bssid, 0xff, WM_SIZE_BSSID);

    SetState( STATE_SCANNING );

    result = WM_StartScanEx(StartScanExCb, &sScanExParam);

    if (result != WM_ERRCODE_OPERATING)
    {
        OS_TPrintf("WM_StartScanEx failed: %d\n", result);
        SetState( STATE_FATAL );
        return FALSE;
    }

    return TRUE;
}

static void StartScanExCb(void *arg)
{
    WMStartScanExCallback *cb = (WMStartScanExCallback *)arg;
    int i;
    WMErrCode result;

    // If the scan command fails
    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        OS_TPrintf("WM_StartScanEx failed: %d\n", cb->errcode);
        SetState( STATE_FATAL );
        return;
    }

    if (GetState() != STATE_SCANNING)
    {
        // End scan if the state has been changed
        SetState( STATE_TERMINATING_SCAN );
        result = WM_EndScan(EndScanCb);
                OS_TPrintf("WM_EndScan returns %d\n", result);
        if (result != WM_ERRCODE_OPERATING)
        {
            OS_TPrintf("WM_EndScan failed: %d\n", result);
            SetState( STATE_FATAL );
        }
        return;
    }

    switch (cb->state)
    {
    case WM_STATECODE_PARENT_NOT_FOUND:
        break;

    case WM_STATECODE_PARENT_FOUND:
        // If a parent is found

        OS_TPrintf("found %d beacons\n", cb->bssDescCount);

        if ( cb->bssDescCount > 0 )
        {
            // Discards the BssDesc cache that is set in the buffer, because the BssDesc information is written from the ARM7
            //  
            DC_InvalidateRange(&sScanBuf, sizeof(sScanBuf));
        }

        for ( i = 0; i < cb->bssDescCount; i++ )
        {
            WMBssDesc* bd = cb->bssDesc[i];
            BOOL isDS;

            OS_TPrintf(" Ch%2d %02x:%02x:%02x:%02x:%02x:%02x ",
                       bd->channel,
                       bd->bssid[0], bd->bssid[1], bd->bssid[2],
                       bd->bssid[3], bd->bssid[4], bd->bssid[5]);

            // First, you must confirm WMBssDesc.gameInfoLength, and check to see if ggid has a valid value.
            // 
            if (WM_IsValidGameInfo(&bd->gameInfo, bd->gameInfoLength))
            {
                // DS parent device
                OS_TPrintf("GGID=%08x TGID=%04x\n", bd->gameInfo.ggid, bd->gameInfo.tgid);
                isDS = TRUE;
            }
            else
            {
                // AP
                char str[WM_SIZE_SSID+1];
                u32 ssidLength = bd->ssidLength;
                // Basically, we do not trust values that come from the network when we do not know whether the library is filtering for us
                // 
                if (ssidLength > WM_SIZE_SSID)
                {
                    ssidLength = WM_SIZE_SSID;
                }
                MI_CpuCopy8(bd->ssid, str, ssidLength);
                str[ssidLength] = 0;
                OS_TPrintf("SSID=%s\n", str);
                isDS = FALSE;
            }

            // Record the information of beacons that were found
            {
                BOOL found;
                BeaconInfo* beaconInfo;
                // NOTE: This is within a callback function, so do not perform processing that has a heavy cost
                found = FindBeaconInfo(bd->bssid, &beaconInfo);
                if (beaconInfo != NULL)
                {
                    u32 timeStamp = OS_GetVBlankCount();
                    if ( timeStamp == 0 ) { timeStamp = (u32)-1; }
                    if ( !found )
                    {
                        // New entry
                        beaconInfo->timeStamp_found = timeStamp;
                    }
                    beaconInfo->timeStamp_updated = timeStamp;
                    beaconInfo->isDS      = isDS;
                    beaconInfo->channel   = bd->channel;
                    MI_CpuCopy8(bd->bssid, beaconInfo->bssid, WM_SIZE_BSSID);
                    MI_CpuCopy8(bd->ssid,  beaconInfo->ssid,  WM_SIZE_SSID);
                }
                else
                {
                    OS_TPrintf("Too many beacons; cannot record beacon info.\n");
                }
            }
        }

        break;
    }

    // Changes the channel and starts another scan
    (void)StartScan();
}


static void EndScanCb(void* arg)
{
    WMCallback *cb = (WMCallback *)arg;

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        OS_TPrintf("EndScanCb failed: %d\n", cb->errcode);
        SetState( STATE_FATAL );
        return;
    }

    SetState( STATE_IDLE );
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  V-Blank interrupt vector.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void)
{
    // Reflect virtual screen in VRAM
    DC_FlushRange(sScreen, sizeof(sScreen));
    /* I/O register is accessed using DMA operation, so cache wait is not needed */
    // DC_WaitWriteBufferEmpty();
    GX_LoadBG0Scr(sScreen, 0, sizeof(sScreen));

    // Sets the IRQ check flag
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         KeyRead

  Description:  Edits key input data.
                Detects press trigger, release trigger, and press-and-hold repeat.

  Arguments:    pKey: Structure that holds key input data to be edited

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void KeyRead(KeyInfo * pKey)
{
    static u16 repeat_count[12];
    int     i;
    u16     r;

    r = PAD_Read();
    pKey->trg = 0x0000;
    pKey->up = 0x0000;
    pKey->rep = 0x0000;

    for (i = 0; i < 12; i++)
    {
        if (r & (0x0001 << i))
        {
            if (!(pKey->cnt & (0x0001 << i)))
            {
                pKey->trg |= (0x0001 << i);     // Press trigger
                repeat_count[i] = 1;
            }
            else
            {
                if (repeat_count[i] > KEY_REPEAT_START)
                {
                    pKey->rep |= (0x0001 << i); // Press-and-hold repeat
                    repeat_count[i] = KEY_REPEAT_START - KEY_REPEAT_SPAN;
                }
                else
                {
                    repeat_count[i]++;
                }
            }
        }
        else
        {
            if (pKey->cnt & (0x0001 << i))
            {
                pKey->up |= (0x0001 << i);      // Release trigger
            }
        }
    }
    pKey->cnt = r;                     // Unprocessed key input
}

/*---------------------------------------------------------------------------*
  Name:         ClearString

  Description:  Clears the virtual screen.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ClearString(void)
{
    MI_CpuClearFast((void *)sScreen, sizeof(sScreen));
}

/*---------------------------------------------------------------------------*
  Name:         PrintString

  Description:  Positions the text string on the virtual screen. The string can be up to 32 chars.

  Arguments:    x: X-coordinate where character string starts (x 8 dots)
                y: Y-coordinate where character string starts (x 8 dots)
                palette: Specify text color by palette number
                text: Text string to position. Null-terminated.
                ...: Virtual argument

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PrintString(s16 x, s16 y, u8 palette, char *text, ...)
{
    va_list vlist;
    char    temp[32 + 2];
    s32     i;

    va_start(vlist, text);
    (void)vsnprintf(temp, 33, text, vlist);
    va_end(vlist);

    *(u16 *)(&temp[32]) = 0x0000;
    for (i = 0;; i++)
    {
        if (temp[i] == 0x00)
        {
            break;
        }
        sScreen[((y * 32) + x + i) % (32 * 32)] = (u16)((palette << 12) | temp[i]);
    }
}

/*---------------------------------------------------------------------------*
  Name:         ColorString

  Description:  Changes the color of character strings printed on the virtual screen.

  Arguments:    x: X-coordinate (x 8 dots ) from which to start color change
                y: Y-coordinate (x 8 dots ) from which to start color change
                length: Number of characters to continue the color change for
                palette: Specify text color by palette number

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ColorString(s16 x, s16 y, s16 length, u8 palette)
{
    s32     i;
    u16     temp;
    s32     index;

    if (length < 0)
        return;

    for (i = 0; i < length; i++)
    {
        index = ((y * 32) + x + i) % (32 * 32);
        temp = sScreen[index];
        temp &= 0x0fff;
        temp |= (palette << 12);
        sScreen[index] = temp;
    }
}

/*---------------------------------------------------------------------------*
  Name:         InitializeAllocateSystem

  Description:  Initializes the memory allocation system within the main memory arena.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitializeAllocateSystem(void)
{
    void   *tempLo;
    OSHeapHandle hh;

    // Based on the premise that OS_Init has been already called
    tempLo = OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1);
    OS_SetArenaLo(OS_ARENA_MAIN, tempLo);
    hh = OS_CreateHeap(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi());
    if (hh < 0)
    {
        OS_Panic("ARM9: Fail to create heap...\n");
    }
    hh = OS_SetCurrentHeap(OS_ARENA_MAIN, hh);
}

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
