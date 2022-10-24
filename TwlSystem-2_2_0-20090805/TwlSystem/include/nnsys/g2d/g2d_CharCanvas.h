/*---------------------------------------------------------------------------*
  Project:  TWL-System - include - nnsys - g2d
  File:     g2d_CharCanvas.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
#ifndef G2D_CHARCANVAS_H_
#define G2D_CHARCANVAS_H_

#include <nnsys/g2d/g2d_Font.h>
#include <nnsys/g2d/g2di_AssertUtil.h>
#include <nnsys/g2d/fmt/g2d_Cell_data.h>

#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------------

//---------------------------------------------------------------------
// CharCanvas Macros
//---------------------------------------------------------------------

#define NNS_G2D_CHARCANVAS_ASSERT( pCC )                                \
    NNS_G2D_ASSERTMSG(                                                  \
        NNS_G2D_IS_VALID_POINTER(pCC)                                   \
        && (0 < (pCC)->areaWidth)                                       \
        && (0 < (pCC)->areaHeight)                                      \
        && ( ((pCC)->dstBpp == 4) || ((pCC)->dstBpp == 8) )             \
        && NNS_G2D_IS_VALID_POINTER( (pCC)->charBase )                  \
        && NNS_G2D_IS_ALIGNED((pCC)->charBase, 4)                       \
        && NNS_G2D_IS_VALID_POINTER( (pCC)->vtable )                    \
        && NNS_G2D_IS_VALID_POINTER( (pCC)->vtable->pDrawGlyph )        \
        && NNS_G2D_IS_VALID_POINTER( (pCC)->vtable->pClear )            \
        && NNS_G2D_IS_VALID_POINTER( (pCC)->vtable->pClearArea )        \
        , "Illegal NNSG2dCharCanvas.")

#define NNS_G2D_COLORMODE_ASSERT( cmode )                               \
    NNS_G2D_ASSERTMSG(                                                  \
        (cmode) == NNS_G2D_CHARA_COLORMODE_16                           \
        || (cmode) == NNS_G2D_CHARA_COLORMODE_256                       \
        , "Illegal Color Mode(=%d)", (cmode) )

#define NNS_G2D_OBJVRAMMODE_ASSERT( vmode )                             \
    NNS_G2D_ASSERTMSG(                                                  \
        (vmode) == NNS_G2D_OBJVRAMMODE_32K                              \
        || (vmode) == NNS_G2D_OBJVRAMMODE_64K                           \
        || (vmode) == NNS_G2D_OBJVRAMMODE_128K                          \
        || (vmode) == NNS_G2D_OBJVRAMMODE_256K                          \
        , "Illegal VRAM Mode(=%d)", (vmode) )

#define NNS_G2D_TEXT_BG_WIDTH_ASSERT( width )                           \
    NNS_G2D_ASSERTMSG(                                                  \
        (width) == NNS_G2D_TEXT_BG_WIDTH_256                            \
        || (width) == NNS_G2D_TEXT_BG_WIDTH_512                         \
        , "Illegal Text BG Width(=%d).", (width) )

#define NNS_G2D_AFFINE_BG_WIDTH_ASSERT( width )                         \
    NNS_G2D_ASSERTMSG(                                                  \
        (width) == NNS_G2D_AFFINE_BG_WIDTH_128                          \
        || (width) == NNS_G2D_AFFINE_BG_WIDTH_256                       \
        || (width) == NNS_G2D_AFFINE_BG_WIDTH_512                       \
        || (width) == NNS_G2D_AFFINE_BG_WIDTH_1024                      \
        , "Illegal Affine BG Width(=%d).", (width) )

#define NNS_G2D_256x16PLTT_BG_WIDTH_ASSERT( width )                     \
    NNS_G2D_ASSERTMSG(                                                  \
        (width) == NNS_G2D_256x16PLTT_BG_WIDTH_128                      \
        || (width) == NNS_G2D_256x16PLTT_BG_WIDTH_256                   \
        || (width) == NNS_G2D_256x16PLTT_BG_WIDTH_512                   \
        || (width) == NNS_G2D_256x16PLTT_BG_WIDTH_1024                  \
        , "Illegal 256x16Pltt BG Width(=%d).", (width) )




// TEXT BG width
typedef enum NNSG2dTextBGWidth
{
    NNS_G2D_TEXT_BG_WIDTH_256   = 32,
    NNS_G2D_TEXT_BG_WIDTH_512   = 64
}
NNSG2dTextBGWidth;

// Affine BG width
typedef enum NNSG2dAffineBGWidth
{
    NNS_G2D_AFFINE_BG_WIDTH_128     = 16,
    NNS_G2D_AFFINE_BG_WIDTH_256     = 32,
    NNS_G2D_AFFINE_BG_WIDTH_512     = 64,
    NNS_G2D_AFFINE_BG_WIDTH_1024    = 128
}
NNSG2dAffineBGWidth;

// affine expansion 256x16 palette type BG width
typedef enum NNSG2d256x16PlttBGWidth
{
    NNS_G2D_256x16PLTT_BG_WIDTH_128     = 16,
    NNS_G2D_256x16PLTT_BG_WIDTH_256     = 32,
    NNS_G2D_256x16PLTT_BG_WIDTH_512     = 64,
    NNS_G2D_256x16PLTT_BG_WIDTH_1024    = 128
}
NNSG2d256x16PlttBGWidth;

// Character color mode
typedef enum NNSG2dCharaColorMode
{
    NNS_G2D_CHARA_COLORMODE_16 = 4,
    NNS_G2D_CHARA_COLORMODE_256 = 8
}
NNSG2dCharaColorMode;

// Size that the OBJ character region can reference
typedef enum NNSG2dOBJVramMode
{
    NNS_G2D_OBJVRAMMODE_32K     = 0,
    NNS_G2D_OBJVRAMMODE_64K     = 1,
    NNS_G2D_OBJVRAMMODE_128K    = 2,
    NNS_G2D_OBJVRAMMODE_256K    = 3
}
NNSG2dOBJVramMode;

//---------------------------------------------------------------------
// CharCanvas Definitions
//---------------------------------------------------------------------


struct NNSG2dCharCanvas;

// Definitions for the CharCanvas glyph rendering function
typedef void (*NNSiG2dDrawGlyphFunc)(
    const struct NNSG2dCharCanvas* pCC,
    const NNSG2dFont* pFont,
    int x,
    int y,
    int cl,
    const NNSG2dGlyph* pGlyph
);

// CharCanvas clear function definitions
typedef void (*NNSiG2dClearFunc)(
    const struct NNSG2dCharCanvas* pCC,
    int cl
);

// CharCanvas sectional clearing function definitions
typedef void (*NNSiG2dClearAreaFunc)(
    const struct NNSG2dCharCanvas* pCC,
    int cl,
    int x,
    int y,
    int w,
    int h
);

typedef struct NNSiG2dCharCanvasVTable
{
    NNSiG2dDrawGlyphFunc    pDrawGlyph;
    NNSiG2dClearFunc        pClear;
    NNSiG2dClearAreaFunc    pClearArea;
}
NNSiG2dCharCanvasVTable;

// CharCanvas structure
typedef struct NNSG2dCharCanvas
{
    u8*     charBase;
    int     areaWidth;
    int     areaHeight;
    u8      dstBpp;
    u8      reserved[3];
    u32     param;
    const NNSiG2dCharCanvasVTable* vtable;
}
NNSG2dCharCanvas;





//****************************************************************************
// BG Screen Structure Functions
//****************************************************************************

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dMapScrToCharText

  Description:  Matches up the characters from the screen on a one-to-one basis for CharCanvas.
                This is for the text BG.

  Arguments:    scnBase:    A pointer to the background screen's screen base used by CharCanvas.
                            
                areaWidth:  CharCanvas width in character units.
                areaHeight: CharCanvas height in character units.
                areaLeft:   CharCanvas upper left X coordinate when BG screen's top left is (0,0)
                            
                areaTop:    CharCanvas upper left Y coordinate when BG screen's top left is (0,0)
                            
                scnWidth:   BG screen width
                charNo:     A pointer to the first character in the character array allocated to CharCanvas.
                            
                cplt:       Color palette number designated for the screen.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_G2dMapScrToCharText(
    void* scnBase,
    int areaWidth,
    int areaHeight,
    int areaLeft,
    int areaTop,
    NNSG2dTextBGWidth scnWidth,
    int charNo,
    int cplt
);



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dMapScrToCharAffine

  Description:  Matches up the characters from the screen on a one-to-one basis for CharCanvas.
                This is for the affine BG.

  Arguments:    areaBase:   Pointer to the screen that corresponds to the CharCanvas upper left.
                areaWidth:  CharCanvas width in character units.
                areaHeight: CharCanvas height in character units.
                scnWidth:   BG screen width
                charNo:     A pointer to the first character in the character array allocated to CharCanvas.
                            

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_G2dMapScrToCharAffine(
    void* areaBase,
    int areaWidth,
    int areaHeight,
    NNSG2dAffineBGWidth scnWidth,
    int charNo
);



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dMapScrToChar256x16Pltt

  Description:  Matches up the characters from the screen on a one-to-one basis for CharCanvas.
                This is for the affine expanded BG 256 x 16 palette type.

  Arguments:    areaBase:   Pointer to the screen that corresponds to the CharCanvas upper left.
                areaWidth:  CharCanvas width in character units.
                areaHeight: CharCanvas height in character units.
                scnWidth:   BG screen width
                charNo:     A pointer to the first character in the character array allocated to CharCanvas.
                            
                cplt:       Color palette number designated for the screen.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_G2dMapScrToChar256x16Pltt(
    void* areaBase,
    int areaWidth,
    int areaHeight,
    NNSG2d256x16PlttBGWidth scnWidth,
    int charNo,
    int cplt
);





//****************************************************************************
// OBJ Array Functions
//****************************************************************************

/*---------------------------------------------------------------------------*
  Name:         NNSi_G2dCalcRequiredOBJ
                NNS_G2dCalcRequiredOBJ1D
                NNS_G2dCalcRequiredOBJ2DRect

  Description:  Calculates the number of OBJ needed by NNS_G2dArrangeOBJ*.
                If the size of CharCanvas is the same, NNS_G2dArrangeOBJ1D and NNS_G2dArrangeOBJ2DRect can have the same array of objects.
                
                As a result, NNS_G2dArrangeOBJ1D and NNS_G2dArrangeOBJ2DRect are the same thing internally.
                

  Arguments:    areaWidth:  CharCanvas width in characters for which to calculate the number of OBJs.
                areaHeight: CharCanvas height in characters for which to calculate the number of OBJs.

  Returns:      The number of OBJs needed.
 *---------------------------------------------------------------------------*/
int NNSi_G2dCalcRequiredOBJ(
    int areaWidth,
    int areaHeight
);

NNS_G2D_INLINE int NNS_G2dCalcRequiredOBJ1D(
    int areaWidth,
    int areaHeight
)
{
    return NNSi_G2dCalcRequiredOBJ(areaWidth, areaHeight);
}

NNS_G2D_INLINE int NNS_G2dCalcRequiredOBJ2DRect(
    int areaWidth,
    int areaHeight
)
{
    return NNSi_G2dCalcRequiredOBJ(areaWidth, areaHeight);
}



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dArrangeOBJ1D

  Description:  Arranges the objects rendered with NNS_G2dCharCanvasInitForOBJ1D so that they can be displayed appropriately.
                
                With OAM as the starting point, uses (NNS_G2dCalcRequireOBJ1D(areaWidth, areaHeight)) number of objects.
                

  Arguments:    oam:        Pointer to the starting point of the OAM string to be used.
                areaWidth:  CharCanvas width in character units.
                areaHeight: CharCanvas height in character units.
                x:          CharCanvas upper left corner display position.
                y:          CharCanvas upper left corner display position.
                color:      CharCanvas color mode.
                charName:   Character name at the top of the character string that the OBJ string uses in display.
                vramMode:   OBJ VRAM capacity

  Returns:

 *---------------------------------------------------------------------------*/
int NNS_G2dArrangeOBJ1D(
    GXOamAttr * oam,
    int areaWidth,
    int areaHeight,
    int x,
    int y,
    GXOamColorMode color,
    int charName,
    NNSG2dOBJVramMode vramMode
);



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dArrangeOBJ2DRect

  Description:  Arranges the objects rendered with NNS_G2dCharCanvasInitForOBJ2DRect so that they can be displayed appropriately.
                
                With OAM as the starting point, uses (NNS_G2dCalcRequireOBJ2DRect(areaWidth, areaHeight)) number of objects.
                

  Arguments:    oam:        Pointer to the starting point of the OAM string to be used.
                areaWidth:  CharCanvas width in character units.
                areaHeight: CharCanvas height in character units.
                x:          CharCanvas upper left corner display position.
                y:          CharCanvas upper left corner display position.
                color:      CharCanvas color mode.
                charName:   Character name at the top of the character string that the OBJ string uses in display.

  Returns:

 *---------------------------------------------------------------------------*/
int NNS_G2dArrangeOBJ2DRect(
    GXOamAttr * oam,
    int areaWidth,
    int areaHeight,
    int x,
    int y,
    GXOamColorMode color,
    int charName
);





//****************************************************************************
// CharCanvas operations
//****************************************************************************

// Render

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dCharCanvasDrawChar

  Description:  Renders 1 character specified by the character code on the CharCanvas.

  Arguments:    pCC:    Pointer to CharCanvas.
                pFont:  Pointer to the font used for rendering.
                x:      Upper left character coordinate
                y:      Upper left character coordinate
                cl:     Color number of character color.
                pGlyph: Character code of the character to render.

  Returns:      Character render width.
 *---------------------------------------------------------------------------*/
int NNS_G2dCharCanvasDrawChar(
    const NNSG2dCharCanvas* pCC,
    const NNSG2dFont* pFont,
    int x,
    int y,
    int cl,
    u16 ccode
);



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dCharCanvasDrawGlyph

  Description:  Renders the specified glyph to CharCanvas.

  Arguments:    pCC:    Pointer to CharCanvas.
                pFont:  Pointer to the font used for rendering.
                x:      Upper-left character coordinate.
                y:      Upper-left character coordinate.
                cl:     Color number of character color.
                pGlyph: Pointer to the glyph to render.

  Returns:      None.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE void NNS_G2dCharCanvasDrawGlyph(
    const NNSG2dCharCanvas* pCC,
    const NNSG2dFont* pFont,
    int x,
    int y,
    int cl,
    const NNSG2dGlyph* pGlyph
)
{
    NNS_G2D_CHARCANVAS_ASSERT( pCC );
    pCC->vtable->pDrawGlyph(pCC, pFont, x, y, cl, pGlyph);
}


// Erase

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dCharCanvasClear

  Description:  Fills all characters associated with CharCanvas with the specified color.
                

  Arguments:    pCC:    Pointer to CharCanvas.
                cl:     Color number of the fill color.

  Returns:      None.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE void NNS_G2dCharCanvasClear(
    const NNSG2dCharCanvas* pCC,
    int cl
)
{
    NNS_G2D_CHARCANVAS_ASSERT( pCC );
    pCC->vtable->pClear(pCC, cl);
}



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dCharCanvasClearArea

  Description:  Fills the specified region of characters associated with CharCanvas with the specified color.
                

  Arguments:    pCC:    Pointer to CharCanvas.
                cl:     Color number of the fill color.
                x:      X coordinate of the upper left of the fill region.
                y:      Y coordinate of the upper left of the fill region.
                w:      Width of the fill region.
                h:      Height of the fill region.

  Returns:      None.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE void NNS_G2dCharCanvasClearArea(
    const NNSG2dCharCanvas* pCC,
    int cl,
    int x,
    int y,
    int w,
    int h
)
{
    NNS_G2D_CHARCANVAS_ASSERT( pCC );
    pCC->vtable->pClearArea(pCC, cl, x, y, w, h);
}



//----------------------------------------------------------------------------
// Build
//----------------------------------------------------------------------------

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dCharCanvasInitForBG

  Description:  Initializes CharCanvas.
                Directly specifies and initializes the parameters for the BG.

  Arguments:    pCC:        Pointer to CharCanvas.
                charBase:   A pointer to the first character in the character array allocated to CharCanvas.
                            
                areaWidth:  CharCanvas width in character units.
                areaHeight: CharCanvas height in character units.
                colorMode:  Color mode of the render target character.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_G2dCharCanvasInitForBG(
    NNSG2dCharCanvas* pCC,
    void* charBase,
    int areaWidth,
    int areaHeight,
    NNSG2dCharaColorMode colorMode
);



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dCharCanvasInitForOBJ1D

  Description:  Initializes the CharCanvas for use by the 1D mapping OBJ.

  Arguments:    pCC:        Pointer to the CharCanvas to initialize.
                charBase:   Pointer to the starting point of the character allocated to CharCanvas.
                areaWidth:  CharCanvas width in character units.
                areaHeight: CharCanvas height in character units.
                colorMode:  Color mode of the render target character.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_G2dCharCanvasInitForOBJ1D(
    NNSG2dCharCanvas* pCC,
    void* charBase,
    int areaWidth,
    int areaHeight,
    NNSG2dCharaColorMode colorMode
);



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dCharCanvasInitForOBJ2DRect

  Description:  Initializes CharCanvas.
                Used when a rectangle within character memory is used as CharCanvas by a two-dimensional mapping object.
                

  Arguments:    pCC:        Pointer to CharCanvas.
                charBase:   Pointer to CharCanvas upper left character.
                areaWidth:  CharCanvas width in character units.
                areaHeight: CharCanvas height in character units.
                colorMode:  Color mode of the render target character.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_G2dCharCanvasInitForOBJ2DRect(
    NNSG2dCharCanvas* pCC,
    void* charBase,
    int areaWidth,
    int areaHeight,
    NNSG2dCharaColorMode colorMode
);



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dCharCanvasMakeCell1D

  Description:  Creates the cell for displaying the CharCanvas initialized with the NNS_G2dCharCanvasInitForOBJ1D function.
                

  Arguments:    pCell:      Buffer that stores generated cell data.
                pCC:        Pointer to CharCanvas
                x:          Cell's center coordinates (based on CharCanvas coordinates)
                y:          Cell's center coordinates (based on CharCanvas coordinates)
                priority:   Cell priority
                mode:       Cell mode
                mosaic:     Cell mosaic
                effect:     Cell effects
                color:      CharCanvas color mode.
                charName:   Starting character name
                cParam:     Color palette number
                vramMode:   Specifies OBJ VRAM capacity setting.
                makeBR:     Specifies whether boundary rectangle information should be appended.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_G2dCharCanvasMakeCell1D(
    NNSG2dCellData* pCell,
    const NNSG2dCharCanvas* pCC,
    int x,
    int y,
    int priority,
    GXOamMode mode,
    BOOL mosaic,
    GXOamEffect effect,
    GXOamColorMode color,
    int charName,
    int cParam,
    NNSG2dOBJVramMode vramMode,
    BOOL makeBR
);



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dCharCanvasMakeCell2DRect

  Description:  Creates the cell for displaying the CharCanvas initialized with the NNS_G2dCharCanvasInitForOBJ2DRect function.
                

  Arguments:    pCell:      Buffer that stores generated cell data.
                pCC:        Pointer to CharCanvas
                x:          Cell's center coordinates (based on CharCanvas coordinates)
                y:          Cell's center coordinates (based on CharCanvas coordinates)
                priority:   Cell priority
                mode:       Cell mode
                mosaic:     Cell mosaic
                effect:     Cell effects
                color:      CharCanvas color mode.
                charName:   Starting character name
                cParam:     Color palette number
                makeBR:     Specifies whether boundary rectangle information should be appended.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_G2dCharCanvasMakeCell2DRect(
    NNSG2dCellData* pCell,
    const NNSG2dCharCanvas* pCC,
    int x,
    int y,
    int priority,
    GXOamMode mode,
    BOOL mosaic,
    GXOamEffect effect,
    GXOamColorMode color,
    int charName,
    int cParam,
    BOOL makeBR
);



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dCharCanvasCalcCellDataSize*

  Description:  Calculates the buffer size to pass for the first argument of NNS_G2dCharCanvasMakeCell*().
                Calculates the buffer size.

  Arguments:    pCC:        Pointer to CharCanvas
                makeBR:     Specifies whether boundary rectangle information should be appended.

  Returns:      None.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE size_t NNSi_G2dCharCanvasCalcCellDataSize(
    const NNSG2dCharCanvas* pCC,
    BOOL makeBR
)
{
    const int numObj        = NNSi_G2dCalcRequiredOBJ(pCC->areaWidth, pCC->areaHeight);
    const size_t oamSize    = sizeof(NNSG2dCellOAMAttrData) * numObj;
    const size_t brSize     = makeBR ? sizeof(NNSG2dCellBoundingRectS16): 0;

    return sizeof(NNSG2dCellData) + brSize + oamSize;
}

NNS_G2D_INLINE size_t NNS_G2dCharCanvasCalcCellDataSize1D(
    const NNSG2dCharCanvas* pCC,
    BOOL makeBR
)
{
    return NNSi_G2dCharCanvasCalcCellDataSize(pCC, makeBR);
}

NNS_G2D_INLINE size_t NNS_G2dCharCanvasCalcCellDataSize2DRect(
    const NNSG2dCharCanvas* pCC,
    BOOL makeBR
)
{
    return NNSi_G2dCharCanvasCalcCellDataSize(pCC, makeBR);
}






//---------------------------------------------------------------------

#ifdef __cplusplus
}/* extern "C" */
#endif

#endif // G2D_CHARCANVAS_H_

