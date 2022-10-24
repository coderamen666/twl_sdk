/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - g2d
  File:     g2di_BGManipulator.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
#ifndef G2DI_BGMANIPULATOR_H_
#define G2DI_BGMANIPULATOR_H_

#include <nitro.h>
#include <nnsys/g2d/g2d_Screen.h>
#include "g2di_Dma.h"

#ifdef __cplusplus
extern "C" {
#endif




/*---------------------------------------------------------------------------*
  Name:         NNSi_G2dBGGetCharSize

  Description:  Gets the size of the target background screen in characters.

  Arguments:    pWidth:     pointer to buffer that holds background screen width
                pHeight:    pointer to buffer that holds background screen height
                n:          target background screen

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNSi_G2dBGGetCharSize(int* pWidth, int* pHeight, NNSG2dBGSelect n);



/*---------------------------------------------------------------------------*
  Name:         GetBGNo

  Description:  Gets the background number of the target background screen.

  Arguments:    n:          target background screen

  Returns:      None.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE int GetBGNo(NNSG2dBGSelect n)
{
    NNS_G2D_BG_ASSERT( n );
    return n & 3;
}

/*---------------------------------------------------------------------------*
  Name:         GetBGnCNT

  Description:  Gets a pointer to the BGnCNT register that controls the target background screen.

  Arguments:    n:          target background screen

  Returns:      None.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE REGType16v* GetBGnCNT(NNSG2dBGSelect n)
{
    extern REGType16v* const NNSiG2dBGCNTTable[];

    NNS_G2D_BG_ASSERT( n );
    return NNSiG2dBGCNTTable[n];
}

/*---------------------------------------------------------------------------*
  Name:         IsBG03D

  Description:  Used to determine whether the main screen BG0 is allocated to 3D.

  Arguments:    None.

  Returns:      TRUE if allocated to 3D.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE BOOL IsBG03D( void )
{
    return (reg_GX_DISPCNT & REG_GX_DISPCNT_BG02D3D_MASK) != 0;
}

/*---------------------------------------------------------------------------*
  Name:         IsMainBG

  Description:  Determines whether the NNSG2dBGSelect type value indicates the main screen background.
                

  Arguments:    bg: the target

  Returns:      TRUE if the main screen background is represented.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE BOOL IsMainBG(NNSG2dBGSelect bg)
{
    NNS_G2D_BG_ASSERT( bg );
    return ( bg <= NNS_G2D_BGSELECT_MAIN3 );
}

/*---------------------------------------------------------------------------*
  Name:         GetBGMode*

  Description:  Gets the current background mode.

  Arguments:    None.

  Returns:      The current BG mode.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE GXBGMode GetBGModeMain( void )
{
    return (GXBGMode)((reg_GX_DISPCNT & REG_GX_DISPCNT_BGMODE_MASK) >> REG_GX_DISPCNT_BGMODE_SHIFT);
}

NNS_G2D_INLINE GXBGMode GetBGModeSub( void )
{
    return (GXBGMode)((reg_GXS_DB_DISPCNT & REG_GXS_DB_DISPCNT_BGMODE_MASK) >> REG_GXS_DB_DISPCNT_BGMODE_SHIFT);
}

/*---------------------------------------------------------------------------*
  Name:         IsBGUseExtPltt*

  Description:  Determines whether the graphics engine handling the target background is using the extended palette.
                

  Arguments:    bg: target background screen

  Returns:      TRUE if the extended palette is enabled.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE BOOL IsBGUseExtPlttMain( void )
{
    return (reg_GX_DISPCNT & REG_GX_DISPCNT_BG_MASK) != 0;
}

NNS_G2D_INLINE BOOL IsBGUseExtPlttSub( void )
{
    return (reg_GXS_DB_DISPCNT & REG_GXS_DB_DISPCNT_BG_MASK) != 0;
}

NNS_G2D_INLINE BOOL IsBGUseExtPltt(NNSG2dBGSelect bg)
{
    return IsMainBG(bg) ? IsBGUseExtPlttMain(): IsBGUseExtPlttSub();
}

/*---------------------------------------------------------------------------*
  Name:         IsSubBGExtPlttAvailable

  Description:  Determines whether the VRAM is allocated to the sub-screen's extended palette.
                
                If allocation in the sub-screen is made to all slots (0, 1, 2 and 3), or not made at all,
                VRAM cannot be allocated.

  Arguments:    None.

  Returns:      TRUE if VRAM is allocated.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE BOOL IsSubBGExtPlttAvailable( void )
{
    return GX_GetBankForSubBGExtPltt() != GX_VRAM_SUB_BGEXTPLTT_NONE;
}

/*---------------------------------------------------------------------------*
  Name:         IsMainBGExtPltt01Available

  Description:  Determines whether VRAM is allocated to main screen extended palette slot 01.
                

  Arguments:    None.

  Returns:      TRUE if VRAM is allocated.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE BOOL IsMainBGExtPltt01Available( void )
{
    GXVRamBGExtPltt pltt = GX_GetBankForBGExtPltt();

    return (pltt == GX_VRAM_BGEXTPLTT_01_F)
            || (pltt == GX_VRAM_BGEXTPLTT_0123_E)
            || (pltt == GX_VRAM_BGEXTPLTT_0123_FG);
}

/*---------------------------------------------------------------------------*
  Name:         IsMainBGExtPltt23Available

  Description:  Determines whether VRAM is allocated to main screen extended palette slot 23.
                

  Arguments:    None.

  Returns:      TRUE if VRAM is allocated.
 *---------------------------------------------------------------------------*/
static BOOL IsMainBGExtPltt23Available( void )
{
    GXVRamBGExtPltt pltt = GX_GetBankForBGExtPltt();

    return (pltt == GX_VRAM_BGEXTPLTT_23_G)
            || (pltt == GX_VRAM_BGEXTPLTT_0123_E)
            || (pltt == GX_VRAM_BGEXTPLTT_0123_FG);
}

/*---------------------------------------------------------------------------*
  Name:         MakeBGnCNTVal*

  Description:  Creates a value for configuring the BGnCNT register.

  Arguments:    screenSize: background screen size
                areaOver:   Area over processing
                colorMode:  color mode
                screenBase: screen base offset
                charBase:   character base offset
                bgExtPltt:  extended palette slot

  Returns:     The created value.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE u16 MakeBGnCNTValText(
    GXBGScrSizeText screenSize,
    GXBGColorMode colorMode,
    GXBGScrBase screenBase,
    GXBGCharBase charBase,
    GXBGExtPltt bgExtPltt
)
{
    GX_BG_SCRSIZE_TEXT_ASSERT(screenSize);
    GX_BG_COLORMODE_ASSERT(colorMode);
    GX_BG_SCRBASE_ASSERT(screenBase);
    GX_BG_CHARBASE_ASSERT(charBase);
    GX_BG_EXTPLTT_ASSERT(bgExtPltt);

    return (u16)(
        (screenSize   << REG_G2_BG0CNT_SCREENSIZE_SHIFT)
        | (screenBase << REG_G2_BG0CNT_SCREENBASE_SHIFT)
        | (charBase   << REG_G2_BG0CNT_CHARBASE_SHIFT)
        | (bgExtPltt  << REG_G2_BG0CNT_BGPLTTSLOT_SHIFT)
        | (colorMode  << REG_G2_BG0CNT_COLORMODE_SHIFT)
    );
}
NNS_G2D_INLINE u16 MakeBGnCNTValAffine(
    GXBGScrSizeAffine screenSize,
    GXBGAreaOver areaOver,
    GXBGScrBase screenBase,
    GXBGCharBase charBase
)
{
    GX_BG_SCRSIZE_AFFINE_ASSERT(screenSize);
    GX_BG_AREAOVER_ASSERT(areaOver);
    GX_BG_SCRBASE_ASSERT(screenBase);
    GX_BG_CHARBASE_ASSERT(charBase);

    return (u16)(
        (screenSize   << REG_G2_BG2CNT_SCREENSIZE_SHIFT)
        | (screenBase << REG_G2_BG2CNT_SCREENBASE_SHIFT)
        | (charBase   << REG_G2_BG2CNT_CHARBASE_SHIFT)
        | (areaOver   << REG_G2_BG2CNT_AREAOVER_SHIFT)
    );
}
NNS_G2D_INLINE u16 MakeBGnCNTVal256x16Pltt(
    GXBGScrSize256x16Pltt screenSize,
    GXBGAreaOver areaOver,
    GXBGScrBase screenBase,
    GXBGCharBase charBase
)
{
    GX_BG_SCRSIZE_256x16PLTT_ASSERT(screenSize);
    GX_BG_AREAOVER_ASSERT(areaOver);
    GX_BG_SCRBASE_ASSERT(screenBase);
    GX_BG_CHARBASE_ASSERT(charBase);

    return (u16)(
        (screenSize   << REG_G2_BG2CNT_SCREENSIZE_SHIFT)
        | (screenBase << REG_G2_BG2CNT_SCREENBASE_SHIFT)
        | (charBase   << REG_G2_BG2CNT_CHARBASE_SHIFT)
        | (areaOver   << REG_G2_BG2CNT_AREAOVER_SHIFT)
        | GX_BG_EXTMODE_256x16PLTT
    );
}

/*---------------------------------------------------------------------------*
  Name:         GetBG*Offset

  Description:  Gets the offset for the main screen's screen or character base.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE int GetBGCharOffset(void)
{
    return (int)(0x10000 * ((reg_GX_DISPCNT & REG_GX_DISPCNT_BGCHAROFFSET_MASK) >>
                            REG_GX_DISPCNT_BGCHAROFFSET_SHIFT));
}

NNS_G2D_INLINE int GetBGScrOffset(void)
{
    return (int)(0x10000 * ((reg_GX_DISPCNT & REG_GX_DISPCNT_BGSCREENOFFSET_MASK) >>
                            REG_GX_DISPCNT_BGSCREENOFFSET_SHIFT));
}

/*---------------------------------------------------------------------------*
  Name:         GetBGn*Ptr

  Description:  Gets a pointer to the target background screen or character base.
                

  Arguments:    n:      target background screen

  Returns:      None.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE void* GetBGnCharPtr(NNSG2dBGSelect n)
{
    const int baseBlock = 0x4000 * ((*GetBGnCNT(n) & REG_G2_BG0CNT_CHARBASE_MASK) >>
                             REG_G2_BG0CNT_CHARBASE_SHIFT);

    return (void *)((IsMainBG(n) ? (HW_BG_VRAM + GetBGCharOffset()): HW_DB_BG_VRAM) + baseBlock);
}

NNS_G2D_INLINE void* GetBGnScrPtr(NNSG2dBGSelect n)
{
    const int baseBlock = 0x800 * ((*GetBGnCNT(n) & REG_G2_BG0CNT_SCREENBASE_MASK) >>
                             REG_G2_BG0CNT_SCREENBASE_SHIFT);

    return (void *)((IsMainBG(n) ? (HW_BG_VRAM + GetBGScrOffset()): HW_DB_BG_VRAM) + baseBlock);
}

/*---------------------------------------------------------------------------*
  Name:         LoadBGnChar

  Description:  Loads character data for the target background screen.

  Arguments:    n:      target background screen
                pSrc:   pointer to data to load
                offset: offset from the character base of the load destination
                szByte: size of the data to load

  Returns:      None.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE void LoadBGnChar(NNSG2dBGSelect n, const void *pSrc, u32 offset, u32 szByte)
{
    u32 ptr;

    NNS_G2D_POINTER_ASSERT( pSrc );
    ptr = (u32)GetBGnCharPtr(n);

    NNSi_G2dDmaCopy16(NNS_G2D_DMA_NO, pSrc, (void *)(ptr + offset), szByte);
}

/*---------------------------------------------------------------------------*
  Name:         SetBGnControl*

  Description:  Performs BGControl of the target background screen.

  Arguments:    n:          target background screen
                screenSize: screen size
                colorMode:  color mode
                areaOver:   area over processing
                screenBase: screen base block
                charBase:   character base block
                bgExtPltt:  extended palette slot selection

  Returns:      None.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE void SetBGnControlText(
    NNSG2dBGSelect n,
    GXBGScrSizeText screenSize,
    GXBGColorMode colorMode,
    GXBGScrBase screenBase,
    GXBGCharBase charBase,
    GXBGExtPltt bgExtPltt
)
{
    *GetBGnCNT(n) = (u16)(
        (*GetBGnCNT(n) & (REG_G2_BG0CNT_PRIORITY_MASK | REG_G2_BG0CNT_MOSAIC_MASK))
        | MakeBGnCNTValText(screenSize, colorMode, screenBase, charBase, bgExtPltt)
    );
}
NNS_G2D_INLINE void SetBGnControlAffine(
    NNSG2dBGSelect n,
    GXBGScrSizeAffine screenSize,
    GXBGAreaOver areaOver,
    GXBGScrBase screenBase,
    GXBGCharBase charBase
)
{
    *GetBGnCNT(n) = (u16)(
        (*GetBGnCNT(n) & (REG_G2_BG2CNT_PRIORITY_MASK | REG_G2_BG2CNT_MOSAIC_MASK))
        | MakeBGnCNTValAffine(screenSize, areaOver, screenBase, charBase)
    );
}
NNS_G2D_INLINE void SetBGnControl256x16Pltt(
    NNSG2dBGSelect n,
    GXBGScrSize256x16Pltt screenSize,
    GXBGAreaOver areaOver,
    GXBGScrBase screenBase,
    GXBGCharBase charBase
)
{
    *GetBGnCNT(n) = (u16)(
        (*GetBGnCNT(n) & (REG_G2_BG2CNT_PRIORITY_MASK | REG_G2_BG2CNT_MOSAIC_MASK))
        | MakeBGnCNTVal256x16Pltt(screenSize, areaOver, screenBase, charBase)
    );
}








#ifdef __cplusplus
}/* extern "C" */
#endif

#endif // G2DI_BGMANIPULATOR_H_

