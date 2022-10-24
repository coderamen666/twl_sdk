/*---------------------------------------------------------------------------*
  Project:  TWL-System - include - nnsys - g2d
  File:     g2d_Screen.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
#ifndef NNS_G2D_SCREEN_H_
#define NNS_G2D_SCREEN_H_

#include <nitro.h>
#include <nnsys/g2d.h>
#include <nnsys/g2d/fmt/g2d_Screen_data.h>

#ifdef __cplusplus
extern "C" {
#endif



//-------------------------------------------------------------------------
// Enumerated Types
//-------------------------------------------------------------------------

// For specifying the processing target background
typedef enum NNSG2dBGSelect
{
    NNS_G2D_BGSELECT_MAIN0,
    NNS_G2D_BGSELECT_MAIN1,
    NNS_G2D_BGSELECT_MAIN2,
    NNS_G2D_BGSELECT_MAIN3,
    NNS_G2D_BGSELECT_SUB0,
    NNS_G2D_BGSELECT_SUB1,
    NNS_G2D_BGSELECT_SUB2,
    NNS_G2D_BGSELECT_SUB3,
    NNS_G2D_BGSELECT_NUM
}
NNSG2dBGSelect;

#define NNS_G2D_BG_ASSERT(bg) SDK_MINMAX_ASSERT( bg, NNS_G2D_BGSELECT_MAIN0, NNS_G2D_BGSELECT_NUM-1 )


//-------------------------------------------------------------------------
// Function Declarations
//-------------------------------------------------------------------------

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dBGSetupEx

  Description:  Sets the background and reads its data into VRAM.

  Arguments:    bg:         The target BG
                pScnData:   A pointer to the screen data used by BG.
                            NULL may not be used.
                pChrData:   A pointer to the character data used by BG.
                            Not loaded into VRAM when NULL is used.
                pPltData:   Pointer to the palette data used by the background.
                            Not loaded into VRAM when NULL is used.
                pPosInfo:   Pointer to character extraction area information.
                pCmpInfo:   A pointer to the palette compression data.
                scnBase:    The background screen base.
                chrBase:    The background character base.

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
);

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dBGLoadElements

  Description:  The graphics data used by BG is read into the appropriate VRAM.
                The BGControl for the target background needs to be set appropriately.
                Automatically determined based on the VRAM allocation status for the extended palette when it is the load target for backgrounds 0 and 1.
                
                The text background's 256x1 palette switches the load target based on whether the extended palette is enabled.
                

  Arguments:    bg:         The background using the data to be loaded
                pScnData:   A pointer to the screen data to be loaded into VRAM.
                            NULL is an acceptable value but only when pPltData is not NULL.
                            
                pChrData:   A pointer to the character data to be loaded into VRAM.
                            Not loaded into VRAM when NULL is used.
                pPltData:   A pointer to the palette data to be loaded into VRAM.
                            Not loaded into VRAM when NULL is used.
                pPosInfo:   Pointer to character extraction area information.
                pCmpInfo:   A pointer to the palette compression data.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_G2dBGLoadElementsEx(
    NNSG2dBGSelect bg,
    const NNSG2dScreenData* pScnData,
    const NNSG2dCharacterData* pChrData,
    const NNSG2dPaletteData* pPltData,
    const NNSG2dCharacterPosInfo* pPosInfo,
    const NNSG2dPaletteCompressInfo* pCmpInfo
);

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dBGLoadScreenRect

  Description:  Copies the specified rectangle in the screen data to the specified position in the buffer.
                

  Arguments:    pScreenDst: Pointer to the transfer destination base point.
                pScnData:   Pointer to the screen data to be transferred.
                srcX:       x coordinate of the upper left corner of the transfer source. (in characters)
                srcY:       y coordinate of the upper left corner of the transfer source. (in characters)
                dstX:       x coordinate of the upper left corner of the transfer destination. (in characters)
                dstY:       y coordinate of the upper left corner of the transfer destination. (in characters)
                dstW:       Width of the transfer destination area. (in characters)
                dstH:       Height of the transfer destination area. (in characters)
                width:      Width of area to transfer. (in characters)
                height:     Height of area to transfer. (in characters)

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
);


//-------------------------------------------------------------------------
// Global variables (Private)
//-------------------------------------------------------------------------

extern GXBGAreaOver NNSi_G2dBGAreaOver;




//-------------------------------------------------------------------------
// Inline Functions
//-------------------------------------------------------------------------

/*---------------------------------------------------------------------------*
  Name:         NNSi_G2dBGGetScreenColorMode

  Description:  Gets the screen data's color mode.

  Arguments:    pScnData:   Pointer to the screen data

  Returns:      None.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE GXBGColorMode NNSi_G2dBGGetScreenColorMode(const NNSG2dScreenData* pScnData)
{
    NNS_G2D_POINTER_ASSERT( pScnData );
    return (pScnData->colorMode == NNS_G2D_SCREENCOLORMODE_16x16) ?
                GX_BG_COLORMODE_16: GX_BG_COLORMODE_256;
}



/*---------------------------------------------------------------------------*
  Name:         NNSi_G2dBGGetScreenFormat

  Description:  Gets the screen data's screen format.

  Arguments:    pScnData:   Pointer to the screen data

  Returns:      None.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE NNSG2dScreenFormat NNSi_G2dBGGetScreenFormat(const NNSG2dScreenData* pScnData)
{
    NNS_G2D_POINTER_ASSERT( pScnData );
    return (NNSG2dScreenFormat)pScnData->screenFormat;
}



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dSetBGAreaOver

  Description:  Specifies the affine (expanded) background out-of-bounds process for the NNS_G2dLoadBGScreenSet function.
                

  Arguments:    areaOver:   Specifies the out-of-bounds process used by subsequent NNS_G2dLoadBGScreenSet function calls.
                            

  Returns:      None.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE void NNS_G2dSetBGAreaOver( GXBGAreaOver areaOver )
{
    SDK_MINMAX_ASSERT( areaOver, GX_BG_AREAOVER_XLU, GX_BG_AREAOVER_REPEAT );
    NNSi_G2dBGAreaOver = areaOver;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dGetBGAreaOver

  Description:  Gets the out-of-bounds process used by the NNS_G2dLoadBGScreenSet() function.
                

  Arguments:    None.

  Returns:      The out-of-bounds processing used with NNS_G2dLoadBGScreenSet().
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE GXBGAreaOver NNS_G2dGetBGAreaOver( void )
{
    return NNSi_G2dBGAreaOver;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dBGSetup

  Description:  Sets the background and reads BG data into VRAM.

  Arguments:    bg:         The target BG
                pScnData:   A pointer to the screen data used by BG.
                            NULL may not be used.
                pChrData:   A pointer to the character data used by BG.
                            Not loaded into VRAM when NULL is used.
                pPltData:   Pointer to the palette data used by the background.
                            Not loaded into VRAM when NULL is used.
                scnBase:    The background screen base.
                chrBase:    The background character base.

  Returns:      None.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE void NNS_G2dBGSetup(
    NNSG2dBGSelect bg,
    const NNSG2dScreenData* pScnData,
    const NNSG2dCharacterData* pChrData,
    const NNSG2dPaletteData* pPltData,
    GXBGScrBase scnBase,
    GXBGCharBase chrBase
)
{
    NNS_G2dBGSetupEx(bg, pScnData, pChrData, pPltData, NULL, NULL, scnBase, chrBase);
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dBGLoadElements

  Description:  The graphics data used by BG is read into the appropriate VRAM.
                The BGControl for the target background needs to be set appropriately.
                Automatically determined based on the VRAM allocation status for the extended palette when it is the load target for backgrounds 0 and 1.
                
                The text background's 256x1 palette switches the load target based on whether the extended palette is enabled.
                

  Arguments:    bg:         The background using the data to be loaded
                pScnData:   A pointer to the screen data to be loaded into VRAM.
                            NULL is an acceptable value but only when pPltData is not NULL.
                            
                pChrData:   A pointer to the character data to be loaded into VRAM.
                            Not loaded into VRAM when NULL is used.
                pPltData:   A pointer to the palette data to be loaded into VRAM.
                            Not loaded into VRAM when NULL is used.

  Returns:      None.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE void NNS_G2dBGLoadElements(
    NNSG2dBGSelect bg,
    const NNSG2dScreenData* pScnData,
    const NNSG2dCharacterData* pChrData,
    const NNSG2dPaletteData* pPltData
)
{
    NNS_G2dBGLoadElementsEx(bg, pScnData, pChrData, pPltData, NULL, NULL);
}


//-------------------------------------------------------------------------





#ifdef __cplusplus
}/* extern "C" */
#endif

#endif // NNS_G2D_SCREEN_H_

