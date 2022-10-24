/*---------------------------------------------------------------------------*
  Project:  TWL-System - include - nnsys - g2d
  File:     g2d_TextCanvas.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
#ifndef G2D_TEXTAREA_H_
#define G2D_TEXTAREA_H_

#include <nnsys/g2d/g2d_Font.h>
#include <nnsys/g2d/g2d_CharCanvas.h>
#include <nnsys/g2d/g2di_AssertUtil.h>
#include <nnsys/g2d/g2di_SplitChar.h>
#include <nnsys/g2d/g2di_Char.h>

#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------------

//---------------------------------------------------------------------
// TextCanvas Macros
//---------------------------------------------------------------------
#define NNS_G2D_TEXTCANVAS_ASSERT( pTxn )                           \
    NNS_G2D_ASSERTMSG(                                              \
        NNS_G2D_IS_VALID_POINTER(pTxn)                              \
            && NNS_G2D_IS_VALID_POINTER((pTxn)->pCanvas)            \
            && NNS_G2D_IS_VALID_POINTER((pTxn)->pFont)              \
        , "Illegal NNSG2dTextCanvas." )

#define NNS_G2D_CHARENCODING_ASSERT( enc )                      \
    NNS_G2D_ASSERTMSG(                                          \
        (0 <= (enc)) && ((enc) < NNS_G2D_NUM_OF_CHARENCODING)   \
        , "Illegal NNSG2dCharEncoding(=%d).", (enc) )           \



//---------------------------------------------------------------------
// TextCanvas Definitions
//---------------------------------------------------------------------

// Vertical origin placement
typedef enum NNSG2dVerticalOrigin
{
    NNS_G2D_VERTICALORIGIN_TOP      = 0x1,
    NNS_G2D_VERTICALORIGIN_MIDDLE   = 0x2,
    NNS_G2D_VERTICALORIGIN_BOTTOM   = 0x4
}
NNSG2dVerticalOrigin;

// Horizontal origin placement
typedef enum NNSG2dHorizontalOrigin
{
    NNS_G2D_HORIZONTALORIGIN_LEFT   = 0x8,
    NNS_G2D_HORIZONTALORIGIN_CENTER = 0x10,
    NNS_G2D_HORIZONTALORIGIN_RIGHT  = 0x20
}
NNSG2dHorizontalOrigin;

// Vertical alignment
typedef enum NNSG2dVerticalAlign
{
    NNS_G2D_VERTICALALIGN_TOP       = 0x40,
    NNS_G2D_VERTICALALIGN_MIDDLE    = 0x80,
    NNS_G2D_VERTICALALIGN_BOTTOM    = 0x100
}
NNSG2dVerticalAlign;

// Horizontal alignment
typedef enum NNSG2dHorizontalAlign
{
    NNS_G2D_HORIZONTALALIGN_LEFT    = 0x200,
    NNS_G2D_HORIZONTALALIGN_CENTER  = 0x400,
    NNS_G2D_HORIZONTALALIGN_RIGHT   = 0x800
}
NNSG2dHorizontalAlign;



// TextCanvas
typedef struct NNSG2dTextCanvas
{
    const NNSG2dCharCanvas* pCanvas;
    const NNSG2dFont* pFont;
    int hSpace;
    int vSpace;
}
NNSG2dTextCanvas;



// Callback parameters for NNS_G2dTextCanvasDrawTaggedText
typedef struct NNSG2dTagCallbackInfo
{
    NNSG2dTextCanvas txn;   // TextCanvas being used for rendering.
    const NNSG2dChar* str;  // Pointer to the character string being rendered.
    int x;                  // X coordinate being rendered
    int y;                  // Y coordinate being rendered
    int clr;                // Color number being rendered.
    void* cbParam;          // 7th argument of NNS_G2dTextCanvasDrawTaggedText.
}
NNSG2dTagCallbackInfo;

// Callback function for NNS_G2dTextCanvasDrawTaggedText
typedef void (*NNSG2dTagCallback)(u16 c, NNSG2dTagCallbackInfo* pInfo);

// Text direction during string rendering
typedef struct NNSiG2dTextDirection
{
    s8 x;
    s8 y;
}
NNSiG2dTextDirection;



//---------------------------------------------------------------------
// TextCanvas Operations
//---------------------------------------------------------------------

//--------------------------------------------
// Render

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dTextCanvasDrawString

  Description:  Renders up to the first line feed character or a terminating character.

  Arguments:    pTxn:   Pointer to the TextCanvas to be rendered.
                x:      Render starting position X coordinate.
                y:      Render starting position Y coordinate.
                cl:     Color number of character color.
                str :    String to be rendered.
                pPos:   Pointer to character after a carriage return when rendering has occurred to a carriage return, or pointer to a buffer storing NULL when rendered to the terminating character.
                        
                        
                        NULL can be specified here if unnecessary.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNSi_G2dTextCanvasDrawString(
    const NNSG2dTextCanvas* pTxn,
    int x,
    int y,
    int cl,
    const void* str,
    const void** pPos
#ifdef NNS_G2D_FONT_ENABLE_DIRECTION_SUPPORT
    , NNSiG2dTextDirection d
#endif
);



/*---------------------------------------------------------------------------*
  Name:         NNSi_G2dDrawTextAlign

  Description:  Renders up to the first line feed character or a terminating character.

  Arguments:    pTxn:       Pointer to the TextCanvas to be rendered.
                x:          Render starting position X coordinate.
                y:          Render starting position Y coordinate.
                cl:         Color number of character color.
                flags:      Flag that designates the rendering position.
                txt:        String to be rendered.
                areaWidth:  Render region width referenced when right-aligning or centering.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNSi_G2dTextCanvasDrawTextAlign(
    const NNSG2dTextCanvas* pTxn,
    int x,
    int y,
    int areaWidth,
    int cl,
    u32 flags,
    const void* txt
#ifdef NNS_G2D_FONT_ENABLE_DIRECTION_SUPPORT
    , NNSiG2dTextDirection d
#endif
);



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dTextCanvasDrawText

  Description:  Draws a multi-line string.
                The rendering position can be designated with a point as the base.

  Arguments:    pTxn:   Pointer to the TextCanvas to be rendered.
                x:      Render base position X coordinate.
                y:      Render base position Y coordinate.
                cl:     Color number of character color.
                flags:  Flag that designates the rendering position.
                txt:    String to be rendered.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNSi_G2dTextCanvasDrawText(
    const NNSG2dTextCanvas* pTxn,
    int x,
    int y,
    int cl,
    u32 flags,
    const void* txt
#ifdef NNS_G2D_FONT_ENABLE_DIRECTION_SUPPORT
    , NNSiG2dTextDirection d
#endif
);



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dTextCanvasDrawTextRect

  Description:  Draws a multi-line string.
                The drawing position can be designated with a rectangle as the base.
                Characters that stick out of the rectangle will be rendered as normal.

  Arguments:    pTxn:   Pointer to the TextCanvas to be rendered.
                x:      Upper left X coordinate of rectangle to be rendered.
                y:      Upper left Y coordinate of rectangle to be rendered.
                cl:     Color number of character color.
                flags:  Flag that designates the rendering position.
                txt:    String to be rendered.
                w:      Width of rectangle to be rendered.
                h:      Height of rectangle to be rendered.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNSi_G2dTextCanvasDrawTextRect(
    const NNSG2dTextCanvas* pTxn,
    int x,
    int y,
    int w,
    int h,
    int cl,
    u32 flags,
    const void* txt
#ifdef NNS_G2D_FONT_ENABLE_DIRECTION_SUPPORT
    , NNSiG2dTextDirection d
#endif
);



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dTextCanvasDrawTaggedText

  Description:  Renders a multi-line character string that has embedded tags.
                Rendering can be controlled with the tags.

  Arguments:    pTxn:       Pointer to the TextCanvas to be rendered.
                x:          Render base position X coordinate.
                y:          Render base position Y coordinate.
                cl:         Color number of character color.
                txt:        String to be rendered.
                cbFunc:     Callback function invoked by the tag.
                cbParam:    User data passed to the callback function.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNSi_G2dTextCanvasDrawTaggedText(
    const NNSG2dTextCanvas* pTxn,
    int x,
    int y,
    int cl,
    const void* txt,
    NNSG2dTagCallback cbFunc,
    void* cbParam
#ifdef NNS_G2D_FONT_ENABLE_DIRECTION_SUPPORT
    , NNSiG2dTextDirection d
#endif
);





//--------------------------------------------
// Build

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dTextCanvasInit*

  Description:  Initializes the TextCanvas.

  Arguments:    pTxn:   Pointer to the TextCanvas to initialize.
                pCC:    Pointer to the CharCanvas to be rendered on.
                pFont:  Pointer to the font used for rendering.
                hSpace: Space between characters.
                vSpace: Correction difference for the font leading.
                encode: Pointer to the character cutout callback function.

  Returns:      None.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE void NNS_G2dTextCanvasInit(
    NNSG2dTextCanvas* pTxn,
    const NNSG2dCharCanvas* pCC,
    const NNSG2dFont* pFont,
    int hSpace,
    int vSpace
)
{
    NNS_G2D_POINTER_ASSERT( pTxn );
    NNS_G2D_CHARCANVAS_ASSERT( pCC );
    NNS_G2D_FONT_ASSERT( pFont );

    pTxn->pCanvas       = pCC;
    pTxn->pFont         = pFont;
    pTxn->hSpace        = hSpace;
    pTxn->vSpace        = vSpace;
}



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dTextCanvasInit*

  Description:  Initializes the TextCanvas.

  Arguments:    pTxn:       Pointer to the TextCanvas to initialize.
                pCC:        Pointer to the CharCanvas to be rendered on.
                pFont:      Pointer to the font used for rendering.
                hSpace:     Space between characters.
                vSpace:     Correction difference for the font leading.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#define NNS_G2dTextCanvasInit1Byte      NNS_G2dTextCanvasInit
#define NNS_G2dTextCanvasInitUTF8       NNS_G2dTextCanvasInit
#define NNS_G2dTextCanvasInitUTF16      NNS_G2dTextCanvasInit
#define NNS_G2dTextCanvasInitShiftJIS   NNS_G2dTextCanvasInit


//--------
// getter

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dTextCanvasGetCharCanvas

  Description:  Configures the CharCanvas on which the TextCanvas is to be rendered.

  Arguments:    pTxn:   Pointer to the TextCanvas for which the font is to be configured.
                pCC:    Pointer to the newly-configured CharCanvas.

  Returns:      None.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE const NNSG2dCharCanvas* NNS_G2dTextCanvasGetCharCanvas(
    const NNSG2dTextCanvas* pTxn
)
{
    NNS_G2D_TEXTCANVAS_ASSERT( pTxn );
    return pTxn->pCanvas;
}



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dTextCanvasGetFont

  Description:  Configures the font in TextCanvas.

  Arguments:    pTxn:   Pointer to the TextCanvas for which the font is to be configured.
                pFont:  Pointer to the newly-configured font.

  Returns:      None.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE const NNSG2dFont* NNS_G2dTextCanvasGetFont( const NNSG2dTextCanvas* pTxn )
{
    NNS_G2D_TEXTCANVAS_ASSERT( pTxn );
    return pTxn->pFont;
}



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dTextCanvasGetHSpace

  Description:  Configures the amount of space TextCanvas uses when rendering a character string.

  Arguments:    pTxn:   Pointer to the TextCanvas for which the font is to be configured.

  Returns:      None.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE int NNS_G2dTextCanvasGetHSpace( const NNSG2dTextCanvas* pTxn )
{
    NNS_G2D_TEXTCANVAS_ASSERT( pTxn );
    return pTxn->hSpace;
}



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dTextCanvasGetVSpace

  Description:  Configures the amount of space TextCanvas uses when rendering a character string.

  Arguments:    pTxn:   Pointer to the TextCanvas for which the font is to be configured.

  Returns:      None.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE int NNS_G2dTextCanvasGetVSpace( const NNSG2dTextCanvas* pTxn )
{
    NNS_G2D_TEXTCANVAS_ASSERT( pTxn );
    return pTxn->vSpace;
}



//--------
// Setters

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dTextCanvasSetCharCanvas

  Description:  Configures the CharCanvas on which the TextCanvas is to be rendered.

  Arguments:    pTxn:   Pointer to the TextCanvas for which the font is to be configured.
                pCC:    Pointer to the newly-configured CharCanvas.

  Returns:      None.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE void NNS_G2dTextCanvasSetCharCanvas(
    NNSG2dTextCanvas* pTxn,
    const NNSG2dCharCanvas* pCC
)
{
    NNS_G2D_TEXTCANVAS_ASSERT( pTxn );
    NNS_G2D_CHARCANVAS_ASSERT( pCC );
    pTxn->pCanvas = pCC;
}



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dTextCanvasSetFont

  Description:  Configures the font in TextCanvas.

  Arguments:    pTxn:   Pointer to the TextCanvas for which the font is to be configured.
                pFont:  Pointer to the newly-configured font.

  Returns:      None.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE void NNS_G2dTextCanvasSetFont( NNSG2dTextCanvas* pTxn, const NNSG2dFont* pFont )
{
    NNS_G2D_TEXTCANVAS_ASSERT( pTxn );
    NNS_G2D_FONT_ASSERT( pFont );
    pTxn->pFont = pFont;
}



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dTextCanvasSetHSpace

  Description:  Configures the amount of space TextCanvas uses when rendering a character string.

  Arguments:    pTxn:   Pointer to the TextCanvas for which the font is to be configured.
                hSpace: The newly-configured amount of space between characters.

  Returns:      None.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE void NNS_G2dTextCanvasSetHSpace( NNSG2dTextCanvas* pTxn, int hSpace )
{
    NNS_G2D_TEXTCANVAS_ASSERT( pTxn );
    pTxn->hSpace = hSpace;
}



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dTextCanvasSetVSpace

  Description:  Configures the amount of space TextCanvas uses when rendering a character string.

  Arguments:    pTxn:   Pointer to the TextCanvas for which the font is to be configured.
                vSpace: The newly-configured amount of leading.

  Returns:      None.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE void NNS_G2dTextCanvasSetVSpace( NNSG2dTextCanvas* pTxn, int vSpace )
{
    NNS_G2D_TEXTCANVAS_ASSERT( pTxn );
    pTxn->vSpace = vSpace;
}



//--------------------------------------------
// Get Information

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dTextCanvasGetStringWidth

  Description:  Looks for the width when rendering up to the first newline or the terminal character.

  Arguments:    pTxn:   Pointer to the TextCanvas that is the base for the calculation.
                str :    String.
                pPos:   Pointer to character after a carriage return when a stop has occurred to a carriage return, or pointer to a buffer storing NULL when stopped at the terminating character.
                        
                        
                        NULL can be specified here if unnecessary.


  Returns:      The width when the character string is rendered.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE int NNS_G2dTextCanvasGetStringWidth(
    const NNSG2dTextCanvas* pTxn,
    const NNSG2dChar* str,
    const NNSG2dChar** pPos
)
{
    return NNS_G2dFontGetStringWidth(
                pTxn->pFont,
                pTxn->hSpace,
                str,
                pPos );
}



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dTextCanvasGetTextHeight

  Description:  Looks for the height when a character string is rendered.

  Arguments:    pTxn:   Pointer to the TextCanvas that is the base for the calculation.
                txt:    String.

  Returns:      Height when the character string is rendered.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE int NNS_G2dTextCanvasGetTextHeight(
    const NNSG2dTextCanvas* pTxn,
    const NNSG2dChar* txt
)
{
    return NNS_G2dFontGetTextHeight(
                pTxn->pFont,
                pTxn->vSpace,
                txt );
}



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dTextCanvasGetTextWidth

  Description:  Calculates the width when a character string is rendered.
                This value will become the maximum width of each line.

  Arguments:    pTxn:   Pointer to the TextCanvas that is the base for the calculation.
                txt:    String.

  Returns:      The width when the character string is rendered.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE int NNS_G2dTextCanvasGetTextWidth(
    const NNSG2dTextCanvas* pTxn,
    const NNSG2dChar* txt
)
{
    return NNS_G2dFontGetTextWidth(
                pTxn->pFont,
                pTxn->hSpace,
                txt );
}



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dTextCanvasGetTextRect

  Description:  Calculates the height and width when a character string is rendered.

  Arguments:    pTxn:   Pointer to the TextCanvas that is the base for the calculation.
                txt:    String.

  Returns:      Height and width when the character string is rendered.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE NNSG2dTextRect NNS_G2dTextCanvasGetTextRect(
    const NNSG2dTextCanvas* pTxn,
    const NNSG2dChar* txt
)
{
    return NNS_G2dFontGetTextRect(
                pTxn->pFont,
                pTxn->hSpace,
                pTxn->vSpace,
                txt );
}



//---------------------------------------------------------------------

/*---------------------------------------------------------------------------*
  Name:         NNSi_G2dGetTextDirection

  Description:  Gets the text direction that corresponds to the font

  Arguments:    pFont:  The font from which to get the text direction.

  Returns:      Returns the text direction that corresponds to the font.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE NNSiG2dTextDirection NNSi_G2dGetTextDirection(const NNSG2dFont* pFont)
{
    NNSiG2dTextDirection d = { 0, 0 };
    switch( NNS_G2dFontGetFlags(pFont) )
    {
    case (NNS_G2D_FONT_FLAG_ROT_0):
    case (NNS_G2D_FONT_FLAG_ROT_270|NNS_G2D_FONT_FLAG_TBRL): d.x = 1; break;
    case (NNS_G2D_FONT_FLAG_ROT_90):
    case (NNS_G2D_FONT_FLAG_ROT_0|NNS_G2D_FONT_FLAG_TBRL):   d.y = 1; break;
    case (NNS_G2D_FONT_FLAG_ROT_180):
    case (NNS_G2D_FONT_FLAG_ROT_90|NNS_G2D_FONT_FLAG_TBRL):  d.x = -1; break;
    case (NNS_G2D_FONT_FLAG_ROT_270):
    case (NNS_G2D_FONT_FLAG_ROT_180|NNS_G2D_FONT_FLAG_TBRL): d.y = -1; break;
    }
    return d;
}

#ifdef NNS_G2D_FONT_ENABLE_DIRECTION_SUPPORT
NNS_G2D_INLINE void NNS_G2dTextCanvasDrawString(const NNSG2dTextCanvas* pTxn, int x, int y, int cl, const NNSG2dChar* str, const NNSG2dChar** pPos)
    { NNSi_G2dTextCanvasDrawString(pTxn, x, y, cl, str, (const void**)pPos,
            NNSi_G2dGetTextDirection(NNS_G2dTextCanvasGetFont(pTxn))); }
NNS_G2D_INLINE void NNS_G2dTextCanvasDrawText(const NNSG2dTextCanvas* pTxn, int x, int y, int cl, u32 flags, const NNSG2dChar* txt)
    { NNSi_G2dTextCanvasDrawText(pTxn, x, y, cl, flags, txt,
            NNSi_G2dGetTextDirection(NNS_G2dTextCanvasGetFont(pTxn))); }
NNS_G2D_INLINE void NNS_G2dTextCanvasDrawTextRect(const NNSG2dTextCanvas* pTxn, int x, int y, int w, int h, int cl, u32 flags, const NNSG2dChar* txt)
    { NNSi_G2dTextCanvasDrawTextRect(pTxn, x, y, w, h, cl, flags, txt,
            NNSi_G2dGetTextDirection(NNS_G2dTextCanvasGetFont(pTxn))); }
NNS_G2D_INLINE void NNS_G2dTextCanvasDrawTaggedText(const NNSG2dTextCanvas* pTxn, int x, int y, int cl, const NNSG2dChar* txt, NNSG2dTagCallback cbFunc, void* cbParam)
    { NNSi_G2dTextCanvasDrawTaggedText(pTxn, x, y, cl, txt, cbFunc, cbParam,
            NNSi_G2dGetTextDirection(NNS_G2dTextCanvasGetFont(pTxn))); }
#else
NNS_G2D_INLINE void NNS_G2dTextCanvasDrawString(const NNSG2dTextCanvas* pTxn, int x, int y, int cl, const NNSG2dChar* str, const NNSG2dChar** pPos)
    { NNSi_G2dTextCanvasDrawString(pTxn, x, y, cl, str, (const void**)pPos); }
NNS_G2D_INLINE void NNS_G2dTextCanvasDrawText(const NNSG2dTextCanvas* pTxn, int x, int y, int cl, u32 flags, const NNSG2dChar* txt)
    { NNSi_G2dTextCanvasDrawText(pTxn, x, y, cl, flags, txt); }
NNS_G2D_INLINE void NNS_G2dTextCanvasDrawTextRect(const NNSG2dTextCanvas* pTxn, int x, int y, int w, int h, int cl, u32 flags, const NNSG2dChar* txt)
    { NNSi_G2dTextCanvasDrawTextRect(pTxn, x, y, w, h, cl, flags, txt); }
NNS_G2D_INLINE void NNS_G2dTextCanvasDrawTaggedText(const NNSG2dTextCanvas* pTxn, int x, int y, int cl, const NNSG2dChar* txt, NNSG2dTagCallback cbFunc, void* cbParam)
    { NNSi_G2dTextCanvasDrawTaggedText(pTxn, x, y, cl, txt, cbFunc, cbParam); }
#endif

//---------------------------------------------------------------------

#ifdef __cplusplus
}/* extern "C" */
#endif

#endif // G2D_TEXTAREA_H_

