/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WM - demos - mp_simple-1
  File:     main.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-08-28#$
  $Rev: 11026 $
  $Author: tominaga_masafumi $
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    A simple functional sample of wireless communication using the WM library.
    Automatically connects to a nearby "simple" demo and delivers a 4-byte frame count by wireless communication.
    

    HOWTO:
        1. Press the A Button to start communications as a parent.
           When a nearby child with the same "simple" demo is found, communications automatically begins with that child.
           Communication with up to 15 children is possible at the same time.
        2. Press the B Button to start communications as a child.
           When a nearby parent with the same 'simple' demo is found, communication with that parent automatically begins.
        3. If the START button is pressed during a busy screen while a connection is being established, or during a communication screen after a connection is established, the connection is reset, and the display returns to the initial screen.   
        
           
 *---------------------------------------------------------------------------*/

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif
#include    <nitro/wm.h>

#include    "font.h"
#include    "wh.h"

/*---------------------------------------------------------------------------*
    Constant definitions
 *---------------------------------------------------------------------------*/
#define     KEY_REPEAT_START    25     // Number of frames until key repeat starts
#define     KEY_REPEAT_SPAN     10     // Number of frames between key repeats

#define     PICTURE_FRAME_PER_GAME_FRAME    1

#define     DEFAULT_GGID            0x003fff10
#define     DEFAULT_CHAN            1
#define     PARENT_DATA_SIZE        4
#define     CHILD_DATA_SIZE         4

/*---------------------------------------------------------------------------*
    Structure definitions
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


/*---------------------------------------------------------------------------*
    Internal Function Definitions
 *---------------------------------------------------------------------------*/
static void ModeSelect(void);          // Parent/child select screen
static void ModeError(void);           // Error display screen
static void ModeWorking(void);         // Busy screen
static void ModeParent(void);          // Parent communications screen
static void ModeChild(void);           // Child communications screen
static void VBlankIntr(void);          // V-Blank interrupt handler

// Functions called when receiving data
static void MpReceiveCallback(u16 aid, u16 *data, u16 length);

// General purpose subroutines
static void KeyRead(KeyInfo * pKey);
static void ClearString(void);
static void PrintString(s16 x, s16 y, u8 palette, char *text, ...);
static void ColorString(s16 x, s16 y, s16 length, u8 palette);
static void InitializeAllocateSystem(void);


/*---------------------------------------------------------------------------*
    Internal Variable Definitions
 *---------------------------------------------------------------------------*/
static u16 gScreen[32 * 32];           // Virtual screen
static KeyInfo gKey;                   // Key input
static u32 gFrame;                     // Frame counter

// Send/receive buffer for display
static u32 gSendBuf[1024] ATTRIBUTE_ALIGN(32);
static u32 gRecvBuf[1 + WM_NUM_MAX_CHILD][1024];
static BOOL gRecvFlag[1 + WM_NUM_MAX_CHILD];


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
    MI_CpuFillFast((void *)gScreen, 0, sizeof(gScreen));
    DC_FlushRange(gScreen, sizeof(gScreen));
    /* I/O register is accessed using DMA operation, so cache wait is not needed */
    // DC_WaitWriteBufferEmpty();
    GX_LoadBG0Scr(gScreen, 0, sizeof(gScreen));

    // Interrupt settings
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    // Memory allocation
    InitializeAllocateSystem();

    //********************************
    // Wireless settings
    WH_SetGgid(DEFAULT_GGID);
    //********************************

    // LCD display start
    GX_DispOn();
    GXS_DispOn();

    // Debug string output
    OS_Printf("ARM9: WM simple demo started.\n");

    // Empty call for getting key input data (strategy for pressing A Button in the IPL)
    KeyRead(&gKey);

    // Main loop
    for (gFrame = 0; TRUE; gFrame++)
    {
        // Get key input data
        KeyRead(&gKey);

        // Clear the screen
        ClearString();

        // Distributes processes based on communication status
        switch (WH_GetSystemState())
        {
        case WH_SYSSTATE_STOP:
            ModeSelect();
            break;
        case WH_SYSSTATE_IDLE:
        case WH_SYSSTATE_ERROR:
        case WH_SYSSTATE_CONNECT_FAIL:
            // End communication
            WH_Finalize();
            while(WH_GetSystemState()==WH_SYSSTATE_BUSY){}
            (void)WH_End();
            while(WH_GetSystemState()==WH_SYSSTATE_BUSY){}
            break;
        case WH_SYSSTATE_FATAL:
            ModeError();
            break;
        case WH_SYSSTATE_BUSY:
        case WH_SYSSTATE_SCANNING:
            ModeWorking();
            break;
        case WH_SYSSTATE_CONNECTED:
            switch (WH_GetConnectMode())
            {
            case WH_CONNECTMODE_MP_PARENT:
                ModeParent();
                break;
            case WH_CONNECTMODE_MP_CHILD:
                ModeChild();
                break;
            }
            break;
        }

        // Waiting for the V-Blank
        OS_WaitVBlankIntr();
    }
}

/*---------------------------------------------------------------------------*
  Name:         ModeSelect

  Description:  Processing in parent/child selection screen.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ModeSelect(void)
{
    PrintString(3, 10, 0xf, "Push A to connect as PARENT");
    PrintString(3, 12, 0xf, "Push B to connect as CHILD");

    if (gKey.trg & PAD_BUTTON_A)
    {
        s32     i;

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
        WH_SetReceiver(MpReceiveCallback);

        //********************************
        for (i = 1; i < (WM_NUM_MAX_CHILD + 1); i++)
        {
            gRecvFlag[i] = FALSE;
        }
        
        (void)WH_ParentConnect(WH_CONNECTMODE_MP_PARENT, 0x0000, DEFAULT_CHAN);
        //********************************
    }
    else if (gKey.trg & PAD_BUTTON_B)
    {
        static const u8 ANY_PARENT[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

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
        WH_SetReceiver(MpReceiveCallback);

        //********************************
        (void)WH_ChildConnectAuto(WH_CONNECTMODE_MP_CHILD, ANY_PARENT, DEFAULT_CHAN);
        //********************************
    }
}

/*---------------------------------------------------------------------------*
  Name:         ModeError

  Description:  Processing in error display screen.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ModeError(void)
{
    PrintString(5, 10, 0x1, "======= ERROR! =======");
    PrintString(5, 13, 0xf, " Fatal error occured.");
    PrintString(5, 14, 0xf, "Please reboot program.");
}

/*---------------------------------------------------------------------------*
  Name:         ModeWorking

  Description:  Processing in busy screen.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ModeWorking(void)
{
    PrintString(9, 11, 0xf, "Now working...");

    if (gKey.trg & PAD_BUTTON_START)
    {
        //********************************
        // End communication
        WH_Finalize();
        while(WH_GetSystemState()==WH_SYSSTATE_BUSY){}
        (void)WH_End();
        while(WH_GetSystemState()==WH_SYSSTATE_BUSY){}
        //********************************
    }
}

/*---------------------------------------------------------------------------*
  Name:         ModeParent

  Description:  Processing in parent communications screen.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ModeParent(void)
{
    PrintString(8, 1, 0x2, "Parent mode");
    PrintString(4, 3, 0x4, "Send:     %08X", gSendBuf[0]);
    PrintString(4, 5, 0x4, "Receive:");
    {
        s32     i;

        for (i = 1; i < (WM_NUM_MAX_CHILD + 1); i++)
        {
            if (gRecvFlag[i])
            {
                PrintString(5, (s16)(6 + i), 0x4, "Child%02d: %08X", i, gRecvBuf[i][0]);
            }
            else
            {
                PrintString(5, (s16)(6 + i), 0x7, "No child");
            }
        }
    }

    if (gKey.trg & PAD_BUTTON_START)
    {
        //********************************
        // End communication
        WH_Finalize();
        while(WH_GetSystemState()==WH_SYSSTATE_BUSY){}
        (void)WH_End();
        while(WH_GetSystemState()==WH_SYSSTATE_BUSY){}
        //********************************
    }
}

/*---------------------------------------------------------------------------*
  Name:         ModeChild

  Description:  Processing in child communications screen.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ModeChild(void)
{
    PrintString(8, 1, 0x2, "Child mode");
    PrintString(4, 3, 0x4, "Send:     %08X", gSendBuf[0]);
    PrintString(4, 5, 0x4, "Receive:");
    PrintString(5, 7, 0x4, "Parent:  %08X", gRecvBuf[0][0]);

    if (gKey.trg & PAD_BUTTON_START)
    {
        //********************************
        // End communication
        WH_Finalize();
        while(WH_GetSystemState()==WH_SYSSTATE_BUSY){}
        (void)WH_End();
        while(WH_GetSystemState()==WH_SYSSTATE_BUSY){}
        //********************************
    }
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  V-Blank interrupt vector.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void)
{
    if( WH_GetSystemState() == WH_SYSSTATE_CONNECTED )
    {
        // Start sending new data
        switch (WH_GetConnectMode())
        {
        case WH_CONNECTMODE_MP_PARENT:
            gSendBuf[0] = gFrame;
            //********************************
            (void)WH_SendData(gSendBuf, PARENT_DATA_SIZE, NULL);
            //********************************
            break;
        case WH_CONNECTMODE_MP_CHILD:
            gSendBuf[0] = gFrame;
            //********************************
            (void)WH_SendData(gSendBuf, CHILD_DATA_SIZE, NULL);
            //********************************
            break;
        }
    }

    // Reflect virtual screen in VRAM
    DC_FlushRange(gScreen, sizeof(gScreen));
    /* I/O register is accessed using DMA operation, so cache wait is not needed */
    // DC_WaitWriteBufferEmpty();
    GX_LoadBG0Scr(gScreen, 0, sizeof(gScreen));

    // Sets the IRQ check flag
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         MpReceiveCallback

  Description:  Function called when MP data is received.

  Arguments:    aid: AID of child originating transmission (if 0, then data is from parent)
                data: Pointer to received data (NULL is disconnection notification)
                length: Size of received data

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MpReceiveCallback(u16 aid, u16 *data, u16 length)
{
#pragma unused( length )
    SDK_MAX_ASSERT(aid, 15);

    if (data != NULL)
    {
        SDK_ASSERT(length >= 4);
        gRecvFlag[aid] = TRUE;
        // Copy source is aligned to 2 bytes (not 4 bytes)
        if (aid == 0)
        {
            // When receiving from a parent
            MI_CpuCopy8(data, &(gRecvBuf[aid][0]), PARENT_DATA_SIZE);
        }
        else
        {
            MI_CpuCopy8(data, &(gRecvBuf[aid][0]), CHILD_DATA_SIZE);
        }
    }
    else
    {
        gRecvFlag[aid] = FALSE;
    }
}

/*---------------------------------------------------------------------------*
  Name:         KeyRead

  Description:  Edits key input data
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
    MI_CpuClearFast((void *)gScreen, sizeof(gScreen));
}

/*---------------------------------------------------------------------------*
  Name:         PrintString

  Description:  Positions the text string on the virtual screen. The string can be up to 32 chars.

  Arguments:    x: X-coordinate where character string starts (x 8 pixels).
                y: Y-coordinate where character string starts (x 8 pixels).
                palette: Specify text color by palette number.
                text: Text string to position. Null-terminated.
                ...: Virtual argument.

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
        gScreen[((y * 32) + x + i) % (32 * 32)] = (u16)((palette << 12) | temp[i]);
    }
}

/*---------------------------------------------------------------------------*
  Name:         ColorString

  Description:  Changes the color of character strings printed on the virtual screen.

  Arguments:    x:         X-coordinate (x 8 pixels) from which to start color change.
                y:         Y-coordinate (x 8 pixels) from which to start color change.
                length:   Number of characters to continue the color change for.
                palette: Specify text color by palette number.

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
        temp = gScreen[index];
        temp &= 0x0fff;
        temp |= (palette << 12);
        gScreen[index] = temp;
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
