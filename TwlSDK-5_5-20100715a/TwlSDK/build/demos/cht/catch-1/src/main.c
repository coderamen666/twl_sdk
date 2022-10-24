/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CHT - demos - catch-1
  File:     main.c

  Copyright 2003-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-08-31#$
  $Rev: 11030 $
  $Author: tominaga_masafumi $
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    This demo uses the PictoChat search feature to detect the beacon from the PictoChat parent.
 *---------------------------------------------------------------------------*/

/*
 * This demo uses the WH library for wireless communications, but does not perform wireless shutdown processing.
 * 
 * For details on WH library wireless shutdown processing, see the comments at the top of the WH library source code or the wm/dataShare-Model demo.
 * 
 * 
 */

#include    <nitro.h>
#include    <nitro/wm.h>
#include    <nitro/cht.h>

#include    "font.h"
#include    "icon.h"
#include    "wh.h"

/*---------------------------------------------------------------------------*
    Constant Definitions
 *---------------------------------------------------------------------------*/
#define     KEY_REPEAT_START    25     // Number of frames until key repeat starts
#define     KEY_REPEAT_SPAN     10     // Number of frames between key repeats

#define     DEFAULT_GGID        0x003fff61

#define     PICTO_CATCH_LIFETIME    300 // 5 seconds

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

typedef struct PictoCatchInfo
{
    int     lifeTime;
    int     clientNum;

}
PictoCatchInfo;


/*---------------------------------------------------------------------------*
    Internal Function Definitions
 *---------------------------------------------------------------------------*/
// Parent discovery callback
static void FoundParentCallback(WMBssDesc *pBssDesc);

// V-Blank interrupt handler
static void VBlankIntr(void);

// General purpose subroutines
static void KeyRead(KeyInfo * pKey);
static void ClearString(void);
static void PrintString(s16 x, s16 y, u8 palette, char *text, ...);
static void ColorString(s16 x, s16 y, s16 length, u8 palette);
static void InitializeAllocateSystem(void);
static void DrawIcon(u8 index, int charName, int x, int y);


/*---------------------------------------------------------------------------*
    Internal Variable Definitions
 *---------------------------------------------------------------------------*/
static u16 gScreen[32 * 32] ATTRIBUTE_ALIGN(32);
static GXOamAttr gOam[128] ATTRIBUTE_ALIGN(32);
static KeyInfo gKey;                   // Key input
static s32 gFrame;                     // Frame counter

static PictoCatchInfo gRoom[4];


/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Initialization and main loop.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
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
    GX_SetBankForOBJ(GX_VRAM_OBJ_16_F);
    GX_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_1D_32K);
    G2_SetBG0Priority(0);
    G2_BG0Mosaic(FALSE);
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D);
    GX_SetVisiblePlane(GX_PLANEMASK_BG0 | GX_PLANEMASK_OBJ);
    GX_LoadBG0Char(d_CharData, 0, sizeof(d_CharData));
    GX_LoadBGPltt(d_PaletteData, 0, sizeof(d_PaletteData));
    GX_LoadOBJ(icons_character, 0, sizeof(icons_character));
    GX_LoadOBJPltt(icons_palette, 0, sizeof(icons_palette));
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
    // Initialize wireless
    if (!WH_Initialize())
    {
        OS_TPanic("WH_Initialize failed.\n");
    }
    WH_SetGgid(DEFAULT_GGID);
    WH_TurnOnPictoCatch();
    {
        const u8 mac[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
        s32     i;

        for (i = 0; i < 4; i++)
        {
            gRoom[i].lifeTime = 0;
        }
        // Wait for initialization to complete
        while (WH_GetSystemState() != WH_SYSSTATE_IDLE)
        {
        }
        // Start search for parent
        if (!WH_StartScan(FoundParentCallback, mac, 0))
        {
            OS_TPanic("WH_StartScan failed.\n");
        }
    }
    //********************************

    // LCD display start
    GX_DispOn();
    GXS_DispOn();

    // Debug string output
    OS_Printf("ARM9: CHT catch-1 demo started.\n");

    // Empty call for getting key input data (strategy for pressing A Button in the IPL)
    KeyRead(&gKey);

    // Main loop
    for (gFrame = 0; TRUE; gFrame++)
    {
        // Get key input data
        KeyRead(&gKey);

        // Clear the screen
        ClearString();

        // Display
        PrintString(1, 1, 0xf, "frame: %d", gFrame);
        {
            s32     i;

            for (i = 0; i < 4; i++)
            {
                if (gRoom[i].lifeTime > 0)
                {
                    gRoom[i].lifeTime--;
                    PrintString(1, (s16)(6 + (3 * i)), 0xf, "Discover pictochat room%d", i);
                    PrintString(2, (s16)(7 + (3 * i)), 0x2, "%d members", gRoom[i].clientNum);
                    // Display icon
                    DrawIcon((u8)i, 9, 208, (s16)(48 + (24 * i)));
                }
                else
                {
                    PrintString(10, (s16)(6 + (3 * i)), 0xe, "pictochat room%d", i);
                }
            }
        }

        // Waiting for the V-Blank
        OS_WaitVBlankIntr();
        // Output ARM7 debug print
    }
}

/*---------------------------------------------------------------------------*
  Name:         FoundParentCallback

  Description:  The callback function that is invoked when a parent is found.

  Arguments:    pBssDesc:      Pointer to beacon information of found parent.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void FoundParentCallback(WMBssDesc *pBssDesc)
{
    if (CHT_IsPictochatParent(pBssDesc))
    {
        int     clientNum;
        int     roomNumber;

        clientNum = CHT_GetPictochatClientNum(pBssDesc);
        roomNumber = CHT_GetPictochatRoomNumber(pBssDesc);
        if (roomNumber < 4)
        {
            if (gRoom[roomNumber].lifeTime == 0)
            {
                /* SE should start here */
            }
            gRoom[roomNumber].clientNum = clientNum;
            gRoom[roomNumber].lifeTime = PICTO_CATCH_LIFETIME;
        }
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
    // Reflect virtual screen in VRAM
    DC_FlushRange(gScreen, sizeof(gScreen));
    /* I/O register is accessed using DMA operation, so cache wait is not needed */
    // DC_WaitWriteBufferEmpty();
    GX_LoadBG0Scr(gScreen, 0, sizeof(gScreen));

    // Reflect virtual OAM in OAM
    DC_FlushRange(gOam, sizeof(gOam));
    /* I/O register is accessed using DMA operation, so cache wait is not needed */
    // DC_WaitWriteBufferEmpty();
    GX_LoadOAM(gOam, 0, sizeof(gOam));

    // Sets the IRQ check flag
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
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
    s32     i;

    MI_CpuClearFast((void *)gScreen, sizeof(gScreen));
    for (i = 0; i < 128; i++)
    {
        G2_SetOBJPosition(&gOam[i], 256, 192);
    }
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
        gScreen[((y * 32) + x + i) % (32 * 32)] = (u16)((palette << 12) | temp[i]);
    }
}

/*---------------------------------------------------------------------------*
  Name:         ColorString

  Description:  Changes the color of character strings printed on the virtual screen.

  Arguments:    x:         X-coordinate (x 8 dots ) from which to start color change.
                y:         Y-coordinate (x 8 dots ) from which to start color change.
                length:   Number of characters to continue the color change for.
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
        temp = gScreen[index];
        temp &= 0x0fff;
        temp |= (palette << 12);
        gScreen[index] = temp;
    }
}

/*---------------------------------------------------------------------------*
  Name:         DrawIcon

  Description:  Display icon.

  Arguments:    index:        Virtual OAM array index.
                charName:    Icon number.
                                0 - 3 : Icons of signal strength against white background.
                                4 - 7 : Icons of signal strength against black background.
                                8      : Communications icon
                                9      : PictoChat icon
                x:            X-coordinate position in dots
                y:             Y-coordinate position in dots

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DrawIcon(u8 index, int charName, int x, int y)
{
    G2_SetOBJAttr(&gOam[index],        // Pointer to OAM structure to edit
                  x,                   // X position
                  y,                   // Y position
                  0,                   // Highest display priority
                  GX_OAM_MODE_NORMAL,  // Normal OBJ
                  FALSE,               // No mosaic
                  GX_OAM_EFFECT_NONE,  // No effects
                  GX_OAM_SHAPE_16x16,  // 2 x 2 character
                  GX_OAM_COLORMODE_16, // 16-color palette
                  charName * 4,        // Character number
                  0,                   // Palette number 0
                  0                    // Affine transformation index (disabled)
        );
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
