/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - g2d
  File:     g2d_Screen.c

  Copyright 2004-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1323 $
 *---------------------------------------------------------------------------*/

#include <nnsys/g2d/g2d_Screen.h>
#include <nnsys/g2d/g2d_config.h>
#include "g2di_Dma.h"
#include "g2di_BGManipulator.h"


#define BG_MODE_WARNING 8

// Size of the extended palette slot
#define NNS_G2D_BGEXTPLTT_SLOTSIZE  0x2000

//---- Unit plane size for the text background (in characters)
#define PLANE_WIDTH     32
#define PLANE_HEIGHT    32


// Enumerator for showing the background extended palette slot
typedef enum NNSG2dBGExtPlttSlot
{
    NNS_G2D_BGEXTPLTTSLOT_MAIN0,
    NNS_G2D_BGEXTPLTTSLOT_MAIN1,
    NNS_G2D_BGEXTPLTTSLOT_MAIN2,
    NNS_G2D_BGEXTPLTTSLOT_MAIN3,
    NNS_G2D_BGEXTPLTTSLOT_SUB0,
    NNS_G2D_BGEXTPLTTSLOT_SUB1,
    NNS_G2D_BGEXTPLTTSLOT_SUB2,
    NNS_G2D_BGEXTPLTTSLOT_SUB3

} NNSG2dBGExtPlttSlot;


//---- Screen size value -> for conversion to register value
typedef struct ScreenSizeMap
{
    u16 width;
    u16 height;
    u16 scnSize;
}
ScreenSizeMap;


// Screen size table for the text background
static const ScreenSizeMap sTextScnSize[4] =
{
    {256, 256, GX_BG_SCRSIZE_TEXT_256x256},
    {256, 512, GX_BG_SCRSIZE_TEXT_256x512},
    {512, 256, GX_BG_SCRSIZE_TEXT_512x256},
    {512, 512, GX_BG_SCRSIZE_TEXT_512x512},
};

// Screen size table for the affine background
static const ScreenSizeMap sAffineScnSize[4] =
{
    {128, 128, GX_BG_SCRSIZE_AFFINE_128x128},
    {256, 256, GX_BG_SCRSIZE_AFFINE_256x256},
    {512, 512, GX_BG_SCRSIZE_AFFINE_512x512},
    {1024, 1024, GX_BG_SCRSIZE_AFFINE_1024x1024},
};

// Screen size table for the affine extended background
static const ScreenSizeMap sAffineExtScnSize[4] =
{
    {128, 128, GX_BG_SCRSIZE_256x16PLTT_128x128},
    {256, 256, GX_BG_SCRSIZE_256x16PLTT_256x256},
    {512, 512, GX_BG_SCRSIZE_256x16PLTT_512x512},
    {1024, 1024, GX_BG_SCRSIZE_256x16PLTT_1024x1024},
};


static const u8 sBGTextModeTable[4][8] =
{
    {
        /* 0 -> */ GX_BGMODE_0,
        /* 1 -> */ GX_BGMODE_1,
        /* 2 -> */ GX_BGMODE_2,
        /* 3 -> */ GX_BGMODE_3,
        /* 4 -> */ GX_BGMODE_4,
        /* 5 -> */ GX_BGMODE_5,
        /* 6 -> */ GX_BGMODE_6,
        /* 7 -> */ BG_MODE_WARNING,                             // No configuration permitted
    },
    {
        /* 0 -> */ GX_BGMODE_0,
        /* 1 -> */ GX_BGMODE_1,
        /* 2 -> */ GX_BGMODE_2,
        /* 3 -> */ GX_BGMODE_3,
        /* 4 -> */ GX_BGMODE_4,
        /* 5 -> */ GX_BGMODE_5,
        /* 6 -> */ GX_BGMODE_6,
        /* 7 -> */ BG_MODE_WARNING,                             // No configuration permitted
    },
    {
        /* 0 -> */ GX_BGMODE_0,
        /* 1 -> */ GX_BGMODE_1,
        /* 2 -> */ GX_BGMODE_1,
        /* 3 -> */ GX_BGMODE_3,
        /* 4 -> */ GX_BGMODE_3,
        /* 5 -> */ GX_BGMODE_3,
        /* 6 -> */ GX_BGMODE_0,
        /* 7 -> */ (GXBGMode)(GX_BGMODE_0 + BG_MODE_WARNING),   // No configuration permitted
    },
    {
        /* 0 -> */ GX_BGMODE_0,
        /* 1 -> */ GX_BGMODE_0,
        /* 2 -> */ (GXBGMode)(GX_BGMODE_0 + BG_MODE_WARNING),   // BG2: affine      -> text
        /* 3 -> */ GX_BGMODE_0,
        /* 4 -> */ (GXBGMode)(GX_BGMODE_0 + BG_MODE_WARNING),   // BG2: affine      -> text
        /* 5 -> */ (GXBGMode)(GX_BGMODE_0 + BG_MODE_WARNING),   // BG2: affine extension     -> text
        /* 6 -> */ (GXBGMode)(GX_BGMODE_0 + BG_MODE_WARNING),   // BG2: large screen BMP    -> text
        /* 7 -> */ (GXBGMode)(GX_BGMODE_0 + BG_MODE_WARNING),   // No configuration permitted
    }
};


// Background mode conversion table for when changing BG2 to affine background
static const u8 sBGAffineModeTable[2][8] =
{
    {
        /* 0 -> */ (GXBGMode)(GX_BGMODE_2 + BG_MODE_WARNING),   // BG2: text  -> affine
        /* 1 -> */ GX_BGMODE_2,
        /* 2 -> */ GX_BGMODE_2,
        /* 3 -> */ GX_BGMODE_4,
        /* 4 -> */ GX_BGMODE_4,
        /* 5 -> */ GX_BGMODE_4,
        /* 6 -> */ GX_BGMODE_4,
        /* 7 -> */ (GXBGMode)(GX_BGMODE_2 + BG_MODE_WARNING),   // No configuration permitted
    },
    {
        /* 0 -> */ GX_BGMODE_1,
        /* 1 -> */ GX_BGMODE_1,
        /* 2 -> */ GX_BGMODE_2,
        /* 3 -> */ GX_BGMODE_1,
        /* 4 -> */ GX_BGMODE_2,
        /* 5 -> */ (GXBGMode)(GX_BGMODE_1 + BG_MODE_WARNING),   // BG2: affine extension     -> text
        /* 6 -> */ (GXBGMode)(GX_BGMODE_1 + BG_MODE_WARNING),   // BG2: large screen   -> text
        /* 7 -> */ (GXBGMode)(GX_BGMODE_1 + BG_MODE_WARNING),   // No configuration permitted
    }
};
static const u8 sBG256x16PlttModeTable[2][8] =
{
    {
        /* 0 -> */ (GXBGMode)(GX_BGMODE_5 + BG_MODE_WARNING),   // BG3: text -> affine extended
        /* 1 -> */ (GXBGMode)(GX_BGMODE_5 + BG_MODE_WARNING),   // BG3: affine -> affine extended
        /* 2 -> */ (GXBGMode)(GX_BGMODE_5 + BG_MODE_WARNING),   // BG3: affine -> affine extended
        /* 3 -> */ GX_BGMODE_5,
        /* 4 -> */ GX_BGMODE_5,
        /* 5 -> */ GX_BGMODE_5,
        /* 6 -> */ GX_BGMODE_5,
        /* 7 -> */ (GXBGMode)(GX_BGMODE_5 + BG_MODE_WARNING),   // No configuration permitted
    },
    {
        /* 0 -> */ GX_BGMODE_3,
        /* 1 -> */ GX_BGMODE_3,
        /* 2 -> */ GX_BGMODE_4,
        /* 3 -> */ GX_BGMODE_3,
        /* 4 -> */ GX_BGMODE_4,
        /* 5 -> */ GX_BGMODE_5,
        /* 6 -> */ (GXBGMode)(GX_BGMODE_3 + BG_MODE_WARNING),   // BG2: large screen   -> text
        /* 7 -> */ (GXBGMode)(GX_BGMODE_3 + BG_MODE_WARNING),   // no configuration permitted
    }
};



GXBGAreaOver NNSi_G2dBGAreaOver = GX_BG_AREAOVER_XLU;



//****************************************************************************
// Static functions
//****************************************************************************

//----------------------------------------------------------------------------
// Inline functions
//----------------------------------------------------------------------------

/*---------------------------------------------------------------------------*
  Name:         IsMainBGExtPlttSlot

  Description:  Determines whether the NNSG2dBGExtPlttSlot-type value represents the main screen's extended palette slot.
                

  Arguments:    slot: Target

  Returns:      TRUE if the main screen extended slot is shown.
 *---------------------------------------------------------------------------*/
static NNS_G2D_INLINE BOOL IsMainBGExtPlttSlot(NNSG2dBGExtPlttSlot slot)
{
    return (slot <= NNS_G2D_BGEXTPLTTSLOT_MAIN3);
}



/*---------------------------------------------------------------------------*
  Name:         CalcTextScreenOffset

  Description:  Gets the offset from the screen base whose position is specified by the text background screen.
                

  Arguments:    x: Specified position x (in characters)
                y: Specified position y (in characters)
                w: Target background screen width (in characters)
                h: Target background screen height (in characters)

  Returns:      Specified position offset (in characters).
 *---------------------------------------------------------------------------*/
static NNS_G2D_INLINE int CalcTextScreenOffset(int x, int y, int w, int h)
{
    const int x_blk  = x / PLANE_WIDTH;
    const int y_blk  = y / PLANE_HEIGHT;
    const int x_char = x % PLANE_WIDTH;
    const int y_char = y % PLANE_HEIGHT;
    const int w_blk  = w / PLANE_WIDTH;
    const int h_blk  = h / PLANE_WIDTH;
    const int blk_w  = (x_blk == w_blk) ? (w % PLANE_WIDTH):  PLANE_WIDTH;
    const int blk_h  = (y_blk == h_blk) ? (h % PLANE_HEIGHT): PLANE_HEIGHT;

    return
        w * PLANE_HEIGHT * y_blk
        + PLANE_WIDTH * blk_h * x_blk
        + blk_w * y_char
        + x_char;
}



/*---------------------------------------------------------------------------*
  Name:         GetCompressedPlttOriginalIndex

  Description:  Gets the offset of the specified palette before compression.

  Arguments:    pCmpInfo: Pointer to the palette compression data
                idx: Palette number

  Returns:      Palette offset (in bytes).
 *---------------------------------------------------------------------------*/
static NNS_G2D_INLINE u32 GetCompressedPlttOriginalIndex(
    const NNSG2dPaletteCompressInfo* pCmpInfo,
    int idx
)
{
    NNS_G2D_POINTER_ASSERT( pCmpInfo );

    return ( (u16*)(pCmpInfo->pPlttIdxTbl) )[idx];
}



/*---------------------------------------------------------------------------*
  Name:         GetPlttSize

  Description:  Gets the size per palette of the palette data.

  Arguments:    pPltData: Pointer to the palette data

  Returns:      Palette size (in bytes).
 *---------------------------------------------------------------------------*/
static NNS_G2D_INLINE u32 GetPlttSize(const NNSG2dPaletteData* pPltData)
{
    NNS_G2D_POINTER_ASSERT( pPltData );

    switch( pPltData->fmt )
    {
    case GX_TEXFMT_PLTT16:     return sizeof(GXBGPltt16);
    case GX_TEXFMT_PLTT256:    return sizeof(GXBGPltt256);
    default:                    SDK_ASSERTMSG(FALSE, "invalid NNSG2dPaletteData");
    }

    return 0;
}






//----------------------------------------------------------------------------
// Ordinary functions
//----------------------------------------------------------------------------

/*---------------------------------------------------------------------------*
  Name:         SelectScnSize

  Description:  Converts the screen size by accessing the table received.
                Returns the smallest background size larger than the specified screen size.
                Returns the maximum background size when a background larger than the specified screen size does not exit.
                

  Arguments:    tbl: Screen size conversion table
                w: Screen width in pixels
                h: Screen height in pixels

  Returns:      Pointer to the supported screen size data.
 *---------------------------------------------------------------------------*/
static const ScreenSizeMap* SelectScnSize(const ScreenSizeMap tbl[4], int w, int h)
{
    int i;
    SDK_NULL_ASSERT(tbl);

    for( i = 0; i < 4; i++ )
    {
        if( w <= tbl[i].width && h <= tbl[i].height )
        {
            return &tbl[i];
        }
    }
    return &tbl[3];
}



/*---------------------------------------------------------------------------*
  Name:         ChangeBGModeByTable*

  Description:  Switches the background mode according to a table.

  Arguments:    modeTable: Mode switching table

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ChangeBGModeByTableMain(const u8 modeTable[])
{
    GXBGMode mode = (GXBGMode)modeTable[GetBGModeMain()];
    GXBG0As bg0as = IsBG03D() ? GX_BG0_AS_3D: GX_BG0_AS_2D;
    SDK_NULL_ASSERT(modeTable);

    // Warns of mode changes that could negatively affect other backgrounds
    if( mode >= BG_MODE_WARNING )
    {
        mode -= BG_MODE_WARNING;
        OS_Warning("Dangerous Main BG mode change: %d => %d", GetBGModeMain(), mode);
    }

    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, mode, bg0as);
}

static void ChangeBGModeByTableSub(const u8 modeTable[])
{
    GXBGMode mode = (GXBGMode)modeTable[GetBGModeSub()];
    SDK_NULL_ASSERT(modeTable);

    // Warns of mode changes that could negatively affect other backgrounds
    if( mode >= BG_MODE_WARNING )
    {
        mode -= BG_MODE_WARNING;
        OS_Warning("Dangerous Sub BG mode change: %d => %d", GetBGModeSub(), mode);
    }

    GXS_SetGraphicsMode(mode);
}



/*---------------------------------------------------------------------------*
  Name:         LoadBGPlttToExtendedPltt

  Description:  Reads palette data into the specified extended palette slot.

  Arguments:    slot: Extended palette slot where the palette data gets read to
                pPltData: Pointer to the palette data

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void LoadBGPlttToExtendedPltt(
    NNSG2dBGExtPlttSlot slot,
    const NNSG2dPaletteData* pPltData,
    const NNSG2dPaletteCompressInfo* pCmpInfo
)
{
    void (*prepairLoad)(void);
    void (*cleanupLoad)(void);
    void (*loader)(const void*, u32, u32);
    u32 offset = (u32)( slot * NNS_G2D_BGEXTPLTT_SLOTSIZE );
    SDK_NULL_ASSERT(pPltData);

    // Cache flush
    DC_FlushRange(pPltData->pRawData, pPltData->szByte);

    if( IsMainBGExtPlttSlot(slot) )
    {
        prepairLoad = GX_BeginLoadBGExtPltt;
        cleanupLoad = GX_EndLoadBGExtPltt;
        loader      = GX_LoadBGExtPltt;
    }
    else
    {
        offset -= NNS_G2D_BGEXTPLTT_SLOTSIZE * NNS_G2D_BGEXTPLTTSLOT_SUB0;

        prepairLoad = GXS_BeginLoadBGExtPltt;
        cleanupLoad = GXS_EndLoadBGExtPltt;
        loader      = GXS_LoadBGExtPltt;
    }

    if( pCmpInfo != NULL )
    {
        const u32 szOnePltt = GetPlttSize( pPltData );
        const int numIdx = pCmpInfo->numPalette;
        int i;

        for( i = 0; i < numIdx; i++ )
        {
        	const u32 offsetAddr = GetCompressedPlttOriginalIndex(pCmpInfo, i) * szOnePltt;
            const void* pSrc = (u8*)pPltData->pRawData + szOnePltt * i;

            prepairLoad();
            loader(pSrc, offset + offsetAddr, szOnePltt);
            cleanupLoad();
        }
    }
    else
    {
        prepairLoad();
        loader(pPltData->pRawData, offset, pPltData->szByte);
        cleanupLoad();
    }
}



/*---------------------------------------------------------------------------*
  Name:         LoadBGPlttToNormalPltt

  Description:  Reads palette data into the standard palette.

  Arguments:    bMainDisplay: Background screen to be read into
                pPltData: Pointer to the palette data

  Returns:
 *---------------------------------------------------------------------------*/
static void LoadBGPlttToNormalPltt(
    NNSG2dBGSelect bg,
    const NNSG2dPaletteData* pPltData,
    const NNSG2dPaletteCompressInfo* pCmpInfo
)
{
    u8* pDst;
    SDK_NULL_ASSERT(pPltData);

    // Cache flush
    DC_FlushRange(pPltData->pRawData, pPltData->szByte);

    pDst = (u8*)( IsMainBG(bg) ? HW_BG_PLTT: HW_DB_BG_PLTT );

    if( pCmpInfo != NULL )
    {
        const u32 szOnePltt = GetPlttSize( pPltData );
        const int numIdx = pCmpInfo->numPalette;
        int i;

        for( i = 0; i < numIdx; i++ )
        {
        	const u32 offsetAddr = GetCompressedPlttOriginalIndex(pCmpInfo, i) * szOnePltt;
            const void* pSrc = (u8*)pPltData->pRawData + szOnePltt * i;

            NNSi_G2dDmaCopy16(NNS_G2D_DMA_NO, pSrc, pDst + offsetAddr, szOnePltt);
        }
    }
    else
    {
        NNSi_G2dDmaCopy16(NNS_G2D_DMA_NO, pPltData->pRawData, pDst, pPltData->szByte);
    }
}



/*---------------------------------------------------------------------------*
  Name:         GetBGExtPlttSlot

  Description:  Gets the extended palette slot used by the specified background.

  Arguments:    bg: Background that uses the extended palette slot number to get

  Returns:      Extended palette slot number used by the background.
 *---------------------------------------------------------------------------*/
static NNSG2dBGExtPlttSlot GetBGExtPlttSlot(NNSG2dBGSelect bg)
{
    // NNSG2dBGSelect => BGxCNT register offset conversion table
    static const u16 addrTable[] =
    {
        REG_BG0CNT_OFFSET,
        REG_BG1CNT_OFFSET,
        0,                      // The slot to be used is fixed
        0,                      // The slot to be used is fixed
        REG_DB_BG0CNT_OFFSET,
        REG_DB_BG1CNT_OFFSET,
        0,                      // The slot to be used is fixed
        0                       // The slot to be used is fixed
    };
    u32 addr;
    NNSG2dBGExtPlttSlot slot = (NNSG2dBGExtPlttSlot)bg;
        // The basic rule is, (int)slot == (int)bg

    NNS_G2D_BG_ASSERT(bg);
    addr = addrTable[bg];

    if( addr != 0 )
    {
        addr += HW_REG_BASE;

        if( (*(u16*)addr & REG_G2_BG0CNT_BGPLTTSLOT_MASK) != 0 )
        {
            // If the BGPLTTSLOT bit is up
            // (int)slot == (int)bg + 2
            slot += 2;
        }
    }

    return slot;
}



/*---------------------------------------------------------------------------*
  Name:         SetBGnControlTo*

  Description:  Sets the BGxCNT register and changes the background mode.

  Arguments:    n: Target background screen
                size: Screen size
                cmode: Color mode
                areaOver: Area over processing
                scnBase: Background screen base block
                chrBase: Background character base block

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SetBGnControlToText(
    NNSG2dBGSelect n,
    GXBGScrSizeText size,
    GXBGColorMode cmode,
    GXBGScrBase scnBase,
    GXBGCharBase chrBase
)
{
    const int bgNo = GetBGNo(n);
    GXBGExtPltt extPltt = GX_BG_EXTPLTT_01;
    if( IsMainBG(n) )
    {
        if( ! IsMainBGExtPltt01Available() ) extPltt = GX_BG_EXTPLTT_23;
        ChangeBGModeByTableMain(sBGTextModeTable[bgNo]);
    }
    else
    {
        ChangeBGModeByTableSub(sBGTextModeTable[bgNo]);
    }
    SetBGnControlText(n, size, cmode, scnBase, chrBase, extPltt);
}
static void SetBGnControlToAffine(
    NNSG2dBGSelect n,
    GXBGScrSizeAffine size,
    GXBGAreaOver areaOver,
    GXBGScrBase scnBase,
    GXBGCharBase chrBase
)
{
    if( IsMainBG(n) )
    {
        ChangeBGModeByTableMain(sBGAffineModeTable[n-2]);
    }
    else
    {
        ChangeBGModeByTableSub(sBGAffineModeTable[n-6]);
    }
    SetBGnControlAffine(n, size, areaOver, scnBase, chrBase);
}
static void SetBGnControlTo256x16Pltt(
    NNSG2dBGSelect n,
    GXBGScrSize256x16Pltt size,
    GXBGAreaOver areaOver,
    GXBGScrBase scnBase,
    GXBGCharBase chrBase
)
{
    if( IsMainBG(n) )
    {
        ChangeBGModeByTableMain(sBG256x16PlttModeTable[n-2]);
    }
    else
    {
        ChangeBGModeByTableSub(sBG256x16PlttModeTable[n-6]);
    }
    SetBGnControl256x16Pltt(n, size, areaOver, scnBase, chrBase);
}



/*---------------------------------------------------------------------------*
  Name:         BGAutoControlText

  Description:  Sets the BGxCNT register as text background in conjunction with screen data.
                Also changes the background mode at the same time.

  Arguments:    bg: Control target background
                pScnData: Pointer to the screen data
                scnBase: Background screen base
                chrBase: Background character base

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SetBGControlText(
    NNSG2dBGSelect bg,
    GXBGColorMode colorMode,
    int screenWidth,
    int screenHeight,
    GXBGScrBase scnBase,
    GXBGCharBase chrBase
)
{
    const ScreenSizeMap* pSizeMap;

    NNS_G2D_BG_ASSERT(bg);

    pSizeMap = SelectScnSize(sTextScnSize, screenWidth, screenHeight);
    SDK_ASSERTMSG( pSizeMap != NULL,
        "Unsupported screen size(%dx%d) in input screen data",
        screenWidth,
        screenHeight
    );

    SetBGnControlToText(bg, (GXBGScrSizeText)pSizeMap->scnSize, colorMode, scnBase, chrBase);
}



/*---------------------------------------------------------------------------*
  Name:         BGAutoControlAffine

  Description:  Sets the BGxCNT register as affine background in conjunction with screen data.
                Also changes the background mode at the same time.

  Arguments:    bg: Control target background
                pScnData: Pointer to the screen data
                scnBase: Background screen base
                chrBase: Background character base

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SetBGControlAffine(
    NNSG2dBGSelect bg,
    int screenWidth,
    int screenHeight,
    GXBGScrBase scnBase,
    GXBGCharBase chrBase
)
{
    const ScreenSizeMap* pSizeMap;

    NNS_G2D_BG_ASSERT(bg);
    SDK_ASSERTMSG( (bg % 4) >= 2,
        "You can not show affine BG on %s BG %d",
        (IsMainBG(bg) ? "Main": "Sub"),
        (IsMainBG(bg) ? bg: bg - NNS_G2D_BGSELECT_SUB0) );

    pSizeMap = SelectScnSize(sAffineScnSize, screenWidth, screenHeight);
    SDK_ASSERTMSG( pSizeMap != NULL,
        "Unsupported screen size(%dx%d) in input screen data",
        screenWidth,
        screenHeight
    );

    SetBGnControlToAffine(bg, (GXBGScrSizeAffine)pSizeMap->scnSize, NNSi_G2dBGAreaOver, scnBase, chrBase);
}



/*---------------------------------------------------------------------------*
  Name:         BGAutoControlAffineExt

  Description:  Sets the BGxCNT register as affine extended background in conjunction with screen data.
                Also changes the background mode at the same time.

  Arguments:    bg: Control target background
                pScnData: Pointer to the screen data
                scnBase: Background screen base
                chrBase: Background character base

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SetBGControl256x16Pltt(
    NNSG2dBGSelect bg,
    int screenWidth,
    int screenHeight,
    GXBGScrBase scnBase,
    GXBGCharBase chrBase
)
{
    const ScreenSizeMap* pSizeMap;

    NNS_G2D_BG_ASSERT(bg);
    SDK_ASSERTMSG( (bg % 4) >= 2,
        "You can not show affine BG on %s BG %d",
        (IsMainBG(bg) ? "Main": "Sub"),
        (IsMainBG(bg) ? bg: bg - NNS_G2D_BGSELECT_SUB0) );

    pSizeMap = SelectScnSize(sAffineExtScnSize, screenWidth, screenHeight);
    SDK_ASSERTMSG( pSizeMap != NULL,
        "Unsupported screen size(%dx%d) in input screen data",
        screenWidth,
        screenHeight
    );

    SetBGnControlTo256x16Pltt(bg, (GXBGScrSize256x16Pltt)pSizeMap->scnSize, NNSi_G2dBGAreaOver, scnBase, chrBase);
}



/*---------------------------------------------------------------------------*
  Name:         IsBGExtPlttSlotAssigned

  Description:  Determines whether VRAM is allocated to the specified extended palette slot.
                

  Arguments:    slot: Target extended palette slot

  Returns:      TRUE if VRAM is allocated.
 *---------------------------------------------------------------------------*/
static BOOL IsBGExtPlttSlotAssigned(NNSG2dBGExtPlttSlot slot)
{
    if( IsMainBGExtPlttSlot(slot) )
    {
        if( slot <= NNS_G2D_BGEXTPLTTSLOT_MAIN1 )
        {
            return IsMainBGExtPltt01Available();
        }
        else
        {
            return IsMainBGExtPltt23Available();
        }
    }
    else
    {
        return IsSubBGExtPlttAvailable();
    }
}



/*---------------------------------------------------------------------------*
  Name:         LoadBGPaletteSelect

  Description:  Reads the palette data into VRAM.
                Specifies the load target based on the palette used by the background and whether the extended palette is used.
                

  Arguments:    bg: Background using the read palette data
                bToExtPltt: When TRUE, loaded to the extended palette; to the standard palette when FALSE
                            
                pPltData: Pointer to the palette data

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void LoadBGPaletteSelect(
    NNSG2dBGSelect bg,
    BOOL bToExtPltt,
    const NNSG2dPaletteData* pPltData,
    const NNSG2dPaletteCompressInfo* pCmpInfo
)
{
    NNS_G2D_BG_ASSERT(bg);
    SDK_NULL_ASSERT(pPltData);

    if( bToExtPltt )
    {
        NNSG2dBGExtPlttSlot slot = GetBGExtPlttSlot(bg);

        SDK_ASSERT( IsBGExtPlttSlotAssigned(slot) );
        LoadBGPlttToExtendedPltt(slot, pPltData, pCmpInfo);
    }
    else
    {
        LoadBGPlttToNormalPltt(bg, pPltData, pCmpInfo);
    }
}



/*---------------------------------------------------------------------------*
  Name:         LoadBGPalette

  Description:  Loads palette data into VRAM.
                Automatically determines the load destination from the background and G2D screen data that use the palette.
                

  Arguments:    bg: Background using the read palette data
                pPltData: Pointer to the palette data
                pScnData: Pointer to screen data that uses the palette data to be loaded
                            

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void LoadBGPalette(
    NNSG2dBGSelect bg,
    const NNSG2dPaletteData* pPltData,
    const NNSG2dScreenData* pScnData,
    const NNSG2dPaletteCompressInfo* pCmpInfo
)
{
/*
    extended palette | enabled/disabled
      corresponding slot | enabled/disabled -
    -------------------------------------
    text    16x16   |standard standard standard
            256x1   |extended (extended) standard
            256x16  |extended (extended) [standard]
    affine  256x1   |standard standard standard
    extend  256x16  |extended (extended) [standard]

    Unless transparent, items in parentheses are black.
    The palette specification is ignored for items in square brackets.
    Both are stopped with ASSERT.
*/
    // TODO: Even when the extended palette is being used, the standard palette is used for the backdrop colors.
    //*(GXRgb*)(HW_BG_PLTT) = pPltData->pRawData[0];

    NNS_G2D_BG_ASSERT(bg);
    SDK_NULL_ASSERT(pPltData);
    SDK_NULL_ASSERT(pScnData);

    if( (pScnData->screenFormat == NNS_G2D_SCREENFORMAT_TEXT)
        && (pScnData->colorMode == NNS_G2D_SCREENCOLORMODE_256x1) )
    {
        LoadBGPaletteSelect(bg, IsBGUseExtPltt(bg), pPltData, pCmpInfo);
    }
    else
    {
        BOOL bExtPlttData = ! (
            (pPltData->fmt == GX_TEXFMT_PLTT16)
            || (pScnData->screenFormat == NNS_G2D_SCREENFORMAT_AFFINE)
        );

        SDK_ASSERTMSG( ! bExtPlttData || IsBGUseExtPltt(bg),
            "Input screen data requires extended BG palette, but unavailable");
        LoadBGPaletteSelect(bg, bExtPlttData, pPltData, pCmpInfo);
    }
}




/*---------------------------------------------------------------------------*
  Name:         LoadBGCharacter

  Description:  Loads the character data for use by the target background.

  Arguments:    bg: Target background screen
                pChrData: Pointer to the character data
                pPosInfo: Pointer to the character position data
                            When pChrData is not a partial character, NULL is specified.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void LoadBGCharacter(
    NNSG2dBGSelect bg,
    const NNSG2dCharacterData* pChrData,
    const NNSG2dCharacterPosInfo* pPosInfo
)
{
    u32 offset = 0;

    NNS_G2D_BG_ASSERT(bg);
    SDK_NULL_ASSERT(pChrData);
    SDK_ASSERT( pPosInfo == NULL || pPosInfo->srcPosX == 0 );

    //---- When a partial character, calculate offset
    if( pPosInfo != NULL )
    {
        int offsetChars = pPosInfo->srcPosY * pPosInfo->srcW;
        u32 szChar = (pChrData->pixelFmt == GX_TEXFMT_PLTT256) ?
                        sizeof(GXCharFmt256): sizeof(GXCharFmt16);

        offset = offsetChars * szChar;
    }

    DC_FlushRange(pChrData->pRawData, pChrData->szByte);
    LoadBGnChar(bg, pChrData->pRawData, offset, pChrData->szByte);
}



/*---------------------------------------------------------------------------*
  Name:         LoadBGScreen

  Description:  Loads the screen data for use by the target background.

  Arguments:    bg: Target background screen
                pScnData: Screen data to load

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void LoadBGScreen(
    NNSG2dBGSelect bg,
    const NNSG2dScreenData* pScnData
)
{
    GXScrFmtText* pDstBase;
    int plane_cwidth;
    int plane_cheight;
    int load_cwidth;
    int load_cheight;

    NNS_G2D_BG_ASSERT(bg);
    SDK_NULL_ASSERT(pScnData);

    //---- Gets the screen base
    pDstBase = (GXScrFmtText*)GetBGnScrPtr(bg);

    //---- Outputs the transferable size
    {
        const int scn_cwidth  = pScnData->screenWidth / 8;
        const int scn_cheight = pScnData->screenHeight / 8;

        NNSi_G2dBGGetCharSize(&plane_cwidth, &plane_cheight, bg);
        load_cwidth = (plane_cwidth > scn_cwidth) ? scn_cwidth: plane_cwidth;
        load_cheight = (plane_cheight > scn_cheight) ? scn_cheight: plane_cheight;
    }


    //---- Transfer
    DC_FlushRange( (void*)pScnData->rawData, pScnData->szByte );
    NNS_G2dBGLoadScreenRect(
        pDstBase,
        pScnData,
        0, 0,
        0, 0,
        plane_cwidth, plane_cheight,
        load_cwidth, load_cheight
    );
}






/*---------------------------------------------------------------------------*
  Name:         SetBGControlAuto

  Description:  Performs background control on the target background screen.

  Arguments:    bg: Target background screen
                screenFormat: Background type
                colorMode: Color mode
                screenWidth: Screen width (in pixels)
                screenHeight: Screen height (in pixels)
                schBase: Screen base block
                chrBase: Character base block

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SetBGControlAuto(
    NNSG2dBGSelect bg,
    NNSG2dScreenFormat screenFormat,
    GXBGColorMode colorMode,
    int screenWidth,
    int screenHeight,
    GXBGScrBase scnBase,
    GXBGCharBase chrBase
)
{
    // BG Control
    switch( screenFormat )
    {
    case NNS_G2D_SCREENFORMAT_TEXT:
        SetBGControlText(bg, colorMode, screenWidth, screenHeight, scnBase, chrBase);
        break;

    case NNS_G2D_SCREENFORMAT_AFFINE:
        SetBGControlAffine(bg, screenWidth, screenHeight, scnBase, chrBase);
        break;

    case NNS_G2D_SCREENFORMAT_AFFINEEXT:
        SetBGControl256x16Pltt(bg, screenWidth, screenHeight, scnBase, chrBase);
        break;

    default:
        SDK_ASSERTMSG(FALSE, "TEXT, AFFINE, and 256x16 format support only");
        break;
    }
}



/*---------------------------------------------------------------------------*
  Name:         LoadScreenPartText

  Description:  Copies the specified rectangle in the screen data to the specified position in the buffer.
                

  Arguments:    pScreenDst: Pointer to the transfer destination base point
                pScnData: Pointer to screen data that is the transfer source
                srcX: X-coordinate of the upper-left corner of the transfer source (in characters)
                srcY: Y-coordinate of the upper-left corner of the transfer source (in characters)
                dstX: X-coordinate of the upper-left corner of the transfer destination (in characters)
                dstY: Y-coordinate of the upper-left corner of the transfer destination (in characters)
                dstW: Width of the transfer destination area (in characters)
                dstH: Height of the transfer destination area (in characters)
                width: Width of area to transfer (in characters)
                height: Height of area to transfer (in characters)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void LoadScreenPartText(
    void* pScreenDst,
    const NNSG2dScreenData* pScnData,
    int srcX,
    int srcY,
    int dstX,
    int dstY,
    int dstW,
    int dstH,
    int width,
    int height
)
{
    NNS_G2D_POINTER_ASSERT( pScreenDst );
    NNS_G2D_POINTER_ASSERT( pScnData );

    //---- Mapping for each character
    {
        const int src_x_end             = srcX + width;
        const int src_y_end             = srcY + height;
        const int srcW                  = pScnData->screenWidth / 8;
        const int srcH                  = pScnData->screenHeight / 8;
        const GXScrFmtText* pSrcBase    = (const GXScrFmtText*)pScnData->rawData;
        GXScrFmtText* pDstBase          = (GXScrFmtText*)pScreenDst;
        int sx, sy;
        int dx, dy;

        for( sy = srcY, dy = dstY; sy < src_y_end; sy++, dy++ )
        {
            for( sx = srcX, dx = dstX; sx < src_x_end; sx++, dx++ )
            {
                const GXScrFmtText* pSrc = pSrcBase + CalcTextScreenOffset(sx, sy, srcW, srcH);
                GXScrFmtText* pDst = pDstBase + CalcTextScreenOffset(dx, dy, dstW, dstH);

                *pDst = *pSrc;
            }
        }
    }
}




/*---------------------------------------------------------------------------*
  Name:         LoadScreenPartAffine

  Description:  Copies the specified rectangle in the screen data to the specified position in the buffer.
                

  Arguments:    pScreenDst: Pointer to the transfer destination base point
                pScnData: Pointer to screen data that is the transfer source
                srcX: X-coordinate of the upper-left corner of the transfer source (in characters)
                srcY: Y-coordinate of the upper-left corner of the transfer source (in characters)
                dstX: X-coordinate of the upper-left corner of the transfer destination (in characters)
                dstY: Y-coordinate of the upper-left corner of the transfer destination (in characters)
                dstW: Width of the transfer destination area (in characters)
                width: Width of area to transfer (in characters)
                height: Height of area to transfer (in characters)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void LoadScreenPartAffine(
    void* pScreenDst,
    const NNSG2dScreenData* pScnData,
    int srcX,
    int srcY,
    int dstX,
    int dstY,
    int dstW,
    int width,
    int height
)
{
    NNS_G2D_POINTER_ASSERT( pScreenDst );
    NNS_G2D_POINTER_ASSERT( pScnData );

    //---- Simply transfers rectangle
    {
        const int src_y_end        = srcY + height;
        const int srcW             = pScnData->screenWidth / 8;
        const u32 szLine           = sizeof(GXScrFmtAffine) * width;
        const GXScrFmtAffine* pSrc = (const GXScrFmtAffine*)pScnData->rawData;
        GXScrFmtAffine*       pDst = (GXScrFmtAffine*)pScreenDst;
        int y;

        pSrc += srcY * srcW + srcX;
        pDst += dstY * dstW + dstX;

        for( y = srcY; y < src_y_end; y++ )
        {
            MI_CpuCopy8(pSrc, pDst, szLine);
            pSrc += srcW;
            pDst += dstW;
        }
    }
}



/*---------------------------------------------------------------------------*
  Name:         LoadScreenPart256x16Pltt

  Description:  Copies the specified rectangle in the screen data to the specified position in the buffer.
                

  Arguments:    pScreenDst: Pointer to the transfer destination base point
                pScnData: Pointer to screen data that is the transfer source
                srcX: X-coordinate of the upper-left corner of the transfer source (in characters)
                srcY: Y-coordinate of the upper-left corner of the transfer source (in characters)
                dstX: X-coordinate of the upper-left corner of the transfer destination (in characters)
                dstY: Y-coordinate of the upper-left corner of the transfer destination (in characters)
                dstW: Width of the transfer destination area (in characters)
                width: Width of area to transfer (in characters)
                height: Height of area to transfer (in characters)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void LoadScreenPart256x16Pltt(
    void* pScreenDst,
    const NNSG2dScreenData* pScnData,
    int srcX,
    int srcY,
    int dstX,
    int dstY,
    int dstW,
    int width,
    int height
)
{
    NNS_G2D_POINTER_ASSERT( pScreenDst );
    NNS_G2D_POINTER_ASSERT( pScnData );

    //---- Simply transfers rectangle
    {
        const int src_y_end      = srcY + height;
        const int srcW           = pScnData->screenWidth / 8;
        const u32 szLine         = sizeof(GXScrFmtText) * width;
        const GXScrFmtText* pSrc = (const GXScrFmtText*)pScnData->rawData;
        GXScrFmtText*       pDst = (GXScrFmtText*)pScreenDst;
        int y;


        pSrc += srcY * srcW + srcX;
        pDst += dstY * dstW + dstX;

        for( y = srcY; y < src_y_end; y++ )
        {
            MI_CpuCopy16(pSrc, pDst, szLine);
            pSrc += srcW;
            pDst += dstW;
        }
    }
}












//****************************************************************************
// Public functions
//****************************************************************************

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dBGLoadElements

  Description:  The graphics data used by BG is read into the appropriate VRAM.
                The BGControl for the target background needs to be set appropriately.
                Automatically determined based on the VRAM allocation status for the extended palette when it is the load target for backgrounds 0 and 1.
                The text background's 256x1 palette switches the load target based on whether the extended palette is enabled.
                
                

  Arguments:    bg: Background using the data to be loaded
                pScnData: Pointer to the screen data to be loaded into VRAM.
                            NULL is an acceptable value but only when pPltData is not NULL.
                            
                pChrData: Pointer to the character data to be loaded into VRAM.
                            Not loaded into VRAM when NULL is used.
                pPltData: Pointer to the palette data to be loaded into VRAM.
                            Not loaded into VRAM when NULL is used.
                pPosInfo: Pointer to character extraction area information
                pCmpInfo: Pointer to the palette compression data

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_G2dBGLoadElementsEx(
    NNSG2dBGSelect bg,
    const NNSG2dScreenData* pScnData,
    const NNSG2dCharacterData* pChrData,
    const NNSG2dPaletteData* pPltData,
    const NNSG2dCharacterPosInfo* pPosInfo,
    const NNSG2dPaletteCompressInfo* pCmpInfo
)
{
    NNS_G2D_BG_ASSERT(bg);
    NNS_G2D_ASSERT( pPltData == NULL || pScnData != NULL );
    NNS_G2D_ASSERT( pPosInfo == NULL || pChrData != NULL );
    NNS_G2D_ASSERT( pCmpInfo == NULL || pPltData != NULL );

    NNS_G2D_ASSERTMSG( pChrData == NULL || pScnData == NULL
                || ( (pChrData->pixelFmt == GX_TEXFMT_PLTT16)
                    == (pScnData->colorMode == NNS_G2D_SCREENCOLORMODE_16x16) ),
                "Color mode mismatch between character data and screen data" );
    NNS_G2D_ASSERTMSG( pPltData == NULL
                || ( (pPltData->fmt == GX_TEXFMT_PLTT16)
                    == (pScnData->colorMode == NNS_G2D_SCREENCOLORMODE_16x16) ),
                "Color mode mismatch between palette data and screen data" );
    NNS_G2D_ASSERT( pChrData == NULL
                || ( NNSi_G2dGetCharacterFmtType( pChrData->characterFmt )
                    == NNS_G2D_CHARACTER_FMT_CHAR ) );

    if( pPltData != NULL && pScnData != NULL )
    {
        LoadBGPalette(bg, pPltData, pScnData, pCmpInfo);
    }
    if( pChrData != NULL )
    {
        LoadBGCharacter(bg, pChrData, pPosInfo);
    }
    if( pScnData != NULL )
    {
        LoadBGScreen(bg, pScnData);
    }
}



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dBGSetupEx

  Description:  Sets the background and reads BG data into VRAM.

  Arguments:    bg: Target BG
                pScnData: Pointer to the screen data used by BG.
                            NULL may not be used.
                pChrData: Pointer to the character data used by BG.
                            Not loaded into VRAM when NULL is used.
                pPltData: Pointer to the palette data used by the background.
                            Not loaded into VRAM when NULL is used.
                pPosInfo: Pointer to character extraction area information
                pCmpInfo: Pointer to the palette compression data
                scnBase: Background screen base
                chrBase: Background character base

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_G2dBGSetupEx(
    NNSG2dBGSelect bg,
    const NNSG2dScreenData* pScnData,
    const NNSG2dCharacterData* pChrData,
    const NNSG2dPaletteData* pPltData,
    const NNSG2dCharacterPosInfo* pPosInfo,
    const NNSG2dPaletteCompressInfo* pCmpInfo,
    GXBGScrBase scnBase,
    GXBGCharBase chrBase
)
{
    NNS_G2D_BG_ASSERT(bg);
    NNS_G2D_NULL_ASSERT( pScnData );
    NNS_G2D_ASSERT( pPosInfo == NULL || pChrData != NULL );
    NNS_G2D_ASSERT( pCmpInfo == NULL || pPltData != NULL );

    NNS_G2D_ASSERTMSG( pChrData == NULL
                || ( (pChrData->pixelFmt == GX_TEXFMT_PLTT16)
                    == (pScnData->colorMode == NNS_G2D_SCREENCOLORMODE_16x16) ),
                "Color mode mismatch between character data and screen data" );
    NNS_G2D_ASSERTMSG( pPltData == NULL
                || ( (pPltData->fmt == GX_TEXFMT_PLTT16)
                    == (pScnData->colorMode == NNS_G2D_SCREENCOLORMODE_16x16) ),
                "Color mode mismatch between palette data and screen data" );
    NNS_G2D_ASSERT( pChrData == NULL
                || ( NNSi_G2dGetCharacterFmtType( pChrData->characterFmt )
                    == NNS_G2D_CHARACTER_FMT_CHAR ) );

    SetBGControlAuto(
        bg,
        NNSi_G2dBGGetScreenFormat(pScnData),
        NNSi_G2dBGGetScreenColorMode(pScnData),
        pScnData->screenWidth,
        pScnData->screenHeight,
        scnBase,
        chrBase );
    NNS_G2dBGLoadElementsEx(bg, pScnData, pChrData, pPltData, pPosInfo, pCmpInfo);
}




/*---------------------------------------------------------------------------*
  Name:         NNS_G2dBGLoadScreenRect

  Description:  Copies the specified rectangle in the screen data to the specified position in the buffer.
                

  Arguments:    pScreenDst: Pointer to the transfer destination base point
                pScnData: Pointer to screen data that is the transfer source
                srcX: X-coordinate of the upper-left corner of the transfer source (in characters)
                srcY: Y-coordinate of the upper-left corner of the transfer source (in characters)
                dstX: X-coordinate of the upper-left corner of the transfer destination (in characters)
                dstY: Y-coordinate of the upper-left corner of the transfer destination (in characters)
                dstW: Width of the transfer destination area (in characters)
                dstH: Height of the transfer destination area (in characters)
                width: Width of area to transfer (in characters)
                height: Height of area to transfer (in characters)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_G2dBGLoadScreenRect(
    void* pScreenDst,
    const NNSG2dScreenData* pScnData,
    int srcX,
    int srcY,
    int dstX,
    int dstY,
    int dstW,
    int dstH,
    int width,
    int height
)
{
    NNS_G2D_POINTER_ASSERT( pScreenDst );
    NNS_G2D_POINTER_ASSERT( pScnData );

    // Adjust parameters such that nothing happens to the region beyond the range for the input source/output target
    if( dstX < 0 )
    {
        const int adj = - dstX;
        srcX  += adj;
        width -= adj;
        dstX   = 0;
    }
    if( dstY < 0 )
    {
        const int adj = - dstY;
        srcY   += adj;
        height -= adj;
        dstY    = 0;
    }
    if( dstX + width > dstW )
    {
        const int adj = (dstX + width) - dstW;
        width -= adj;
    }
    if( dstY + height > dstH )
    {
        const int adj = (dstY + height) - dstH;
        height -= adj;
    }
    if( srcX < 0 )
    {
        const int adj = - srcX;
        dstX  += adj;
        width -= adj;
        srcX   = 0;
    }
    if( srcY < 0 )
    {
        const int adj = - srcY;
        dstY   += adj;
        height -= adj;
        srcY    = 0;
    }
    if( srcX + width > pScnData->screenWidth / 8 )
    {
        const int adj = (srcX + width) - (pScnData->screenWidth / 8);
        width -= adj;
    }
    if( srcY + height > pScnData->screenHeight / 8 )
    {
        const int adj = (srcY + height) - (pScnData->screenHeight / 8);
        height -= adj;
    }

    if( width <= 0 || height <= 0 )
    {
        return;
    }

    switch( pScnData->screenFormat )
    {
    case NNS_G2D_SCREENFORMAT_TEXT:
        LoadScreenPartText(pScreenDst, pScnData, srcX, srcY, dstX, dstY, dstW, dstH, width, height);
        break;

    case NNS_G2D_SCREENFORMAT_AFFINE:
        LoadScreenPartAffine(pScreenDst, pScnData, srcX, srcY, dstX, dstY, dstW, width, height);
        break;

    case NNS_G2D_SCREENFORMAT_AFFINEEXT:
        LoadScreenPart256x16Pltt(pScreenDst, pScnData, srcX, srcY, dstX, dstY, dstW, width, height);
        break;

    default:
        NNS_G2D_ASSERTMSG(FALSE, "Unknown screen format(=%d)", pScnData->screenFormat );
    }
}


